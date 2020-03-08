#ifndef __ANIMCONSTRUCTOR_H
#define __ANIMCONSTRUCTOR_H

#pragma warning(disable:4530)

#include "Max.h"
#include "CMList.h"
#include <list>

using namespace std;

enum ANIMATIONMETHOD {
	AM_TRANSFORM = 0,
	AM_VERTEX	= 1,
	AM_KEYFRAME	= 2
};

struct RSVisibilityKey {
	float time,value;
};

struct RSScaleKey {
	float time;
	ScaleValue value;
};

struct RSRotationKey {
	float time;
	Quat value;
};

struct RSPositionKey {
	float time;
	Point3 value;
};

template<class NEW_TYPE> class RSKeyController : public list < NEW_TYPE* > {
protected:
	list < NEW_TYPE* > :: iterator m_Current;
public:
	virtual ~RSKeyController() {
		while(size())
		{
			delete *(begin());
			erase(begin());
		}
	}

	bool Save(FILE *file) {
		list < NEW_TYPE* > ::iterator i;
		int nSize=size();
		fwrite(&nSize,sizeof(int),1,file);
		for(i=begin();i!=end();i++)
			fwrite(*i,sizeof(NEW_TYPE),1,file);
		return true;
	}

	virtual void Get(float fTime,NEW_TYPE *ret) {}
};

class RSVisList : public RSKeyController <RSVisibilityKey>
{
public:
	virtual void Get(float fTime,RSVisibilityKey *ret);
};

class AnimNode
{
public:
	AnimNode() { name=NULL;tm=NULL;vertanim=NULL;nV=0; }
	~AnimNode() { if(name) delete []name;if(tm) delete []tm;if(vertanim) delete []vertanim; }

	char	*name;
	Matrix3 *tm;
	Point3	*vertanim;
	int		nV;
	
	RSVisList Visibility;
	RSKeyController <RSScaleKey> ScaleKeyList;
	RSKeyController <RSRotationKey> RotationKeyList;
	RSKeyController <RSPositionKey> PositionKeyList;
};

class ACAnimNodeList : public list<AnimNode*>
{
public:
	virtual ~ACAnimNodeList();

	iterator Get(char *Name);
};

class AnimConstructor
{
public:
	AnimConstructor();
	bool BuildAnimation(Interface *ip,char *name,float fSpeed,ANIMATIONMETHOD method);
	void Save(FILE *pStream);
	void SetDetailLog(bool bDLog) { m_bDetailLog=bDLog; }

	ACAnimNodeList objlist;

private:
	TriObject* GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	bool CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale);
	bool isNotBipName(const char *name);
	bool nodeEnum(INode* node, int indentLevel);
	bool BuildVisibility(RSVisList *vl, INode *node);
	bool IsKnownController(Control* cont);
	bool BuildKeys(AnimNode *an, INode *node);

	Interface *ip;
	int nFrame,delta;
	ANIMATIONMETHOD m_AnimationMethod;
	bool m_bDetailLog;
	char m_szName[256];
	float m_fSpeed;
	float m_fFrameSpeed;
};

#endif