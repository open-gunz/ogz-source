#include "stdafx.h"
#ifdef MFC
#include "TrayIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CTrayIcon::CTrayIcon()
{

}

CTrayIcon::~CTrayIcon()
{

}

BOOL CTrayIcon::Create(CWnd* pWnd, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID)
{
	m_tnd.cbSize = sizeof(NOTIFYICONDATA);
	m_tnd.hWnd = pWnd->GetSafeHwnd();
	m_tnd.uID = uID;
	m_tnd.hIcon = icon;
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tnd.uCallbackMessage = uCallbackMessage;
	strcpy_safe(m_tnd.szTip, szToolTip);

	return Shell_NotifyIcon(NIM_ADD, &m_tnd);
}

BOOL CTrayIcon::SetIcon(HICON hIcon)
{
	m_tnd.uFlags = NIF_ICON;
	m_tnd.hIcon = hIcon;

	return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CTrayIcon::SetTooltipText(LPCTSTR pszTip)
{
	m_tnd.uFlags = NIF_TIP;
	strcpy_safe(m_tnd.szTip, pszTip);

	return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

void CTrayIcon::RemoveIcon()
{
	m_tnd.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &m_tnd);
}
#endif