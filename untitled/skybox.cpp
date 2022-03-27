#include "SkyBox.h"

#include "texture.h"

SkyBox::SkyBox()
{

}

void SkyBox::Load(QString strDir, QString strBack, QString strDown, QString strFront, QString strLeft, QString strRight, QString strUp)
{
    // back
    texture[0].Load((strDir + strBack).toStdString(), Texture::REPEAT_OFF);

    vertices[0] = glm::vec3(-1,-1,-1); vertices[1] = glm::vec3(1,-1,-1); vertices[2] = glm::vec3(1,1,-1);
    textures[0] = glm::vec2(0,0);      textures[1] = glm::vec2(1,0);     textures[2] = glm::vec2(1,1);

    vertices[3] = glm::vec3(-1,-1,-1); vertices[4] = glm::vec3(1,1,-1); vertices[5] = glm::vec3(-1,1,-1);
    textures[3] = glm::vec2(0,0);      textures[4] = glm::vec2(1,1);    textures[5] = glm::vec2(0,1);

    // down
    texture[1].Load((strDir + strDown).toStdString(), Texture::REPEAT_OFF);

    vertices[6] = glm::vec3(-1,-1,-1); vertices[7] = glm::vec3(-1,-1, 1); vertices[8] = glm::vec3( 1,-1, 1);
    textures[6] = glm::vec2( 0, 0);    textures[7] = glm::vec2( 1, 0);    textures[8] = glm::vec2( 1, 1);

    vertices[9] = glm::vec3(-1,-1,-1); vertices[10] = glm::vec3( 1,-1, 1); vertices[11] = glm::vec3( 1,-1,-1);
    textures[9] = glm::vec2( 0, 0);    textures[10] = glm::vec2( 1, 1);    textures[11] = glm::vec2( 0, 1);

    // front
    texture[2].Load((strDir + strFront).toStdString(), Texture::REPEAT_OFF);

    vertices[12] = glm::vec3( 1, 1, 1); vertices[13] = glm::vec3( 1,-1, 1); vertices[14] = glm::vec3(-1,-1, 1);
    textures[12] = glm::vec2( 0, 1);    textures[13] = glm::vec2( 0, 0);    textures[14] = glm::vec2( 1, 0);

    vertices[15] = glm::vec3( 1, 1, 1); vertices[16] = glm::vec3(-1,-1, 1); vertices[17] = glm::vec3(-1, 1, 1);
    textures[15] = glm::vec2( 0, 1);    textures[16] = glm::vec2( 1, 0);    textures[17] = glm::vec2( 1, 1);

    // left
    texture[3].Load((strDir + strLeft).toStdString(), Texture::REPEAT_OFF);

    vertices[18] = glm::vec3(-1,-1, 1); vertices[19] = glm::vec3(-1,-1,-1); vertices[20] = glm::vec3(-1, 1, 1);
    textures[18] = glm::vec2( 0, 0);    textures[19] = glm::vec2( 1, 0);    textures[20] = glm::vec2( 0, 1);

    vertices[21] = glm::vec3(-1, 1, 1); vertices[22] = glm::vec3(-1,-1,-1); vertices[23] = glm::vec3(-1, 1,-1);
    textures[21] = glm::vec2( 0, 1);    textures[22] = glm::vec2( 1, 0);    textures[23] = glm::vec2( 1, 1);

    // right
    texture[4].Load((strDir + strRight).toStdString(), Texture::REPEAT_OFF);

    vertices[24] = glm::vec3( 1,-1,-1); vertices[25] = glm::vec3( 1,-1, 1); vertices[26] = glm::vec3( 1, 1, 1);
    textures[24] = glm::vec2( 0, 0);    textures[25] = glm::vec2( 1, 0);    textures[26] = glm::vec2( 1, 1);

    vertices[27] = glm::vec3( 1,-1,-1); vertices[28] = glm::vec3( 1, 1, 1); vertices[29] = glm::vec3( 1, 1,-1);
    textures[27] = glm::vec2( 0, 0);    textures[28] = glm::vec2( 1, 1);    textures[29] = glm::vec2( 0, 1);

    // up
    texture[5].Load((strDir + strUp).toStdString(), Texture::REPEAT_OFF);

    vertices[30] = glm::vec3(-1, 1,-1); vertices[31] = glm::vec3( 1, 1,-1); vertices[32] = glm::vec3(-1, 1, 1);
    textures[30] = glm::vec2( 0, 1);    textures[31] = glm::vec2( 0, 0);    textures[32] = glm::vec2( 1, 1);

    vertices[33] = glm::vec3(-1, 1, 1); vertices[34] = glm::vec3( 1, 1,-1); vertices[35] = glm::vec3( 1, 1, 1);
    textures[33] = glm::vec2( 1, 1);    textures[34] = glm::vec2( 0, 0);    textures[35] = glm::vec2( 1, 0);
}

void SkyBox::Draw(glm::vec3 v3Center, float fZFar)
{
    glActiveTexture(GL_TEXTURE0);

    for(int i = 0; i < 6; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i].ID() );

        glBegin(GL_TRIANGLES);
        {
            for(int j = 0; j < 6; j++)
            {
                glm::vec2 t = textures[(i * 6) + j];
                glm::vec3 v = v3Center + ( vertices[(i * 6) + j] * fZFar );

                glTexCoord2f(t.x, t.y);
                glVertex3f(v.x, v.y, v.z);
            }
        }
        glEnd();
    }
}

void SkyBox::Release()
{
    for(int i = 0; i < 6; i++)
    {
        texture[i].Release();
    }
}
