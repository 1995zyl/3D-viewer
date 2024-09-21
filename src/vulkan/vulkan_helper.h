#ifndef __VULKAN_HELPER_H__
#define __VULKAN_HELPER_H__

#include "utils/model_loader_manager.h"
#include "utils/utils.h"
#include <QVulkanInstance>
#include <QVector3D>
#include <QMatrix4x4>


class VulkanMesh
{
public:
    struct MeshData
    {
        int vertexCount = 0;
        std::shared_ptr<QByteArray> geom; // x, y, z, u, v, nx, ny, nz
    };

public:
    bool load(const QString & modelPath);
    MeshData *data(){ return &m_data; }
    bool isValid() { return m_data.vertexCount > 0; }

private:
    MeshData m_data;
};

class VulkanShader
{
public:
    void load(QVulkanInstance *inst, VkDevice dev, const QString &fn);
    VkShaderModule getShaderModule() { return m_shaderModule; }
    bool isValid() { return m_shaderModule != VK_NULL_HANDLE; }
    void resetShaderModule() { m_shaderModule = VK_NULL_HANDLE; }

private:
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
};


class Camera
{
public:
    Camera(const QVector3D &pos);

    void yaw(float degrees);
    void pitch(float degrees);
    void walk(float amount);
    void strafe(float amount);

    QMatrix4x4 viewMatrix() const;

private:
    QVector3D m_forward;
    QVector3D m_right;
    QVector3D m_up;
    QVector3D m_pos;
    float m_yaw;
    float m_pitch;
    QMatrix4x4 m_yawMatrix;
    QMatrix4x4 m_pitchMatrix;
};


#endif
