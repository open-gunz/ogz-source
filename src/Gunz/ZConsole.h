#ifndef _ZCONSOLE_H
#define _ZCONSOLE_H

#include "MConsoleFrame.h"

void CreateConsole(MCommandManager* pCM);
void DestroyConsole();
void OutputToConsole(const char* pFormat,...);
void ConsoleInputEvent(const char* szInputStr);


MConsoleFrame* ZGetConsole();

#endif