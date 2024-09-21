#ifndef __MODEL_LOAD_MANAGER_H__
#define __MODEL_LOAD_MANAGER_H__

#include "lru_queue.h"
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define OBJ_BYTE_COUNT ((3 + 2 + 3) * sizeof(float))

class ModelLoadManager
{
public:
    //////////////////////////////////////////////////////////////////
    // parse obj file, not cache
    struct ObjData
    {
        QVector<float> m_vPoints;
        QVector<float> m_tPoints;
        QVector<float> m_nPoints;
    };

    bool parseObjModel(const QString &modelPath, ObjData &objData);
    bool parseObjModel(const QString& modelPath, QByteArray& objData);

public:
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
    bool import3DModel(const QString& modelPath, std::shared_ptr<QByteArray> &byteArrayPtr);
    float getModelMaxPos(const QString &modelPath);
    void cleanImageData(unsigned char *data);

public:
    static ModelLoadManager* instance();

private:
    ModelLoadManager();
    ~ModelLoadManager();

    bool parseObjModel(const QString& modelPath, QVector<float>& vertextPoints, QVector<float>& texturePoints, QVector<float>& normalPoints,
        QVector<std::tuple<int, int, int>>& facesIndexs);
    void  processNode(aiNode* node, const aiScene* scene, QVector<ModelMesh>& modelMeshs);
    void  processMesh(aiMesh* mesh, const aiScene* scene, ModelMesh& modelMesh);
    void  loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene, std::vector<Texture>& textures);

private:
    LRUQueue<QString, std::shared_ptr<QVector<ModelMesh>>> m_modelMeshMaps;
    LRUQueue<QString, std::shared_ptr<QByteArray>> m_byteArrayMaps;
    QMap<QString, float> m_modelMaxPosMaps;
    QString m_currentModelName;
    Assimp::Importer m_importer;
};

#endif
