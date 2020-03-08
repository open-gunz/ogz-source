#pragma once
#include "afxwin.h"


// CLightmapDialog dialog

class CGenerateLightmapDialog : public CDialog
{
	DECLARE_DYNAMIC(CGenerateLightmapDialog)

public:
	CGenerateLightmapDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGenerateLightmapDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_LIGHTMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_MaxSize;
	CString m_Toler;
	CString m_SuperSample;
};
