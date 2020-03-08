#ifndef _MPROFILER_H
#define _MPROFILER_H

#include <stack>
#include <list>
#include "GlobalTypes.h"

#define MPROFILE_ITEM_NAME_LENGTH	64

// One Profile Item
struct MPROFILEITEM{
	char	szName[MPROFILE_ITEM_NAME_LENGTH];
	u64		nStartTime;
	u64		nEndTime;
};

// Accumulated Profile Log
struct MPROFILELOG{
	char	szName[MPROFILE_ITEM_NAME_LENGTH];
	int		nCount;
	int		nDepth;
	u64		nTotalTime;
	u64		nMaxTime;
	u64		nMinTime;
};

// Temporary Profile Call Stack
class MProfileStack : public std::stack<MPROFILEITEM*>{
public:
	virtual ~MProfileStack();
};

// One Loop Log
class MProfileLoop : public std::list<MPROFILELOG*>{
public:
	void AddProfile(MPROFILELOG* pPL);
	int GetTotalTime();
};


// Profiler
class MProfiler{
protected:
	MProfileStack	m_ProfileStack;
	MProfileLoop	m_ProfileLoop;

	bool			m_bEnableOneLoopProfile;
	MProfileLoop*	m_pOneLoopProfile;
	MProfileLoop*	m_pOneLoopProfileResult;

	char*			m_szFirstProfileName;

public:
	MProfiler();
	virtual ~MProfiler();

	void BeginProfile(char* szProfileName);
	void EndProfile(char* szProfileName);

	bool FinalAnalysis(char* szFileName);

	int GetTotalTime(void);

	// One Loop Profiling
	void EnableOneLoopProfile(bool bEnable);
	bool IsOneLoopProfile(void);
	// if not enabled, return NULL
	MProfileLoop* GetOneLoopProfile();

	// Accumulated Profile Result
	MProfileLoop* GetProfile();
};


// Global Profiler
extern MProfiler	g_DefaultProfiler;

// Instance ÇüÅÂÀÇ Profile
class MProfileInstance{
	char	m_szProfileName[MPROFILE_ITEM_NAME_LENGTH];
public:
	MProfileInstance(char* szProfileName){
		g_DefaultProfiler.BeginProfile(szProfileName);
		strcpy_safe(m_szProfileName, szProfileName);
	}
	virtual ~MProfileInstance(){
		g_DefaultProfiler.EndProfile(m_szProfileName);
	}
};


#ifdef _DO_NOT_USE_PROFILER

#define BEGINPROFILE(_szProfileName)	;
#define ENDPROFILE(_szProfileName)		;
#define FINALANALYSIS(_szFileName)		;
#define RUNWITHPROFILER(_Function)		_Function;
#define ENABLEONELOOPPROFILE(_bEnable)	;
#define GETONELOOPPROFILE()	0;
#define GETPROFILE();
#define PROFILEINSTANCE(_szProfileName)	;

#else

// Macro for Global Profiler
#define BEGINPROFILE(_szProfileName)	g_DefaultProfiler.BeginProfile(_szProfileName)
#define ENDPROFILE(_szProfileName)		g_DefaultProfiler.EndProfile(_szProfileName)
#define FINALANALYSIS(_szFileName)		g_DefaultProfiler.FinalAnalysis(_szFileName)
#define RUNWITHPROFILER(_Function)		BEGINPROFILE(#_Function); _Function; ENDPROFILE(#_Function);
#define ENABLEONELOOPPROFILE(_bEnable)	g_DefaultProfiler.EnableOneLoopProfile(_bEnable)
#define GETONELOOPPROFILE()				g_DefaultProfiler.GetOneLoopProfile()
#define GETPROFILE()					g_DefaultProfiler.GetProfile()

#define PROFILEINSTANCE(_szProfileName)	MProfileInstance __ProfileInstance(_szProfileName);

#endif

// Simple Macro
#define _BP(_szProfileName)				BEGINPROFILE(_szProfileName)
#define _EP(_szProfileName)				ENDPROFILE(_szProfileName)
#define _RP(_Function)					RUNWITHPROFILER(_Function)
#define _FA(_szFileName)				FINALANALYSIS(_szFileName)
#define _PI(_szProfileName)				PROFILEINSTANCE(_szProfileName)


#endif
