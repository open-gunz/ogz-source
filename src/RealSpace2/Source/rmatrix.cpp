#include "stdafx.h"
#include "rmatrix.h"

rmatrix::rmatrix(const D3DMATRIX& mat) {
	memcpy(m, mat.m, sizeof(rmatrix));
	static_assert(sizeof(rmatrix) == sizeof(D3DMATRIX), "Wrong rmatrix size");
}

rmatrix::rmatrix(const D3DXMATRIX& mat) {
	memcpy(m, mat.m, sizeof(rmatrix));
	static_assert(sizeof(rmatrix) == sizeof(D3DXMATRIX), "Wrong rmatrix size");
}

rmatrix::operator D3DMATRIX* () { return reinterpret_cast<D3DMATRIX*>(&_11); }
rmatrix::operator const D3DMATRIX* () const { return reinterpret_cast<const D3DMATRIX*>(&_11); }

rmatrix::operator D3DXMATRIX* () { return reinterpret_cast<D3DXMATRIX*>(&_11); }
rmatrix::operator const D3DXMATRIX* () const { return reinterpret_cast<const D3DXMATRIX*>(&_11); }