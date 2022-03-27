#pragma once


#include "GL/glew.h"
#include "GL/wglew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform2.hpp"

#include "Texture.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class Shader
{
	GLuint ProgramID;
public:
    bool Load(std::string vertex_file_path, std::string fragment_file_path)
	{
		// Create the shaders
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the Vertex Shader code from the file
        std::string VertexShaderCode;
        std::ifstream VertexShaderStream(vertex_file_path.c_str(), std::ios::in);
		if(VertexShaderStream.is_open())
		{
            std::string Line = "";
			while(getline(VertexShaderStream, Line))
				VertexShaderCode += "\n" + Line;
			VertexShaderStream.close();
		}
		else
		{
            std::cout << "Nem talalom a fajlt!: " << vertex_file_path << std::endl;
			return false;
		}

		// Read the Fragment Shader code from the file
        std::string FragmentShaderCode;
        std::ifstream FragmentShaderStream(fragment_file_path.c_str(), std::ios::in);
		if(FragmentShaderStream.is_open())
		{
            std::string Line = "";
			while(getline(FragmentShaderStream, Line))
				FragmentShaderCode += "\n" + Line;
			FragmentShaderStream.close();
		}
		else
		{
            std::cout << "Nem talalom a fajlt!: " << fragment_file_path << std::endl;
			return false;
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Vertex Shader
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
		glCompileShader(VertexShaderID);
		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result)
		{
            std::vector < char > VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            std::cout << "HIBA!: Vertex Shader nem fordult le!" << std::endl << &VertexShaderErrorMessage[0] << std::endl;
			return false;
		}

		// Compile Fragment Shader
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
		glCompileShader(FragmentShaderID);
		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result)
		{
            std::vector < char > FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            std::cout << "HIBA!: Fragment Shader nem fordult le!" << std::endl << &FragmentShaderErrorMessage[0] << std::endl;
			return false;
		}

		// Link the program
		ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);
		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result)
		{
            std::vector < char > ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            std::cout << "HIBA!: Shader Linkelesi hiba!" << std::endl << &ProgramErrorMessage[0] << std::endl;
			return false;
		}

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return true;
	}
	void Release()
	{
		glDeleteProgram(ProgramID);
	}

	void Begin()
	{
		glUseProgram(ProgramID);
	}
	void End()
	{
		glUseProgram(0);
	}

    void SetVector3(const char *varname, glm::vec3 *val)
    {
        GLint loc = glGetUniformLocation(ProgramID, varname);
        glUniform3fv(loc, 1, (float*)&(val[0]));
    }

    void SetVector4(const char *varname, glm::vec4 *val)
    {
        GLint loc = glGetUniformLocation(ProgramID, varname);
        glUniform4fv(loc, 1, (float*)&(val[0]));
    }

    void SetVector4(const char *varname, glm::vec3 *val, float w)
    {
        glm::vec4 val2(*val, w);

        GLint loc = glGetUniformLocation(ProgramID, varname);
        glUniform4fv(loc, 1, (float*)&(val2[0]));
    }

    void SetMatrix(const char *varname, glm::mat4 *val)
	{
		GLint loc = glGetUniformLocation(ProgramID, varname);
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&(val[0][0]));
	}
    void SetMatrixArray(const char *varname, glm::mat4 *val, unsigned int count)
	{
		GLint loc = glGetUniformLocation(ProgramID, varname);
		glUniformMatrix4fv(loc, count, GL_FALSE, (float*)&(val[0][0]));
	}
	void SetTexture(const char *varname, Texture *texture, unsigned int i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture->ID());

		GLint loc = glGetUniformLocation(ProgramID, varname);
		glUniform1i(loc, i);
	}
    void SetTexture(const char *varname, GLuint id, unsigned int i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, id);

        GLint loc = glGetUniformLocation(ProgramID, varname);
        glUniform1i(loc, i);
    }
    void DisableTexture(unsigned int i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

	void BindVector4Buffer(const char *attribName, GLuint buffer)
	{
		GLuint attribID = glGetAttribLocation(ProgramID, attribName);

		glEnableVertexAttribArray(attribID);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer
			(
			attribID,                     // The attribute we want to configure
			4,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
			);
	}
	void BindVector3Buffer(const char *attribName, GLuint buffer)
	{
		GLuint attribID = glGetAttribLocation(ProgramID, attribName);

		glEnableVertexAttribArray(attribID);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer
			(
			attribID,                     // The attribute we want to configure
			3,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
			);
	}
	void BindVector2Buffer(const char *attribName, GLuint buffer)
	{
		GLuint attribID = glGetAttribLocation(ProgramID, attribName);

		glEnableVertexAttribArray(attribID);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer
			(
			attribID,                     // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
			);
	}
	void UnbindBuffer(const char *attribName)
	{
		GLuint attribID = glGetAttribLocation(ProgramID, attribName);
		glDisableVertexAttribArray(attribID);

	}
	void DrawTriangles(unsigned int first, unsigned int count)
	{
		glDrawArrays(GL_TRIANGLES, first, count);
	}
};
