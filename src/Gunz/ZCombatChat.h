#pragma once

#include "ZInterface.h"

class MEdit;
class MTextArea;

#define ZPLB_ITEM_PICKPLAYER "picked"

class ZTabPlayerList : public MListBox {
public:
	ZTabPlayerList(const char* szName, MWidget* pParent = NULL, MListener* pListener = NULL);
	void SetChatControl(MEdit* pEdit) { m_pEditChat = pEdit; }

	virtual bool OnShow() override final;
	virtual void OnHide() override final;
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override final;

	void OnPickPlayer();

protected:
	MEdit* m_pEditChat{};
};


class ZCombatChat final
{
public:
	ZCombatChat();
	~ZCombatChat();
	bool Create(const char* szOutputTxtarea, bool bUsePlayerList);
	void Destroy();

	void Update();
	void EnableInput(bool bEnable, bool bToTeam=false);
	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);

	void OnDraw(MDrawContext* pDC);
	bool IsChat() const;
	bool IsTeamChat() const;
	bool IsShow() const;
	void SetFont(MFont* pFont);

	void ShowOutput(bool bShow);

	MTextArea*			m_pChattingOutput{};

protected:
	void SetTeamChat(bool bVal) { m_bTeamChat = bVal; }
	void UpdateChattingBox();
	void ProcessChatMsg();

	ZIDLResource*		m_pIDLResource{};
	MEdit*				m_pInputEdit{};
	ZTabPlayerList*		m_pTabPlayerList{};
	bool				m_bChatInputVisible = true;
	u32					m_nLastChattingMsgTime{};
	bool				m_bTeamChat{};
	bool				m_bShowOutput = true;
};