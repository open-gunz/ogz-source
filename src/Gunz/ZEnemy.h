#ifndef _ZENEMY_H
#define _ZENEMY_H

#include "MRTTI.h"
#include "ZObject.h"
#include "ZActor.h"

_USING_NAMESPACE_REALSPACE2

class ZEnemy;

class ZEnemy : public ZActor
{
	MDeclareRTTI;
protected:

public:
	ZEnemy();
	virtual ~ZEnemy();
};

class ZHumanEnemy : public ZEnemy
{
	MDeclareRTTI;
private:
protected:
public:
	ZHumanEnemy();
	virtual ~ZHumanEnemy();
};

class ZFlyEnemy : public ZEnemy
{
	MDeclareRTTI;
private:
protected:
public:
	ZFlyEnemy();
	virtual ~ZFlyEnemy();
};

class ZJacoEnemy : public ZEnemy
{
	MDeclareRTTI;
private:
protected:
	virtual void ProcessAI(float fDelta);
public:
	ZJacoEnemy();
	virtual ~ZJacoEnemy();
};


#endif