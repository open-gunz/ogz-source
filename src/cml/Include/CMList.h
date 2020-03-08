//	CMList.h
//		Linked List
//		이장호 ( 97-12-30 4:15:03 오후 )
//////////////////////////////////////////////////////////////////////

#ifndef LIST_H
#define LIST_H

#ifndef _ASSERT
#include <cassert>
#define _ASSERT assert
#endif

// 리스트를 구성할 레코드
template<class NEW_TYPE>
class CMRecord{
	CMRecord<NEW_TYPE>*	m_lpPrevious;
	CMRecord<NEW_TYPE>*	m_lpNext;
public:
	NEW_TYPE*			m_lpContent;

	// 멤버 초기화
	CMRecord(void){
		m_lpNext		=NULL;
		m_lpPrevious	=NULL;
		m_lpContent		=NULL;
	}
	// 이전 노드 세팅
	void SetPreviousPointer(CMRecord<NEW_TYPE>* pprevious){
		m_lpPrevious=pprevious;
	}
	// 이전 노드 얻기
	CMRecord<NEW_TYPE>* GetPreviousPointer(void){
		return m_lpPrevious;
	}
	// 다음 노드 세팅
	void SetNextPointer(CMRecord<NEW_TYPE>* pnext){
		m_lpNext=pnext;
	}
	// 다음 노드 얻기
	CMRecord<NEW_TYPE>* GetNextPointer(void){
		return m_lpNext;
	}
	// 내용 얻기
	NEW_TYPE *Get(void){
		return m_lpContent;
	}
	/*
	레퍼런스는 모호함을 유발함으로, 사용하지 않는다.
	NEW_TYPE &Get(void){
		return *m_lpContent;
	}
	*/
};

// 리스트 클래스
template<class NEW_TYPE>
class CMLinkedList{
private:
	CMRecord<NEW_TYPE>*		m_lpFirstRecord;	// 첫번째 레코드
	CMRecord<NEW_TYPE>*		m_lpLastRecord;		// 첫번째 레코드
	CMRecord<NEW_TYPE>*		m_lpCurrentRecord;	// 현재 레코드
	int						m_nListCount;		// 레코드의 개수
	int						m_nCurrentPosition;	// 현재 위치

	//MString					m_lpszErrorString;	// 에러 스트링
public:
	// 변수 초기화
	CMLinkedList(void);
	// 리스트 해제(알아서 레코드를 모두 삭제한다)
	virtual ~CMLinkedList(void);

	// 에러 메세지 얻기
	//MString GetErrorMessage(void){return m_lpszErrorString;}
	
	// 맨 앞에 레코드 삽입(new로 생성된 객체가 그대로 박힌다.)
	bool InsertHead(NEW_TYPE *lpRecord);
	// 현재 위치 다음에 레코드 삽입(new로 생성된 객체가 그대로 박힌다.)
	bool AddAfter(NEW_TYPE *lpRecord);
	// 현재 위치 다음에 레코드 삽입(new로 생성된 객체가 그대로 박힌다.)
	bool Insert(NEW_TYPE *lpRecord);
	// 맨 뒤에 레코드 추가(new로 생성된 객체가 그대로 박힌다.)
	bool Add(NEW_TYPE *lpRecord);
	
	// 현재 위치의 레코드를 삭제
	void Delete(void);
	// 지정된 위치의 레코드를 삭제
	void Delete(int iindex);
	// 모든 레코드 삭제
	void DeleteAll(void);

	// Added by Kim young ho
	// 현재 위치의 레코드를 삭제, but Record의 content는 삭제하지 않음 
	void DeleteRecord(void);
	// 지정된 위치의 레코드를 삭제, but Record의 content는 삭제하지 않음 
	void DeleteRecord(int iindex);
	// 모든 레코드 삭제, but Record의 content는 삭제하지 않음 
	void DeleteRecordAll(void);

	// 레코드의 개수 얻어내기
	int GetCount(void){return m_nListCount;}
	// 레코드의 현재 위치 얻어내기
	int GetIndex(void){return m_nCurrentPosition;}

	////////////////////////////////////////////
	// 리스트내의 포인터를 이용한 이동및 값 얻기
	// 이전 레코드로
	bool PreviousRecord(void);
	// 다음 레코드로
	bool NextRecord(void);
	// 지정된 위치로
	void MoveRecord(int iindex);
	// 맨 처음으로
	void MoveFirst(void);
	// 맨 마지막으로
	void MoveLast(void);

	// 현재 레코드의 내용을 얻어낸다.
	NEW_TYPE *Get(void);

	// 지정된 인덱스의 값을 얻어낸다.
	NEW_TYPE *Get(int iindex);

	// 지정된 인덱스의 값을 얻어낸다.
	// 레퍼런스에 의한 모호함으로 사용하지 않는다.
	//NEW_TYPE &operator[](int iindex);

	// 두개의 레코드의 값을 Swap한다.
	void Swap(int nIndex1,int nIndex2);

	// Record Compare Function. Sort를 위해 virtual로 계승받아야 한다.
	virtual int Compare(NEW_TYPE *lpRecord1,NEW_TYPE *lpRecord2){return -10;/*false*/}

	// Quick Sort를 실행한다.
	void Sort(void);
private:
	// Quick Sort의 서브 함수
	void QuickSort(int first,int last);
};

// 변수 초기화
template<class NEW_TYPE>
CMLinkedList<NEW_TYPE>::CMLinkedList(void)
{
	m_lpFirstRecord=NULL;
	m_lpLastRecord=NULL;
	m_lpCurrentRecord=NULL;
	m_nListCount=0;
	m_nCurrentPosition=0;
}

// 리스트 해제(알아서 레코드를 모두 삭제한다)
template<class NEW_TYPE>
CMLinkedList<NEW_TYPE>::~CMLinkedList(void)
{
	DeleteAll();

	m_lpFirstRecord=NULL;
	m_lpLastRecord=NULL;
	m_lpCurrentRecord=NULL;
	m_nListCount=0;
	m_nCurrentPosition=0;
}

// 레코드 삽입
template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::AddAfter(NEW_TYPE *lpRecord)
{
	_ASSERT(lpRecord!=NULL);

	// 리스트의 첫 레코드이면
	if(m_nListCount==0){
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL){
			//m_lpszErrorString="CMLinkedList::Insert() - Memory Allocation Error";
			return false;
		}
		// 단순히 데이타 포인터 대입. 데이타는 기존에 동적으로 할당되어 있어야 한다.
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		
		m_lpFirstRecord=ptemp;
		m_lpLastRecord=ptemp;
		m_lpCurrentRecord=m_lpFirstRecord;
	}
	else{
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL)return false;
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		// 현재 레코드과 다음 레코드 사이에 삽입
		// 새 레코드
		ptemp->SetPreviousPointer(m_lpCurrentRecord);
		ptemp->SetNextPointer(m_lpCurrentRecord->GetNextPointer());
		// 다음 레코드
		if((ptemp->GetNextPointer()!=NULL))
			(ptemp->GetNextPointer())->SetPreviousPointer(ptemp);
		else{
			// 마지막에 추가하는 것이므로
			m_lpLastRecord=ptemp;
		}

		// 현재(앞) 레코드
		m_lpCurrentRecord->SetNextPointer(ptemp);

		// 현재 포인터를 새로 생성한 레코드으로 대치
		m_lpCurrentRecord=ptemp;

		m_nCurrentPosition++;
	}

	// 리스트 카운트 증가
	m_nListCount++;
	return true;
}

// 레코드 삽입
template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::Insert(NEW_TYPE *lpRecord)
{
	_ASSERT(lpRecord!=NULL);

	// 리스트의 첫 레코드이면
	if(m_nListCount==0){
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL){
			//m_lpszErrorString="CMLinkedList::Insert() - Memory Allocation Error";
			return false;
		}
		// 단순히 데이타 포인터 대입. 데이타는 기존에 동적으로 할당되어 있어야 한다.
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		
		m_lpFirstRecord=ptemp;
		m_lpLastRecord=ptemp;
		m_lpCurrentRecord=m_lpFirstRecord;
	}
	else{
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL)return false;
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		// 현재 레코드과 다음 레코드 사이에 삽입
		// 새 레코드
		ptemp->SetPreviousPointer(m_lpCurrentRecord);
		ptemp->SetNextPointer(m_lpCurrentRecord->GetNextPointer());
		// 다음 레코드
		if((ptemp->GetNextPointer()!=NULL))
			(ptemp->GetNextPointer())->SetPreviousPointer(ptemp);
		else{
			// 마지막에 추가하는 것이므로
			m_lpLastRecord=ptemp;
		}

		// 현재(앞) 레코드
		m_lpCurrentRecord->SetNextPointer(ptemp);

		// 현재 포인터를 새로 생성한 레코드으로 대치
		m_lpCurrentRecord=ptemp;

		m_nCurrentPosition++;
	}

	// 리스트 카운트 증가
	m_nListCount++;
	return true;
}

template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::InsertHead(NEW_TYPE *lpRecord)
{
	_ASSERT(lpRecord!=NULL);

	MoveFirst();

	// 리스트의 첫 레코드이면
	if(m_nListCount==0){
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL){
			//m_lpszErrorString="CMLinkedList::Insert() - Memory Allocation Error";
			return false;
		}
		// 단순히 데이타 포인터 대입. 데이타는 기존에 동적으로 할당되어 있어야 한다.
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		
		m_lpFirstRecord=ptemp;
		m_lpLastRecord=ptemp;
		m_lpCurrentRecord=m_lpFirstRecord;
	}
	else{
		CMRecord<NEW_TYPE> *ptemp;
		ptemp=new CMRecord<NEW_TYPE>;
		if(ptemp==NULL)return false;
		ptemp->m_lpContent=lpRecord;
		//memcpy(&(ptemp->m_lpContent),lpRecord,sizeof(NEW_TYPE));
		// 현재 레코드과 다음 레코드 사이에 삽입
		// 새 레코드
		ptemp->SetPreviousPointer(NULL);
		ptemp->SetNextPointer(m_lpCurrentRecord);
		// 다음 레코드
		if((ptemp->GetNextPointer()!=NULL))
			(ptemp->GetNextPointer())->SetPreviousPointer(ptemp);
		else{
			// 마지막에 추가하는 것이므로
			m_lpLastRecord=ptemp;
		}

		m_lpFirstRecord=ptemp;

		// 현재 포인터를 새로 생성한 레코드으로 대치
		m_lpCurrentRecord=ptemp;

		//m_nCurrentPosition++;
	}

	// 리스트 카운트 증가
	m_nListCount++;
	return true;
}

// 레코드 추가
template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::Add(NEW_TYPE *lpRecord)
{
	MoveLast();
	return Insert(lpRecord);
}

// 현재 위치의 레코드를 삭제
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::Delete(void)
{
	// 현재 레코드의 포인터는 NULL일수 없다.
	_ASSERT(m_lpCurrentRecord!=NULL);

	CMRecord<NEW_TYPE>* pprevious;
	CMRecord<NEW_TYPE>* pnext;

	pprevious	=m_lpCurrentRecord->GetPreviousPointer();
	pnext		=m_lpCurrentRecord->GetNextPointer();

	// 앞 레코드
	if(pprevious!=NULL){
	// 앞 레코드가 처음이 아니면...
		pprevious->SetNextPointer(pnext);
	}
	else{
	// 앞 레코드가 첫 레코드일 경우
		m_lpFirstRecord=pnext;
	}
	
	// 뒤 레코드
	if(pnext!=NULL){
	// 이 레코드가 마지막이 아니면...
		pnext->SetPreviousPointer(pprevious);
	}
	else{
	// 뒤 레코드가 마지막이면
		m_lpLastRecord=pprevious;
	}

	// 현재 레코드의 내용 메모리에서 삭제
	_ASSERT(m_lpCurrentRecord->m_lpContent!=NULL);
	delete m_lpCurrentRecord->m_lpContent;
	// 현재 레코드 메모리에서 삭제
	delete m_lpCurrentRecord;

	// 현재 포인터 지정
	if(pnext!=NULL)
		m_lpCurrentRecord=pnext;
	else{
		if(pprevious!=NULL){
			m_lpCurrentRecord=pprevious;
			m_nCurrentPosition--;
			_ASSERT(m_nCurrentPosition>=0);
		}
		else
			m_lpCurrentRecord=NULL;
	}

	// 리스트 카운트 감소
	m_nListCount--;
}

// 원하는 인덱스를 지움
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::Delete(int iindex)
{
	if(iindex<0 || iindex>=m_nListCount) {
		_ASSERT(false);
		return ;
	}

	MoveRecord(iindex);
	Delete();
}

// 모든 레코드 삭제
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::DeleteAll(void)
{
	while(m_nListCount!=0)
		Delete();
	m_nCurrentPosition=0;
}

// 현재 위치의 레코드를 삭제, but Record의 content는 삭제하지 않음  
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::DeleteRecord(void)
{
	// 현재 레코드의 포인터는 NULL일수 없다.
	_ASSERT(m_lpCurrentRecord!=NULL);

	CMRecord<NEW_TYPE>* pprevious;
	CMRecord<NEW_TYPE>* pnext;

	pprevious	=m_lpCurrentRecord->GetPreviousPointer();
	pnext		=m_lpCurrentRecord->GetNextPointer();

	// 앞 레코드
	if(pprevious!=NULL){
	// 앞 레코드가 처음이 아니면...
		pprevious->SetNextPointer(pnext);
	}
	else{
	// 앞 레코드가 첫 레코드일 경우
		m_lpFirstRecord=pnext;
	}
	
	// 뒤 레코드
	if(pnext!=NULL){
	// 이 레코드가 마지막이 아니면...
		pnext->SetPreviousPointer(pprevious);
	}
	else{
	// 뒤 레코드가 마지막이면
		m_lpLastRecord=pprevious;
	}

	// 현재 레코드 메모리에서 삭제
	delete m_lpCurrentRecord;

	// 현재 포인터 지정
	if(pnext!=NULL)
		m_lpCurrentRecord=pnext;
	else{
		if(pprevious!=NULL){
			m_lpCurrentRecord=pprevious;
			m_nCurrentPosition--;
			_ASSERT(m_nCurrentPosition>=0);
		}
		else
			m_lpCurrentRecord=NULL;
	}

	// 리스트 카운트 감소
	m_nListCount--;
}

// 원하는 인덱스를 지움, but Record의 content?삭제하지 않음 
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::DeleteRecord(int iindex)
{
	if(iindex<0 || iindex>=m_nListCount) {
		_ASSERT(false);
		return;
	}

	MoveRecord(iindex);
	DeleteRecord();
}

// 모든 레코드 삭제, but Record의 content는 삭제하지 않음 
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::DeleteRecordAll(void)
{
	while(m_nListCount!=0)
		DeleteRecord();
	m_nCurrentPosition=0;
}


// 이전 레코드로
template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::PreviousRecord(void)
{
	// 현재 레코드의 포인터는 NULL일수 없다.
	// 즉, 아무것도 저장되어 있지 않은 상태
	_ASSERT(m_lpCurrentRecord!=NULL);

	if((m_lpCurrentRecord->GetPreviousPointer())!=NULL){
		m_nCurrentPosition--;
		_ASSERT(m_nCurrentPosition>=0);

		m_lpCurrentRecord=m_lpCurrentRecord->GetPreviousPointer();
		return true;
	}	
	else{
		//m_lpszErrorString="CMLinkedList::PreviousRecord() - Out of Range";
		return false;
	}
}

// 다음 레코드로
template<class NEW_TYPE>
bool CMLinkedList<NEW_TYPE>::NextRecord(void)
{
	// 현재 레코드의 포인터는 NULL일수 없다.
	// 즉, 아무것도 저장되어 있지 않은 상태
	_ASSERT(m_lpCurrentRecord!=NULL);

	if((m_lpCurrentRecord->GetNextPointer())!=NULL){
		m_nCurrentPosition++;
		_ASSERT(m_nCurrentPosition<m_nListCount);

		m_lpCurrentRecord=m_lpCurrentRecord->GetNextPointer();
		return true;
	}	
	else{
		//m_lpszErrorString="CMLinkedList::NextRecord() - Out of Range";
		return false;
	}
}

// 지정된 위치로
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::MoveRecord(int iindex)
{
	// 인덱스는 지정된 범위안에 있어야 한다.
	if(iindex<0 || iindex>=m_nListCount) {
		_ASSERT(false);
		return;
	}
	_ASSERT(iindex>=0);
	if(iindex==0)
	{	
		MoveFirst();
		return;
	}
	if(iindex>m_nCurrentPosition){
		while(iindex!=m_nCurrentPosition)
			NextRecord();
	}
	else if(iindex<m_nCurrentPosition){
		while(iindex!=m_nCurrentPosition)
			PreviousRecord();
	}
}

// 맨 처음으로
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::MoveFirst(void)
{
	m_nCurrentPosition=0;
	m_lpCurrentRecord=m_lpFirstRecord;
}

// 맨 마지막으로
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::MoveLast(void)
{
	if(m_nListCount>0){
		m_nCurrentPosition=m_nListCount-1;
		m_lpCurrentRecord=m_lpLastRecord;
	}
}

// 현재 레코드의 내용을 얻어낸다.
template<class NEW_TYPE>
NEW_TYPE *CMLinkedList<NEW_TYPE>::Get(void)
{
	// 현재 레코드의 포인터는 NULL일수 없다.
	// 즉, 아무것도 저장되어 있지 않은 상태
	_ASSERT(m_lpCurrentRecord!=NULL);

	return(m_lpCurrentRecord->Get());
}

// 지정된 인덱스의 값을 얻어낸다.
template<class NEW_TYPE>
NEW_TYPE *CMLinkedList<NEW_TYPE>::Get(int iindex)
{
	if(iindex<0 || iindex>=m_nListCount) {
		_ASSERT(false);
		return NULL;
	}

	MoveRecord(iindex);
	return Get();
}

// 지정된 인덱스의 값을 얻어낸다.
/*
template<class NEW_TYPE>
NEW_TYPE &CMLinkedList<NEW_TYPE>::operator[](int iindex)
{
	MoveRecord(iindex);
	return Get();
}
*/

// Quick Sort를 실행한다.
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::Sort(void)
{
	if(GetCount()<=1)return;
	QuickSort(0,GetCount()-1);
}

// Quick Sort의 서브 함수
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::QuickSort(int first,int last)
{
	int i,j;

	i=first,j=last;
	NEW_TYPE *pMiddle=Get((first+last)/2);

	// Compare()를 virtual로 계승받지 않으면 Assetion을 발생시킴
	_ASSERT(Compare(Get(i),pMiddle)!=-10);

	for(;;){
		while(Compare(Get(i),pMiddle)<0)i++;
		while(Compare(Get(j),pMiddle)>0)j--;
		if(i>=j)break;
		Swap(i,j);
		i++;j--;
	}
	if(first<i-1)QuickSort(first,i-1);
	if(j+1<last)QuickSort(j+1,last);
}

// 두개의 레코드의 값을 Swap한다.
template<class NEW_TYPE>
void CMLinkedList<NEW_TYPE>::Swap(int nIndex1,int nIndex2)
{
	if(nIndex1<0 || nIndex1>=m_nListCount) {
		_ASSERT(false);
		return;
	}

	MoveRecord(nIndex1);
	CMRecord<NEW_TYPE>* pRecord1=m_lpCurrentRecord;


	if(nIndex2<0 || nIndex2>=m_nListCount) {
		_ASSERT(false);
		return;
	}

	MoveRecord(nIndex2);
	CMRecord<NEW_TYPE>* pRecord2=m_lpCurrentRecord;

	NEW_TYPE *pData=pRecord1->m_lpContent;
	pRecord1->m_lpContent=pRecord2->m_lpContent;
	pRecord2->m_lpContent=pData;
}

#endif
