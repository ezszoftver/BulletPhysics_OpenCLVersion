#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "GL/glew.h"
#include "GL/wglew.h"

#include <vector>

class RenderTarget
{
public:
    RenderTarget();

    bool Load(std::vector<int> listFormats, int nWidth = 8, int nHeight = 8);
    void Release();

    void Bind();
    void Unbind();
    void Resize(int nWidth, int nHeight);
    GLuint GetTextureID(int i);

private:
    int m_nWidth;
    int m_nHeight;

    std::vector<int> m_listFormats;

    GLuint m_framebuffer;
    std::vector<GLuint> m_listTextures;
    GLuint m_depthTexture;
};

#endif // RENDERTARGET_H
