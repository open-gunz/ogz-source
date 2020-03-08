// 3dsmax plugin for realspace models by dubble since 2000.1.7

#include "BatchExporter.h"
#include "BECommandProcessor.h"
#include "BELog.h"
#include "BEWorkSheet.h"

#define BATCHEXPORTER_CLASS_ID	Class_ID(0x5888515c, 0xbd2d4f)

static BatchExporter theBatchExporter;
class BatchExporterClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theBatchExporter;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return BATCHEXPORTER_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("BatchExporter"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};
static BatchExporterClassDesc BatchExporterDesc;
ClassDesc2* GetBatchExporterDesc() {return &BatchExporterDesc;}

static BOOL CALLBACK BatchExporterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theBatchExporter.Init(hWnd);
			theBatchExporter.ReadConfig();
			break;

		case WM_DESTROY:
			theBatchExporter.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BUTTON_EXPORT:
					theBatchExporter.Do(hWnd);
					break;
			}
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theBatchExporter.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- BatchExporter -------------------------------------------------------
BatchExporter::BatchExporter()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

BatchExporter::~BatchExporter()
{

}

void BatchExporter::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		BatchExporterDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void BatchExporter::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void BatchExporter::Init(HWND hWnd)
{

}

void BatchExporter::Destroy(HWND hWnd)
{

}

static TCHAR rbefile[MAX_PATH];

TSTR BatchExporter::GetCfgFilename()
{
	TSTR filename;
	
	filename += ip->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "rsBatch.cfg";

	return filename;
}

void BatchExporter::ReadConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "r");
	if (!cfgStream) return;
	fgets(rbefile, MAX_PATH , cfgStream );
	fclose(cfgStream);
}

void BatchExporter::WriteConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "w+");
	if (!cfgStream) return;
	fputs(rbefile, cfgStream );
	fclose(cfgStream);
}

void BatchExporter::Do(HWND hWnd)
{
	BEWorkSheet_SetFileName(rbefile);
	if(BEWorkSheet_Work(hWnd)==BEWORKSHEET_OK)
	{
		BECommandList *pCommandList=BEWorkSheet_GetCommandList();
		BEInitLog(hWnd);
		log("starting batch processing containing %d works.\n",pCommandList->GetCount());
		DoBatchExport(ip,hWnd,BEWorkSheet_GetCommandList());
		strcpy(rbefile,BEWorkSheet_GetFileName());
		WriteConfig();
		log("Done.");
		BECloseLog();
	}
}