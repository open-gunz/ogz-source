
#ifndef __ASCIIEXP__H
#define __ASCIIEXP__H

#include "rsbspexporter.h"

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"

//#include "asciitok.h"

extern ClassDesc* GetRbsExpDesc();
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define VERSION			100			// Version number * 100

class MtlKeeper {
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

// This is the main class for the exporter.

class RbsExp : public SceneExport {
public:
	RbsExp();
	~RbsExp();

	// SceneExport methods
	int    ExtCount();     // Number of extensions supported 
	const TCHAR * Ext(int n);     // Extension #n (i.e. "ASC")
	const TCHAR * LongDesc();     // Long ASCII description (i.e. "Ascii Export") 
	const TCHAR * ShortDesc();    // Short ASCII description (i.e. "Ascii")
	const TCHAR * AuthorName();    // ASCII Author name
	const TCHAR * CopyrightMessage();   // ASCII Copyright message 
	const TCHAR * OtherMessage1();   // Other message #1
	const TCHAR * OtherMessage2();   // Other message #2
	unsigned int Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void	ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file

	
	BOOL	ExportObject(INode *node,char* MapName);	// elu 로 익스포트한다.

	// Node enumeration
	BOOL	nodeEnum(INode* node,int nDepth=0);
	void	PreProcess(INode* node, int& nodeCount);

	// High level export
	void	ExportGlobalInfo();
	void	ExportMaterialList();
	void	ExportGeomObject(INode* node); 
	void	ExportLightObject(INode* node); 
	void	ExportHelperObject(INode* node);
	void	ExportShapeObject(INode* node);

	// 이거 충돌용 map 을 생성하기 위한 펑션
	//void	ExportMesh_Extrude(INode* node, TimeValue t);
	//void	ExportMesh_CSG(INode* node, TimeValue t);

	// Mid level export
	void	ExportMesh(INode* node, TimeValue t); 
//	void	ExportAnimMesh(INode* node); 
//	void	ExportIKJoints(INode* node);
	void	ExportNodeTM(INode* node);
	void	ExportNodeHeader(INode* node, TCHAR* type);
//	void	ExportCameraSettings(CameraState* cs, CameraObject* cam, TimeValue t);
//	void	ExportLightSettings(LightState* ls, GenLight* light, TimeValue t);
	void	ExportNavigationMesh(INode* node, TimeValue t);		// ExportMesh에서 호출함

	// Low level export
	void	DumpPoly(PolyLine* line, Matrix3 tm);
	void	DumpMatrix3(Matrix3* m);
	void	DumpMaterial(Mtl* mtl, int mtlID, int subNo);
	void	DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt);
	void	DumpJointParams(JointParams* joint);
	void	DumpUVGen(StdUVGen* uvGen); 

	// Misc methods
//	TCHAR*	GetMapID(Class_ID cid, int subNo);
	void	SelectNode(INode *node);
	BOOL	CheckForAnimation(INode *node);
	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh); 
	BOOL	TMNegParity(Matrix3 &m);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

	// A collection of overloaded value to string converters.

	inline TimeValue GetStaticFrame()		{ return nStaticFrame; }
	inline Interface*	GetInterface()		{ return ip; }

private:
	int m_nRecommendedDepth;
	void GetRecommendedDepth(dpoint bbsize);

	TimeValue	nStaticFrame;

	Interface*	ip;
	int			nTotalNodeCount;
	int			nCurNode;
	TCHAR		szFmtStr[16];
	MtlKeeper	mtlList;

	RSBspExporter	*rsm;

	bool m_bExtrudeTest;
};

#endif // __ASCIIEXP__H

