#pragma once


// CGeneratePathDataDialog dialog

class CGeneratePathDataDialog : public CDialog
{
	DECLARE_DYNAMIC(CGeneratePathDataDialog)

public:
	CGeneratePathDataDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGeneratePathDataDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_GENERATEPATH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_WalkableAngle;
	CString m_Toler;
};
