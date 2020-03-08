/**********************************************************************
 *<
	FILE: BatchExporter.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __BATCHEXPORTER__H
#define __BATCHEXPORTER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "utilapi.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

class BatchExporter : public UtilityObj {
	public:
		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		
		void DeleteThis() { }		
		//Constructor/Destructor
		BatchExporter();
		~BatchExporter();

		TSTR GetCfgFilename();
		void ReadConfig();
		void WriteConfig();
		void Do(HWND hWnd);
};

#endif // __BATCHEXPORTER__H
