#ifndef _ZHelpScreen_h
#define _ZHelpScreen_h

class MBitmapR2;

class ZHelpScreen
{
public:
	ZHelpScreen();
	~ZHelpScreen();

	void ChangeMode();

	void DrawHelpScreen();

public:

	MBitmapR2* m_pHelpScreenBitmap;

	bool m_bDrawHelpScreen;

};

#endif//_ZHelpScreen_h