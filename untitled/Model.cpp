#include "Model.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "texture.h"

Model::Model()
{

}

Model::~Model()
{
    Release();
}

void Model::LoadFromOBJFile(std::string strDir, std::string strFilename, glm::vec3 v3Translate)
{
    Assimp::Importer importer;
    const aiScene *pScene = importer.ReadFile((strDir + "/" + strFilename), aiProcessPreset_TargetRealtime_MaxQuality);

    for (unsigned int m = 0; m < pScene->mNumMaterials; m++)
    {
        Material material;

        if (pScene->mMaterials[m]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            pScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            std::string strTextureFilename(path.C_Str());

            material.m_glTextureId = Texture::GetInstance()->LoadTextureFromFile(strDir + "/" + strTextureFilename);
        }

        m_listMaterials.push_back(material);
    }

    for(unsigned int m = 0; m < pScene->mNumMeshes; m++)
    {
        aiMesh *pMesh = pScene->mMeshes[m];

        Material &material = m_listMaterials[pMesh->mMaterialIndex];

        for(unsigned int f = 0; f < pMesh->mNumFaces; f++)
        {
            aiFace face = pMesh->mFaces[f];

            for(int i = 0; i < 3; i++)
            {
                aiVector3D v = pMesh->mVertices[ face.mIndices[i] ];
                aiVector3D n = pMesh->mNormals[ face.mIndices[i] ];
                aiVector3D t = pMesh->mTextureCoords[0][ face.mIndices[i] ];

                Vertex vertex;
                vertex.v3Position = glm::vec3(v.x, v.y, v.z) + v3Translate;
                vertex.v3Normal = glm::vec3(n.x, n.y, n.z);
                vertex.v2TextCoord = glm::vec2(t.x, t.y);

                material.m_listVertices.push_back(vertex);
            }
        }
    }

    int id = 0;
    for (int m = 0; m < m_listMaterials.size(); m++)
    {
        Material& material = m_listMaterials[m];

        for (int i = 0; i < material.m_listVertices.size(); i++)
        {
            Vertex vertex = material.m_listVertices[i];
            m_listAllVertices.push_back(vertex);

            material.m_listIndices.push_back(id);
            id++;
        }
    }
}

void Model::CreateOpenGLBuffers()
{
    glGenBuffers(1, &m_glVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_listAllVertices.size(), m_listAllVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int m = 0; m < m_listMaterials.size(); m++)
    {
        Material &material = m_listMaterials[m];

        if (material.m_listIndices.size() == 0)
        {
            continue;
        }

        glGenBuffers(1, &material.m_glIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, material.m_glIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * material.m_listIndices.size(), material.m_listIndices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void Model::Draw(GLuint program)
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3)) );
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec3)) );

    for(int m = 0; m < m_listMaterials.size(); m++)
    {
        Material &material = m_listMaterials[m];

        if (material.m_listVertices.size() == 0)
        {
            continue;
        }

        glUniform1i(glGetUniformLocation(program, "g_Texture"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.m_glTextureId);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, material.m_glIndexBuffer);
        glDrawElements(GL_TRIANGLES, (int)material.m_listIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Model::Release()
{
    for(int m = 0; m < m_listMaterials.size(); m++)
    {
        Material &material = m_listMaterials[m];

        material.m_listVertices.clear();

        if (0 != material.m_glTextureId)
        {
            glDeleteTextures(1, &material.m_glTextureId);
            material.m_glTextureId = 0;
        }
    }

    m_listMaterials.clear();
    m_listAllVertices.clear();
}

std::vector< Vertex >* Model::GetVertices()
{
    return &m_listAllVertices;
}
