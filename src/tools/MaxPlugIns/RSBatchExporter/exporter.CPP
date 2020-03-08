#include "exporter.h"

char g_RMLFileName[256];

Exporter::Exporter()
{
}

Exporter::~Exporter()
{
}

int Exporter::DoExport(const TCHAR *name,Interface *i,RSMObject *rsm)
{
	// Grab the interface pointer.
	ip = i;
	Exporter::rsm=rsm;

	nTotalNodeCount = 0;
	nCurNode = 0;
	PreProcess(ip->GetRootNode(), nTotalNodeCount);
	

	ExportGlobalInfo();
	ExportMaterialList();

	int numChildren = ip->GetRootNode()->NumberOfChildren();
	
	// Call our node enumerator.
	// The nodeEnum function will recurse into itself and 
	// export each object found in the scene.
	
	for (int idx=0; idx<numChildren; idx++) {
		nodeEnum(ip->GetRootNode()->GetChildNode(idx), 0);
	}

	// We're done. Finish the progress bar.
	ip->ProgressEnd();

	return 1;
}

BOOL Exporter::nodeEnum(INode* node, int indentLevel) 
{
	nCurNode++;
	ObjectState os = node->EvalWorldState(0); 
	if (os.obj) {
		switch(os.obj->SuperClassID()) {
		case GEOMOBJECT_CLASS_ID: 
			ExportGeomObject(node, indentLevel); 
			break;
		case CAMERA_CLASS_ID:
			ExportCameraObject(node, indentLevel); 
			break;
/*		case LIGHT_CLASS_ID:
			if (GetIncludeObjLight()) ExportLightObject(node, indentLevel); 
			break;
		case SHAPE_CLASS_ID:
			if (GetIncludeObjShape()) ExportShapeObject(node, indentLevel); 
			break;
		case HELPER_CLASS_ID:
			if (GetIncludeObjHelper()) ExportHelperObject(node, indentLevel); 
			break;
*/		}
	}
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c), indentLevel))
			return FALSE;
	}
	
	return TRUE;
}


void Exporter::PreProcess(INode* node, int& nodeCount)
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
