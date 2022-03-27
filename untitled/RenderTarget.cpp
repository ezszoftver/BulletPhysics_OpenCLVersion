#include "RenderTarget.h"

RenderTarget::RenderTarget()
{
    m_nWidth = 0;
    m_nHeight = 0;

    m_framebuffer = 0;
    m_depthTexture = 0;
}

bool RenderTarget::Load(std::vector<int> listFormats, int nWidth, int nHeight)
{
    m_listFormats.clear();
    m_listFormats.insert( m_listFormats.end(), listFormats.begin(), listFormats.end() );

    m_listTextures.clear();
    for(unsigned int i = 0; i < m_listFormats.size(); i++)
    {
        m_listTextures.push_back(0);
    }

    Resize(nWidth, nHeight);

    return true;
}

void RenderTarget::Release()
{
    if (0 != m_framebuffer)
    {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    for(unsigned int i = 0; i < m_listTextures.size(); i++)
    {
        if (0 != m_listTextures[i])
        {
            glDeleteTextures(1, &m_listTextures[i]);
            m_listTextures[i] = 0;
        }
    }
    if (0 != m_depthTexture)
    {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }
}

void RenderTarget::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    std::vector<GLenum> listDrawBuffers;
    for(unsigned int i = 0; i < m_listTextures.size(); i++)
    {
        listDrawBuffers.push_back( GL_COLOR_ATTACHMENT0 + i );
    }
    glDrawBuffers(m_listTextures.size(), listDrawBuffers.data());
}

void RenderTarget::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum arrDrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, arrDrawBuffers);
}

void RenderTarget::Resize(int nWidth, int nHeight)
{
    // release
    Release();

    // update
    m_nWidth = nWidth;
    m_nHeight = nHeight;

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    for(unsigned int i = 0; i < m_listTextures.size(); i++)
    {
        glGenTextures(1, &(m_listTextures[i]));
        glBindTexture(GL_TEXTURE_2D, m_listTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, m_listFormats[i], m_nWidth, m_nHeight, 0, GL_RGBA, GL_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_listTextures[i], 0);
    }

    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_nWidth, m_nHeight, 0, GL_DEPTH_COMPONENT, GL_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint RenderTarget::GetTextureID(int i)
{
    return m_listTextures[i];
}
