/* 

Realspace for Samsung SyncMaster3d file exporter (*.ss3)
coded by dubble@maietgames.com since 2001-05-04
Copyright (c) MAIET Entertainment 2001

HISTORY: 

*/

#include "ss3exp.h"
#include "exporter.h"
#include "BELog.h"
#include <mmsystem.h>

#define SS3EXP_CLASS_ID	Class_ID(0x35cfb344, 0x51bf9b17)

class Ss3exp : public SceneExport {
	public:
		static HWND hParams;

		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
		int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file
		
		bool bAnimation,bSky,bVertex;

		//Constructor/Destructor
		Ss3exp();
		~Ss3exp();	
		

};


class Ss3expClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new Ss3exp();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}
	Class_ID		ClassID() {return SS3EXP_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("Ss3exp"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static Ss3expClassDesc Ss3expDesc;
ClassDesc2* GetSs3expDesc() {return &Ss3expDesc;}

BOOL CALLBACK Ss3expOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static Ss3exp *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (Ss3exp *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD (wParam)){
			case ID_OK:
				imp->bAnimation=0!=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_ANIMATION));
				imp->bSky=0!=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_SKY));
				imp->bVertex=0!=Button_GetCheck(GetDlgItem(hWnd,IDC_CHECK_VERTEXANI));
				EndDialog(hWnd, 1);
				return TRUE;
			}
			break;
	}
	return FALSE;
}


//--- Ss3exp -------------------------------------------------------
Ss3exp::Ss3exp()
{

}

Ss3exp::~Ss3exp() 
{

}

int Ss3exp::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *Ss3exp::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return _T("ss3");
}

const TCHAR *Ss3exp::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return _T("Realspace for Samsung SyncMaster3D");
}
	
const TCHAR *Ss3exp::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return _T("Realspace for Samsung");
}

const TCHAR *Ss3exp::AuthorName()
{			
	//TODO: Return ASCII Author name
	return _T("dubble@maietgames.com");
}

const TCHAR *Ss3exp::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return _T("(c) MAIET Entertainment");
}

const TCHAR *Ss3exp::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *Ss3exp::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int Ss3exp::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void Ss3exp::ShowAbout(HWND hWnd)
{			
	// Optional
}

int	Ss3exp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)	// Export file
{
	//TODO: Implement the actual file import here and 
	//		return TRUE If the file is imported properly

	if(!suppressPrompts)
	{
		if(!DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				Ss3expOptionsDlgProc, (LPARAM)this))
				return true;
	}


	DWORD start,end;

	start=timeGetTime();
	BEInitLog(NULL,hInstance);
	log("Export Started ( %d )\n",start);
	Exporter exporter;
	RSMObject rsm;
	exporter.DoExport(name,i,&rsm);
	log("Model exported.\n");
	rsm.Optimize(bVertex);
	log("Model Optimized.\n");

	if(bAnimation)
	{
		log("Constructing Animation.\n");
		AnimConstructor *ac=new AnimConstructor;
		ac->BuildAnimation(i,"default",1.0f,bVertex);
		rsm.AddAnimation(ac);
		log("Animation built.\n");
	}

	FILE *file=fopen(name,"wb+");
	if(!file) return false;

	rsm.SaveRML(file);
	rsm.SaveRSM(file);
	rsm.SaveCameraInfo(file);

	fputc(bSky ? 1 : 0,file);
	fclose(file);
/*
	rsm.SaveRML("ss3test.rml");
	rsm.SaveRSM("ss3test.rsm");
*/
	end=timeGetTime();
	log("Export Ended ( %d ) %d seconds elapsed \n",end, (end-start)/1000);
	BECloseLog();
	return true;
}
	
