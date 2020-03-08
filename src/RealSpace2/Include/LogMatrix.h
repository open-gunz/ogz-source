#pragma once

#include "RTypes.h"
#include "MDebug.h"

inline void LogMatrix(const rmatrix& mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			MLog("%f ", mat(i, j));
		}

		MLog("\n");
	}
}

#ifdef _DEBUG
inline void DLogMatrix(const rmatrix& mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			DMLog("%f ", mat(i, j));
		}

		DMLog("\n");
	}
}
#else
inline void DLogMatrix(...) {}
#endif