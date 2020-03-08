#pragma once

#include <map>

#include "MTypes.h"
#include "MEvent.h"

class MDrawContext;
class MWidget;
class MFont;
class MBitmap;
class MResourceMap;
class MIDLResource;
class MListener;
class MEvent; 

#define MINT_VERSION	2
#define	MVersion()	MINT_VERSION

typedef bool(MGLOBALEVENTCALLBACK)(MEvent* pEvent);

class Mint{
protected:
	static Mint*			m_pInstance;
	MWidget*				m_pMainFrame;
	MDrawContext*			m_pDC;
	MGLOBALEVENTCALLBACK*	m_fnGlobalEventCallBack;

	// Drag & Drop
	char		m_szDragObjectString[256];
	char		m_szDragObjectItemString[256];
	MBitmap*	m_pDragObjectBitmap;
	MPOINT		m_GrabPoint;
	bool		m_bVisibleDragObject;
	MWidget*	m_pDropableObject;
	MWidget*	m_pDragSourceObject;

	// Workspace Size
	int			m_nWorkspaceWidth;
	int			m_nWorkspaceHeight;
	bool		Stretch = true;

	// IME
	void*	m_pCandidateList;
	int		m_nCandidateListSize;
	MPOINT	m_CandidateListPos;

	bool	m_bEnableIME;

public:
	u32		m_nCompositionAttributeSize;
	u8		m_nCompositionAttributes[MIMECOMPOSITIONSTRING_LENGTH];
	int		m_nCompositionCaretPosition;

protected:
	virtual void DrawCandidateList(MDrawContext* pDC, MPOINT& p);

public:
	Mint();
	virtual ~Mint();

	bool Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont);
	void Finalize();

#ifdef WIN32
	bool ProcessEvent(struct HWND__* hwnd, u32 message, u32 wparam, u32 lparam);
#endif

	virtual void Run();
	virtual void Draw();
	virtual void Update(){}
	MWidget* GetMainFrame();
	MDrawContext* GetDrawContext();
	static Mint* GetInstance();

#ifdef WIN32
	void SetHWND(struct HWND__* hWnd);
	struct HWND__* GetHWND();

	struct HIMC__* m_hImc; // IME Composition handle
#endif

	void EnableIME(bool bEnable);
	bool IsEnableIME();

	int RegisterHotKey(u32 nModifier, u32 nVirtKey);
	void UnregisterHotKey(int nID);

	MWidget* SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	MWidget* GetDragObject();

	virtual MWidget* NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener);

	MWidget* FindWidgetDropAble(MPOINT& p);
	MWidget* FindWidget(MPOINT& p);
	MWidget* FindWidget(int x, int y);

	int GetWorkspaceWidth();
	int GetWorkspaceHeight();
	void SetWorkspaceSize(int w, int h);

	auto GetStretch() const { return Stretch; }
	void SetStretch(bool b) { Stretch = b; }

	virtual MBitmap* OpenBitmap(const char* szName) = 0;
	virtual MFont* OpenFont(const char* szName, int nHeight) = 0;

	void SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback);

	const char* GetDefaultFontName() const;

	int GetPrimaryLanguageIdentifier() const;

	int GetSubLanguageIdentifier() const;

	const char* GetLanguageIndicatorString() const;

	bool IsNativeIME() const;

	void OpenCandidateList();
	void CloseCandidateList();

	const char* GetCandidate(int nIndex) const;
	int GetCandidateCount() const;
	int GetCandidateSelection() const;
	int GetCandidatePageStart() const;
	int GetCandidatePageSize() const;

	void SetCandidateListPosition(MPOINT& p, int nWidgetHeight);
	int GetCandidateListWidth();
	int GetCandidateListHeight();

	u32 GetCompositionAttributeSize() const { return m_nCompositionAttributeSize; }
	const u8* GetCompositionAttributes() const { return m_nCompositionAttributes; }

	int DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i);
	void DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition);
	void DrawIndicator(MDrawContext* pDC, MRECT& r);
};

inline int MGetWorkspaceWidth(){
	return Mint::GetInstance()->GetWorkspaceWidth();
}
inline int MGetWorkspaceHeight(){
	return Mint::GetInstance()->GetWorkspaceHeight();
}

inline int MGetCorrectedWorkspaceWidth() {
	if (Mint::GetInstance()->GetStretch())
		return MGetWorkspaceWidth();

	auto Aspect = static_cast<float>(MGetWorkspaceWidth()) / MGetWorkspaceHeight();
	return static_cast<int>(MGetWorkspaceWidth() / Aspect * (4.f / 3.f));
}

void MCreateSample();
void MDestroySample();

#define MMODIFIER_ALT	1
#define MMODIFIER_CTRL	2
#define MMODIFIER_SHIFT	4

#define MIsActionKeyPressed(_ActionID)	(Mint::GetInstance()->IsActionKeyPressed(_ActionID))

#define IsHangul(x) ((unsigned char)(x)>127)