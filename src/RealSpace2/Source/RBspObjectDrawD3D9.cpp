#include "stdafx.h"
#include "RBspObjectDrawD3D9.h"
#include "RealSpace2.h"
#include <algorithm>
#include <numeric>
#include "RBspObject.h"
#include "ShaderGlobals.h"
#include "ShaderUtil.h"
#include "RS2.h"
#include "RUtil.h"

_NAMESPACE_REALSPACE2_BEGIN

struct MaterialBatch
{
	int TriangleCount;
	int VertexBase;
	int IndexBase;
};

using namespace rsx;
using IndexType = u32;

RBspObjectDrawD3D9::RBspObjectDrawD3D9(RBspObject& bsp) : bsp{ bsp }, dev{ RGetDevice() }
{
	CreateShaderStuff();

	if (UsingLighting())
		GetRenderer().UpdateRTs();
}

RBspObjectDrawD3D9::RBspObjectDrawD3D9(RBspObjectDrawD3D9&&) = default;
RBspObjectDrawD3D9::~RBspObjectDrawD3D9() = default;

void RBspObjectDrawD3D9::OnInvalidate(){}
void RBspObjectDrawD3D9::OnRestore(){}

void RBspObjectDrawD3D9::CreateTextures()
{
	std::unordered_map<std::string, int> TexMap;

	Materials.resize(State.Materials.size());
	for (size_t i{}; i < State.Materials.size(); ++i)
	{
		auto& EluMat = State.Materials[i];

		auto LoadTexture = [&TexMap, this](auto& name) -> int
		{
			if (name.empty())
				return -1;

			// Check if already loaded and use the same index if so.
			{
				auto it = TexMap.find(name);
				if (it != TexMap.end())
					return it->second;
			}

			auto* Tex = RCreateBaseTexture(name.c_str(), RTextureType::Map, true);
			if (!Tex)
			{
				MLog("RBspObjectDrawD3D9::CreateTextures -- Failed to load texture %s\n",
					name.c_str());
				return -1;
			}

			TextureMemory.emplace_back(Tex);

			auto idx = TextureMemory.size() - 1;
			TexMap.emplace(std::make_pair(name, idx));
			return idx;
		};

		auto& Mat = Materials[i];
		Mat.Diffuse = LoadTexture(EluMat.tDiffuse);
		Mat.Normal = LoadTexture(EluMat.tNormal);
		Mat.Specular = LoadTexture(EluMat.tSpecular);
		Mat.Opacity = LoadTexture(EluMat.tOpacity);
		Mat.Emissive = LoadTexture(EluMat.tEmissive);
		Mat.AlphaTestValue = EluMat.AlphaTestValue;
		Mat.TwoSided = EluMat.TwoSided;
		Mat.Shininess = EluMat.shininess;
	}
}

static D3DPtr<IDirect3DVertexDeclaration9> CreateUnlitVertexDecl()
{
	D3DVERTEXELEMENT9 Decl[] = {
		{ 0, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0,     D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	return CreateVertexDeclaration(Decl);
}

static D3DPtr<IDirect3DVertexDeclaration9> CreateLitVertexDecl()
{
	D3DVERTEXELEMENT9 Decl[] = {
		{ 0, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0,     D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 2, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 3, 0,     D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
		D3DDECL_END()
	};
	return CreateVertexDeclaration(Decl);
}

void RBspObjectDrawD3D9::CreateShaderStuff()
{
	UnlitVertexDecl = CreateUnlitVertexDecl();
	LitVertexDecl = CreateLitVertexDecl();
}

template <typename VertexType>
bool CreateVB(D3DPtr<IDirect3DVertexBuffer9>& ptr, size_t VertexCount, u32 FVF)
{
	auto hr = RGetDevice()->CreateVertexBuffer(VertexCount * sizeof(VertexType), 0,
		FVF, D3DPOOL_MANAGED,
		MakeWriteProxy(ptr), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex buffer\n");
		return false;
	}
	return true;
}

bool RBspObjectDrawD3D9::CreateBuffers()
{
#define CREATE_VB(x) if (!CreateVB<decltype(EluObjectData{}.Meshes[0].x[0])>(VBs.x, \
		State.TotalVertexCount, GetFVF())) return false;
	CREATE_VB(Positions);
	if (GetRenderer().SupportsDynamicLighting())
	{
		CREATE_VB(TexCoords);
		CREATE_VB(Normals);
		CREATE_VB(Tangents);
	}
#undef CREATE_VB
	
	auto hr = RGetDevice()->CreateIndexBuffer(State.TotalIndexCount * sizeof(IndexType), 0,
		GetD3DFormat<IndexType>(), D3DPOOL_MANAGED,
		MakeWriteProxy(IndexBuffer), nullptr);
	if (FAILED(hr))
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create index buffer\n");
		return false;
	}

	return true;
}

void RBspObjectDrawD3D9::CreateBatches()
{
	struct DrawPropStuff
	{
		int Mesh;
		u32 IndexBase;
		u32 Count;
	};

	struct MaterialStuff
	{
		int Data;
		std::vector<DrawPropStuff> DrawProps;
	};

	std::vector<MaterialStuff> MaterialProps;
	MaterialProps.resize(State.Materials.size());

	// Sort draw props per material
	for (size_t DataIndex{}; DataIndex < State.ObjectData.size(); ++DataIndex)
	{
		auto& Data = State.ObjectData[DataIndex];
		for (size_t MeshIndex{}; MeshIndex < Data.Meshes.size(); ++MeshIndex)
		{
			auto& Mesh = Data.Meshes[MeshIndex];
			for (auto& dp : Mesh.DrawProps)
			{
				auto& Mat = MaterialProps[Data.MaterialStart + dp.material];
				Mat.Data = DataIndex;

				Mat.DrawProps.push_back({
					static_cast<int>(MeshIndex),
					dp.indexBase,
					dp.count });

			}
		}
	}

	if (!CreateBuffers())
	{
		MLog("RBspObjectDrawD3D9::Create -- Failed to create vertex and index buffers\n");
		return;
	}

	// Lock vertex and index buffers.
	auto Lock = [](auto& Buffer, auto*& Pointer)
	{
		if (FAILED(Buffer->Lock(0, 0, reinterpret_cast<void**>(&Pointer), 0)))
		{
			MLog("Oh no!\n");
			return false;
		}
		return true;
	};
	char* Positions{};
	char* TexCoords{};
	char* Normals{};
	char* Tangents{};
#define LOCK(x) Lock(VBs.x, x)
	LOCK(Positions);
	if (GetRenderer().SupportsDynamicLighting())
	{
		LOCK(TexCoords);
		LOCK(Normals);
		LOCK(Tangents);
	}
#undef LOCK
	IndexType* Indices{}; if (!Lock(IndexBuffer, Indices)) return;

	// Insert transformed vertices for each object into the vertex buffer
	// and keep a list of the offset of each mesh's vertices.

	// Dimensions: [Data][Object][Mesh]
	// TODO: Change this terrible type
	std::vector<std::vector<std::vector<int>>> MeshVertexOffsets;
	MeshVertexOffsets.resize(State.Objects.size());
	int CumulativeVertexBase{};
	for (auto& pair : State.ObjectMap)
	{
		auto& Data = State.ObjectData[pair.first];
		auto& DataOffsets = MeshVertexOffsets[pair.first];
		auto ObjectIndexCount = pair.second.size();
		DataOffsets.resize(ObjectIndexCount);
		for (size_t ObjectListIndex{}; ObjectListIndex < ObjectIndexCount; ++ObjectListIndex)
		{
			auto ObjectIndex = pair.second[ObjectListIndex];
			auto& Object = State.Objects[ObjectIndex];

			auto& MeshOffsets = DataOffsets[ObjectListIndex];
			auto MeshCount = Data.Meshes.size();
			MeshOffsets.resize(MeshCount);
			for (size_t MeshIndex{}; MeshIndex < MeshCount; ++MeshIndex)
			{
				auto& Mesh = Data.Meshes[MeshIndex];
				MeshOffsets[MeshIndex] = CumulativeVertexBase;

				for (size_t i{}; i < Mesh.VertexCount; ++i)
				{
					auto AddVertexData = [&](auto& stream, auto&& val) {
						memcpy(stream, &val, sizeof(val));
						stream += sizeof(val);
					};
					if (!GetRenderer().SupportsDynamicLighting())
					{
						AddVertexData(Positions, Mesh.Positions[i] * Mesh.World * Object.World);
						AddVertexData(Positions, Mesh.TexCoords[i]);
					}
					else
					{
						AddVertexData(Positions, Mesh.Positions[i] * Mesh.World * Object.World);
						AddVertexData(Normals, TransformNormal(Mesh.Normals[i], Mesh.World * Object.World));
						AddVertexData(TexCoords, Mesh.TexCoords[i]);
						v4 tan = Mesh.Tangents[i];
						tan.w = 0;
						tan = Transform(tan, Mesh.World * Object.World);
						tan.w = Mesh.Tangents[i].w;
						AddVertexData(Tangents, tan);
					}
				}
				CumulativeVertexBase += Mesh.VertexCount;
			}
		}
	}

	// Insert indices for each object
	MaterialBatches.resize(State.Materials.size());
	int CumulativeIndexBase{};
	for (size_t MaterialIndex{}; MaterialIndex < Materials.size(); ++MaterialIndex)
	{
		auto& Mat = MaterialProps[MaterialIndex];
		auto& Batch = MaterialBatches[MaterialIndex];

		Batch.TriangleCount = 0;
		Batch.VertexBase = 0;
		Batch.IndexBase = CumulativeIndexBase;

		auto& Data = State.ObjectData[Mat.Data];
		auto& ObjectIndices = State.ObjectMap[Mat.Data];
		assert(!ObjectIndices.empty());

		DMLog("Create indices for material %d\n", MaterialIndex);

		for (size_t ObjectListIndex{}; ObjectListIndex < ObjectIndices.size(); ++ObjectListIndex)
		{
			for (auto& dp : Mat.DrawProps)
			{
				for (size_t i{}; i < dp.Count; ++i)
				{
					for (size_t j{}; j < 3; ++j)
					{
						auto Index = Data.Meshes[dp.Mesh].Indices[dp.IndexBase + i * 3 + j];
						auto VertexOffset = MeshVertexOffsets[Mat.Data][ObjectListIndex][dp.Mesh];
						*Indices = VertexOffset + Index;
						++Indices;
					}
				}

				CumulativeIndexBase += dp.Count * 3;
				Batch.TriangleCount += dp.Count;
			}
		}

		DMLog("Batch: TriangleCount = %d, VertexBase = %d, IndexBase = %d\n",
			Batch.TriangleCount, Batch.VertexBase, Batch.IndexBase);
	}

	DMLog("Added %d vertices, %d indices\n", CumulativeVertexBase, CumulativeIndexBase);

	// Unlock vertex and index buffers.
	VBs.Positions->Unlock(); VBs.TexCoords->Unlock(); VBs.Normals->Unlock(); VBs.Tangents->Unlock(); 
	IndexBuffer->Unlock();

	// Sort materials by state changes.
	auto NumMaterials = MaterialBatches.size();
	NormalMaterialsEnd = OpacityMaterialsEnd = AlphaTestMaterialsEnd = NumMaterials;

	auto SortMaterials = [&](auto Start, auto& End, auto& Pred)
	{
		for (auto i = Start; i < End; ++i)
		{
			if (!Pred(Materials[i]))
				continue;

			--End;
			auto DestIndex = End;
			std::swap(MaterialBatches[i], MaterialBatches[End]);
			std::swap(Materials[i], Materials[End]);
			--i;
		}
	};

	SortMaterials(0, NormalMaterialsEnd, [](auto& Mat) { return Mat.HasOpacity(); });
	SortMaterials(NormalMaterialsEnd, OpacityMaterialsEnd, [](auto& Mat) { return Mat.HasAlphaTest(); });

	DMLog("NormalMaterialsEnd = %d\n"
		"OpacityMaterialsEnd = %d\n"
		"AlphaTestMaterialsEnd = %d\n",
		NormalMaterialsEnd, OpacityMaterialsEnd, AlphaTestMaterialsEnd);
}

void RBspObjectDrawD3D9::Create(rsx::LoaderState && srcState)
{
	Materials.clear();
	TextureMemory.clear();
	MaterialBatches.clear();
	VBs.Positions.reset(); VBs.TexCoords.reset(); VBs.Normals.reset(); VBs.Tangents.reset(); 
	IndexBuffer.reset();
	NormalMaterialsEnd = 0;
	OpacityMaterialsEnd = 0;
	AlphaTestMaterialsEnd = 0;

	State = std::move(srcState);

	CreateTextures();
	CreateShaderStuff();
	CreateBatches();
	
	DMLog("RBspObjectDrawD3D9 created\n");
}
void RBspObjectDrawD3D9::SetWireframeStates()
{
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void RBspObjectDrawD3D9::SetDefaultStates()
{
	bool Trilinear = RIsTrilinear();

	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, Trilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, Trilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void RBspObjectDrawD3D9::DrawBatch(const MaterialBatch& Mat)
{
	auto hr = RGetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,    // Primitive type
		Mat.VertexBase,        // BaseVertexIndex
		0,                     // MinVertexIndex
		Mat.TriangleCount * 3, // NumVertices
		Mat.IndexBase,         // Start index
		Mat.TriangleCount);    // Primitive count

	assert(SUCCEEDED(hr));
}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::SetMaterial(Material& Mat)
{
	if (!UsingLighting())
	{
		dev->SetTexture(0, GetTexture(Mat.Diffuse));
		if (Type > Normal)
			dev->SetTexture(1, GetTexture(Mat.Opacity));
		if (Type > Opacity)
			dev->SetRenderState(D3DRS_ALPHAREF, Mat.AlphaTestValue);
	}
	else
	{
		int TexIndices[] = { Mat.Diffuse, Mat.Normal, Mat.Specular, Mat.Opacity, Mat.Emissive };
		for (size_t i{}; i < std::size(TexIndices); ++i)
			dev->SetTexture(i, GetTexture(TexIndices[i]));
		SetPSFloat(DeferredShaderConstant::Opacity, Type <= Normal);
		SetPSFloat(DeferredShaderConstant::SpecLevel, Mat.Shininess / 255.f);
	}
}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::SetPrerenderStates()
{
	switch (Type)
	{
	case Normal:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		break;
	case Opacity:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		break;
	case AlphaTest:
		dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
		// disable alpha test in deferred rendering mode
		// as it is handled in the shader
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		if (!UsingLighting()) {
			dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			dev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
			dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

			dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		}
		break;
	}

}

template <RBspObjectDrawD3D9::MaterialType Type>
void RBspObjectDrawD3D9::RenderMaterials(int StartIndex, int EndIndex)
{
	if (StartIndex == EndIndex)
		return;

	SetPrerenderStates<Type>();

	for (int i = StartIndex; i < EndIndex; ++i)
	{
		auto& Batch = MaterialBatches[i];
		auto& Mat = Materials[i];

		SetMaterial<Type>(Mat);

		if (Mat.TwoSided)
		{
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			DrawBatch(Batch);
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}
		else
			DrawBatch(Batch);
	}
}

LPDIRECT3DTEXTURE9 RBspObjectDrawD3D9::GetTexture(int Index) const
{
	if (Index == static_cast<u16>(-1))
		return nullptr;

	return TextureMemory[Index].get()->GetTexture();
}

u32 RBspObjectDrawD3D9::GetFVF() const
{
	return D3DFVF_XYZ | D3DFVF_TEX1;
}

bool RBspObjectDrawD3D9::UsingLighting() const
{
	return GetRenderer().SupportsDynamicLighting() && bsp.m_bisDrawLightMap;
}

void RBspObjectDrawD3D9::SetPrologueStates()
{
	if (bsp.m_bWireframe)
		SetWireframeStates();
	else
		SetDefaultStates();

	// Enable Z-buffer testing and writing.
	RSetWBuffer(true);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	// Map triangles are clockwise, so cull counter-clockwise faces.
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	if (!GetRenderer().SupportsDynamicLighting())
	{
		dev->SetStreamSource(0, VBs.Positions.get(), 0, sizeof(SimpleVertex));
	}
	else
	{
		dev->SetStreamSource(0, VBs.Positions.get(), 0, sizeof(v3));
		dev->SetStreamSource(1, VBs.TexCoords.get(), 0, sizeof(v2));
		if (UsingLighting())
		{
			dev->SetStreamSource(2, VBs.Normals.get(), 0, sizeof(v3));
			dev->SetStreamSource(3, VBs.Tangents.get(), 0, sizeof(v4));
		}
	}
	dev->SetIndices(IndexBuffer.get());

	if (!UsingLighting())
	{
		dev->SetFVF(GetFVF());
		dev->SetVertexDeclaration(UnlitVertexDecl.get());
		dev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
	}
	else
	{
		dev->SetFVF(0);
		dev->SetVertexDeclaration(LitVertexDecl.get());
	}
}

void RBspObjectDrawD3D9::SetEpilogueStates()
{
	// Disable alpha testing
	dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Disable alpha blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// Reset all textures and texture states
	dev->SetTexture(0, nullptr);
	dev->SetTexture(1, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	// Disable shaders
	dev->SetVertexShader(nullptr);
	dev->SetPixelShader(nullptr);
}

void RBspObjectDrawD3D9::Draw()
{
	SetPrologueStates();

	if (UsingLighting())
	{
		GetRenderer().BeginLighting();
	}

	RenderMaterials<Normal>(0, NormalMaterialsEnd);
	RenderMaterials<Opacity>(NormalMaterialsEnd, OpacityMaterialsEnd);
	RenderMaterials<AlphaTest>(OpacityMaterialsEnd, AlphaTestMaterialsEnd);

	// Disable alpha testing
	dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Disable alpha blending
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	if (UsingLighting())
	{
		GetRenderer().EndLighting(bsp.StaticMapLightList);
	}

	SetEpilogueStates();
}

void RBspObjectDrawD3D9::SetLighting(bool b)
{
	GetRenderer().SetDynamicLights(b);
	GetRenderer().UpdateRTs();
}

_NAMESPACE_REALSPACE2_END