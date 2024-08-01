#include "opengl_helper.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <QFile>
#include <QFileInfo>

namespace OpenGLHelper
{
    QString sModelName;
    Assimp::Importer importer;

    bool parseObjModel(const QString &modelPath, ObjData &objData)
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

        QVector<float> vertextPoints, texturePoints, normalPoints;
        QVector<std::tuple<int, int, int>> facesIndexs;
        while (!objFile.atEnd())
        {
            QByteArray lineData = objFile.readLine();
            QList<QByteArray> strValues = lineData.trimmed().split(' ');
            QString dataType = strValues.takeFirst();
            if (dataType == "v")
            {
                std::transform(strValues.begin(), strValues.end(), std::back_inserter(vertextPoints), [](QByteArray &str)
                               { return str.toFloat(); });
            }
            else if (dataType == "vt")
            {
                std::transform(strValues.begin(), strValues.end(), std::back_inserter(texturePoints), [](QByteArray &str)
                               { return str.toFloat(); });
            }
            else if (dataType == "vn")
            {
                std::transform(strValues.begin(), strValues.end(), std::back_inserter(normalPoints), [](QByteArray &str)
                               { return str.toFloat(); });
            }
            else if (dataType == "f")
            {
                std::transform(strValues.begin(), strValues.end(), std::back_inserter(facesIndexs), [](QByteArray &str)
                               {
                QList<QByteArray> intStr = str.split('/');
                return std::make_tuple(intStr.first().toInt(), intStr.at(1).toInt(), intStr.last().toInt()); });
            }
        }
        objFile.close();

        try
        {
            for (auto &verFaceInfo : facesIndexs)
            {
                int vIndex = std::get<0>(verFaceInfo);
                int tIndex = std::get<1>(verFaceInfo);
                int nIndex = std::get<2>(verFaceInfo);
                objData.m_vPoints << vertextPoints.at((vIndex - 1) * 3 + 0);
                objData.m_vPoints << vertextPoints.at((vIndex - 1) * 3 + 1);
                objData.m_vPoints << vertextPoints.at((vIndex - 1) * 3 + 2);

                objData.m_tPoints << texturePoints.at((tIndex - 1) * 2 + 0);
                objData.m_tPoints << texturePoints.at((tIndex - 1) * 2 + 1);

                objData.m_nPoints << normalPoints.at((nIndex - 1) * 3 + 0);
                objData.m_nPoints << normalPoints.at((nIndex - 1) * 3 + 1);
                objData.m_nPoints << normalPoints.at((nIndex - 1) * 3 + 2);
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

    bool import3DModel(const QString &modelPath, QVector<ModelMesh> &modelMeshs)
    {
        stbi_set_flip_vertically_on_load(true);

        const aiScene *scene = importer.ReadFile(modelPath.toStdString(),
                                                 aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            spdlog::error("importer read file failed. file: {0}, reason: {1}", modelPath.toStdString(), importer.GetErrorString());
            return false;
        }
        sModelName = modelPath;

        processNode(scene->mRootNode, scene, modelMeshs);

        return true;
    }

    void processNode(aiNode *node, const aiScene *scene, QVector<ModelMesh> &modelMeshs)
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

    void processMesh(aiMesh *mesh, const aiScene *scene, ModelMesh &modelMesh)
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

    void loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName, const aiScene *scene, std::vector<Texture> &textures)
    {
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            if (aiReturn_SUCCESS != mat->GetTexture(type, i, &str))
            {
                spdlog::error("aiMaterial get texture failed. ai texture type: {}", type);
                continue;
            }

            Texture texture;
            texture.m_type = typeName;
            const aiTexture *aiTex = scene->GetEmbeddedTexture(str.C_Str());
            if (aiTex)
            {
                // glb格式文件的纹理信息直接在模型上，其它格式文件的纹理信息保存在单独的文件中
                if (aiTex->mHeight == 0)
                    texture.m_data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(aiTex->pcData), aiTex->mWidth,
                                                           &texture.m_width, &texture.m_height, &texture.m_channel, 0);
                else
                    texture.m_data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(aiTex->pcData), aiTex->mWidth * aiTex->mHeight,
                                                           &texture.m_width, &texture.m_height, &texture.m_channel, 0);
            }
            else
            {
                QString filename = QFileInfo(sModelName).filePath() + '/' + filename;
                texture.m_data = stbi_load(filename.toStdString().c_str(), &texture.m_width, &texture.m_height, &texture.m_channel, 0);
            }
            textures.emplace_back(texture);
        }
    }

    void cleanImageData(unsigned char *data)
    {
        if (!data)
            return;
        stbi_image_free(data);
    }
}
