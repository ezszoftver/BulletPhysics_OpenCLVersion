#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "GL/glew.h"
#include "FreeImage.h"

class Texture
{
private:
    Texture();
public:
    static Texture* GetInstance();

    static const int REPEAT         = 1;
    static const int CLAMP_TO_EDGE  = 2;

    int LoadTextureFromFile(std::string strFilename, int nRepeatType = REPEAT);
private:
    static Texture *m_sTexture;

};

#endif // TEXTURE_H
