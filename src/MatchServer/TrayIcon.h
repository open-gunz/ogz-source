#ifndef _TRAYICON_H
#define _TRAYICON_H

class CTrayIcon
{
private:
protected:
	NOTIFYICONDATA	m_tnd;
public:
	CTrayIcon();
	virtual ~CTrayIcon();

	BOOL Create(CWnd* pWnd, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID);
	BOOL SetIcon(HICON hIcon);
	BOOL SetTooltipText(LPCTSTR pszTip);
	void RemoveIcon();
};
#endif
