#pragma once

#include "MTime.h"
#include "GlobalTypes.h"
#include <thread>
#include "optional.h"

class MThread {
public:
	MThread();
	virtual ~MThread();

	void Create();
	void Destroy();

	auto GetThreadHandle() { return Thread.native_handle(); }

	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void Run() {}

protected:
	std::thread Thread;
};