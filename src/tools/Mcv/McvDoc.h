// McvDoc.h : interface of the CMcvDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCVDOC_H__ED4AB444_D9DE_42F7_AEB5_15CDF9E2B860__INCLUDED_)
#define AFX_MCVDOC_H__ED4AB444_D9DE_42F7_AEB5_15CDF9E2B860__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMcvDoc : public CDocument
{
protected: // create from serialization only
	CMcvDoc();
	DECLARE_DYNCREATE(CMcvDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcvDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMcvDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMcvDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCVDOC_H__ED4AB444_D9DE_42F7_AEB5_15CDF9E2B860__INCLUDED_)
