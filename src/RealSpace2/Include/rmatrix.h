#pragma once

typedef struct _D3DMATRIX D3DMATRIX;
struct D3DXMATRIX;

struct rmatrix
{
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;
		};
		float m[4][4];
	};

	rmatrix() = default;
	rmatrix(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44) :
		_11{ _11 }, _12{ _12 }, _13{ _13 }, _14{ _14 },
		_21{ _21 }, _22{ _22 }, _23{ _23 }, _24{ _24 },
		_31{ _31 }, _32{ _32 }, _33{ _33 }, _34{ _34 },
		_41{ _41 }, _42{ _42 }, _43{ _43 }, _44{ _44 }
	{}

	float & operator () (int row, int col) { return m[row][col]; }
	float operator () (int row, int col) const { return m[row][col]; }

	operator float* () { return reinterpret_cast<float*>(this); }
	operator const float* () const { return reinterpret_cast<const float*>(this); }

	rmatrix(const D3DMATRIX&);
	rmatrix(const D3DXMATRIX&);

	explicit operator D3DMATRIX* ();
	explicit operator const D3DMATRIX* () const;
	explicit operator D3DXMATRIX* ();
	explicit operator const D3DXMATRIX* () const;

	rmatrix& operator +=(const rmatrix& rhs) {
		for (size_t i{}; i < 4; ++i)
			for (size_t j{}; j < 4; ++j)
				m[i][j] += rhs.m[i][j];
		return *this;
	}
	
	rmatrix& operator -=(const rmatrix& rhs) {
		for (size_t i{}; i < 4; ++i)
			for (size_t j{}; j < 4; ++j)
				m[i][j] -= rhs.m[i][j];
		return *this;
	}

	rmatrix& operator *=(const rmatrix& rhs) {
		rmatrix ret;
		for (size_t i{}; i < 4; ++i)
		{
			for (size_t j{}; j < 4; ++j)
			{
				ret.m[i][j] = 0;
				for (size_t k{}; k < 4; ++k)
					ret.m[i][j] += m[i][k] * rhs.m[k][j];
			}
		}
		*this = ret;
		return *this;
	}

	rmatrix& operator *=(float rhs) {
		for (size_t i{}; i < 4; ++i)
			for (size_t j{}; j < 4; ++j)
				m[i][j] *= rhs;
		return *this;
	}

	rmatrix& operator /=(float rhs) {
		auto inv = 1.0f / rhs;
		for (size_t i{}; i < 4; ++i)
			for (size_t j; j < 4; ++j)
				m[i][j] *= inv;
		return *this;
	}

	rmatrix operator +() const {
		return *this;
	}
	rmatrix operator -() const {
		rmatrix ret;
		for (size_t i{}; i < 4; ++i)
			for (size_t j{}; j < 4; ++j)
				ret(i, j) = -(*this)(i, j);
		return ret;
	}
};

#define NONMEMBER_OP(op, rhs_type) \
inline rmatrix operator op(const rmatrix& a, rhs_type b) { \
	auto ret = a; ret op##= b; return ret; }

#define NONMEMBER_MATRIX_OP(op) NONMEMBER_OP(op, const rmatrix&)
#define NONMEMBER_FLOAT_OP(op) NONMEMBER_OP(op, float)

NONMEMBER_MATRIX_OP(+)
NONMEMBER_MATRIX_OP(-)
NONMEMBER_MATRIX_OP(*)
NONMEMBER_FLOAT_OP(*)
NONMEMBER_FLOAT_OP(/)

#undef NONMEMBER_OP
#undef NONMEMBER_MATRIX_OP
#undef NONMEMBER_FLOAT_OP