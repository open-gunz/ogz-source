#ifndef _ZENVOBJECT_H
#define _ZENVOBJECT_H

#include "MRTTI.h"
#include <list>
#include <string>
#include "ZObject.h"
#include "ZActor.h"

using namespace std;

_USING_NAMESPACE_REALSPACE2


// 부서지는 박스등의 환경 오브젝트 상위 클래스
class ZEnvObject : public ZActor
{
	MDeclareRTTI;
private:
protected:

public:
	ZEnvObject() : ZActor() {  }
	virtual ~ZEnvObject() { }
};





#endif