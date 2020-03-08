#pragma once

#include "RTypes.h"
#include "ZInterfaceListener.h"

class ZOptionInterface
{

	int		mOldScreenWidth;
	int		mOldScreenHeight;

	RPIXELFORMAT	mnOldBpp;

	bool	mbTimer;
	DWORD	mTimerTime;

	int Sensitivity{};


public:
	ZOptionInterface(void);

	void Update(void);

	void SetListeners();

	void InitInterfaceOption(void);
	bool SaveInterfaceOption(void);

	void OptimizationVideoOption();

	// 리사이즈 관련
	void Resize(int w, int h);		/// 리사이즈를 통해 해상도에 적응하는 인터페이스를 구현한다.
	bool ResizeWidgetRecursive( MWidget* pWidget, int w, int h);
	bool ResizeWidget(const char* szName, int w, int h);

	void ShowResizeConfirmDialog( bool Resized );
	bool IsDiffScreenResolution();
	bool TestScreenResolution();
	void GetOldScreenResolution();
	bool SetTimer( bool b = false,  float time = 0.0f );

	// 네트워크 포트변경
	void ShowNetworkPortConfirmDialog();
	bool IsDiffNetworkPort();

	// 액션키 중복 확인
	void OnActionKeySet(class ZActionKey* pActionKey, int key);
};



///////////////////// 이하 interface listener

// 옵션
DECLARE_LISTENER(ZGetOptionGammaSliderChangeListener)
DECLARE_LISTENER(ZGetLoadDefaultKeySettingListener)
DECLARE_LISTENER(ZSetOptimizationListener)
DECLARE_LISTENER(ZGetSaveOptionButtonListener)
DECLARE_LISTENER(ZGetCancelOptionButtonListener)
DECLARE_LISTENER(ZGetGammaOption)
DECLARE_LISTENER(ZGetCancelResizeConfirmListener)
DECLARE_LISTENER(ZGetRequestResizeListener)
DECLARE_LISTENER(ZGetNetworkPortChangeRestartListener)
DECLARE_LISTENER(ZGetNetworkPortChangeCancelListener)
DECLARE_LISTENER(ZGetViewConfirmCancelListener)
DECLARE_LISTENER(ZGetViewConfrimAcceptListener)
DECLARE_LISTENER(ZGet8BitSoundListener)
DECLARE_LISTENER(ZGet16BitSoundListener)
DECLARE_LISTENER(ZGetBGMVolumeSizeSliderListener)
DECLARE_LISTENER(ZGetEffectVolumeSizeSliderListener)
