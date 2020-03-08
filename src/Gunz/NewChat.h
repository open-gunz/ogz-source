#pragma once
#include <vector>
#include <string>
#include <memory>
#include "MUtil.h"

constexpr u32 CHAT_DEFAULT_TEXT_COLOR = XRGB(0xC8, 0xC8, 0xC8);
constexpr u32 CHAT_DEFAULT_INTERFACE_COLOR = XRGB(0, 0xA5, 0xC3);
constexpr u32 CHAT_DEFAULT_BACKGROUND_COLOR = 0;
constexpr u32 CHAT_DEFAULT_SELECTION_COLOR = ARGB(0xA0, 0, 0x80, 0xFF);

struct v2i
{
	int x, y;
};

enum class ChatWindowAction {
	None,
	Moving,
	Resizing,
	Selecting,
};

class Chat
{
public:
	Chat(const std::string& FontName, bool Bold, int FontSize);
	~Chat();

	void EnableInput(bool Enable, bool ToTeam);
	void OutputChatMsg(const char *Msg);
	void OutputChatMsg(const char *Msg, u32 Color);

	void OnUpdate(float TimeDelta);
	void OnDraw(MDrawContext* pDC);
	bool OnEvent(MEvent* pEvent);

	void Scale(double WidthRatio, double HeightRatio);
	void Resize(int Width, int Height);

	void ClearHistory();

	const D3DRECT &GetRect() const { return Border; }
	void SetRect(D3DRECT &r) { Border = r; }
	void SetRect(int x1, int y1, int x2, int y2) { Border.x1 = x1; Border.y1 = y1; Border.x2 = x2; Border.y2 = y2; }

	auto GetFadeTime() const { return FadeTime; }
	void SetFadeTime(float fFade) { FadeTime = fFade; }

	const std::string &GetFont() const { return FontName; }
	int GetFontSize() const { return FontSize; }
	void SetFont(std::string s, bool b) { FontName = std::move(s); BoldFont = b; ResetFonts(); }
	void SetFontSize(int nSize) { FontSize = nSize; ResetFonts(); }

	D3DCOLOR GetTextColor() const { return TextColor; }
	D3DCOLOR GetInterfaceColor() const { return InterfaceColor; }
	D3DCOLOR GetBackgroundColor() const { return BackgroundColor; }
	void SetTextColor(D3DCOLOR Color) { TextColor = Color; }
	void SetBackgroundColor(D3DCOLOR Color) { BackgroundColor = Color; }
	void SetInterfaceColor(D3DCOLOR Color) { InterfaceColor = Color; }

	using TimeType = u64;
	static TimeType GetTime();
	static TimeType Seconds(float Value) { return TimeType(Value * 1000); }

	bool IsInputEnabled() const { return InputEnabled; }
	bool IsTeamChat() const { return TeamChat; }

	// If HideAlways is true, or HideDuringReplays is true during a replay, the chat will be hidden.
	// They're controlled by ZACTION_TOGGLE_CHAT and the T or H key during replays, respectively.
	bool HideAlways{};
	bool HideDuringReplays{};

private:
	std::string FontName = "Arial";
	bool BoldFont = true;
	int FontSize = 16;
	int FontHeight{};
	TimeType FadeTime = Seconds(10);
	std::vector<struct ChatMessage> Msgs;
	std::vector<struct LineSegmentInfo> LineSegments;
	bool InputEnabled{};
	bool TeamChat{};
	MPOINT Cursor{};
	D3DRECT Border{};
	MFontR2 DefaultFont;
	MFontR2 ItalicFont;
	// The normal font is already bold right now.
	//MFontR2 BoldFont;
	//MFontR2 BoldItalicFont;
	u32 TextColor = CHAT_DEFAULT_TEXT_COLOR;
	u32 InterfaceColor = CHAT_DEFAULT_INTERFACE_COLOR;
	u32 BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
	u32 SelectionColor = CHAT_DEFAULT_SELECTION_COLOR;
	ChatWindowAction Action = ChatWindowAction::None;
	u32 ResizeFlags{};
	struct SelectionStateType {
		const ChatMessage *FromMsg;
		int FromPos;
		const ChatMessage *ToMsg;
		int ToPos;
	} SelectionState{};
	std::vector<std::wstring> InputHistory;
	int CurInputHistoryEntry{};
	std::wstring InputField;
	int CaretPos = -1;
	/*bool bPlayerList;
	ID3DXLine *pPlayerListLine;
	std::vector<std::string> vstrPlayerList;
	int nCurPlayer;
	int nPlayerListWidth;*/
	int InputHeight{};
	v2i CaretCoord{};

	// Number of newly added lines.
	// These have a gradual, animated entry into the chat box.
	// Once a message is fully visible, it's not longer newly added
	// and this value will get decremented.
	int NumNewlyAddedLines{};
	// The vertical offset when rendering messages, due to
	// the animation of new messages coming in.
	float ChatLinesPixelOffsetY{};

	void UpdateNewMessagesAnimation(float TimeDelta);

	void DrawBorder(MDrawContext* pDC);
	void DrawBackground(MDrawContext* pDC, TimeType Time, int Limit, bool ShowAll);
	void DrawChatLines(MDrawContext* pDC, TimeType Time, int Limit, bool ShowAll);
	void DrawSelection(MDrawContext* pDC);
	void DrawFrame(MDrawContext* pDC, TimeType Time);
	MFontR2& GetFont(u32 Emphasis);

	D3DRECT GetOutputRect();
	D3DRECT GetInputRect();
	D3DRECT GetTotalRect();
	template <typename T>
	void DivideIntoLines(int ChatLineIndex, T&& OutputIterator);
	std::pair<bool, v2i> GetPos(const ChatMessage &cl, u32 nPos);
	bool CursorInRange(int x, int y, int Width, int Height);
	int GetTextLen(ChatMessage &cl, int Pos, int Count);
	int GetTextLen(const char *Msg, int Count);

	int GetTextLength(MFontR2& Font, const wchar_t* Format, ...);
	int DrawTextWordWrap(MFontR2& Font, const WStringView& Str, const D3DRECT &r, u32 Color);
	void DrawTextN(MFontR2& Font, const WStringView& Str, const D3DRECT &r, u32 Color);

	void ResetFonts();
};