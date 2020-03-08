/**********************************************************************
 *<
	FILE: MCplug2.h

	DESCRIPTION:	max-plug Utility

	CREATED BY: elhorus

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __MCPLUG2__H
#define __MCPLUG2__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"
#include "polyobj.h"
#include "asciitok.h"

#include "el_mesh.h"

extern ClassDesc2* GetMCplug2Desc();
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define VERSION			200					// Version number * 100
#define CFGFILENAME		_T("mcp.CFG")		// Configuration file

class MtlKeeper 
{
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

class MCplug2 : public SceneExport 
{
public:
	static HWND hParams;
		
	MCplug2();
	~MCplug2();	

	int				ExtCount();				// Number of extensions supported 
	const TCHAR *	Ext(int n);				// Extension #n (i.e. "ASC")
	const TCHAR *	LongDesc();				// Long ASCII description (i.e. "Ascii Export") 
	const TCHAR *	ShortDesc();			// Short ASCII description (i.e. "Ascii")
	const TCHAR *	AuthorName();			// ASCII Author name
	const TCHAR *	CopyrightMessage();		// ASCII Copyright message 
	const TCHAR *	OtherMessage1();		// Other message #1
	const TCHAR *	OtherMessage2();		// Other message #2
	unsigned int	Version();				// Version number * 100 (i.e. v3.01 = 301) 
	void			ShowAbout(HWND hWnd);	// Show DLL's "About..." bowx

	BOOL			SupportsOptions(int ext, DWORD options);

	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0); // Export	file

	Point3			GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);


	//////////////////////////////////////////////////////

	BOOL	export_all_object(INode* node);
	void	get_mtrl_list(INode* node, int& nodeCount);

	////////////////////////////////////////////////////////////////////

	void	export_info();
	void	export_mtrl_list();
	void	export_object_list(INode* node); 

	void	export_meshani(INode* node,mesh_data* mesh_node);
	void	export_meshani_sub(INode* node,TimeValue t,mesh_data* mesh_node,int cnt);

	////////////////////////////////////////////////////////////////////

	void	export_mesh(INode* node, TimeValue t, mesh_data* mesh_node); 
	void	export_anikey(INode* node, mesh_data* mesh_node);
	void	export_physique( ObjectState *os, INode *node , mesh_data* mesh_node);

	void	DumpMaterial(Mtl* mtl, int mtlID, int subNo);
	void	DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt,mtrl_data* mtrl_node);
	void	DumpPosSample(INode* node, mesh_data* mesh_node); 
	void	DumpRotSample(INode* node, mesh_data* mesh_node); 
	void	DumpScaleSample(INode* node, mesh_data* mesh_node);
	void	DumpVisSample(INode* node, mesh_data* mesh_node);

	void	DumpTMSample(INode* node, mesh_data* mesh_node);
	void	DumpVertexAni(INode* node, mesh_data* mesh_node);

	TCHAR*	GetMapID(Class_ID cid, int subNo);
//	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh); 
	void	make_face_uv(Face *f, Point3 *tv);
	BOOL	TMNegParity(Matrix3 &m);
	TCHAR*	FixupName(TCHAR* name);
	void	CommaScan(TCHAR* buf);
	
	BOOL		 CheckForAnimation(INode* node, BOOL& pos, BOOL& rot, BOOL& scale);
	TriObject*	 GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	PatchObject* GetPatchObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	PolyObject*	 GetPolyObjectFromNode(INode *node, TimeValue t, int &deleteIt);

	BOOL	IsKnownController(Control* cont);

	TSTR	Format(int value);
	TSTR	Format(float value);
	TSTR	Format(Color value);
	TSTR	Format(Point3 value); 
	TSTR	Format(AngAxis value); 
	TSTR	Format(Quat value);
	TSTR	Format(ScaleValue value);

	TSTR	GetCfgFilename();
//	BOOL	ReadConfig();
//	void	WriteConfig();
	
	inline BOOL	GetAlwaysSample()			{ return bAlwaysSample; }
	inline int	GetMeshFrameStep()			{ return nMeshFrameStep; }
	inline int	GetKeyFrameStep()			{ return nKeyFrameStep; }
	inline int	GetPrecision()				{ return nPrecision; }
	inline TimeValue GetStaticFrame()		{ return nStaticFrame; }
	inline Interface*	GetInterface()		{ return ip; }

	inline void	SetAlwaysSample(BOOL val)			{ bAlwaysSample = val; }
	inline void SetMeshFrameStep(int val)			{ nMeshFrameStep = val; }
	inline void SetKeyFrameStep(int val)			{ nKeyFrameStep = val; }
	inline void SetPrecision(int val)				{ nPrecision = val; }
	inline void SetStaticFrame(TimeValue val)		{ nStaticFrame = val; }


	////////////////////////////////////////////////////////

	inline BOOL get_debug_bip_mesh_out()	{ return m_is_debug_bip_mesh_out;}
	inline BOOL get_debug_text_out()		{ return m_is_debug_text_out;}
	inline BOOL get_mesh_out()				{ return m_is_mesh_out;}
	inline BOOL get_ani_out()				{ return m_is_ani_out;}
	inline BOOL get_vertex_ani_out()		{ return m_is_vertex_ani_out;}
	inline BOOL get_tm_ani_out()			{ return m_is_tm_ani_out;}
	
	inline void set_debug_bip_mesh_out(BOOL v)	{ m_is_debug_bip_mesh_out=v;}
	inline void set_debug_text_out(BOOL v)		{ m_is_debug_text_out=v;}
	inline void set_mesh_out(BOOL v)			{ m_is_mesh_out=v;}
	inline void set_ani_out(BOOL v)				{ m_is_ani_out=v;}
	inline void set_vertex_ani_out(BOOL v)		{ m_is_vertex_ani_out=v;}
	inline void set_tm_ani_out(BOOL v)			{ m_is_tm_ani_out=v;}


private:


	BOOL		exportSelected;

	BOOL		bAlwaysSample;
	int			nMeshFrameStep;
	int			nKeyFrameStep;
	int			nPrecision;
 	TimeValue	nStaticFrame;

	int			m_nBeginFrame;


	///////////////////////////////////////////////////////////////


	BOOL	m_is_debug_text_out;
	BOOL	m_is_debug_bip_mesh_out;
	BOOL	m_is_mesh_out;
	BOOL	m_is_ani_out;
	BOOL	m_is_vertex_ani_out;
	BOOL	m_is_tm_ani_out;

	Interface*	ip;

	FILE*		pStream;
	FILE*		pDebug;

	char		FileName[80];

	int			nTotalNodeCount;
	int			nCurNode;
	TCHAR		szFmtStr[16];

	el_mesh		m_mesh_list;

	MtlKeeper	mtlList;
};

#endif // __MCPLUG2__H
