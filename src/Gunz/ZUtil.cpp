#include "stdafx.h"
#include "ZUtil.h"


void ShowIExplorer(BOOL bShow, const char* szTitleTok)
{
#ifndef _PUBLISH
	return;
#endif

	int max_wnd_count = 500;
	HWND hWnd = ::GetTopWindow( NULL );
		
	char strBuf[1024];
	memset( strBuf, 0, sizeof(strBuf) );

	static int restore_count = 0;

	if( restore_count >= 0 && bShow ) return;
	else if( restore_count < 0 && !bShow ) return;
	
	if( bShow ) restore_count += 1;
	else restore_count -= 1;

	HWND BasehWnd = NULL;

	while( max_wnd_count > 0 )
	{
		max_wnd_count--;
		BasehWnd = hWnd;

		::GetClassNameA((HWND)BasehWnd, strBuf, 1023);
		if( lstrcmpi(strBuf, "IEFrame") == 0 )
		{
			if( (BasehWnd = ::FindWindowEx((HWND)BasehWnd, NULL, "WorkerW", NULL)) )
			{
				::GetWindowText( hWnd, strBuf, 1023 );

				if (strstr(strBuf, szTitleTok) != NULL)
				{
					BasehWnd = ::GetNextWindow( hWnd, GW_HWNDNEXT );
					if( bShow ) ::ShowWindow( hWnd, SW_RESTORE );
					else ::ShowWindow( hWnd, SW_SHOWMINIMIZED );

					hWnd = BasehWnd;
					continue;
				}
			}
		}
		
		hWnd = ::GetNextWindow( hWnd, GW_HWNDNEXT );
	}
}
