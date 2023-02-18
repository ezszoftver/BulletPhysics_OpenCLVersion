#pragma once

#include "GL/glew.h"
#include "GL/wglew.h"
#include "FreeImage.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>


class Texture
{
	GLuint textureID;
    std::string name;
public:
	static unsigned int REPEAT_ON;
	static unsigned int REPEAT_OFF;

    bool Load(std::string Filename, unsigned int Flag = Texture::REPEAT_ON)
	{
		FREE_IMAGE_FORMAT fileFormat = FIF_UNKNOWN;
		FIBITMAP *image(0);
		BYTE* bits(0);

        fileFormat = FreeImage_GetFileType(Filename.c_str(), 0);
        image = FreeImage_Load(fileFormat, Filename.c_str());

        if(!image)
        {
            return false;
        }

		image = FreeImage_ConvertTo32Bits(image);
		bits = FreeImage_GetBits(image);
		int width = FreeImage_GetWidth(image);
		int height = FreeImage_GetHeight(image);

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);

		// anisotropy
		GLfloat MaxAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropy);

        if (Flag == Texture::REPEAT_ON)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
        if (Flag == Texture::REPEAT_OFF)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

		FreeImage_Unload(image);

        name = Filename;

		return true;
	}
	void Release()
	{
        glDeleteTextures(1, &textureID);
	}

	GLuint ID()
	{
		return textureID;
	}
    std::string GetName()
	{
		return name;
	}
};
