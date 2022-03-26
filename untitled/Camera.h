#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"

class Camera
{
public:
	void Init(glm::vec3 pos, glm::vec3 at) 
	{
		this->pos = pos;
		this->dir = glm::normalize(at - pos);
		this->up = glm::vec3(0, 1, 0);
	}

	void Update(float dt) 
	{
		this->dt = dt;
	}

	void MoveW() 
	{
		pos += dir * fMoveSpeed * dt;
	}
	void MoveS() 
	{
		pos -= dir * fMoveSpeed * dt;
	}
	void MoveD()
	{
		glm::vec3 dir = glm::cross(this->dir, up);
		pos += dir * fMoveSpeed * dt;
	}
	void MoveA()
	{
		glm::vec3 dir = glm::cross(this->dir, up);
		pos -= dir * fMoveSpeed * dt;
	}
	

	void Rotate(int nCurrentMouseX, int nCurrentMouseY) 
	{
		static int nElapsedMouseX = nCurrentMouseX;
		static int nElapsedMouseY = nCurrentMouseY;
		int diffX = nCurrentMouseX - nElapsedMouseX;
		int diffY = nCurrentMouseY - nElapsedMouseY;
		nElapsedMouseX = nCurrentMouseX;
		nElapsedMouseY = nCurrentMouseY;

		if (diffX < -20) { diffX = -20; }
		if (diffY < -20) { diffY = -20; }
		if (diffX > 20) { diffX = 20; }
		if (diffY > 20) { diffY = 20; }

		// rotate X
		glm::vec3 axisX = glm::cross(this->dir, up);
		glm::mat4 T1 = glm::rotate(glm::mat4(1), (float)-diffY * fRotateSpeed, axisX);

		// rotateY
		glm::vec3 axisY = glm::vec3(0, 1, 0);
		glm::mat4 T2 = glm::rotate(glm::mat4(1), (float)-diffX * fRotateSpeed, axisY);

		// apply
		dir = glm::vec3( (T2 * T1) * glm::vec4(dir, 0) );
		dir = glm::normalize(dir);
	}

	glm::vec3 GetPos() 
	{
		return pos;
	}

	glm::vec3 GetAt() 
	{
		return (pos + dir);
	}

    void SetPos(glm::vec3 pos)
    {
        this->pos = pos;
    }

private:
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 up;

	float fMoveSpeed = 5.0f;
	float fRotateSpeed = 0.003f;

	float dt;
}; 

