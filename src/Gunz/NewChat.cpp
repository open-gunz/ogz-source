#include "stdafx.h"
#include "NewChat.h"
#include "RGMain.h"
#include "ZCharacterManager.h"
#include "ZInput.h"
#include "Config.h"
#include "defer.h"
#include "MClipboard.h"
#include "CodePageConversion.h"

namespace ResizeFlagsType
{
enum
{
	X1 = 1 << 0,
	Y1 = 1 << 1,
	X2 = 1 << 2,
	Y2 = 1 << 3,
};
}

// Note that while Wrap and Linebreak both act as linebreaks,
// the former is placed by the line-wrapping mechanism and
// the latter is explicitly placed by the message creator.
enum class FormatSpecifierType {
	Unknown = -1,
	Wrap,
	Linebreak,
	Color,
	Default,
	Bold,
	Italic,
	BoldItalic,
	Underline,
	Strikethrough,
};

struct FormatSpecifier {
	int nStartPos;
	FormatSpecifierType ft;
	D3DCOLOR Color;

	FormatSpecifier(int nStart, D3DCOLOR c) : nStartPos(nStart), ft(FormatSpecifierType::Color), Color(c) { }
	FormatSpecifier(int nStart, FormatSpecifierType type) : nStartPos(nStart), ft(type) { }
};


struct ChatMessage {
	Chat::TimeType Time{};
	std::wstring Msg;
	u32 DefaultColor;
	std::vector<FormatSpecifier> FormatSpecifiers;
	int Lines{};

	void SubstituteFormatSpecifiers();

	int GetLines() const {
		return Lines;
	}

	void ClearWrappingLineBreaks() {
		erase_remove_if(FormatSpecifiers, [&](auto&& x) { return x.ft == FormatSpecifierType::Wrap; });
	}

	const FormatSpecifier *GetLineBreak(int n) const {
		int i = 0;
		for (auto it = FormatSpecifiers.begin(); it != FormatSpecifiers.end(); it++) {
			if (it->ft == FormatSpecifierType::Wrap || it->ft == FormatSpecifierType::Linebreak) {
				if (i == n)
					return &*it;

				i++;
			}
		}

		return 0;
	}

	// Returns an iterator to the format specifier that was inserted.
	auto AddWrappingLineBreak(int n) {
		assert(n >= 0);
		if (n < 0)
			n = 0;

		if (FormatSpecifiers.empty()) {
			FormatSpecifiers.emplace_back(n, FormatSpecifierType::Wrap);
			// Return the last iterator, since we appended to the end.
			return std::prev(FormatSpecifiers.end());
		}

		for (auto it = FormatSpecifiers.rbegin(); it != FormatSpecifiers.rend(); it++) {
			if (it->nStartPos < n) {
				// it.base() is AFTER it (in terms of normal order, not reversed),
				// and insert inserts BEFORE the passed iterator, so this inserts
				// after the current iterator, which is correct since the
				// desired index n is after the current format specifier.
				return FormatSpecifiers.insert(it.base(), FormatSpecifier(n, FormatSpecifierType::Wrap));
			}
		}

		// The loop was unable to find a format specifier that precedes
		// the position, so we must add it at the start.
		return FormatSpecifiers.insert(FormatSpecifiers.begin(), FormatSpecifier(n, FormatSpecifierType::Wrap));
	}
};

namespace EmphasisType
{
enum
{
	Default = 0,
	Italic = 1 << 0,
	Bold = 1 << 1,
	Underline = 1 << 2,
	Strikethrough = 1 << 3,
};
}

// A substring of a line to be displayed.
// The substring may be the entire line, but cannot span more than one line.
struct LineSegmentInfo
{
	// Index into Chat::vMsgs.
	int ChatMessageIndex;
	// Offset into ChatMessage::Msg at which the substring to be displayed begins.
	u16 Offset;
	// Length of the substring, in characters.
	u16 LengthInCharacters;
	// Pixel offset on the X axis at which this segment starts.
	u16 PixelOffsetX;
	struct {
		// Is this the start of the line?
		u16 IsStartOfLine : 1;
		// Emphasis, i.e. italic, bold, etc.
		// This is a bitmask; it can hold a combination of multiple emphases.
		u16 Emphasis : 15;
	};
	u32 TextColor;
};

// Stuff crashes if this is increased
static constexpr int MAX_INPUT_LENGTH = 230;

void ChatMessage::SubstituteFormatSpecifiers()
{
	// TODO: Properly handle multiple emphases at once, e.g. both italic and underlined,
	// and remove only one when one is ended.

	auto CharToFT = [&](char c) {
		switch (c) {
		case 'b': return FormatSpecifierType::Bold;
		case 'i': return FormatSpecifierType::Italic;
		case 's': return FormatSpecifierType::Strikethrough;
		case 'u': return FormatSpecifierType::Underline;
		//case 'n': return FormatSpecifierType::Linebreak;
		default:  return FormatSpecifierType::Unknown;
		};
	};

	const auto npos = std::wstring::npos;

	bool Erased = false;

	for (auto Pos = Msg.find_first_of(L"^[", 0);
		Pos != npos && Pos <= Msg.length() - 2;
		Pos = Pos < Msg.length() ? Msg.find_first_of(L"^[", Erased ? Pos : Pos + 1) : npos)
	{
		Erased = false;

		auto Erase = [&](std::wstring::size_type Count) {
			Msg.erase(Pos, Count);

			Erased = true;
		};

		auto RemainingLength = Msg.length() - Pos;
		auto CurrentChar = Msg[Pos];

		if (CurrentChar == '^')
		{
			// Handles color specifiers, like "Normal text ^1Red text"
			auto NextChar = Msg[Pos + 1];
			if (isdigit(NextChar))
			{
				// Simple specifier, e.g. "^1Red text"

				FormatSpecifiers.emplace_back(Pos, MMColorSet[NextChar - '0']);
				Erase(2);
			}
			else if (NextChar == '#')
			{
				// Elaborate specifier, e.g. "^#80FF0000Transparent red text"

				auto ishexdigit = [&](auto c) {
					c = tolower(c);
					return isdigit(c) || (c >= 'a' && c <= 'f');
				};

				auto ColorStart = Pos + 2;
				auto ColorEnd = ColorStart;
				while (ColorEnd < Msg.length() &&
					ColorEnd - ColorStart < 8 &&
					ishexdigit(Msg[ColorEnd])) {
					++ColorEnd;
				}

				auto Distance = ColorEnd - ColorStart;

				// Must be 8 digits
				if (Distance != 8)
					continue;

				wchar_t ColorString[32];
				strncpy_safe(ColorString, &Msg[ColorStart], Distance);

				wchar_t* endptr;
				auto Color = static_cast<D3DCOLOR>(wcstoul(ColorString, &endptr, 16));
				assert(endptr == ColorString + Distance);

				FormatSpecifiers.emplace_back(Pos, Color);
				Erase(ColorEnd - Pos);
			}
		}
		else if (CurrentChar == '[')
		{
			// Handles specifiers like "Normal text [b]Bold text[/b]"
			auto EndBracket = Msg.find_first_of(L"]", Pos + 1);

			if (EndBracket == npos)
				continue; // Malformed specifier

			auto Distance = EndBracket - Pos;

			if (Msg[Pos + 1] == '/' && (Distance == 2 || Distance == 3))
			{
				// End of sequence
				// Matches e.g. [/], [/b], [/i]

				// Go back to default text
				FormatSpecifiers.emplace_back(Pos, FormatSpecifierType::Default);
			}
			else
			{
				// Beginning of sequence
				// Matches e.g. [b], [i]
				auto ft = CharToFT(Msg[Pos + 1]);
				if (ft == FormatSpecifierType::Unknown)
					continue;

				FormatSpecifiers.emplace_back(Pos, ft);
			}

			Erase(Distance + 1);
		}
	}
}

Chat::Chat(const std::string& FontName, bool BoldFont, int FontSize)
	: FontName{ FontName }, BoldFont{ BoldFont }, FontSize{ FontSize }
{
	const auto ScreenWidth = RGetScreenWidth();
	const auto ScreenHeight = RGetScreenHeight();

	Border.x1 = 10;
	Border.y1 = double(1080 - 300) / 1080 * ScreenHeight;
	Border.x2 = (double)500 / 1920 * ScreenWidth;
	Border.y2 = double(1080 - 100) / 1080 * ScreenHeight;

	Cursor.x = ScreenWidth / 2;
	Cursor.y = ScreenHeight / 2;

	const auto Scale = 1.f;
	DefaultFont.Create("NewChatFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont);
	ItalicFont.Create("NewChatItalicFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont, true);

	FontHeight = DefaultFont.GetHeight();
}

Chat::~Chat() = default;

void Chat::EnableInput(bool Enable, bool ToTeam){
	InputEnabled = Enable;
	TeamChat = ToTeam;

	if (Enable){
		InputField.clear();

		CaretPos = -1;

		SetCursorPos(RGetScreenWidth() / 2, RGetScreenHeight() / 2);
	}
	else{
		ZGetInput()->ResetRotation();

		SelectionState = SelectionStateType{};
	}

	ZGetGameInterface()->SetCursorEnable(Enable);

	ZPostPeerChatIcon(Enable);
}

void Chat::OutputChatMsg(const char *Msg){
	OutputChatMsg(Msg, TextColor);
}

void Chat::OutputChatMsg(const char *szMsg, u32 dwColor)
{
	wchar_t WideMsg[4096];
	auto ret = CodePageConversion<CP_UTF8>(WideMsg, szMsg);
	if (ret == ConversionError)
	{
		MLog("Chat::OutputChatMsg -- Conversion error\n");
		assert(false);
		return;
	}

	Msgs.emplace_back();
	auto&& Msg = Msgs.back();
	Msg.Time = GetTime();
	Msg.Msg = WideMsg;
	Msg.DefaultColor = dwColor;

	Msg.SubstituteFormatSpecifiers();
	DivideIntoLines(Msgs.size() - 1, std::back_inserter(LineSegments));

	NumNewlyAddedLines += Msg.GetLines();
	if (ChatLinesPixelOffsetY <= 0)
		ChatLinesPixelOffsetY = FontHeight;
}

void Chat::Scale(double WidthRatio, double HeightRatio){
	Border.x1 *= WidthRatio;
	Border.x2 *= WidthRatio;
	Border.y1 *= HeightRatio;
	Border.y2 *= HeightRatio;

	ResetFonts();
}

void Chat::Resize(int nWidth, int nHeight)
{
	Border.x1 = 10;
	Border.y1 = double(1080 - 300) / 1080 * RGetScreenHeight();
	Border.x2 = (double)500 / 1920 * RGetScreenWidth();
	Border.y2 = double(1080 - 100) / 1080 * RGetScreenHeight();

	ResetFonts();
}

void Chat::ClearHistory()
{
	Msgs.clear();
	LineSegments.clear();
	NumNewlyAddedLines = 0;
	ChatLinesPixelOffsetY = 0;
}

Chat::TimeType Chat::GetTime()
{
	return ZGetApplication()->GetTime();
}

bool Chat::CursorInRange(int x1, int y1, int x2, int y2){
	return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
}

int Chat::GetTextLength(MFontR2& Font, const wchar_t* Format, ...)
{
	wchar_t buf[1024];
	va_list va;
	va_start(va, Format);
	vsprintf_safe(buf, Format, va);
	va_end(va);
	return Font.GetWidth(buf);
}

struct CaretType
{
	int TotalTextHeight;
	v2i CaretPos;
};
static CaretType GetCaretPos(MFontR2& Font, const wchar_t* Text, int CaretPos, int Width)
{
	CaretType ret{ 1, { 0, 1 } };
	v2i Cursor{ 0, 1 };
	for (auto c = Text; *c != 0; ++c)
	{
		auto CharWidth = Font.GetWidth(c, 1);

		Cursor.x += CharWidth;
		if (Cursor.x > Width)
		{
			++Cursor.y;
			Cursor.x = CharWidth;
		}
		
		auto Distance = c - Text;
		if (Distance == CaretPos)
			ret.CaretPos = Cursor;
	}
	ret.TotalTextHeight = Cursor.y;
	return ret;
}

std::pair<bool, v2i> Chat::GetPos(const ChatMessage &c, u32 Pos)
{
	std::pair<bool, v2i> ret{ false, {0, 0} };
	if (Pos > c.Msg.length())
		return ret;

	D3DRECT Output = GetOutputRect();

	int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;

	int nLines = 0;

	for (int i = Msgs.size() - 1; nLines < Limit && i >= 0; i--){
		auto &cl = Msgs.at(i);

		if (&c == &cl){
			int nOffset = 0;

			if (c.GetLines() == 1){
				ret.second.y = Output.y2 - 5 - (nLines) * FontHeight - FontHeight * .5;
			}
			else{
				int nLine = 0;

				for (int i = 0; i < c.GetLines() - 1; i++){
					if (int(Pos) < c.GetLineBreak(i)->nStartPos)
						break;

					nLine++;
				}

				ret.second.y = Output.y2 - 5 - (nLines - nLine) * FontHeight - FontHeight * .5;

				if (nLine > 0)
					nOffset = c.GetLineBreak(nLine - 1)->nStartPos;
			}

			ret.second.x = Output.x1 + 5 + GetTextLength(DefaultFont, L"%.*s_", Pos - nOffset,
				&c.Msg.at(nOffset)) - GetTextLength(DefaultFont, L"_");

			ret.first = true;
			return ret;
		}

		nLines += cl.GetLines();
	}

	return ret;
}

bool Chat::OnEvent(MEvent* pEvent) {
	// We want to open the chat when the chat action key is pressed and close it when enter is pressed.
	// This is because a chat action key bound to something other than enter still has to be
	// inputtable. E.g., if it's bound to 'y', the user still has to be able to input 'y'.
	//
	// However, there's a problem with this when the user has chat bound to enter: When the chat is
	// open and the user presses enter, the char message with enter is sent, closing the chat,
	// but then the chat action key message is sent immediately after, opening it again.
	// Therefore, the chat would be unclosable. To fix this, we ignore the next chat action key
	// message when enter is pressed.

	const auto ActionPressed = pEvent->nMessage == MWM_ACTIONPRESSED;
	const auto CharMessage = pEvent->nMessage == MWM_CHAR;

	bool ChatPressed = false;
	{
		static bool IgnoreNextChatActionKey = false;

		auto&& Key = ZGetConfiguration()->GetKeyboard()->ActionKeys[ZACTION_CHAT];
		if (InputEnabled)
		{
			ChatPressed = CharMessage && pEvent->nKey == VK_RETURN;
			if (Key.nVirtualKey == DIK_RETURN || Key.nVirtualKeyAlt == DIK_RETURN)
				IgnoreNextChatActionKey = true;
		}
		else
		{
			auto ChatActionKeyPressed = ActionPressed && pEvent->nKey == ZACTION_CHAT;
			if (IgnoreNextChatActionKey && ChatActionKeyPressed)
			{
				IgnoreNextChatActionKey = false;
			}
			else
			{
				ChatPressed = ChatActionKeyPressed;
			}
		}
	}

	const auto TeamChatPressed = !InputEnabled && ActionPressed && pEvent->nKey == ZACTION_TEAMCHAT;

	if (ChatPressed || TeamChatPressed)
	{
		if (InputEnabled && ChatPressed && !InputField.empty())
		{
			char MultiByteString[1024];
			CodePageConversion<CP_UTF8>(MultiByteString, InputField.c_str());

			ZGetGameInterface()->GetChat()->Input(MultiByteString);

			InputHistory.push_back(InputField);
			CurInputHistoryEntry = InputHistory.size();

			InputField.clear();
			CaretPos = -1;
		}

		EnableInput(!InputEnabled, TeamChatPressed);
	}

	if (pEvent->nMessage == MWM_KEYDOWN) {
		switch (pEvent->nKey) {

		case VK_HOME:
			CaretPos = -1;
			break;

		case VK_END:
			CaretPos = InputField.length() - 1;
			break;

		case VK_TAB:
			/*bPlayerList = !bPlayerList;
			if (bPlayerList){
			#ifdef DEBUG
			vstrPlayerList.push_back(std::string("test1"));
			vstrPlayerList.push_back(std::string("test2"));
			#endif DEBUG
			nPlayerListWidth = 0;
			for (auto &it : *ZGetCharacterManager()){
			ZCharacter &Player = *it.second;
			vstrPlayerList.push_back(std::string(Player.GetProperty()->szName));

			int nLen = GetTextLen(vstrPlayerList.back().c_str(), -1);
			if (nLen > nPlayerListWidth)
			nPlayerListWidth = nLen;
			}

			nCurPlayer = 0;
			}
			else{
			std::string &strEntry = vstrPlayerList.at(nCurPlayer);
			InputField.insert(CaretPos + 1, strEntry);
			CaretPos += strEntry.length();
			vstrPlayerList.clear();
			}*/

		{
			size_t StartPos = InputField.rfind(' ');
			if (StartPos == std::string::npos)
				StartPos = 0;
			else
				StartPos++;

			if (StartPos == InputField.length())
				break;

			size_t PartialNameLength = InputField.size() - StartPos;

			auto PartialName = InputField.data() + StartPos;

			for (auto &it : *ZGetCharacterManager())
			{
				ZCharacter &Player = *it.second;
				const char *PlayerName = Player.GetProperty()->szName;
				size_t PlayerNameLength = strlen(PlayerName);

				if (PlayerNameLength < PartialNameLength)
					continue;

				wchar_t WidePlayerName[256];
				auto len = CodePageConversion<CP_ACP>(WidePlayerName, PlayerName);
				if (len == ConversionError)
				{
					MLog("Chat::OnEvent -- Conversion error while autocompleting name %s\n", PlayerName);
					assert(false);
					continue;
				}

				if (!_wcsnicmp(PartialName, WidePlayerName, PartialNameLength))
				{
					if (InputField.size() + PlayerNameLength - PartialNameLength > MAX_INPUT_LENGTH)
						break;

					for (size_t i = 0; i < PartialNameLength; i++)
						InputField.erase(InputField.size() - 1);

					InputField.append(WidePlayerName);
					CaretPos += PlayerNameLength - PartialNameLength;
					break;
				}
			}
		}

		break;

		case VK_UP:
			/*if (bPlayerList){
				if (nCurPlayer > 0)
					nCurPlayer--;
				break;
			}*/

			if (CurInputHistoryEntry > 0) {
				CurInputHistoryEntry--;
				InputField.assign(InputHistory.at(CurInputHistoryEntry));
				CaretPos = InputHistory.at(CurInputHistoryEntry).length() - 1;
			}
			break;

		case VK_DOWN:
			/*if (bPlayerList){
				if (nCurPlayer < int(vstrPlayerList.size()) - 1)
					nCurPlayer++;
				break;
			}*/

			if (CurInputHistoryEntry < int(InputHistory.size()) - 1) {
				CurInputHistoryEntry++;
				auto&& strEntry = InputHistory.at(CurInputHistoryEntry);
				InputField.assign(strEntry);
				CaretPos = strEntry.length() - 1;
			}
			else {
				InputField.clear();
				CaretPos = -1;
			}

			break;

		case VK_LEFT:
			if (CaretPos >= 0)
				CaretPos--;
			break;

		case VK_RIGHT:
			if (CaretPos < int(InputField.length()) - 1)
				CaretPos++;
			break;

		case 'V':
		{
			if (!pEvent->bCtrl)
				break;

			wchar_t Clipboard[256];
			MClipboard::Get(g_hWnd, Clipboard, std::size(Clipboard));
			if (InputField.length() + wcslen(Clipboard) > MAX_INPUT_LENGTH)
			{
				InputField.append(Clipboard, Clipboard + MAX_INPUT_LENGTH - InputField.length());
			}
			else
			{
				InputField += Clipboard;
			}

			break;
		}

		};
	}
	else if (pEvent->nMessage == MWM_CHAR) {
		switch (pEvent->nKey) {

		case VK_TAB:
		case VK_RETURN:
			break;

		case VK_BACK:
			if (CaretPos >= 0) {
				InputField.erase(CaretPos, 1);
				CaretPos--;
			}
			break;
		case VK_ESCAPE:
			EnableInput(false, false);
			break;

		default:
			if (InputField.length() < MAX_INPUT_LENGTH) {
				if (pEvent->nKey < 27) // Ctrl + A-Z
					break;

				InputField.insert(CaretPos + 1, 1, pEvent->nKey);

				auto SlashR = L"/r ";
				auto SlashWhisper = L"/whisper ";
				if (iequals(InputField, SlashR))
				{
					wchar_t LastSenderWide[512];
					auto* LastSender = ZGetGameInterface()->GetChat()->m_szWhisperLastSender;
					auto len = CodePageConversion<CP_ACP>(LastSenderWide, LastSender);
					if (len == ConversionError)
					{
						MLog("Chat::OnEvent -- Conversion error while handling /r on name %s\n", LastSender);
						assert(false);
						break;
					}

					InputField = SlashWhisper;
					InputField += LastSenderWide;
					InputField += ' ';
					CaretPos = InputField.length() - 1;
				}
				else
				{
					CaretPos++;
				}
			}
		};
	}

	auto ret = GetCaretPos(DefaultFont, InputField.c_str(), CaretPos, Border.x2 - (Border.x1 + 5));
	InputHeight = ret.TotalTextHeight;
	CaretCoord = ret.CaretPos;

	return true;
}

int Chat::GetTextLen(ChatMessage &cl, int Pos, int Count){
	return GetTextLength(DefaultFont, L"_%.*s_", Count, &cl.Msg.at(Pos)) - GetTextLength(DefaultFont, L"__");
}

int Chat::GetTextLen(const char *Msg, int Count){
	return GetTextLength(DefaultFont, L"_%.*s_", Count, Msg) - GetTextLength(DefaultFont, L"__");
}

void Chat::OnUpdate(float TimeDelta){
	UpdateNewMessagesAnimation(TimeDelta);

	if (!IsInputEnabled())
		return;

	auto PrevCursorPos = Cursor;
	Cursor = MEvent::LatestPos;

	v2 MinimumSize{ 192.f * RGetScreenWidth() / 1920.f, 108.f * RGetScreenHeight() / 1080.f };

	if (ResizeFlags){
		if (ResizeFlags & ResizeFlagsType::X1 &&
			Border.x1 + Cursor.x - PrevCursorPos.x < Border.x2 - MinimumSize.x) {
			Border.x1 += Cursor.x - PrevCursorPos.x;
		}
		if (ResizeFlags & ResizeFlagsType::X2 &&
			Border.x2 + Cursor.x - PrevCursorPos.x > Border.x1 + MinimumSize.x) {
			Border.x2 += Cursor.x - PrevCursorPos.x;
		}
		if (ResizeFlags & ResizeFlagsType::Y1 &&
			Border.y1 + Cursor.y - PrevCursorPos.y < Border.y2 - MinimumSize.y) {
			Border.y1 += Cursor.y - PrevCursorPos.y;
		}
		if (ResizeFlags & ResizeFlagsType::Y2 &&
			Border.y2 + Cursor.y - PrevCursorPos.y > Border.y1 + MinimumSize.y) {
			Border.y2 += Cursor.y - PrevCursorPos.y;
		}

		LineSegments.clear();
		for (int i = 0; i < int(Msgs.size()); ++i)
			DivideIntoLines(i, std::back_inserter(LineSegments));
	}

	if (Action == ChatWindowAction::Moving){
		Border.x1 += Cursor.x - PrevCursorPos.x;
		Border.y1 += Cursor.y - PrevCursorPos.y;
		Border.x2 += Cursor.x - PrevCursorPos.x;
		Border.y2 += Cursor.y - PrevCursorPos.y;
	}

	if (SelectionState.FromMsg && SelectionState.ToMsg &&
		MEvent::IsKeyDown(VK_CONTROL) && MEvent::IsKeyDown('C')){
		if (OpenClipboard(g_hWnd)){
			EmptyClipboard();

			if (SelectionState.FromMsg == SelectionState.ToMsg){
				auto index = min(SelectionState.FromPos, SelectionState.ToPos);
				auto str = SelectionState.FromMsg->Msg.substr(index);
				MClipboard::Set(g_hWnd, str);
			}
			else{
				std::wstring str;

				bool FirstFound = false;

				for (auto it = Msgs.begin(); it != Msgs.end(); it++){
					auto* pcl = &*it;

					if (pcl == SelectionState.FromMsg || pcl == SelectionState.ToMsg){
						if (!FirstFound){
							auto nPos = pcl == SelectionState.FromMsg ?
								SelectionState.FromPos : SelectionState.ToPos;
							str.append(&pcl->Msg.at(nPos));

							FirstFound = true;
							continue;
						}
						else{
							auto nPos = pcl == SelectionState.FromMsg ?
								SelectionState.FromPos : SelectionState.ToPos;
							str.append(L"\n");
							str.append(pcl->Msg.c_str(), nPos + 2);

							break;
						}
					}

					if (FirstFound){
						str.append(L"\n");
						str.append(pcl->Msg.c_str());
					}
				}

				if (FirstFound){
					MClipboard::Set(g_hWnd, str);
				}
			}

			CloseClipboard();
		}
	}

	const int nBorderWidth = 5;

	// TODO: Move to OnEvent
	if (MEvent::IsKeyDown(VK_LBUTTON)) {
		if (Action == ChatWindowAction::None) {
			D3DRECT tr = GetTotalRect();

			if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth,
				tr.x1 + nBorderWidth, tr.y2 + nBorderWidth)) {
				ResizeFlags |= ResizeFlagsType::X1;
			}
			if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth,
				tr.x2 + nBorderWidth, tr.y1 + nBorderWidth)) {
				ResizeFlags |= ResizeFlagsType::Y1;
			}
			if (CursorInRange(tr.x2 - nBorderWidth, tr.y1 - nBorderWidth,
				tr.x2 + nBorderWidth, tr.y2 + nBorderWidth)) {
				ResizeFlags |= ResizeFlagsType::X2;
			}
			if (CursorInRange(tr.x1 - nBorderWidth, tr.y2 - nBorderWidth,
				tr.x2 + nBorderWidth, tr.y2 + nBorderWidth)) {
				ResizeFlags |= ResizeFlagsType::Y2;
			}

			if (ResizeFlags)
				Action = ChatWindowAction::Resizing;
		}

		if (CursorInRange(Border.x2 - 15, Border.y1 - 18, Border.x2 - 15 + 12, Border.y1 - 18 + FontHeight) &&
			Action == ChatWindowAction::None) {
			Border.x1 = 10;
			Border.y1 = double(1080 - 300) / 1080 * RGetScreenHeight();
			Border.x2 = (double)500 / 1920 * RGetScreenWidth();
			Border.y2 = double(1080 - 100) / 1080 * RGetScreenHeight();
		}
		else if (CursorInRange(Border.x1 + 5, Border.y1 + 5, Border.x2 - 5, Border.y2 - 5)) {
			if (Action != ChatWindowAction::Selecting) {
				auto&& Output = GetOutputRect();

				int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;
				int Line = Limit - ((Output.y2 - 5) - Cursor.y) / FontHeight;

				int i = Msgs.size() - 1;
				int CurLine = Limit + 1;

				while (i >= 0){
					auto&& cl = Msgs[i];

					if (CurLine - cl.GetLines() <= Line){
						SelectionState.FromMsg = &cl;
						Action = ChatWindowAction::Selecting;

						auto Pos = CurLine - cl.GetLines() == Line ?
							0 : cl.GetLineBreak(Line - (CurLine - cl.GetLines()) - 1)->nStartPos;
						int x = Cursor.x - (Output.x1 + 5);
						int Len = 0;

						while (x > Len && Pos < int(cl.Msg.length())){
							Len += GetTextLen(cl, Pos, 1);
							Pos++;
						}

						Pos--;

						if (Len - GetTextLen(cl, Pos, 1) / 2 > x)
							SelectionState.FromPos = Pos - 1;
						else
							SelectionState.FromPos = Pos;

						break;
					}

					CurLine -= cl.GetLines();
					i--;
				}

				if (i < 0){
					SelectionState.FromMsg = 0;
					SelectionState.ToMsg = 0;
				}
			}
			else{
				auto&& Output = GetOutputRect();

				int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;
				int Line = Limit - ((Output.y2 - 5) - Cursor.y) / FontHeight;

				int i = Msgs.size() - 1;
				int CurLine = Limit + 1;

				while (i >= 0){
					auto&& cl = Msgs.at(i);

					if (CurLine - cl.GetLines() <= Line || i == 0){
						SelectionState.ToMsg = &cl;

						int Pos;

						if (CurLine - cl.GetLines() <= Line)
							Pos = CurLine - cl.GetLines() == Line ?
								0 : cl.GetLineBreak(Line - (CurLine - cl.GetLines()) - 1)->nStartPos;
						else
							Pos = 0;

						int x = Cursor.x - (Output.x1 + 5);
						int nLen = 0;

						while (x > nLen && Pos < int(cl.Msg.length())){
							nLen += GetTextLen(cl, Pos, 1);
							Pos++;
						}

						Pos--;

						if (nLen - GetTextLen(cl, Pos, 1) / 2 > x)
							SelectionState.ToPos = Pos - 1;
						else
							SelectionState.ToPos = Pos;

						break;
					}

					CurLine -= cl.GetLines();
					i--;
				}
			}
		}
		else if (Action != ChatWindowAction::Selecting){
			SelectionState.FromMsg = 0;
			SelectionState.ToMsg = 0;
		}

		if (Action == ChatWindowAction::None &&
			CursorInRange(Border.x1, Border.y1 - 20, Border.x2 + 1, Border.y1))
			Action = ChatWindowAction::Moving;
	}
	else
	{
		Action = ChatWindowAction::None;
		ResizeFlags = 0;
	}
}

void Chat::UpdateNewMessagesAnimation(float TimeDelta)
{
	if (ChatLinesPixelOffsetY <= 0) {
		return;
	}

	constexpr auto LinesPerSecond = 4;

	auto PixelDelta = TimeDelta * FontHeight * LinesPerSecond;
	ChatLinesPixelOffsetY -= PixelDelta;

	if (ChatLinesPixelOffsetY <= 0)
	{
		NumNewlyAddedLines--;
		ChatLinesPixelOffsetY = NumNewlyAddedLines > 0 ? FontHeight + ChatLinesPixelOffsetY : 0;
	}
}

D3DRECT Chat::GetOutputRect(){
	D3DRECT r = { Border.x1, Border.y1, Border.x2, Border.y2 - FontHeight };
	return r;
}

D3DRECT Chat::GetInputRect(){
	D3DRECT r = { Border.x1, Border.y2 - FontHeight, Border.x2, Border.y2 + (InputHeight - 1) * FontHeight };
	return r;
}

D3DRECT Chat::GetTotalRect(){
	D3DRECT r = { Border.x1, Border.y1 - 20, Border.x2, Border.y2 };
	return r;
}

// Converts a D3DRECT, which is specified in terms of the coordinates of each corner,
// to an MRECT, which is specified in terms of the top left coordinate and the extents.
static MRECT MakeMRECT(const D3DRECT& src)
{
	return{
		src.x1,
		src.y1,
		src.x2 - src.x1,
		src.y2 - src.y1,
	};
}

void Chat::OnDraw(MDrawContext* pDC)
{
	if (HideAlways ||
		(HideDuringReplays && ZGetGame()->IsReplay()))
		return;

	bool ShowAll = ZIsActionKeyDown(ZACTION_SHOW_FULL_CHAT) && !InputEnabled;
	auto&& Output = GetOutputRect();

	int CeiledLimit, FlooredLimit;
	if (ShowAll)
	{
		CeiledLimit = FlooredLimit = (Output.y2 - 5) / FontHeight;
	}
	else
	{
		auto Limit = float(Output.y2 - Output.y1 - 10) / FontHeight;
		FlooredLimit = int(Limit);
		CeiledLimit = int(ceil(Limit));
	}

	auto Time = GetTime();

	DrawBackground(pDC, Time, NumNewlyAddedLines > 0 ? CeiledLimit : FlooredLimit, ShowAll);
	DrawChatLines(pDC, Time, InputEnabled ? CeiledLimit : FlooredLimit, ShowAll);
	DrawSelection(pDC);
	
	if (IsInputEnabled()) {
		DrawFrame(pDC, Time);
	}
}

int Chat::DrawTextWordWrap(MFontR2& Font, const WStringView& Str, const D3DRECT &r, u32 Color)
{
	int Lines = 1;
	int StringLength = int(Str.size());
	int CurrentLineLength = 0;
	int MaxLineLength = r.x2 - r.x1;

	for (int i = 0; i < StringLength; i++)
	{
		int CharWidth = Font.GetWidth(&Str[i], 1);
		int CharHeight = Font.GetHeight();

		if (CurrentLineLength + CharWidth > MaxLineLength)
		{
			CurrentLineLength = 0;
			Lines++;
		}

		auto x = r.x1 + CurrentLineLength;
		auto y = r.y1 + (CharHeight + 1) * max(0, Lines - 1);
		Font.m_Font.DrawText(x, y,
			Str.substr(i, 1),
			Color);

		CurrentLineLength += CharWidth;
	}

	return Lines;
}

void Chat::DrawTextN(MFontR2& pFont, const WStringView& Str, const D3DRECT &r, u32 Color)
{
	pFont.m_Font.DrawText(r.x1, r.y1, Str, Color);
}

void Chat::DrawBorder(MDrawContext* pDC)
{
	auto rect = Border;
	rect.y2 += (InputHeight - 1) * FontHeight;

	// Draw the box outline
	v2 vs[] = {
		{ float(rect.x1), float(rect.y1) },
		{ float(rect.x2), float(rect.y1) },
		{ float(rect.x2), float(rect.y2) },
		{ float(rect.x1), float(rect.y2) },
	};

	for (size_t i = 0; i < std::size(vs); i++)
	{
		auto a = vs[i];
		auto b = vs[(i + 1) % std::size(vs)];
		pDC->Line(a.x, a.y, b.x, b.y);
	}

	// Draw the line between the output and input
	rect.y2 -= 2;
	rect.y2 -= InputHeight * FontHeight;
	pDC->Line(rect.x1, rect.y2, rect.x2, rect.y2);
}

void Chat::DrawBackground(MDrawContext* pDC, TimeType Time, int Limit, bool ShowAll)
{
	if (BackgroundColor & 0xFF000000)
	{
		if (!InputEnabled)
		{
			// Need to store this value instead of calculating it every frame
			int Lines = -max(0, NumNewlyAddedLines - 1);
			// i needs to be signed since it terminates on -1
			for (int i = int(Msgs.size() - 1); Lines < Limit && i >= 0; i--)
			{
				auto&& cl = Msgs.at(i);

				if (cl.Time + FadeTime < Time && !ShowAll && !InputEnabled)
					break;

				Lines += cl.GetLines();
			}

			Lines = min(Lines, Limit);

			if (Lines > 0)
			{
				auto&& Output = GetOutputRect();
				D3DRECT Rect = {
					Output.x1,
					Output.y2 - 5 - Lines * FontHeight,
					Output.x2,
					Output.y2,
				};

				if (NumNewlyAddedLines > 0) {
					Rect.y1 += ChatLinesPixelOffsetY;
					if (!ShowAll) {
						Rect.y1 = max(Rect.y1, Output.y1);
					}
				}

				pDC->SetColor(BackgroundColor);
				pDC->FillRectangle(MakeMRECT(Rect));
			}
		}
		else
		{
			auto Rect = Border;
			Rect.y2 += (InputHeight - 1) * FontHeight;

			pDC->SetColor(BackgroundColor);
			pDC->FillRectangle(MakeMRECT(Rect));
		}
	}
}

template <typename T>
struct LineDivisionState
{
	T&& OutputIterator;
	LineSegmentInfo CurLineSegmentInfo;
	int ChatMessageIndex = 0;
	int MsgIndex = 0;
	int Lines = 0;
	int CurrentLinePixelLength = 0;
	u32 CurTextColor;
	u32 CurEmphasis = EmphasisType::Default;

	LineDivisionState(T&& OutputIterator, int ChatMessageIndex, u32 CurTextColor) :
		OutputIterator{ std::forward<T>(OutputIterator) },
		ChatMessageIndex{ ChatMessageIndex },
		CurTextColor{ CurTextColor }
	{}

	void AddSegment(bool IsEndOfLine)
	{
		// Compute the length from the distance from the current character index
		// to the one the substring started at.
		CurLineSegmentInfo.LengthInCharacters = MsgIndex - int(CurLineSegmentInfo.Offset);

		// Add this LineSegmentInfo to the vector.
		OutputIterator++ = CurLineSegmentInfo;

		if (IsEndOfLine)
		{
			CurrentLinePixelLength = 0;
			Lines++;
		}

		// Reset to zero-initialized LineSegmentInfo.
		CurLineSegmentInfo = LineSegmentInfo{};
		// Set data.
		CurLineSegmentInfo.ChatMessageIndex = ChatMessageIndex;
		CurLineSegmentInfo.Offset = MsgIndex;
		CurLineSegmentInfo.PixelOffsetX = CurrentLinePixelLength;
		CurLineSegmentInfo.IsStartOfLine = CurrentLinePixelLength == 0;
		CurLineSegmentInfo.TextColor = CurTextColor;
		CurLineSegmentInfo.Emphasis = CurEmphasis;
	}

	void HandleFormatSpecifier(FormatSpecifier& FormatSpec)
	{
		switch (FormatSpec.ft) {
		case FormatSpecifierType::Color:
			CurTextColor = FormatSpec.Color;
			break;

		case FormatSpecifierType::Default:
			CurEmphasis = EmphasisType::Default;
			break;

		case FormatSpecifierType::Bold:
			CurEmphasis |= EmphasisType::Bold;
			break;

		case FormatSpecifierType::Italic:
			CurEmphasis |= EmphasisType::Italic;
			break;

		case FormatSpecifierType::Underline:
			CurEmphasis |= EmphasisType::Underline;
			break;

		case FormatSpecifierType::Strikethrough:
			CurEmphasis |= EmphasisType::Strikethrough;
			break;

		case FormatSpecifierType::Linebreak:
			AddSegment(true);
			return;
		};

		if (MsgIndex - int(CurLineSegmentInfo.Offset) == 0)
		{
			// If MsgIndex - int(CurLineSegmentInfo.Offset) equals zero,
			// the substring would be empty if we were to add a line.
			// Instead, we want to add the modified text attributes to the current segment.
			CurLineSegmentInfo.TextColor = CurTextColor;
			CurLineSegmentInfo.Emphasis = CurEmphasis;
		}
		else
		{
			AddSegment(false);
		}
	}
};

template <typename T>
void Chat::DivideIntoLines(int ChatMessageIndex, T&& OutputIterator)
{
	auto&& cl = Msgs[ChatMessageIndex];
	// Clear the previous wrapping line breaks, since we're going to add new ones.
	cl.ClearWrappingLineBreaks();

	auto MaxLineLength = (Border.x2 - 5) - (Border.x1 + 5);

	LineDivisionState<T> State{ std::forward<T>(OutputIterator), ChatMessageIndex, cl.DefaultColor };

	// Initialize the first segment.
	State.CurLineSegmentInfo.ChatMessageIndex = ChatMessageIndex;
	State.CurLineSegmentInfo.Offset = 0;
	State.CurLineSegmentInfo.PixelOffsetX = 0;
	State.CurLineSegmentInfo.IsStartOfLine = true;
	State.CurLineSegmentInfo.TextColor = cl.DefaultColor;
	State.CurLineSegmentInfo.Emphasis = EmphasisType::Default;

	auto FormatIterator = cl.FormatSpecifiers.begin();
	for (State.MsgIndex = 0; State.MsgIndex < int(cl.Msg.length()); ++State.MsgIndex)
	{
		// Process all the format specifiers at this index.
		// There may be more than one, so we do a loop.
		while (FormatIterator != cl.FormatSpecifiers.end() &&
			FormatIterator->nStartPos == State.MsgIndex)
		{
			State.HandleFormatSpecifier(*FormatIterator);
			++FormatIterator;
		}

		auto CharWidth = DefaultFont.GetWidth(cl.Msg.data() + State.MsgIndex, 1);

		// If adding this character would make the line length exceed the max,
		// we add a new line for this character to go on.
		if (State.CurrentLinePixelLength + CharWidth > MaxLineLength)
		{
			// ChatMessage::AddWrappingLineBreak returns an iterator to the line break
			// that was inserted, so we want to set FormatIterator to the next one.
			// We do this since the current iterator may have been invalidated by the mutation.
			FormatIterator = std::next(cl.AddWrappingLineBreak(State.MsgIndex));
			State.AddSegment(true);
		}

		State.CurrentLinePixelLength += CharWidth;
	}

	// Add the final segment.
	State.AddSegment(true);

	cl.Lines = State.Lines;
}

MFontR2& Chat::GetFont(u32 Emphasis)
{
	if (Emphasis & EmphasisType::Italic)
		return ItalicFont;

	return DefaultFont;
}

static auto GetDrawLinesRect(const D3DRECT& OutputRect, int LinesDrawn,
	v2i PixelOffset, int FontHeight)
{
	return D3DRECT{
		OutputRect.x1 + 5 + PixelOffset.x,
		OutputRect.y2 - 5 - ((LinesDrawn + 1) * FontHeight) + PixelOffset.y,
		OutputRect.x2 - 5,
		OutputRect.y2 - 5
	};
}

static u32 ScaleAlpha(u32 Color, float MessageTime, float CurrentTime,
	float BeginFadeTime, float EndFadeTime)
{
	auto Delta = CurrentTime - MessageTime;

	auto A =  (Color & 0xFF000000) >> 24;
	auto RGB = Color & 0x00FFFFFF;

	if (Delta < BeginFadeTime)
		return Color; // 100% alpha
	if (Delta > EndFadeTime)
		return RGB;   // 0% alpha

	auto Scale = 1 - ((Delta - BeginFadeTime) / (EndFadeTime - BeginFadeTime));
	auto AS = static_cast<u8>(A * Scale);

	return (AS << 24) | RGB;
}

void Chat::DrawChatLines(MDrawContext* pDC, TimeType Time, int Limit, bool ShowAll)
{
	auto Reverse = [&](auto&& Container, int Offset = 0) {
		return MakeRange(Container.rbegin() + Offset, Container.rend());
	};

	DefaultFont.m_Font.BeginFont();

	auto PrevClipRect = pDC->GetClipRect();
	{
		auto ClipRect = GetOutputRect();
		if (ShowAll)
			ClipRect.y1 = 0;
		pDC->SetClipRect(MakeMRECT(ClipRect));
	}

	if (ChatLinesPixelOffsetY > 0)
		Limit++;

	auto MessagesOffset = max(0, NumNewlyAddedLines - 1);

	int LinesDrawn = 0;
	for (auto&& LineSegment : Reverse(LineSegments, MessagesOffset))
	{
		v2i PixelOffset{ LineSegment.PixelOffsetX, int(ChatLinesPixelOffsetY) };
		auto&& Rect = GetDrawLinesRect(GetOutputRect(), LinesDrawn, PixelOffset, FontHeight);
		auto&& cl = Msgs[LineSegment.ChatMessageIndex];

		if (!ShowAll && !InputEnabled && Time > cl.Time + FadeTime)
			break;

		auto String = cl.Msg.data() + LineSegment.Offset;
		auto Length = LineSegment.LengthInCharacters;
		auto&& Font = GetFont(LineSegment.Emphasis);
		auto Color = LineSegment.TextColor;

		if (!ShowAll && !InputEnabled)
			Color = ScaleAlpha(Color, cl.Time, Time, FadeTime * 0.8f, FadeTime);

		DrawTextN(Font, { String, Length }, Rect, Color);

		if (LineSegment.IsStartOfLine)
		{
			++LinesDrawn;
			if (LinesDrawn >= Limit)
				break;
		}
	}

	pDC->SetClipRect(PrevClipRect);
	DefaultFont.m_Font.EndFont();
}

void Chat::DrawSelection(MDrawContext * pDC)
{
	if (SelectionState.FromMsg && SelectionState.ToMsg)
	{
		auto ret = GetPos(*SelectionState.FromMsg, SelectionState.FromPos);
		if (!ret.first)
			return;
		auto From = ret.second;

		ret = GetPos(*SelectionState.ToMsg, SelectionState.ToPos);
		if (!ret.first)
			return;
		auto To = ret.second;

		auto ShouldSwap = From.y > To.y || From.y == To.y && From.x > To.x;

		std::pair<const ChatMessage*, int> Stuff;
		if (ShouldSwap)
		{
			std::swap(From, To);
			Stuff = { SelectionState.FromMsg, SelectionState.FromPos };
		}
		else
		{
			Stuff = { SelectionState.ToMsg, SelectionState.ToPos };
		}

		ret = GetPos(*Stuff.first, Stuff.second + 1);
		if (!ret.first)
			return;
		To = ret.second;

		auto Fill = [&](auto&&... Coords) {
			pDC->FillRectangle(MakeMRECT({ Coords... }));
		};

		pDC->SetColor(SelectionColor);
		// Get half of font, adjusted in each direction to compensate for odd font heights.
		// For instance, if the font height is 15, we want to subtract 8 and add 7,
		// since adjusting by a single integer value would make the rectangles we draw
		// overlap each other slightly. (The height would be even when it ought to be odd.)
		auto TopOffset = int(ceil(FontHeight / 2.f));
		auto BottomOffset = FontHeight / 2;
		if (From.y == To.y)
		{
			Fill(From.x,
				From.y - TopOffset,
				To.x,
				To.y + BottomOffset);
		}
		else {
			Fill(From.x,
				From.y - TopOffset,
				Border.x2 - 5,
				From.y + BottomOffset);

			for (int i = FontHeight; i < To.y - From.y; i += FontHeight) {
				Fill(Border.x1 + 5,
					From.y + i - TopOffset,
					Border.x2 - 5,
					From.y + i + BottomOffset);
			}
			Fill(Border.x1,
				To.y - TopOffset,
				To.x - 5,
				To.y + BottomOffset);
		}
	}
}

void Chat::DrawFrame(MDrawContext * pDC, TimeType Time)
{
	// Draw top of border
	{
		D3DRECT Rect = {
			Border.x1,
			Border.y1 - 20,
			Border.x2 + 1,
			Border.y1
		};

		pDC->SetColor(InterfaceColor);
		pDC->FillRectangle(MakeMRECT(Rect));
	}

	DrawBorder(pDC);

	// Draw D button
	{
		D3DRECT Rect = {
			Border.x2 - 15,
			Border.y1 - 18,
			Border.x2 - 15 + 12,
			Border.y1 - 18 + FontHeight,
		};

		DrawTextN(DefaultFont, L"D", Rect, TextColor);
	}

	D3DRECT Rect = {
		Border.x1 + 5,
		Border.y2 - 2 - FontHeight,
		Border.x2,
		Border.y2,
	};

	int x = Rect.x1 + CaretCoord.x;
	int y = Rect.y1 + (CaretCoord.y - 1) * FontHeight;

	// Alternate every 0.4 seconds
	auto Period = Seconds(0.4f);
	if (Time % (Period * 2) > Period)
	{
		// Draw caret
		pDC->SetColor(TextColor);
		pDC->Line(x, y, x, y + FontHeight);
	}

	DrawTextWordWrap(DefaultFont, InputField.c_str(), Rect, TextColor);
}

void Chat::ResetFonts(){
	DefaultFont.Destroy();
	ItalicFont.Destroy();

	const auto Scale = 1.f;
	DefaultFont.Create("NewChatFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont);
	ItalicFont.Create("NewChatItalicFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont, true);

	FontHeight = DefaultFont.GetHeight();
}
