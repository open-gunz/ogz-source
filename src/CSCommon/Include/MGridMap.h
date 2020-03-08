#pragma once

#include <list>
#include <math.h>

template<class _T>
class MGridMap{
public:
	struct MITEM{
		float	x, y, z;
		_T		Obj;
	};
	class MRefCell : public std::list<MITEM>{};

	struct HREF{
		MRefCell*			pRefCell;
		typename MRefCell::iterator	RefIterator;
	};

protected:
	MRefCell*	m_pGridMap;
	float		m_fSX;
	float		m_fSY;
	float		m_fEX;
	float		m_fEY;
	int			m_nXDivision;
	int			m_nYDivision;

protected:
	MRefCell* GetCell(float x, float y){
		int nXPos = int((x-m_fSX)/(GetXSize()/(float)m_nXDivision));
		int nYPos = int((y-m_fSY)/(GetYSize()/(float)m_nYDivision));

		// 영영 검사
		_ASSERT(nXPos>=0 && nYPos>=0);
		if(nXPos>=m_nXDivision) return NULL;
		if(nYPos>=m_nYDivision) return NULL;

		return &(m_pGridMap[nXPos+nYPos*m_nXDivision]);
	}
public:
	MGridMap(){
		m_pGridMap = NULL;
	}
	virtual ~MGridMap(){
		Destroy();
	}

	void Create(float fSX, float fSY, float fEX, float fEY, int nXDivision, int nYDivision){
		m_pGridMap = new MRefCell[nXDivision*nYDivision];
		m_fSX = fSX;
		m_fSY = fSY;
		m_fEX = fEX;
		m_fEY = fEY;
		m_nXDivision = nXDivision;
		m_nYDivision = nYDivision;
	}

	void Destroy(){
		if(m_pGridMap!=NULL){
			delete[] m_pGridMap;
			m_pGridMap = NULL;
		}
	}

	HREF Add(float x, float y, float z, _T Obj){
		_ASSERT(x>=m_fSX && y>=m_fSY);
		_ASSERT(x<=m_fEX && y<=m_fEY);
		HREF hPos;
		MRefCell* pCell = GetCell(x, y);
		if(pCell==NULL){
			hPos.pRefCell = NULL;
			return hPos;
		}
		MITEM item;
		item.x = x;
		item.y = y;
		item.z = z;
		item.Obj = Obj;
		hPos.RefIterator = pCell->insert(pCell->end(), item);
		hPos.pRefCell = pCell;
		return hPos;
	}

	void Del(HREF hRef){
		_ASSERT(hRef.pRefCell!=NULL);
		hRef.pRefCell->erase(hRef.RefIterator);
	}

	void Get(std::list<_T>* pObjs, float x, float y, float z, float fRadius){
		float fXCellSize = GetXSize()/(float)m_nXDivision;
		float fYCellSize = GetYSize()/(float)m_nYDivision;
		int nXPos = int((x-m_fSX)/fXCellSize);
		int nYPos = int((y-m_fSY)/fYCellSize);
#define MORE_SEARCH	2
		int nXRadius = int(fRadius/fXCellSize) + MORE_SEARCH;
		int nYRadius = int(fRadius/fYCellSize) + MORE_SEARCH;
		float fRadiusPow = fRadius*fRadius;
		for(int yp=-nYRadius; yp<=nYRadius; yp++){
			for(int xp=-nXRadius; xp<=nXRadius; xp++){
				float fCellX = (nXPos+xp+(xp>=0?1:0))*fXCellSize + m_fSX;
				float fCellY = (nYPos+yp+(yp>=0?1:0))*fYCellSize + m_fSY;
				float f2DLenPow = float(pow(fCellX-x, 2) + pow(fCellY-y, 2));
				if(f2DLenPow>fRadiusPow) continue;
				int nX = nXPos+xp;
				int nY = nYPos+yp;
				if(nX<0 || nX>=m_nXDivision) continue;
				if(nY<0 || nY>=m_nYDivision) continue;
				MRefCell* pRefCell = &(m_pGridMap[nX+nY*m_nXDivision]);
				for(auto it=pRefCell->begin(); it!=pRefCell->end(); it++){
					MITEM* pItem = &(*it);
					float f3DLenPow = float(pow(pItem->x-x, 2)+pow(pItem->y-y, 2)+pow(pItem->z-z, 2));
					if(f3DLenPow<=fRadiusPow){
						pObjs->insert(pObjs->end(), pItem->Obj);
					}
				}
			}
		}
	}

	HREF Move(float x, float y, float z, _T Obj, HREF& hRef){
		_ASSERT(hRef.pRefCell!=NULL);
		_ASSERT((&(*hRef.RefIterator))->Obj==Obj);

		MRefCell* pRefCell = GetCell(x, y);
		if(pRefCell==hRef.pRefCell) return hRef;
		hRef.pRefCell->erase(hRef.RefIterator);
		return Add(x, y, z, Obj);
	}

	float GetSX() const { return m_fSX; }
	float GetSY() const { return m_fSY; }
	float GetEX() const { return m_fEX; }
	float GetEY() const { return m_fEY; }

	float GetXSize() const { return m_fEX-m_fSX; }
	float GetYSize() const { return m_fEY-m_fSY; }
	int GetXDivision() const { return m_nXDivision; }
	int GetYDivision() const { return m_nYDivision; }

	float GetXDivisionSize() const { return GetXSize() / (float)m_nXDivision; }
	float GetYDivisionSize() const { return GetYSize() / (float)m_nYDivision; }

	MRefCell* GetCell(int x, int y){
		if(x<0 || x>=m_nXDivision) return NULL;
		if(y<0 || y>=m_nYDivision) return NULL;

		return &(m_pGridMap[x+y*m_nXDivision]);
	}

	MRefCell* GetCell(int i){
		if(i<0 || i>=m_nXDivision*m_nYDivision) return NULL;
		return &(m_pGridMap[i]);
	}

	int GetCellCount(){
		return m_nXDivision*m_nYDivision;
	}

	void ClearAllCell(){
		int nCellCount = GetCellCount();
		for(int i=0; i<nCellCount; i++){
			MRefCell* pRefCell = GetCell(i);
			for(auto it=pRefCell->begin(); it!=pRefCell->end(); it++){
				auto pItem = &(*it);
			}
			pRefCell->clear();
		}
	}
};