//* rbsexp.cpp	- Ascii File Exporter

#include "stdafx.h"
#include "rbsexp.h"
#include "DialogProc.h"
#include "belog.h"
#include "FileInfo.h"
#include "RToken.h"
#include "mmsystem.h"

#include "MDebug.h"

#pragma comment( lib, "core.lib" )
#pragma comment( lib, "geom.lib" )
#pragma comment( lib, "mesh.lib" )
#pragma comment( lib, "maxutil.lib" )
#pragma comment( lib, "winmm.lib" )

HINSTANCE hInstance;
int controlsInit = FALSE;

static char g_FileName[256];

// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
#define RBSEXP_CLASS_ID	Class_ID(0x28a910ad, 0x7edd2489)

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	// Initialize the custom controls. This should be done only once.
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
	case 0: return GetRbsExpDesc();
	default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

class RbsExpClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new RbsExp; } 
	const TCHAR*	ClassName() { return GetString(IDS_ASCIIEXP); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		ClassID() { return RBSEXP_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
};

static RbsExpClassDesc RbsExpDesc;

ClassDesc* GetRbsExpDesc()
{
	return &RbsExpDesc;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}

RbsExp::RbsExp()
{
	nStaticFrame = 0;

}

RbsExp::~RbsExp()
{
}

int RbsExp::ExtCount()
{
	return 1;
}

const TCHAR * RbsExp::Ext(int n)
{
	switch(n) {
	case 0:
		// This cause a static string buffer overwrite
//		return GetString(IDS_EXTENSION1);
		return _T("rs");
	}
	return _T("");
}

const TCHAR * RbsExp::LongDesc()
{
	return GetString(IDS_LONGDESC);
}

const TCHAR * RbsExp::ShortDesc()
{
	return GetString(IDS_SHORTDESC);
}

const TCHAR * RbsExp::AuthorName() 
{
	return _T("dubble");
}

const TCHAR * RbsExp::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR * RbsExp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * RbsExp::OtherMessage2() 
{
	return _T("");
}

unsigned int RbsExp::Version()
{
	return 100;
}

void RbsExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

int g_nObjectCount;

void RbsExp::SelectNode(INode *node)
{
	char *name=node->GetName();

	ip->SelectNode(node,0);

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		SelectNode(node->GetChildNode(c));
	}
}

BOOL RbsExp::ExportObject(INode *node,char* MapName)
{
#define ASCIIEXP_CLASS_ID	Class_ID(0x85548e0b, 0x4a26450c)
#define MCPLUG2_CLASS_ID	Class_ID(0x501e02c0, 0x1a538efa)

	if (ip->GetCancel())
		return FALSE;

//	Class_ID cid=ASCIIEXP_CLASS_ID;
	Class_ID cid=MCPLUG2_CLASS_ID;

	ObjectState os = node->EvalWorldState(0); 
	_ASSERT(os.obj);
//	_ASSERT(os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID);

	char filename[256];

	sprintf(filename,"%s_%s.elu",MapName,node->GetName());
//	sprintf(filename,"%s.elu",node->GetName());
//	sprintf(filename,"test.ELU");

	log("Export Objects : %s \n",filename);

	theHold.Begin();
	ip->ClearNodeSelection(FALSE);
	SelectNode(node);
	
//	BOOL ret=ip->ExportToFile(filename,FALSE,0,&cid);
	BOOL ret=ip->ExportToFile(filename,TRUE,SCENE_EXPORT_SELECTED,&cid);
	_ASSERT(ret);

	theHold.Cancel();

	/*
	TSTR undostr; undostr.printf("Select");
	theHold.Accept(undostr);
*/

	//Begin User Defined Properties Export
	int nItem = 0;

	RDUMMY *pdm = new RDUMMY;
	pdm->name	= node->GetName();

	int nPropValue = 0;
	float fPropValue = 0.f;
	char Buffer[256];
	TSTR cstrBuffer;

	// Flag일 경우
	const char* cstrFlag = "obj_flag";

	if( ( strnicmp( pdm->name.c_str(), cstrFlag, sizeof(cstrFlag) ) == 0 ) )
	{
		//파일이름
		pdm->SetUserPropValue(RTOK_FLAG_NAME, filename );

		if( node->GetUserPropInt("direction", nPropValue) )
			pdm->SetUserPropValue(RTOK_FLAG_DIRECTION,nPropValue);
		if( node->GetUserPropFloat("power", fPropValue) )
			pdm->SetUserPropValue(RTOK_FLAG_POWER, fPropValue);


		if( node->GetUserPropInt("restriction", nPropValue) )
		{
			pdm->SetUserPropValue(RTOK_RESTRICTION,nPropValue);
			int nRestriction = nPropValue;
			for( int i = 0 ; i < nRestriction; ++i )
			{
				sprintf( Buffer, "%s%d\0", "axis", i );
				if( node->GetUserPropString(Buffer,cstrBuffer) )
				{
					sprintf( Buffer, "%s%d\0", RTOK_RESTRICTION_AXIS, i );
					if( cstrBuffer[0] == 'x' ) 				pdm->SetUserPropValue(Buffer,0);
					else if( cstrBuffer[0] == 'y' ) 		pdm->SetUserPropValue(Buffer,1);
					else /*if( Buffer2[0] == 'z' )*/ 	pdm->SetUserPropValue(Buffer,2);
				}
				sprintf( Buffer, "%s%d\0", "pos", i );
				if( node->GetUserPropFloat(Buffer, fPropValue) )
					sprintf( Buffer, "%s%d\0", RTOK_RESTRICTION_POSITION, i );
					pdm->SetUserPropValue(Buffer, fPropValue);
				sprintf( Buffer, "%s%d\0", "compare", i );
				if( node->GetUserPropString(Buffer, cstrBuffer) )
				{
					sprintf( Buffer, "%s%d\0", RTOK_RESTRICTION_COMPARE, i );
					if(cstrBuffer[0] == 'g' || cstrBuffer[0] == 'b' )	pdm->SetUserPropValue(Buffer, 0);
					else if(cstrBuffer[0] == 'l' || cstrBuffer[0] == 's' )	pdm->SetUserPropValue(Buffer, 1);
				}
			}
		}
		if( node->GetUserPropInt("type", nPropValue) )
			pdm->SetUserPropValue(RTOK_WINDTYPE,nPropValue);
		if( node->GetUserPropInt("delay", nPropValue) )
			pdm->SetUserPropValue(RTOK_WINDDELAY,nPropValue);
	}

	rsm->m_DummyList.push_back( pdm );
	//End


	return TRUE;
}

#define MIN_NODE_SIZE			1500		// 이거보다 크면 쪼갠다. 단위는 cm
#define MAX_RECOMMENDED_DEPTH	10

void RbsExp::GetRecommendedDepth(dpoint bbsize)
{
	m_nRecommendedDepth=0;
	double maxlength;
	do {
		// 가장 큰 축을 찾는다..
		int nAxis =		bbsize.x > bbsize.y
			? bbsize.z > bbsize.x ? 2 : 0 
			: bbsize.z > bbsize.y ? 2 : 1;

		maxlength=bbsize[nAxis];
		bbsize[nAxis]*=.5f;

		m_nRecommendedDepth++;
	} while(maxlength>MIN_NODE_SIZE && m_nRecommendedDepth<MAX_RECOMMENDED_DEPTH);
}

void GetName(char* str_in,char* str_out)
{
	int len = strlen(str_in);

	int start = 0;
	int end = 0;

	end = len-3; // .rs

	for(int i=len-1; i>=0; i--) {
		if( (str_in[i]=='\\') || (str_in[i]=='/')) {
			start = i+1;
			break;
		}
	}

	memcpy(str_out,&str_in[start],end-start);
	str_out[end-start] = NULL;
}

// Start the exporter!
// This is the real entrypoint to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int RbsExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
	m_bExtrudeTest=true;

	char txtname[256];
	strcpy(txtname,"NUL");
	strcpy(g_FileName,name);

	// Grab the interface pointer.
	ip = i;

	BEInitLog(ip->GetMAXHWnd(),hInstance);
	log("Starting BSP Export...\n");
	
	rsm=new RSBspExporter;
	rsm->ip=i;

	GetName(g_FileName,rsm->MapName);

	// Startup the progress bar.
	ip->ProgressStart("Preparing ... ", TRUE, fn, NULL);

	// Get a total node count by traversing the scene
	// We don't really need to do this, but it doesn't take long, and
	// it is nice to have an accurate progress bar.
	nTotalNodeCount = 0;
	nCurNode = 0;
	PreProcess(ip->GetRootNode(), nTotalNodeCount);
	
	// First we write out a file header with global information. 
	ExportGlobalInfo();

	// Export list of material definitions
	ExportMaterialList();

	int numChildren = ip->GetRootNode()->NumberOfChildren();
	
	// Call our node enumerator.
	// The nodeEnum function will recurse into itself and 
	// export each object found in the scene.
	
	for (int idx=0; idx<numChildren; idx++) {
		if (ip->GetCancel())
			break;
		nodeEnum(ip->GetRootNode()->GetChildNode(idx));
	}

	// We're done. Finish the progress bar.

	rsm->Preprocessing();
	
	GetRecommendedDepth(rsm->m_bb.vmax-rsm->m_bb.vmin);
	g_nTreeDepth=m_nRecommendedDepth;

	if(!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_RBSEXPORT),
		ip->GetMAXHWnd(), RBSExportDlgProc, (LPARAM)this)){
			ip->ProgressEnd();
			BECloseLog();
			delete rsm;
			return 1;
		}

	time_t   start, finish;

	time( &start );

	char DescFileName[256];

	if(g_bExportObjects)
	{
		log("Export Objects.\n");

		for(NODELIST::iterator i=rsm->m_ObjectsList.begin();i!=rsm->m_ObjectsList.end();i++)
			ExportObject(*i,rsm->MapName);
	}

	if(g_bExportXML)
	{
		log("Export description.. \n");

		sprintf(DescFileName,"%s.xml",name);
		if(!rsm->SaveDesc(DescFileName))
			log("Error while save description\n");

		if (!rsm->SaveSpawn("spawn.xml"))
			log("Error while save spawn\n");

		if(!rsm->SaveFlag("flag.xml"))
			log("Error while save flag\n");

		if(!rsm->SaveSmoke("smoke.xml"))
			log("Error while save smoke\n");
	}

	ip->ProgressEnd();
	ip->ProgressStart("Export Objects ... ", TRUE, fn, NULL);

	if (ip->GetCancel())
		return FALSE;

	ip->ProgressEnd();
	ip->ProgressStart("Construct Bsp ... ", TRUE, fn, NULL);

	if(g_bExportRBS)
	{
		if(rsm->SaveRS(name))
			log("%s file saved\n",name);
		else
		{
			log("Error while save %s file. error code = %d \n",name, GetLastError());
		}

		// bsp 파일을 기록한다
		char BspFileName[256];
		sprintf(BspFileName,"%s.bsp",name);

		if(rsm->SaveBSP(BspFileName))
			log("%s file saved\n",BspFileName);
		else
			log("Error while save %s file\n",BspFileName);

		{
			char ColBspFileName[256];
			sprintf(ColBspFileName,"%s.col",name);

			if(rsm->SaveCol(ColBspFileName))
				log("%s file saved\n",ColBspFileName);
			else
				log("Error while save %s file\n",ColBspFileName);
		}


		// 퀘스트용 Navigation mesh를 기록한다.
		char szNavFileName[256];
		sprintf(szNavFileName, "%s.nav", name);

		rsm->SaveNavigation(szNavFileName);
	}

	delete rsm;

	ip->ProgressEnd();

	time( &finish );

	double elapsed_time = difftime( finish, start );
	log( "Exporting takes %d minutes and %6.0f seconds.\n", int(elapsed_time / 60.f) , fmod(elapsed_time,60.) );

	log("Done.\n");
	BECloseLog();

	sndPlaySound("done.wav",SND_ASYNC);
/*
	Beep(1000,200);
	Beep(1100,200);
	Beep(1200,200);
	Beep(1300,200);
	Beep(1400,200);
*/
	return 1;
}


#define ALMOST_ZERO 1.0e-3f

// Not truly the correct way to compare floats of arbitary magnitude...
BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.y - p2.y) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.z - p2.z) > ALMOST_ZERO)
		return FALSE;

	return TRUE;
}

char g_nodepath[32][256];

BOOL RbsExp::CheckForAnimation(INode *node)
{
	BOOL bPos,bRot,bScale,bVis;

	bPos = bRot = bScale = bVis = FALSE;

	ObjectState os = node->EvalWorldState(0); 
	if(!os.obj || (os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID)) return FALSE;

	if(strnicmp(node->GetName(),RTOK_MAX_EXPORTOBJ,strlen(RTOK_MAX_EXPORTOBJ))==0)
		return TRUE;

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame();
	Matrix3 tm;
	AffineParts ap;
	Point3 firstPos;
	float rotAngle, firstRotAngle;
	Point3 rotAxis, firstRotAxis;
	Point3 firstScaleFactor;

	for (t=start; t<=end; t+=delta) {
		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		AngAxisFromQ(ap.q, &rotAngle, rotAxis);

		if (t != start) {
			if (!bPos) {
				if (!EqualPoint3(ap.t, firstPos)) {
					bPos = TRUE;
				}
			}
			if (!bRot) {
				if (fabs(rotAngle - firstRotAngle) > ALMOST_ZERO) {
					bRot = TRUE;
				}
				else if (!EqualPoint3(rotAxis, firstRotAxis)) {
					bRot = TRUE;
				}
			}

			if (!bScale) {
				if (!EqualPoint3(ap.k, firstScaleFactor)) {
					bScale = TRUE;
				}
			}

			Control* cont= node->GetVisController();
			if(cont)
			{
				float fVis,fVisStart;
				Interval range = cont->GetTimeRange(TIMERANGE_ALL);
				cont->GetValue(t,&fVis,range);
				cont->GetValue(start,&fVisStart,range);
				if(fVis!=fVisStart)
					bVis = TRUE;
			}
			

		}
		else {
			firstPos = ap.t;
			firstRotAngle = rotAngle;
			firstRotAxis = rotAxis;
			firstScaleFactor = ap.k;
		}

		if (bPos || bRot || bScale || bVis )
			return TRUE;
	}

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if(CheckForAnimation(node->GetChildNode(c)))
			return TRUE;
	}

	return FALSE;
}

BOOL RbsExp::nodeEnum(INode* node,int nDepth)
{
	strcpy(g_nodepath[nDepth],node->GetName());

	nCurNode++;
	ip->ProgressUpdate((int)((float)nCurNode/nTotalNodeCount*100.0f)); 

	if (ip->GetCancel())
		return FALSE;

	if(nDepth==0 && CheckForAnimation(node))
	{
		rsm->m_ObjectsList.push_back(node);
		return TRUE;
	}

	ObjectState os = node->EvalWorldState(0); 

	if (os.obj) {

		switch(os.obj->SuperClassID()) {
		case GEOMOBJECT_CLASS_ID: 
			ExportGeomObject(node); 
			break;
		case SHAPE_CLASS_ID:
			ExportShapeObject(node);
			break;
		case LIGHT_CLASS_ID:
			ExportLightObject(node); 
			break;
		case HELPER_CLASS_ID:
			ExportHelperObject(node); 
			break;
		}
	}
	
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c),nDepth+1))
			return FALSE;
	}

	return TRUE;
}


void RbsExp::PreProcess(INode* node, int& nodeCount)
{
	nodeCount++;
	
	// Add the nodes material to out material list
	// Null entries are ignored when added...
	mtlList.AddMtl(node->GetMtl());

	// For each child of this node, we recurse into ourselves 
	// and increment the counter until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		PreProcess(node->GetChildNode(c), nodeCount);
	}
}

BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
	if (!mtl) {
		return FALSE;
	}

	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return FALSE;
		}
	}
	mtlTab.Append(1, &mtl, 25);

	return TRUE;
}

int MtlKeeper::GetMtlID(Mtl* mtl)
{
	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return i;
		}
	}
	return -1;
}

int MtlKeeper::Count()
{
	return mtlTab.Count();
}

Mtl* MtlKeeper::GetMtl(int id)
{
	return mtlTab[id];
}

