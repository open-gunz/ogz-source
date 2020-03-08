#include "stdafx.h"
#include "ZModule.h"
#include "crtdbg.h"

MImplementRootRTTI(ZModuleContainer);

ZModule *ZModuleContainer::GetModule(int nID)
{
	auto it = Modules.find(nID);
	if (it == Modules.end())
		return nullptr;

	return it->second.get();
}

void ZModuleContainer::ActivateModule(int nID, bool bActive)
{
	auto it = Modules.find(nID);
	if (it == Modules.end())
		return;

	it->second->Active = bActive;
}

bool ZModuleContainer::IsActiveModule(int nID)
{
	auto it = Modules.find(nID);
	if(it == Modules.end())
		return false;

	return it->second->Active;
}

void ZModuleContainer::UpdateModules(float Elapsed)
{
	for (auto&& Module : MakePairValueAdapter(Modules))
		Module->Update(Elapsed);
}

void ZModuleContainer::InitModuleStatus(void)
{
	for (auto&& Module : MakePairValueAdapter(Modules))
		Module->InitStatus();
}