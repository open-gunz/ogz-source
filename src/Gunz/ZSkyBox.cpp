#include "stdafx.h"
#include "ZSkybox.h"
#include "MDebug.h"

using namespace RealSpace2;

ZSkyBox::ZSkyBox(std::unique_ptr<RVisualMesh> VMesh) : VMesh{ std::move(VMesh) }
{
	this->VMesh->m_pMesh->mbSkyBox = true;
}

ZSkyBox::~ZSkyBox() = default;

void ZSkyBox::Render()
{
	VMesh->SetWorldMatrix(GetIdentityMatrix());
	VMesh->Frame();
	VMesh->Render();
}