#ifndef __OPENGL_HELPER_H__
#define __OPENGL_HELPER_H__

#include "assimp/scene.h"
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>

namespace OpenGLHelper
{
    //////////////////////////////////////////////////////////////////
    // camera
    struct CameraParam
    {
        float m_fovy = 45.0;                // 观察者视角的大小
        float m_zoom = 1.0;                 // 观察者距被观察物体中心点的距离
        QVector3D m_eye{0, 0, m_zoom * 20}; // 观察者在被观察物体的三维坐标系中的位置
        QVector3D m_center{0.0, 0.0, 0.0};  // 观察者在被观察物体的三维坐标系中的位置
        QVector3D m_up{0.0, 1.0, 0.0};      // 观察者的头部朝向
        QMatrix4x4 m_projection;            // 透视矩阵
        QMatrix4x4 m_translation;           // 平移矩阵
        QMatrix4x4 m_rotation;              // 旋转矩阵
        int m_xRot = 0;                     // 绕x轴旋转的角度
        int m_yRot = 0;                     // 绕y轴旋转的角度
        int m_zRot = 0;                     // 绕z轴旋转的角度
        qreal m_xTrans = 0.0;               // 沿x轴移动的位置
        qreal m_yTrans = 0.0;               // 沿y轴移动的位置
        float m_zNear = 0.01;               // 透视矩阵视野最小值，太小容易出现黑影
        float m_zFar = 10000.0;             // 透视矩阵视野最大值
    };

    //////////////////////////////////////////////////////////////////
    // parse obj file
    struct ObjData
    {
        QVector<float> m_vPoints;
        QVector<float> m_tPoints;
        QVector<float> m_nPoints;
    };

    bool parseObjModel(const QString &modelPath, ObjData &objData);

    /////////////////////////////////////////////////////////////////
    // assimp
    struct Vertex
    {
        float m_positions[3];  // position
        float m_normals[3];    // normal
        float m_texCoords[2];  // texCoords
        float m_tangents[3];   // tangent
        float m_bitangents[3]; // bitangent
        int m_boneIDs[4];      // bone indexes which will influence this vertex
        float m_weights[4];    // weights from each bone
    };

    struct Texture
    {
        unsigned int m_id;
        std::string m_type;
        int m_width;
        int m_height;
        int m_channel;
        unsigned char *m_data;
    };

    struct ModelMesh
    {
        std::vector<Vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        std::vector<Texture> m_textures;
        unsigned int m_VAO;
        unsigned int m_VBO;
        unsigned int m_EBO;
    };

    bool import3DModel(const QString &modelPath, std::shared_ptr<QVector<ModelMesh>> &modelMeshsPtr);
    float getModelMaxPos(const QString &modelPath);
    void cleanImageData(unsigned char *data);
};

#endif
