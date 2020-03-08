#include "stdafx.h"
#include "ZEnemy.h"
#include "ZModule_HPAP.h"

//////////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZEnemy, ZActor);

ZEnemy::ZEnemy() : ZActor()
{

}

ZEnemy::~ZEnemy()
{

}



///////////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZHumanEnemy, ZEnemy);

ZHumanEnemy::ZHumanEnemy() : ZEnemy()
{

}

ZHumanEnemy::~ZHumanEnemy()
{

}

///////////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZFlyEnemy, ZEnemy);

ZFlyEnemy::ZFlyEnemy() : ZEnemy()
{

}

ZFlyEnemy::~ZFlyEnemy()
{

}


///////////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZJacoEnemy, ZEnemy);

ZJacoEnemy::ZJacoEnemy() : ZEnemy()
{

}

ZJacoEnemy::~ZJacoEnemy()
{

}

void ZJacoEnemy::ProcessAI(float fDelta)
{

}


