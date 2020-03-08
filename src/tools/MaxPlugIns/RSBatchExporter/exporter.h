
#ifndef __EXPORTER__H
#define __EXPORTER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"

#include "rsmobject.h"

class MtlKeeper {
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

// This is the main class for the exporter.

class MaxMaterial;
class MaxStdMaterial;

class Exporter{
public:
	Exporter();
	~Exporter();

	int		DoExport(const TCHAR *name,Interface *i,RSMObject *rsm);
	//,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE); // Export	file

	// Node enumeration
	BOOL	nodeEnum(INode* node, int indentLevel);
	void	PreProcess(INode* node, int& nodeCount);

	// High level export
	void	ExportGlobalInfo();
	void	ExportMaterialList();
	void	ExportGeomObject(INode* node, int indentLevel); 
	void	ExportCameraObject(INode* node, int indentLevel); 

	// Mid level export
	void	ExportMesh(INode* node, TimeValue t, int indentLevel); 
	void	ExportAnimKeys(INode* node, int indentLevel);
	void	ExportMaterial(INode* node, int indentLevel); 
	void	ExportAnimMesh(INode* node, int indentLevel); 
	void	ExportIKJoints(INode* node, int indentLevel);
	void	ExportNodeTM(INode* node, int indentLevel,rmatrix *mat);
	void	ExportLightSettings(LightState* ls, GenLight* light, TimeValue t, int indentLevel);

	inline BOOL	GetIncludeObjGeom()			{ return true; }
	inline TimeValue GetStaticFrame()		{ return 0; }
	void DumpMaterial(MaxMaterial *maxm,Mtl* mtl, int mtlID, int subNo, int indentLevel);
	void DumpTexture(MaxStdMaterial *stdm,Texmap* tex, Class_ID cid, int subNo, float amt, int indentLevel);
	void DumpUVGen(MaxStdMaterial *stdm,StdUVGen* uvGen, int indentLevel);
	TCHAR	*FixupName(TCHAR* name);
	void ExportNodeHeader(INode* node);

	// Misc methods
	TCHAR*	GetMapID(Class_ID cid, int subNo);
	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel); 
	void	make_face_uv(Face *f, Point3 *tv);
	BOOL	TMNegParity(Matrix3 &m);
	void	CommaScan(TCHAR* buf);
	BOOL	CheckForAnimation(INode* node, BOOL& pos, BOOL& rot, BOOL& scale);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	BOOL	IsKnownController(Control* cont);

private:
	Interface*	ip;
	int			nTotalNodeCount;
	int			nCurNode;
	TCHAR		szFmtStr[16];
	MtlKeeper	mtlList;

	RSMObject	*rsm;
};

#endif // __Exporter__H

