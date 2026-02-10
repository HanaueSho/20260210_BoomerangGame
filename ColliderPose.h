/*
	ColliderPose.h
	20260125  hanaue sho
	ColliderComponent ‚Å TransformComponent + offset ‚Ì“–‚½‚è”»’è
*/
#ifndef COLLIDERPOSE_H_
#define COLLIDERPOSE_H_
#include "Vector3.h"
#include "Quaternion.h"

struct ColliderPose
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;

	// •Ö—˜
	Vector3 WorldRight()   const 
	{ 
		//return rotation.Rotate({ 1, 0, 0 }).normalized(); // ‚±‚Á‚¿‚¾‚Æ‚È‚º‚©•sˆÀ’è‚É‚È‚é
		Matrix4x4 R = rotation.ToMatrix();
		return Vector3(R.m[0][0], R.m[0][1], R.m[0][2]).normalized();
	}
	Vector3 WorldUp()	   const 
	{
		//return rotation.Rotate({ 0, 1, 0 }).normalized();
		Matrix4x4 R = rotation.ToMatrix();
		return Vector3(R.m[1][0], R.m[1][1], R.m[1][2]).normalized();
	}
	Vector3 WorldForward() const 
	{
		//return rotation.Rotate({ 0, 0, 1 }).normalized();
		Matrix4x4 R = rotation.ToMatrix();
		return Vector3(R.m[2][0], R.m[2][1], R.m[2][2]).normalized();
	}
};

#endif