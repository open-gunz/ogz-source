#include "stdafx.h"
#include "MThread.h"
#include "MDebug.h"

MThread::MThread() = default;
MThread::~MThread()
{
	if (Thread.joinable())
		Destroy();
}

void MThread::Create()
{
	DMLog("MThread::Create %p -- Pre-join, joinable = %d\n", static_cast<void*>(this), Thread.joinable());
	if (Thread.joinable())
		Thread.join();
	DMLog("MThread::Create %p -- Post-join\n", static_cast<void*>(this));
	Thread = std::thread{ [&] { Run(); } };
	OnCreate();
}

void MThread::Destroy()
{
	OnDestroy();
	DMLog("MThread::Destroy %p -- Pre-join, joinable = %d\n", static_cast<void*>(this), Thread.joinable());
	if (Thread.joinable())
		Thread.join();
	DMLog("MThread::Destroy %p -- Post-join\n", static_cast<void*>(this));
}