#include "Shader.h"

#include <QApplication>
#include <QMessageBox>

Shader::Shader()
{

}

GLuint Shader::Load(QString strVertexShaderFilename, QString strFragmentShaderFilename)
{
    std::vector<GLchar> sourcecode;

    //LoadFileContents(strVertexShaderFilename, sourcecode);
    GLuint vertex_shader = CompileShader(strVertexShaderFilename, GL_VERTEX_SHADER);

    //LoadFileContents(strFragmentShaderFilename, sourcecode);
    GLuint frag_shader = CompileShader(strFragmentShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, frag_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);

    glLinkProgram(program);

    GLint result = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &result);

    if(result == GL_FALSE)
    {
        GLint length = 0;
        std::vector<char> log;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        log.resize(length);

        glGetProgramInfoLog(program, length, &result, &log[0]);

        glDeleteProgram(program);

        //std::printf("ERROR!: %s\n", std::string(log.begin(), log.end()).c_str());
        QMessageBox msgBox;
        msgBox.setWindowTitle("ERROR!");
        msgBox.setText(QString("Program:\n") + QString(log.data()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        QApplication::exit(0);

        return 0;
    }

    return program;
}

GLuint Shader::CompileShader(QString strFilename, GLenum type)
{
    std::vector<GLchar> shader_source;
    LoadFileContents(strFilename.toStdString(), shader_source);

    GLuint shader = glCreateShader(type);

    GLint len = static_cast<GLint>(shader_source.size());
    GLchar const* source_array = &shader_source[0];

    glShaderSource(shader, 1, &source_array, &len);
    glCompileShader(shader);

    GLint result = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if(result == GL_FALSE)
    {
        std::vector<char> log;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        log.resize(len);

        glGetShaderInfoLog(shader, len, &result, &log[0]);

        glDeleteShader(shader);

        //std::printf("ERROR! %s: %s\n", strFilename.c_str(), std::string(log.begin(), log.end()).c_str());
        //std::system("pause");
        QMessageBox msgBox;
        msgBox.setWindowTitle("ERROR!");
        msgBox.setText(strFilename + QString(":\n") + QString(log.data()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        QApplication::exit(0);

        return 0;
    }

    return shader;
}

void Shader::LoadFileContents(std::string const& name, std::vector<char>& contents, bool binary)
{
    std::ifstream in(name, std::ios::in | (std::ios_base::openmode)(binary?std::ios::binary : 0));

    if (in)
    {
        contents.clear();

        std::streamoff beg = in.tellg();

        in.seekg(0, std::ios::end);

        std::streamoff fileSize = in.tellg() - beg;

        in.seekg(0, std::ios::beg);

        contents.resize(static_cast<unsigned>(fileSize));

        in.read(&contents[0], fileSize);
    }
    else
    {
        throw std::string("Cannot read the contents of a file");
    }
}
