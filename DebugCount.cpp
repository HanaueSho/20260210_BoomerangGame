/*
	DebugCount.h
	20251010  hanaue sho
	デバッグでカウントするよう
*/
#include "DebugCount.h"

std::map<std::string, int> DebugCount::m_Counters;

void DebugCount::Count(const std::string& name, int n)
{
	// name が存在しない場合は０で自動生成される
	m_Counters[name] += n;
}

void DebugCount::Printf(const std::string& name)
{
	if (m_Counters.find(name) == m_Counters.end())
	{
		printf("[DebugCount] don't find value (%s)\n", name.c_str());
		return;
	}
	int num = m_Counters[name];
	printf("%s : %d\n", name.c_str(), num);
}

void DebugCount::PrintfAll()
{
	printf("[DebugCount] PrintfAll ----- Begin -----\n");
	for (const auto& counter : m_Counters)
	{
		DebugCount::Printf(counter.first);
	}
	printf("[DebugCount] PrintfAll ----- End -----\n");
}

void DebugCount::Reset(const std::string& name)
{
	if (m_Counters.find(name) == m_Counters.end()) return;
	m_Counters[name] = 0;
}

void DebugCount::ResetAll()
{
	for (const auto& counter : m_Counters)
	{
		DebugCount::Reset(counter.first);
	}
}
