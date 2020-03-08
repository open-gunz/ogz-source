#include "stdafx.h"
#include "ReplayControl.h"
#include "RGMain.h"
#include "NewChat.h"
#include "defer.h"

ReplayControl g_ReplayControl;

static void DrawBorder(MDrawContext* pDC, v2 p1, v2 p2)
{
	v2 vs[] = {
		{ float(p1.x), float(p1.y) },
		{ float(p2.x), float(p1.y) },
		{ float(p2.x), float(p2.y) },
		{ float(p1.x), float(p2.y) },
	};

	for (size_t i = 0; i < std::size(vs); i++)
	{
		auto a = vs[i];
		auto b = vs[(i + 1) % std::size(vs)];
		pDC->Line(a.x, a.y, b.x, b.y);
	}
}

void ReplayControl::OnDraw(MDrawContext* pDC)
{
	if (!ZGetGame()->IsReplay())
		return;

	if (!GetRGMain().GetChat().IsInputEnabled())
		return;

	pDC->SetColor(CHAT_DEFAULT_INTERFACE_COLOR);

	v2 p1, p2;
	p1.x = RELWIDTH(1920.f / 2 - 220);
	p1.y = RELHEIGHT(830);
	p2.x = RELWIDTH(1920.f / 2 + 220);
	p2.y = RELHEIGHT(870);

	DrawBorder(pDC, p1, p2);

	p1.x = RELWIDTH(1920.f / 2 - 170);
	p1.y = RELHEIGHT(850);
	p2.x = RELWIDTH(1920.f / 2 + 170);
	p2.y = RELHEIGHT(850);

	pDC->Line(p1.x, p1.y, p2.x, p2.y);

	float ReplayTime = ZGetGame()->GetReplayTime();
	float ReplayLength = ZGetGame()->GetReplayLength();

	float Index = ReplayTime / ReplayLength;

	p1.x = RELWIDTH(1920.f / 2 - 170 + Index * 340 - 5);
	p1.y = RELHEIGHT(835);
	p2.x = RELWIDTH(1920.f / 2 - 170 + Index * 340 + 5);
	p2.y = RELHEIGHT(865);

	pDC->FillRectangle(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);

	char buf[64];

	sprintf_safe(buf, "%02d:%02d", int(ReplayTime / 60), int(fmod(ReplayTime, 60)));

	pDC->Text(RELWIDTH(1920.f / 2 - 215), RELHEIGHT(835), buf);

	sprintf_safe(buf, "%02d:%02d", int(ReplayLength / 60), int(fmod(ReplayLength, 60)));

	pDC->Text(RELWIDTH(1920.f / 2 + 185), RELHEIGHT(835), buf);
}

static bool CursorInRange(const POINT &Cursor, int x1, int y1, int x2, int y2){
	return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
}

bool ReplayControl::OnEvent(MEvent *pEvent)
{
	if (!ZGetGame()->IsReplay() || !GetRGMain().GetChat().IsInputEnabled())
		return false;

	static bool LastLb = false;
	auto CurLb = MEvent::IsKeyDown(VK_LBUTTON);
	DEFER([&] { LastLb = CurLb; });

	if (!(CurLb && !LastLb))
	{
		return false;
	}

	POINT p;
	GetCursorPos(&p);

	if (!CursorInRange(p, RELWIDTH(1920.f / 2 - 170), RELHEIGHT(835), RELWIDTH(1920.f / 2 + 170), RELHEIGHT(865)))
		return false;

	float Index = float(p.x - RELWIDTH(1920.f / 2 - 170)) / 340;

	for (auto* Player : MakePairValueAdapter(*ZGetCharacterManager()))
	{
		Player->UpdateValidShotTime(0, 0);
	}

	ZGetGame()->SetReplayTime(Index * ZGetGame()->GetReplayLength());

	return true;
}