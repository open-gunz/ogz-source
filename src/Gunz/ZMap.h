#pragma once
#include "ZFilePath.h"

template<size_t size>
void ZGetCurrMapPath(char(&outPath)[size]) {
	ZGetCurrMapPath(outPath, size);
}
void ZGetCurrMapPath(char* outPath, int maxlen);

bool InitMapSelectionWidget();