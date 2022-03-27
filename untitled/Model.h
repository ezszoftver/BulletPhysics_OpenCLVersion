#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

#include "GL/glew.h"
#include "GL/wglew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "Texture.h"
#include "Shader.h"

typedef struct _Vertex
{
    glm::vec3 v3Position;
    glm::vec3 v3Normal;
    glm::vec2 v2TextCoord;
}
Vertex;

typedef struct _Material
{
    std::vector< Vertex > m_listVertices;

    Texture *m_pTexture = nullptr;

    std::vector< uint32_t > m_listIndices;
    GLuint m_glIndexBuffer = 0;
}
Material;

class Model
{
public:
    explicit Model();
    ~Model();

    void Load(std::string strDir, std::string strFilename, glm::mat4 matTransform = glm::mat4(1.0f), bool bIsLoadTextures = true);
    void Release();

    void Begin(Shader* shader);
    void Draw(Shader* shader);
    void End(Shader* shader);

    void CreateOpenGLBuffers();
    std::vector< Vertex >* GetVertices();
    std::vector< Material >* GetMaterials();

private:
    std::vector< Material > m_listMaterials;

    GLuint m_glVertexBuffer = 0;
    std::vector< Vertex > m_listAllVertices;

    bool m_bIsLoadTextures;
};

#endif // MODEL_H
