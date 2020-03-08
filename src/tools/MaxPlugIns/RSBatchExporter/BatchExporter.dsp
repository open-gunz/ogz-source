# Microsoft Developer Studio Project File - Name="BatchExporter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=BatchExporter - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BatchExporter.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BatchExporter.mak" CFG="BatchExporter - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BatchExporter - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BatchExporter - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "BatchExporter - Win32 Hybrid" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Max PlugIns/RSBatchExporter", QGCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BatchExporter - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /I "C:\3dsmax3_1\Maxsdk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib cml.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /machine:I386 /out:"C:\3dsmax3_1\Plugins\BatchExporter.dlu" /libpath:"C:\3dsmax3_1\Maxsdk\lib" /release
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "c:\3dsmax3_1\plugins\BatchExporter.dlu"  "\\databank\TeamWorks\MAIET\TeamTools\3D Studio MAX plug-ins"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "BatchExporter - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /ZI /Od /I "C:\3dsmax3_1\Maxsdk\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib cmld.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1\Plugins\BatchExporter.dlu" /pdbtype:sept /libpath:"C:\3dsmax3_1\Maxsdk\lib"

!ELSEIF  "$(CFG)" == "BatchExporter - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BatchExporter___Win32_Hybrid"
# PROP BASE Intermediate_Dir "BatchExporter___Win32_Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BatchExporter___Win32_Hybrid"
# PROP Intermediate_Dir "BatchExporter___Win32_Hybrid"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MD /W3 /Gm /ZI /Od /I "C:\3dsmax3_1\Maxsdk\include" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib paramblk2.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1\Plugins\BatchExporter.dlu" /pdbtype:sept /libpath:"C:\3dsmax3_1\Maxsdk\lib"

!ENDIF 

# Begin Target

# Name "BatchExporter - Win32 Release"
# Name "BatchExporter - Win32 Debug"
# Name "BatchExporter - Win32 Hybrid"
# Begin Group "almost fixed"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BatchExporter.def
# End Source File
# Begin Source File

SOURCE=.\BatchExporter.rc
# End Source File
# Begin Source File

SOURCE=.\BELog.cpp
# End Source File
# Begin Source File

SOURCE=.\BELog.h
# End Source File
# Begin Source File

SOURCE=.\BEParser.cpp
# End Source File
# Begin Source File

SOURCE=.\BEParser.h
# End Source File
# Begin Source File

SOURCE=.\Dib.cpp
# End Source File
# Begin Source File

SOURCE=.\DllEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\Image24.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RSMaterialList.cpp
# End Source File
# Begin Source File

SOURCE=.\RSMaterialList.h
# End Source File
# Begin Source File

SOURCE=.\RSScanner.cpp
# End Source File
# Begin Source File

SOURCE=.\RSScanner.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AnimConstructor.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimConstructor.h
# End Source File
# Begin Source File

SOURCE=.\BatchExporter.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchExporter.h
# End Source File
# Begin Source File

SOURCE=.\BECommandProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\BECommandProcessor.h
# End Source File
# Begin Source File

SOURCE=.\BEWorkSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\BEWorkSheet.h
# End Source File
# Begin Source File

SOURCE=.\Export.cpp
# End Source File
# Begin Source File

SOURCE=.\exporter.CPP
# End Source File
# Begin Source File

SOURCE=.\exporter.h
# End Source File
# Begin Source File

SOURCE=.\OpenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RSMObject.cpp
# End Source File
# Begin Source File

SOURCE=.\RSMObject.h
# End Source File
# Begin Source File

SOURCE=.\rtexture.cpp
# End Source File
# Begin Source File

SOURCE=.\rtexture.h
# End Source File
# Begin Source File

SOURCE=.\rutils_max.cpp
# End Source File
# Begin Source File

SOURCE=.\rutils_max.h
# End Source File
# End Target
# End Project
