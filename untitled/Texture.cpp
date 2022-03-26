#include "Texture.h"

Texture *Texture::m_sTexture = new Texture();

Texture::Texture()
{

}

Texture* Texture::GetInstance()
{
    return m_sTexture;
}

int Texture::LoadTextureFromFile(std::string strFilename, int nRepeatType)
{
    FREE_IMAGE_FORMAT fileFormat = FreeImage_GetFileType(strFilename.c_str(), 0);
    FIBITMAP* image = FreeImage_Load(fileFormat, strFilename.c_str());

    if (!image) return 0;

    image = FreeImage_ConvertTo32Bits(image);
    BYTE* bits = FreeImage_GetBits(image);
    int width = FreeImage_GetWidth(image);
    int height = FreeImage_GetHeight(image);

    GLuint nId;
    glGenTextures(1, &nId);
    glBindTexture(GL_TEXTURE_2D, nId);

    if (nRepeatType == REPEAT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else if (nRepeatType == CLAMP_TO_EDGE)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // anisotropy
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
    // filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_BGRA, GL_UNSIGNED_BYTE, bits);

    glBindTexture(GL_TEXTURE_2D, 0);

    FreeImage_Unload(image);

    return nId;
}
