#ifndef __VULKAN_RENDER_H__
#define __VULKAN_RENDER_H__

#include "vulkan_helper.h"
#include <QVulkanWindowRenderer>

class VulkanRenderer : public QVulkanWindowRenderer
{
public:
    VulkanRenderer(QVulkanWindow *w, const QColor& color, std::shared_ptr<VulkanMesh>& vulkanMeshPtr);
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

    void startAnimation(int animationType) { m_animationType = animationType; };
    void stopAnimation(){ m_animationType = 0; }
    void yaw(float degrees);
    void pitch(float degrees);
    void walk(float amount);
    void strafe(float amount);
    void setBgColor(const QColor& color);

private:
    bool checkValid();
    void createItemPipeline();
    void ensureBuffers();
    void ensureInstanceBuffer();
    void getMatrices(QMatrix4x4 *mvp, QMatrix4x4 *model, QMatrix3x3 *modelNormal, QVector3D *eyePos);
    void writeFragUni(quint8 *p, const QVector3D &eyePos);
    void buildDrawCall();
    void markViewProjDirty() { m_vpDirty = m_window->concurrentFrameCount(); }
    void setAnimationType();

private:
    struct VulkanRenderMaterial
    {
        VkDeviceSize vertUniSize = 0;
        VkDeviceSize fragUniSize = 0;
        VkDeviceSize uniMemStartOffset = 0;
        VulkanShader vs;
        VulkanShader fs;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descSet = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };

private:
    QVulkanWindow *m_window = nullptr;
    QVulkanDeviceFunctions *m_devFuncs = nullptr;
    VkBuffer m_blockVertexBuf = VK_NULL_HANDLE;
    VulkanRenderMaterial m_itemMaterial;
    VkDeviceMemory m_bufMem = VK_NULL_HANDLE;
    VkBuffer m_uniBuf = VK_NULL_HANDLE;
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    QVector3D m_lightPos;
    Camera m_cam;
    QMatrix4x4 m_proj;
    int m_vpDirty = 0;
    int m_animationType = 0;
    float m_rotation = 0.0f;
    int m_swayLoopNum = 0;
    float m_hoverHeight = 0;
    QByteArray m_instData;
    VkBuffer m_instBuf = VK_NULL_HANDLE;
    VkDeviceMemory m_instBufMem = VK_NULL_HANDLE;
    std::array<float, 4> m_bgColor;
    std::shared_ptr<VulkanMesh> m_vulkanMeshPtr;
};

#endif
