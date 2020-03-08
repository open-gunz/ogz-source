#pragma once

#include "MUtil.h"
#include "RTypes.h"
#include "RealSpace2.h"
#include "RUtil.h"
#include <vector>

namespace RealSpace2 {

template <typename VertexType, typename IndexType, u32 FVF>
class SimpleMesh
{
public:
	template <size_t NumVerts, size_t NumIdx>
	void Create(const VertexType(&Vertices)[NumVerts], const IndexType(&Indices)[NumIdx])
	{
		Create(Vertices, NumVerts, Indices, NumIdx);
	}

	void Create(const VertexType* Vertices, size_t NumVerts, const IndexType* Indices, size_t NumIdx)
	{
		NumVertices = NumVerts;
		NumIndices = NumIdx;

		auto VBSizeInBytes = sizeof(VertexType) * NumVertices;

		if (FAILED(RGetDevice()->CreateVertexBuffer(VBSizeInBytes, 0,
			FVF, D3DPOOL_MANAGED,
			MakeWriteProxy(VB), nullptr)))
			return;

		VertexType* vptr{};
		if (FAILED(VB->Lock(0, 0, reinterpret_cast<void**>(&vptr), 0)))
			return;

		memcpy(vptr, Vertices, VBSizeInBytes);

		if (FAILED(VB->Unlock()))
			return;

		auto IBSizeInBytes = sizeof(IndexType) * NumIndices;

		if (FAILED(RGetDevice()->CreateIndexBuffer(IBSizeInBytes, 0,
			GetD3DFormat<IndexType>(), D3DPOOL_MANAGED,
			MakeWriteProxy(IB), nullptr)))
			return;

		IndexType* iptr{};
		if (FAILED(IB->Lock(0, 0, reinterpret_cast<void**>(&iptr), 0)))
			return;

		memcpy(iptr, Indices, IBSizeInBytes);

		if (FAILED(IB->Unlock()))
			return;
	}

	bool Draw()
	{
		if (!VB || !IB)
			return false;

		RGetDevice()->SetFVF(FVF);
		RGetDevice()->SetStreamSource(0, VB.get(), 0, sizeof(VertexType));
		RGetDevice()->SetIndices(IB.get());

		RGetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumVertices, 0, NumIndices / 3);

		return true;
	}

	void Destroy()
	{
		VB.reset();
		IB.reset();
		NumVertices = 0;
		NumIndices = 0;
	}

private:
	D3DPtr<struct IDirect3DVertexBuffer9> VB;
	D3DPtr<struct IDirect3DIndexBuffer9> IB;
	size_t NumVertices, NumIndices;
};

template <typename VertexType, typename IndexType, u32 FVF, size_t NumVertices, size_t NumIndices>
SimpleMesh<VertexType, IndexType, FVF> CreateSimpleMesh(
	const VertexType(&Vertices)[NumVertices], const IndexType(&Indices)[NumIndices])
{
	return SimpleMesh<VertexType, IndexType, FVF>{Vertices, Indices};
}

struct SimpleVertex
{
	v3 pos;
};

using SphereMesh = SimpleMesh<SimpleVertex, u16, D3DFVF_XYZ>;

inline SphereMesh CreateSphere(float Radius, int Slices, int Stacks)
{
	std::vector<SimpleVertex> Vertices;
	std::vector<u16> Indices;

	auto AddTriangle = [&](auto&& v1, auto&& v2, auto&& v3) {
		for (size_t i{}; i < 3; ++i)
			Indices.emplace_back(static_cast<u16>(Vertices.size() + i));
		assert(Vertices.size() < USHRT_MAX);
		for (auto&& v : { v1, v2, v3 })
			Vertices.emplace_back(SimpleVertex{ v });
	};

	for (int i{}; i < Stacks; ++i)
	{
		float theta1 = (float(i) / Stacks) * PI_FLOAT;
		float theta2 = (float(i + 1) / Stacks) * PI_FLOAT;
		for (int j{}; j < Slices; ++j)
		{
			float phi1 = (float(j) / Slices) * 2 * PI_FLOAT;
			float phi2 = (float(j + 1) / Slices) * 2 * PI_FLOAT;

			auto MakeVertex = [&](auto&& theta, auto&& phi) {
				return v3{
					Radius * sin(theta) * cos(phi),
					Radius * sin(theta) * sin(phi),
					Radius * cos(phi),
				};
			};

			v3 v[] = {
				MakeVertex(theta1, phi1),
				MakeVertex(theta1, phi2),
				MakeVertex(theta2, phi2),
				MakeVertex(theta2, phi1),
			};

			if (i == 0)
			{
				AddTriangle(v[0], v[2], v[3]);
			}
			else if (i + 1 == Stacks)
			{
				AddTriangle(v[2], v[0], v[1]);
			}
			else
			{
				AddTriangle(v[0], v[1], v[3]);
				AddTriangle(v[1], v[2], v[3]);
			}
		}
	}

	SphereMesh Mesh;
	Mesh.Create(Vertices.data(), Vertices.size(), Indices.data(), Indices.size());
	return Mesh;
}

}