#include "vulkan_render.h"
#include <QRandomGenerator>
#include <QVulkanFunctions>


const VkDeviceSize PER_INSTANCE_DATA_SIZE = 6 * sizeof(float); // instTranslate, instDiffuseAdjust

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

static inline float gen(int a, int b)
{
    return float(QRandomGenerator::global()->bounded(double(b - a)) + a);
}

VulkanRenderer::VulkanRenderer(QVulkanWindow *w, const QColor& color, std::shared_ptr<VulkanMesh>& vulkanMeshPtr)
    : m_window(w),
      m_lightPos(0.0f, 0.0f, 25.0f),
      m_cam(QVector3D(0.0f, 0.0f, 20.0f)),
      m_vulkanMeshPtr(vulkanMeshPtr)
{
    QRgb rgba = color.rgba();
    m_bgColor = { (float)qRed(rgba) / 255, (float)qGreen(rgba) / 255, (float)qBlue(rgba) / 255, (float)qAlpha(rgba) / 255 };
}

void VulkanRenderer::initResources()
{
    if (!checkValid())
        return;

    QVulkanInstance* inst = m_window->vulkanInstance();
    VkDevice dev = m_window->device();
    m_devFuncs = inst->deviceFunctions(dev);
    const VkPhysicalDeviceLimits* pdevLimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
    m_itemMaterial.vertUniSize = aligned(2 * 64 + 48, uniAlign);
    m_itemMaterial.fragUniSize = aligned(6 * 16 + 12 + 2 * 4, uniAlign);
    if (!m_itemMaterial.vs.isValid())
        m_itemMaterial.vs.load(inst, dev, QStringLiteral(":/color_phong_vert.spv"));
    if (!m_itemMaterial.fs.isValid())
        m_itemMaterial.fs.load(inst, dev, QStringLiteral(":/color_phong_frag.spv"));

    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    createItemPipeline();
    ensureBuffers();
    ensureInstanceBuffer();
}

bool VulkanRenderer::checkValid()
{
    if (!m_vulkanMeshPtr || !m_vulkanMeshPtr->isValid())
        return false;
    return true;
}

void VulkanRenderer::initSwapChainResources()
{
    if (!checkValid())
        return;

    m_proj = m_window->clipCorrectionMatrix();
    const QSize sz = m_window->swapChainImageSize();
    m_proj.perspective(45.0f, sz.width() / (float)sz.height(), 0.01f, 1000.0f);
    markViewProjDirty();
}

void VulkanRenderer::releaseSwapChainResources()
{
}

void VulkanRenderer::startNextFrame()
{
    if (!checkValid())
        return;

    VkCommandBuffer cb = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();
    VkClearColorValue clearColor = { {m_bgColor[0], m_bgColor[1], m_bgColor[2], m_bgColor[3]} };
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[3];
    memset(clearValues, 0, sizeof(clearValues));
    clearValues[0].color = clearValues[2].color = clearColor;
    clearValues[1].depthStencil = clearDS;

    VkRenderPassBeginInfo rpBeginInfo;
    memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
    rpBeginInfo.pClearValues = clearValues;
    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        0, 0,
        float(sz.width()), float(sz.height()),
        0, 1 };
    m_devFuncs->vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor = {
        {0, 0},
        {uint32_t(sz.width()), uint32_t(sz.height())} };
    m_devFuncs->vkCmdSetScissor(cb, 0, 1, &scissor);

    buildDrawCall();
    m_devFuncs->vkCmdEndRenderPass(cmdBuf);
    m_window->frameReady();
    m_window->requestUpdate();
}

void VulkanRenderer::releaseResources()
{
    if (!checkValid())
        return;

    VkDevice dev = m_window->device();

    if (m_itemMaterial.descSetLayout)
    {
        m_devFuncs->vkDestroyDescriptorSetLayout(dev, m_itemMaterial.descSetLayout, nullptr);
        m_itemMaterial.descSetLayout = VK_NULL_HANDLE;
    }

    if (m_itemMaterial.descPool)
    {
        m_devFuncs->vkDestroyDescriptorPool(dev, m_itemMaterial.descPool, nullptr);
        m_itemMaterial.descPool = VK_NULL_HANDLE;
    }

    if (m_itemMaterial.pipeline)
    {
        m_devFuncs->vkDestroyPipeline(dev, m_itemMaterial.pipeline, nullptr);
        m_itemMaterial.pipeline = VK_NULL_HANDLE;
    }

    if (m_itemMaterial.pipelineLayout)
    {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_itemMaterial.pipelineLayout, nullptr);
        m_itemMaterial.pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache)
    {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_blockVertexBuf)
    {
        m_devFuncs->vkDestroyBuffer(dev, m_blockVertexBuf, nullptr);
        m_blockVertexBuf = VK_NULL_HANDLE;
    }

    if (m_uniBuf)
    {
        m_devFuncs->vkDestroyBuffer(dev, m_uniBuf, nullptr);
        m_uniBuf = VK_NULL_HANDLE;
    }

    if (m_bufMem)
    {
        m_devFuncs->vkFreeMemory(dev, m_bufMem, nullptr);
        m_bufMem = VK_NULL_HANDLE;
    }

    if (m_instBuf)
    {
        m_devFuncs->vkDestroyBuffer(dev, m_instBuf, nullptr);
        m_instBuf = VK_NULL_HANDLE;
    }

    if (m_instBufMem)
    {
        m_devFuncs->vkFreeMemory(dev, m_instBufMem, nullptr);
        m_instBufMem = VK_NULL_HANDLE;
    }

    if (m_itemMaterial.vs.isValid())
    {
        m_devFuncs->vkDestroyShaderModule(dev, m_itemMaterial.vs.getShaderModule(), nullptr);
        m_itemMaterial.vs.resetShaderModule();
    }
    if (m_itemMaterial.fs.isValid())
    {
        m_devFuncs->vkDestroyShaderModule(dev, m_itemMaterial.fs.getShaderModule(), nullptr);
        m_itemMaterial.fs.resetShaderModule();
    }
}

void VulkanRenderer::createItemPipeline()
{
    VkDevice dev = m_window->device();

    // Vertex layout.
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {0, // binding
         8 * sizeof(float),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {1,
         6 * sizeof(float),
         VK_VERTEX_INPUT_RATE_INSTANCE} };
    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        {
            // position
            0, // location
            0, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0 // offset
        },
        {// normal
         1,
         0,
         VK_FORMAT_R32G32B32_SFLOAT,
         5 * sizeof(float)},
        {// instTranslate
         2,
         1,
         VK_FORMAT_R32G32B32_SFLOAT,
         0},
        {// instDiffuseAdjust
         3,
         1,
         VK_FORMAT_R32G32B32_SFLOAT,
         3 * sizeof(float)} };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = sizeof(vertexBindingDesc) / sizeof(vertexBindingDesc[0]);
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(vertexAttrDesc) / sizeof(vertexAttrDesc[0]);
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc;

    // Descriptor set layout.
    VkDescriptorPoolSize descPoolSizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2} };
    VkDescriptorPoolCreateInfo descPoolInfo;
    memset(&descPoolInfo, 0, sizeof(descPoolInfo));
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = 1; // a single set is enough due to the dynamic uniform buffer
    descPoolInfo.poolSizeCount = sizeof(descPoolSizes) / sizeof(descPoolSizes[0]);
    descPoolInfo.pPoolSizes = descPoolSizes;
    VkResult err = m_devFuncs->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &m_itemMaterial.descPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor pool: %d", err);

    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        {0, // binding
         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
         1, // descriptorCount
         VK_SHADER_STAGE_VERTEX_BIT,
         nullptr},
        {1,
         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
         1,
         VK_SHADER_STAGE_FRAGMENT_BIT,
         nullptr} };
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        sizeof(layoutBindings) / sizeof(layoutBindings[0]),
        layoutBindings };
    err = m_devFuncs->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &m_itemMaterial.descSetLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);

    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        m_itemMaterial.descPool,
        1,
        &m_itemMaterial.descSetLayout };
    err = m_devFuncs->vkAllocateDescriptorSets(dev, &descSetAllocInfo, &m_itemMaterial.descSet);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate descriptor set: %d", err);

    // Graphics pipeline.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_itemMaterial.descSetLayout;

    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_itemMaterial.pipelineLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);

    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_VERTEX_BIT,
         m_itemMaterial.vs.getShaderModule(),
         "main",
         nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_FRAGMENT_BIT,
         m_itemMaterial.fs.getShaderModule(),
         "main",
         nullptr} };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia;
    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &ia;

    VkPipelineViewportStateCreateInfo vp;
    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineRasterizationStateCreateInfo rs;
    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms;
    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = m_window->sampleCountFlagBits();
    pipelineInfo.pMultisampleState = &ms;

    VkPipelineDepthStencilStateCreateInfo ds;
    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendStateCreateInfo cb;
    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att;
    memset(&att, 0, sizeof(att));
    att.colorWriteMask = 0xF;
    cb.attachmentCount = 1;
    cb.pAttachments = &att;
    pipelineInfo.pColorBlendState = &cb;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;
    pipelineInfo.layout = m_itemMaterial.pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();

    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_itemMaterial.pipeline);
    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);
}

void VulkanRenderer::ensureBuffers()
{
    VkDevice dev = m_window->device();
    const int concurrentFrameCount = m_window->concurrentFrameCount();
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    const int blockMeshByteCount = m_vulkanMeshPtr->data()->vertexCount * 8 * sizeof(float);
    bufInfo.size = blockMeshByteCount;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_blockVertexBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create vertex buffer: %d", err);

    VkMemoryRequirements blockVertMemReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_blockVertexBuf, &blockVertMemReq);
    bufInfo.size = (m_itemMaterial.vertUniSize + m_itemMaterial.fragUniSize) * concurrentFrameCount;
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_uniBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create uniform buffer: %d", err);

    VkMemoryRequirements uniMemReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_uniBuf, &uniMemReq);
    m_itemMaterial.uniMemStartOffset = aligned(blockVertMemReq.size, uniMemReq.alignment);
    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        m_itemMaterial.uniMemStartOffset + uniMemReq.size,
        m_window->hostVisibleMemoryIndex()};
    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_bufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_blockVertexBuf, m_bufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind vertex buffer memory: %d", err);
    err = m_devFuncs->vkBindBufferMemory(dev, m_uniBuf, m_bufMem, m_itemMaterial.uniMemStartOffset);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind uniform buffer memory: %d", err);

    // Copy vertex data.
    quint8 *p;
    err = m_devFuncs->vkMapMemory(dev, m_bufMem, 0, m_itemMaterial.uniMemStartOffset, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    memcpy(p, m_vulkanMeshPtr->data()->geom->constData(), blockMeshByteCount);
    m_devFuncs->vkUnmapMemory(dev, m_bufMem);

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo vertUni = {m_uniBuf, 0, m_itemMaterial.vertUniSize};
    VkDescriptorBufferInfo fragUni = {m_uniBuf, m_itemMaterial.vertUniSize, m_itemMaterial.fragUniSize};

    VkWriteDescriptorSet descWrite[2];
    memset(descWrite, 0, sizeof(descWrite));
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[0].dstSet = m_itemMaterial.descSet;
    descWrite[0].dstBinding = 0;
    descWrite[0].descriptorCount = 1;
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[0].pBufferInfo = &vertUni;

    descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[1].dstSet = m_itemMaterial.descSet;
    descWrite[1].dstBinding = 1;
    descWrite[1].descriptorCount = 1;
    descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[1].pBufferInfo = &fragUni;

    m_devFuncs->vkUpdateDescriptorSets(dev, 2, descWrite, 0, nullptr);
}

void VulkanRenderer::ensureInstanceBuffer()
{
    VkDevice dev = m_window->device();
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = PER_INSTANCE_DATA_SIZE;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    m_instData.resize(bufInfo.size);
    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_instBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create instance buffer: %d", err);

    VkMemoryRequirements memReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_instBuf, &memReq);

    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        m_window->hostVisibleMemoryIndex()};
    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_instBufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_instBuf, m_instBufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind instance buffer memory: %d", err);

    {
        char *p = m_instData.data();
        float t[] = {gen(-5, 5), gen(-4, 6), gen(-30, 5)};
        memcpy(p, t, 12);
        float d[] = {gen(-6, 3) / 10.0f, gen(-6, 3) / 10.0f, gen(-6, 3) / 10.0f};
        memcpy(p + 12, d, 12);
    }

    quint8 *p;
    err = m_devFuncs->vkMapMemory(dev, m_instBufMem, 0, PER_INSTANCE_DATA_SIZE, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    memcpy(p, m_instData.constData(), m_instData.size());
    m_devFuncs->vkUnmapMemory(dev, m_instBufMem);
}

void VulkanRenderer::getMatrices(QMatrix4x4 *vp, QMatrix4x4 *model, QMatrix3x3 *modelNormal, QVector3D *eyePos)
{
    model->setToIdentity();
    model->rotate(m_rotation, 1, 1, 0);
    *modelNormal = model->normalMatrix();
    QMatrix4x4 view = m_cam.viewMatrix();
    *vp = m_proj * view;
    *eyePos = view.inverted().column(3).toVector3D();
}

void VulkanRenderer::writeFragUni(quint8 *p, const QVector3D &eyePos)
{
    float ECCameraPosition[] = {eyePos.x(), eyePos.y(), eyePos.z()};
    memcpy(p, ECCameraPosition, 12);
    p += 16;

    // Material
    float ka[] = {0.05f, 0.05f, 0.05f};
    memcpy(p, ka, 12);
    p += 16;

    float kd[] = {0.7f, 0.7f, 0.7f};
    memcpy(p, kd, 12);
    p += 16;

    float ks[] = {0.66f, 0.66f, 0.66f};
    memcpy(p, ks, 12);
    p += 16;

    // Light parameters
    float ECLightPosition[] = {m_lightPos.x(), m_lightPos.y(), m_lightPos.z()};
    memcpy(p, ECLightPosition, 12);
    p += 16;

    float att[] = {1, 0, 0};
    memcpy(p, att, 12);
    p += 16;

    float color[] = {1.0f, 1.0f, 1.0f};
    memcpy(p, color, 12);
    p += 12; // next we have two floats which have an alignment of 4, hence 12 only

    float intensity = 0.8f;
    memcpy(p, &intensity, 4);
    p += 4;

    float specularExp = 150.0f;
    memcpy(p, &specularExp, 4);
    p += 4;
}

void VulkanRenderer::buildDrawCall()
{
    VkDevice dev = m_window->device();
    VkCommandBuffer cb = m_window->currentCommandBuffer();
    VkDeviceSize vbOffset = 0;
    uint32_t frameUniOffset = m_window->currentFrame() * (m_itemMaterial.vertUniSize + m_itemMaterial.fragUniSize);
    uint32_t frameUniOffsets[] = { frameUniOffset, frameUniOffset };
    m_devFuncs->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_itemMaterial.pipeline);
    m_devFuncs->vkCmdBindVertexBuffers(cb, 0, 1, &m_blockVertexBuf, &vbOffset);
    m_devFuncs->vkCmdBindVertexBuffers(cb, 1, 1, &m_instBuf, &vbOffset);
    m_devFuncs->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_itemMaterial.pipelineLayout, 0, 1,
                                        &m_itemMaterial.descSet, 2, frameUniOffsets);

    setAnimationType();
    if (m_animationType || m_vpDirty)
    {
        if (m_vpDirty)
            --m_vpDirty;
        QMatrix4x4 vp, model;
        QMatrix3x3 modelNormal;
        QVector3D eyePos;
        getMatrices(&vp, &model, &modelNormal, &eyePos);

        quint8 *p;
        VkResult err = m_devFuncs->vkMapMemory(dev, m_bufMem,
                                               m_itemMaterial.uniMemStartOffset + frameUniOffset,
                                               m_itemMaterial.vertUniSize + m_itemMaterial.fragUniSize,
                                               0, reinterpret_cast<void **>(&p));
        if (err != VK_SUCCESS)
            qFatal("Failed to map memory: %d", err);

        // Vertex shader uniforms
        memcpy(p, vp.constData(), 64);
        memcpy(p + 64, model.constData(), 64);
        const float *mnp = modelNormal.constData();
        memcpy(p + 128, mnp, 12);
        memcpy(p + 128 + 16, mnp + 3, 12);
        memcpy(p + 128 + 32, mnp + 6, 12);

        // Fragment shader uniforms
        p += m_itemMaterial.vertUniSize;
        writeFragUni(p, eyePos);

        m_devFuncs->vkUnmapMemory(dev, m_bufMem);
    }

    m_devFuncs->vkCmdDraw(cb, m_vulkanMeshPtr->data()->vertexCount, 1, 0, 0);
}

void VulkanRenderer::yaw(float degrees)
{
    m_cam.yaw(degrees);
    markViewProjDirty();
}

void VulkanRenderer::pitch(float degrees)
{
    m_cam.pitch(degrees);
    markViewProjDirty();
}

void VulkanRenderer::walk(float amount)
{
    m_cam.walk(amount);
    markViewProjDirty();
}

void VulkanRenderer::strafe(float amount)
{
    m_cam.strafe(amount);
    markViewProjDirty();
}

void VulkanRenderer::setBgColor(const QColor& color)
{
    QRgb rgba = color.rgba();
    m_bgColor = { (float)qRed(rgba) / 255, (float)qGreen(rgba) / 255, (float)qBlue(rgba) / 255, (float)qAlpha(rgba) / 255 };
}

void VulkanRenderer::setAnimationType()
{
    switch ((AnimationHelper::AnimationType)(m_animationType))
    {
    case AnimationHelper::Turntable:
        m_rotation += 0.5;
        if (m_rotation > 360)
            m_rotation -= 360;
        break;
    case AnimationHelper::Sway:
        if (m_swayLoopNum <= 180 / 0.5)
            m_rotation += 0.5;
        else if (m_swayLoopNum <= 180 / 0.5 * 2)
            m_rotation -= 0.5;
        else
            m_swayLoopNum -= 180 / 0.5 * 2;
        ++m_swayLoopNum;
        break;
    case AnimationHelper::Hover:
        m_hoverHeight += 0.05;
        if (m_hoverHeight <= 5)
            pitch(0.3f);
        else
            pitch(-0.3f);
        if (m_hoverHeight > 10)
            m_hoverHeight -= 10;
        break;
    default:
        break;
    }
}
