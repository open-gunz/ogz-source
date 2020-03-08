#ifndef ZMESHVIEWLIST_H
#define ZMESHVIEWLIST_H

#include "MGroup.h"
#include "RMesh.h"

//class MButton;
class MBmButton;
class ZMeshView;

// 프레임을 그리기 위해 MGroup을 상속받는다.
class ZMeshViewList : public MGroup{
protected:
	int	m_nItemStartIndex;	// 아이템 시작 인덱스
	int	m_nItemWidth;		// 아이템 가로 크기
	//MButton*	m_pLeft;	// 왼쪽 이동
	//MButton*	m_pRight;	// 오른쪽 이동
	MBmButton* m_pBmLeft;
	MBmButton* m_pBmRight;
	//list<ZMeshView*>	m_Items;
	float m_ScrollButtonWidth;

protected:
	virtual void OnDraw(MDrawContext* pDC);
	virtual void OnSize(int w, int h);
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);
	int GetItemVisibleWidth(void);
	int GetItemWidth(void);
	int GetVisibleCount(void);
	void RecalcBounds(void);
public:
	ZMeshViewList(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZMeshViewList(void);

	int GetItemCount(void);
	ZMeshView* GetItem(int i);

	void Add(RealSpace2::RMesh* pMeshRef);
	void RemoveAll(void);
	void Remove(int i);

	void SetItemWidth(int nWidth);
};

#endif