#include "stdafx.h"
#ifdef _MSC_VER
#include "MHttpSpooler.h"
#include <chrono>
#include "MDebug.h"

void MHttpSpooler::OnCreate()
{
}

void MHttpSpooler::OnDestroy()
{
	Shutdown();
}

void MHttpSpooler::Run()
{
	while(CheckShutdown() == false) {
		MHttpSpoolerNode* pNode = m_RequestQueue.Pop();
		if (pNode) {
			if (m_AsyncHttp.Get(pNode->GetURL().c_str()))
				m_ResultQueue.Post(pNode);
			else
				delete pNode;
		}
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}
}
#endif
