#ifndef SHADER_H
#define SHADER_H

#include "GL/glew.h"
#include "GL/wglew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

#include <QString>

class Shader
{
public:
    Shader();

    static GLuint Load(QString strVertexShaderFilename, QString strFragmentShaderFilename);

private:
    static void LoadFileContents(std::string const& name, std::vector<char>& contents, bool binary = false);
    static GLuint CompileShader(QString strFilename, GLenum type);
};

#endif // SHADER_H
