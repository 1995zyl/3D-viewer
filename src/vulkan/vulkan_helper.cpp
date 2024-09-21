#include "vulkan_helper.h"
#include <spdlog/spdlog.h>
#include <QFile>
#include <QFileInfo>
#include <QVulkanFunctions>


bool VulkanMesh::load(const QString &modelPath)
{
    if (!ModelLoadManager::instance()->import3DModel(modelPath, m_data.geom))
        return false;
    m_data.vertexCount = m_data.geom->size() / OBJ_BYTE_COUNT;
    return true;
}

void VulkanShader::load(QVulkanInstance *inst, VkDevice dev, const QString &fn)
{
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning("Failed to open %s", qPrintable(fn));
        return;
    }
    QByteArray blob = f.readAll();
    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(shaderInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(blob.constData());
    VkResult err = inst->deviceFunctions(dev)->vkCreateShaderModule(dev, &shaderInfo, nullptr, &m_shaderModule);
    if (err != VK_SUCCESS)
        qWarning("Failed to create shader module: %d", err);
}


Camera::Camera(const QVector3D &pos)
    : m_forward(0.0f, 0.0f, -1.0f),
      m_right(1.0f, 0.0f, 0.0f),
      m_up(0.0f, 1.0f, 0.0f),
      m_pos(pos),
      m_yaw(0.0f),
      m_pitch(0.0f)
{
}

static inline void clamp360(float *v)
{
    if (*v > 360.0f)
        *v -= 360.0f;
    if (*v < -360.0f)
        *v += 360.0f;
}

void Camera::yaw(float degrees)
{
    m_yaw += degrees;
    clamp360(&m_yaw);
    m_yawMatrix.setToIdentity();
    m_yawMatrix.rotate(m_yaw, 0, 1, 0);

    QMatrix4x4 rotMat = m_pitchMatrix * m_yawMatrix;
    m_forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D();
    m_right = (QVector4D(1.0f, 0.0f, 0.0f, 0.0f) * rotMat).toVector3D();
}

void Camera::pitch(float degrees)
{
    m_pitch += degrees;
    clamp360(&m_pitch);
    m_pitchMatrix.setToIdentity();
    m_pitchMatrix.rotate(m_pitch, 1, 0, 0);

    QMatrix4x4 rotMat = m_pitchMatrix * m_yawMatrix;
    m_forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D();
    m_up = (QVector4D(0.0f, 1.0f, 0.0f, 0.0f) * rotMat).toVector3D();
}

void Camera::walk(float amount)
{
    m_pos[0] += amount * m_forward.x();
    m_pos[2] += amount * m_forward.z();
}

void Camera::strafe(float amount)
{
    m_pos[0] += amount * m_right.x();
    m_pos[2] += amount * m_right.z();
}

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 m = m_pitchMatrix * m_yawMatrix;
    m.translate(-m_pos);
    return m;
}
