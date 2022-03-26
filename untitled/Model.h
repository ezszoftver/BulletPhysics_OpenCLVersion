#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

#include "GL/glew.h"
#include "GL/wglew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

typedef struct _Vertex
{
    glm::vec3 v3Position;
    glm::vec3 v3Normal;
    glm::vec2 v2TextCoord;
}
Vertex;

typedef struct _Material
{
    int m_nNumVertices = 0;
    std::vector< Vertex > m_listVertices;

    GLuint m_glTextureId = 0;

    std::vector< uint32_t > m_listIndices;
    GLuint m_glIndexBuffer = 0;
}
Material;

class Model
{
public:
    explicit Model();
    ~Model();

    void LoadFromOBJFile(std::string strDir, std::string strFilename, glm::vec3 v3Translate = glm::vec3(0,0,0));
    void Draw(GLuint program);
    void Release();

    void CreateOpenGLBuffers();
    std::vector< Vertex >* GetVertices();

private:
    std::vector< Material > m_listMaterials;

    GLuint m_glVertexBuffer = 0;
    std::vector< Vertex > m_listAllVertices;
};

#endif // MODEL_H
