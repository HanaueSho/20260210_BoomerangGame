/*
	DebugCount.h
	20251010  hanaue sho
	デバッグでカウントするよう
*/
#ifndef DEBUGCOUNTER_H_
#define DEBUGCOUNTER_H_
#include "Vector3.h"
#include <map>
#include <string>

class DebugCount
{
private:
	static std::map<std::string, int> m_Counters;

public:
	static void Count (const std::string& name, int n = 1);
	static void Printf(const std::string& name);
	static void PrintfAll();
	static void Reset(const std::string& name);
	static void ResetAll();
};



#endif