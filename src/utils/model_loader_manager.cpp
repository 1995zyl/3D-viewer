#include "model_loader_manager.h"
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <QFile>
#include <QFileInfo>

ModelLoadManager* ModelLoadManager::instance()
{
    static ModelLoadManager instance;
    return &instance;
}

ModelLoadManager::ModelLoadManager()
    : m_modelMeshMaps(20), m_byteArrayMaps(20)
{

}

ModelLoadManager::~ModelLoadManager()
{
    
}

bool ModelLoadManager::parseObjModel(const QString& modelPath, QVector<float>& vertextPoints, QVector<float>& texturePoints, QVector<float>& normalPoints,
    QVector<std::tuple<int, int, int>>& facesIndexs)
{
    if (QFileInfo(modelPath).suffix().compare("obj", Qt::CaseInsensitive))
    {
        spdlog::error("model path is invalid. path: {}", modelPath.toStdString());
        return false;
    }

    QFile objFile(modelPath);
    if (!objFile.open(QIODevice::ReadOnly))
    {
        spdlog::error("open model path failed. path: {}", "modelPath");
        return false;
    }

    while (!objFile.atEnd())
    {
        QByteArray lineData = objFile.readLine();
        QList<QByteArray> strValues = lineData.trimmed().split(' ');
        QString dataType = strValues.takeFirst();
        if (dataType == "v")
        {
            std::transform(strValues.begin(), strValues.end(), std::back_inserter(vertextPoints), [](QByteArray& str)
                { return str.toFloat(); });
        }
        else if (dataType == "vt")
        {
            std::transform(strValues.begin(), strValues.end(), std::back_inserter(texturePoints), [](QByteArray& str)
                { return str.toFloat(); });
        }
        else if (dataType == "vn")
        {
            std::transform(strValues.begin(), strValues.end(), std::back_inserter(normalPoints), [](QByteArray& str)
                { return str.toFloat(); });
        }
        else if (dataType == "f")
        {
            std::transform(strValues.begin(), strValues.end(), std::back_inserter(facesIndexs), [](QByteArray& str)
                {
                    QList<QByteArray> intStr = str.split('/');
                    return std::make_tuple(intStr.first().toInt(), intStr.at(1).toInt(), intStr.last().toInt()); });
        }
    }
    objFile.close();
    return true;
}

bool ModelLoadManager::parseObjModel(const QString &modelPath, ObjData &objData)
{
    QVector<float> vertextPoints, texturePoints, normalPoints;
    QVector<std::tuple<int, int, int>> facesIndexs;
    if (!parseObjModel(modelPath, vertextPoints, texturePoints, normalPoints, facesIndexs))
        return false;

    try
    {
        for (auto& verFaceInfo : facesIndexs)
        {
            int vIndex = std::get<0>(verFaceInfo);
            vIndex = (vIndex - 1) * 3;
            int tIndex = std::get<1>(verFaceInfo);
            tIndex = (tIndex - 1) * 2;
            int nIndex = std::get<2>(verFaceInfo);
            nIndex = (nIndex - 1) * 3;

            objData.m_vPoints << vertextPoints.at(vIndex + 0);
            objData.m_vPoints << vertextPoints.at(vIndex + 1);
            objData.m_vPoints << vertextPoints.at(vIndex + 2);

            if (texturePoints.size() < tIndex + 0)
            {
                objData.m_tPoints << texturePoints.at(tIndex + 0);
                objData.m_tPoints << texturePoints.at(tIndex + 1);
            }
            else
            {
                objData.m_tPoints << 0.0 << 0.0;
            }

            objData.m_nPoints << normalPoints.at(nIndex + 0);
            objData.m_nPoints << normalPoints.at(nIndex + 1);
            objData.m_nPoints << normalPoints.at(nIndex + 2);
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("parse object file failed. file: {0}, reason: {1}", modelPath.toStdString(), e.what());
        objData.m_vPoints.clear();
        objData.m_tPoints.clear();
        objData.m_nPoints.clear();
        return false;
    }

    return true;
}

bool ModelLoadManager::parseObjModel(const QString& modelPath, QByteArray& objData)
{
    QVector<float> vertextPoints, texturePoints, normalPoints;
    QVector<std::tuple<int, int, int>> facesIndexs;
    if (!parseObjModel(modelPath, vertextPoints, texturePoints, normalPoints, facesIndexs))
        return false;

    objData.resize(facesIndexs.size() * OBJ_BYTE_COUNT);
    char* p = objData.data();
    try
    {
        for (auto& verFaceInfo : facesIndexs)
        {
            int vIndex = std::get<0>(verFaceInfo);
            vIndex = (vIndex - 1) * 3;
            int tIndex = std::get<1>(verFaceInfo);
            tIndex = (tIndex - 1) * 2;
            int nIndex = std::get<2>(verFaceInfo);
            nIndex = (nIndex - 1) * 3;

            memcpy(p, &vertextPoints.at(vIndex + 0), 1 * sizeof(float));
            memcpy(p + 1 * sizeof(float), &vertextPoints.at(vIndex + 1), 1 * sizeof(float));
            memcpy(p + 2 * sizeof(float), &vertextPoints.at(vIndex + 2), 1 * sizeof(float));

            if (texturePoints.size() < tIndex + 0)
            {
                memcpy(p + 3 * sizeof(float), &texturePoints.at(tIndex + 0), 1 * sizeof(float));
                memcpy(p + 4 * sizeof(float), &texturePoints.at(tIndex + 1), 1 * sizeof(float));
            }
            else
            {
                float texPoint[] = { 0.0, 0.0 };
                memcpy(p + 3 * sizeof(float), texPoint, 2 * sizeof(float));
            }

            memcpy(p + 5 * sizeof(float), &normalPoints.at(nIndex + 0), 1 * sizeof(float));
            memcpy(p + 6 * sizeof(float), &normalPoints.at(nIndex + 1), 1 * sizeof(float));
            memcpy(p + 7 * sizeof(float), &normalPoints.at(nIndex + 2), 1 * sizeof(float));
            p += 8 * sizeof(float);
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("parse object file failed. file: {0}, reason: {1}", modelPath.toStdString(), e.what());
        objData.clear();
        return false;
    }

    return true;
}

bool ModelLoadManager::import3DModel(const QString &modelPath, std::shared_ptr<QVector<ModelMesh>> &modelMeshsPtr)
{
    if (modelPath.isEmpty())
    {
        spdlog::error("model path is empty. modelPath: {0}", modelPath.toStdString());
        return false;
    }
    else if (m_modelMeshMaps.contains(modelPath))
    {
        modelMeshsPtr = m_modelMeshMaps[modelPath];
        return true;
    }
        
    stbi_set_flip_vertically_on_load(true);

    const aiScene *scene = m_importer.ReadFile(modelPath.toStdString(),
                                                aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        spdlog::error("importer read file failed. file: {0}, reason: {1}", modelPath.toStdString(), m_importer.GetErrorString());
        return false;
    }
    m_currentModelName = modelPath;
    m_modelMeshMaps.insert(modelPath, std::make_shared<QVector<ModelMesh>>());

    processNode(scene->mRootNode, scene, *m_modelMeshMaps[modelPath]);
    modelMeshsPtr = m_modelMeshMaps[modelPath];
    return true;
}
    
bool ModelLoadManager::import3DModel(const QString& modelPath, std::shared_ptr<QByteArray>& byteArrayPtr)
{
    if (modelPath.isEmpty())
    {
        spdlog::error("model path is empty. modelPath: {0}", modelPath.toStdString());
        return false;
    }
    else if (m_byteArrayMaps.contains(modelPath))
    {
        byteArrayPtr = m_byteArrayMaps[modelPath];
        return true;
    }

    std::shared_ptr<QVector<ModelMesh>> modelMeshsPtr;
    if (!import3DModel(modelPath, modelMeshsPtr))
        return false;

    int totalCount = 0;
    std::for_each(modelMeshsPtr->begin(), modelMeshsPtr->end(), [&totalCount, &modelMeshsPtr](const ModelMesh& modelMesh)
        { totalCount += modelMesh.m_vertices.size(); });
    if (!byteArrayPtr)
        byteArrayPtr = std::make_shared<QByteArray>();
    byteArrayPtr->resize(totalCount * OBJ_BYTE_COUNT);
    char* p = byteArrayPtr->data();
    for(const auto& modelMesh : *modelMeshsPtr)
    {
        for (const auto& vertice : modelMesh.m_vertices)
        {
            memcpy(p, vertice.m_positions, 3 * sizeof(float));
            memcpy(p + 3 * sizeof(float), vertice.m_texCoords, 2 * sizeof(float));
            memcpy(p + 5 * sizeof(float), vertice.m_normals, 3 * sizeof(float));
            p += 8 * sizeof(float);
        }
    }

    m_byteArrayMaps.insert(modelPath, byteArrayPtr);
    return true;
}

void ModelLoadManager::processNode(aiNode *node, const aiScene *scene, QVector<ModelMesh> &modelMeshs)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ModelMesh modelMesh;
        processMesh(mesh, scene, modelMesh);
        modelMeshs.emplace_back(std::move(modelMesh));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, modelMeshs);
    }
}

void ModelLoadManager::processMesh(aiMesh *mesh, const aiScene *scene, ModelMesh &modelMesh)
{
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.m_positions[0] = mesh->mVertices[i].x;
        vertex.m_positions[1] = mesh->mVertices[i].y;
        vertex.m_positions[2] = mesh->mVertices[i].z;
        // normals
        if (mesh->HasNormals())
        {
            vertex.m_normals[0] = mesh->mNormals[i].x;
            vertex.m_normals[1] = mesh->mNormals[i].y;
            vertex.m_normals[2] = mesh->mNormals[i].z;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            vertex.m_texCoords[0] = mesh->mTextureCoords[0][i].x;
            vertex.m_texCoords[1] = mesh->mTextureCoords[0][i].y;

            // tangent
            vertex.m_tangents[0] = mesh->mTangents[i].x;
            vertex.m_tangents[1] = mesh->mTangents[i].y;
            vertex.m_tangents[2] = mesh->mTangents[i].z;

            // bitangent
            vertex.m_bitangents[0] = mesh->mBitangents[i].x;
            vertex.m_bitangents[1] = mesh->mBitangents[i].y;
            vertex.m_bitangents[2] = mesh->mBitangents[i].z;
        }
        else
        {
            vertex.m_texCoords[0] = 0.0f;
            vertex.m_texCoords[1] = 0.0f;
        }

        modelMesh.m_vertices.emplace_back(std::move(vertex));
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            modelMesh.m_indices.emplace_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps;
    loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene, diffuseMaps);
    modelMesh.m_textures.insert(modelMesh.m_textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps;
    loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene, specularMaps);
    modelMesh.m_textures.insert(modelMesh.m_textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps;
    loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene, normalMaps);
    modelMesh.m_textures.insert(modelMesh.m_textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps;
    loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", scene, heightMaps);
    modelMesh.m_textures.insert(modelMesh.m_textures.end(), heightMaps.begin(), heightMaps.end());
}

void ModelLoadManager::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName, const aiScene *scene, std::vector<Texture> &textures)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        if (aiReturn_SUCCESS != mat->GetTexture(type, i, &str))
        {
            spdlog::error("aiMaterial get texture failed. ai texture type: {}", (int)type);
            continue;
        }

        Texture texture;
        texture.m_type = typeName;
        const aiTexture *aiTex = scene->GetEmbeddedTexture(str.C_Str());
        if (aiTex)
        {
            bool iscompressed = aiTex->mHeight == 0;
            uint textureSize = aiTex->mWidth * (iscompressed ? 1 : aiTex->mHeight);
            // glb格式文件的纹理信息直接在模型上，其它格式文件的纹理信息保存在单独的文件中
            texture.m_data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(aiTex->pcData), textureSize,
                                                        &texture.m_width, &texture.m_height, &texture.m_channel, 0);
        }
        else
        {
            QString filename = QFileInfo(m_currentModelName).filePath() + '/' + filename;
            texture.m_data = stbi_load(filename.toStdString().c_str(), &texture.m_width, &texture.m_height, &texture.m_channel, 0);
        }
        textures.emplace_back(texture);
    }
}

float ModelLoadManager::getModelMaxPos(const QString &modelPath)
{
    if (modelPath.isEmpty())
    {
        spdlog::error("model path is empty. modelPath: {0}", modelPath.toStdString());
        return false;
    }

    if (m_modelMaxPosMaps.contains(modelPath))
        return m_modelMaxPosMaps[modelPath];

    float maxPosition = 1.0;
    if (!m_modelMeshMaps.contains(modelPath))
    {
        std::shared_ptr<QVector<ModelMesh>> modelMeshsPtr;
        import3DModel(modelPath, modelMeshsPtr);
        if (!m_modelMeshMaps.contains(modelPath))
            return maxPosition;
    }
        
    for (const auto &modelMesh : *m_modelMeshMaps[modelPath])
    {
        for (const auto &vertex : modelMesh.m_vertices)
        {
            maxPosition = qMax(qMax(qMax(qAbs(vertex.m_positions[0]), qAbs(vertex.m_positions[1])), qAbs(vertex.m_positions[2])), maxPosition);
        }
    }

    spdlog::info("model name: {0}, max position: {1}.", modelPath.toStdString(), maxPosition);
    m_modelMaxPosMaps.insert(modelPath, maxPosition);
    return maxPosition;
}

void ModelLoadManager::cleanImageData(unsigned char *data)
{
    if (!data)
        return;
    stbi_image_free(data);
}

