/*
	Bezier.h
	ベジェ曲線
	20250617  hanaue sho
*/
#ifndef BEZIER_H_
#define BEZIER_H_
#include "vector3.h"
#include <vector>


// ベジェ曲線のクラス
class Bezier
{
private:
	std::vector<Vector3> m_ControllPoints;

public:
	Bezier() = default;
	~Bezier() = default;

	void CreatePoint(Vector3 point)
	{
		m_ControllPoints.push_back(point);
	}

	void DeletePoint(int id)
	{
		if (id < 0) return;
		if (m_ControllPoints.size() < 1) return;

		auto iter = m_ControllPoints.begin() + id;
		m_ControllPoints.erase(iter);
	}

	// 引数は０～１の浮動小数点
	Vector3 GetValue(float t)
	{
		if (t < 0) t = 0; if (t > 1) t = 1;

		int n = m_ControllPoints.size() - 1;
		Vector3 result = Vector3(0, 0, 0);
		for (int i = 0; i <= n ; i++)
		{
			result = result + m_ControllPoints[i] * ((float)Factorial(n) / (Factorial(i) * Factorial(n - i)) * (float)pow((1 - t), n - i) * (float)pow(t, i));
		}
		return result;
	}

	std::vector<Vector3>& GetControllPoints() { return m_ControllPoints; }

private:
	int Factorial(int num)
	{
		int result = 1;
		for (int i = 2; i <= num; i++)
			result *= i;
		return result;
	}

};



#endif