#ifndef MMASTER_H
#define MMASTER_H

#include "MCommandCommunicator.h"
#include "MServer.h"
#include "MUID.h"

/// 모든 서버를 구성/관리하는 마스터 컨트롤러
/// - UID를 발급/관리한다.
class MMaster : public MServer{
private:
	static MMaster*	m_pInstance;		///< 전역 인스턴스
protected:
	MUIDRefMap	m_GlobalUIDRefMap;	///< 전역적인 Global UID Reference Map
public:
protected:
	/// 새로운 UID 얻어내기
	virtual MUID UseUID(void);

	virtual int OnAccept(MCommObject* pCommObj);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	bool OnCommand(MCommand* pCommand);

	/// Master의 세부 정보를 Target으로 전송
	void ResponseInfo(MUID& Target);
public:
	MMaster(void);
	virtual ~MMaster(void);

	/// 전역 인스턴스 얻기
	static MMaster* GetInstance(void);

	bool Create(int nPort);
	void Destroy(void);
};

#define MASTER_UID	MUID(0, 1)	///< 마스터의 고유 UID(불변)


#endif