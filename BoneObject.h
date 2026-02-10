/*
	BoneObject.h
	20260122  hanaue sho
	ボーン専用のゲームオブジェクト
	※基本は何もしない
*/
#ifndef BONEOBJECT_H_
#define BONEOBJECT_H_
#include <string>
#include "GameObject.h"

class BoneObject : public GameObject
{
private:
	std::string m_Name;
	int m_BoneIndex = -1;
public:
	void SetName(const std::string& name) { m_Name = name; }
	std::string Name() const { return m_Name; }
	void SetBoneIndex(int index) { m_BoneIndex = index; }
	int BoneIndex() const { return m_BoneIndex; }
};

#endif