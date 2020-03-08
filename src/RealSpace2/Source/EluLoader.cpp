#include "stdafx.h"
#include "EluLoader.h"
#include "XMLParser.h"
#include "XMLFileStructs.h"
#include "RBspObject.h"
#include "gli/gli.hpp"
#include "RealSpace2.h"
#include "defer.h"

namespace rsx
{

#define V(expr) if (!expr) return false

static bool ReadName(MZFile& file, EluMesh& mesh)
{
	u32 name_length{};
	V(file.Read(name_length));
	char name_buffer[256];
	V(file.Read(name_buffer, name_length));
	mesh.Name = name_buffer;
	return true;
}

static bool ReadWorld(MZFile& file, EluMesh& mesh)
{
	V(file.Read(mesh.World));
	return true;
}

template <typename T>
static bool ReadVector(MZFile& file, std::vector<T>& vec)
{
	u32 size;
	V(file.Read(size));
	vec.resize(size);
	V(file.Read(vec.data(), size * sizeof(vec[0])));
	return true;
}

template <typename T>
static bool SkipVector(MZFile& file)
{
	u32 size;
	V(file.Read(size));
	V(file.Seek(size * sizeof(T)));
	return true;
}

struct VertexData
{
	std::vector<v3> Positions, Normals, TexCoords;
	std::vector<v4> Tangents;
};

enum VertexAttributeOrder
{
	Pos,
	Nor,
	Tan,
	Tex,
	Skip4,
	SkipVecV3,
};

static bool ReadVertexData(MZFile& file, EluMesh& mesh, VertexData& data,
	const VertexAttributeOrder (&Order)[6])
{
	for (auto& Attribute : Order)
	{
		switch (Attribute)
		{
		case Pos: V(ReadVector(file, data.Positions)); break;
		case Nor: V(ReadVector(file, data.Normals)); break;
		case Tan: V(ReadVector(file, data.Tangents)); break;
		case Tex: V(ReadVector(file, data.TexCoords)); break;
		case Skip4: V(file.Seek(4)); break;
		case SkipVecV3: V(SkipVector<v3>(file)); break;
		}
	}

	return true;
}

static bool ReadIndices(MZFile& file)
{
	u32 ntris;
	V(file.Read(ntris));
	if (ntris > 0)
	{
		V(file.Seek(4));
		V(file.Read(ntris));

		for (u32 i = 0; i < ntris; ++i)
		{
			u32 nverts;
			V(file.Read(nverts));
			V(file.Seek(12 * nverts + 2));
		}
	}

	V(SkipVector<v3>(file));
	V(file.Seek(4));

	return true;
}

static bool ReadSubindices(MZFile& file, std::vector<std::array<u16, 6>>& subindices)
{
	V(ReadVector(file, subindices));
	u32 size; V(file.Read(size));
	V(file.Seek((64 + 2) * size));
	return true;
}

static bool ReadBisomething(MZFile& file)
{
	u32 bcount; V(file.Read(bcount));
	for (u32 i = 0; i < bcount; ++i)
	{
		u32 bi; V(file.Read(bi));
		V(file.Seek(8 * bi));
	}

	V(file.Seek(4));

	return true;
}

static bool ReadMeshData(MZFile& file, EluMesh& mesh,
	const std::vector<std::array<u16, 6>>& subindices, const VertexData& vertexData)
{
	mesh.VertexCount = subindices.size();
	u32 idxcnt; V(file.Read(idxcnt));
	mesh.IndexCount = idxcnt;
	mesh.Indices = decltype(mesh.Indices){new u16[mesh.IndexCount]};
	V(file.Read(mesh.Indices.get(), mesh.IndexCount * sizeof(mesh.Indices[0])));

	auto MakeVector = [&](auto& vec) {
		vec = std::remove_reference_t<decltype(vec)>{
			new std::remove_reference_t<decltype(vec[0])>[subindices.size()]};
	};
	MakeVector(mesh.Positions);
	MakeVector(mesh.Normals);
	MakeVector(mesh.TexCoords);
	MakeVector(mesh.Tangents);

	for (size_t i = 0; i < subindices.size(); ++i)
	{
		mesh.Positions[i] = vertexData.Positions[subindices[i][0]];
		mesh.Normals[i] = vertexData.Normals[subindices[i][1]];

		auto& tx = vertexData.TexCoords[subindices[i][2]];
		mesh.TexCoords[i] = { tx.x, tx.y };

		mesh.Tangents[i] = vertexData.Tangents[subindices[i][4]];
	}

	return true;
}

static bool LoadSubmeshData(MZFile& file, EluMesh& mesh)
{
	u32 nsubmesh; V(file.Read(nsubmesh));

	mesh.DrawProps.resize(nsubmesh);

	for (u32 i = 0; i < nsubmesh; ++i)
	{
		auto& dp = mesh.DrawProps[i];

		u32 mat; V(file.Read(mat));
		dp.material = mat;
		u16 idx; V(file.Read(idx));
		dp.indexBase = idx;
		u16 cnt; V(file.Read(cnt));
		dp.count = cnt;
		dp.vertexBase = 0;
		V(file.Seek(4));

		if (dp.material < 0)
		{
			if (mesh.DrawProps.size() == 1)
			{
				mesh.DrawProps.clear();
				return true;
			}

			mesh.DrawProps.erase(mesh.DrawProps.begin() + i);
			mesh.DrawProps.resize(mesh.DrawProps.size() - 1);
			--i;
		}
	}

	return true;
}

static bool LoadMesh5012(MZFile& file, EluMesh &mesh)
{
	V(ReadName(file, mesh));

	V(SkipVector<char>(file));
	V(file.Seek(12));
	V(ReadWorld(file, mesh));
	V(file.Seek(8));

	VertexData vertexData;
	V(ReadVertexData(file, mesh, vertexData, { Pos, Nor, Tan, Skip4, Tex, SkipVecV3 }));

	V(ReadIndices(file));
	V(ReadBisomething(file));

	std::vector<std::array<u16, 6>> subindices;
	V(ReadSubindices(file, subindices));
	V(ReadMeshData(file, mesh, subindices, vertexData));
	V(LoadSubmeshData(file, mesh));

	V(file.Seek(2 * sizeof(v3)));

	return true;
}

static bool LoadMesh5013(MZFile& file, EluMesh &mesh)
{
	V(ReadName(file, mesh));

	V(file.Seek(4));
	V(SkipVector<char>(file));
	V(ReadWorld(file, mesh));
	V(file.Seek(16));

	VertexData vertexData;
	V(ReadVertexData(file, mesh, vertexData, { Pos, Tex, SkipVecV3, Nor, Tan, SkipVecV3 }));

	V(ReadIndices(file));
	V(ReadBisomething(file));

	std::vector<std::array<u16, 6>> subindices;
	V(ReadSubindices(file, subindices));
	V(LoadSubmeshData(file, mesh));
	V(ReadMeshData(file, mesh, subindices, vertexData));

	V(file.Seek(2 * sizeof(v3)));

	return true;
}

#undef V

template <typename T>
static bool TryForEachPath(const std::vector<std::string>& Paths,
	const std::string& Filename, T& func)
{
	for (auto& Path : Paths)
		if (func(Path + Filename))
			return true;
	return false;
}

static bool FileExists(const char *filename)
{
	return g_pFileSystem->GetFileDesc(filename) != nullptr;
}

bool loadMaterial(LoaderState& State, const char* name)
{
	XMLMaterialVector xMats;

	auto found = TryForEachPath(State.Paths, name, [&](auto& path) {
		return XMLParser::parseXMLMaterial(path.c_str(), xMats);	
	});
	if (!found)
	{
		DMLog("loadMaterial on %s failed\n", name);
		return false;
	}

	XMLMaterialVector::iterator itor;
	State.Materials.reserve(xMats.size());

	for (auto& mat : xMats)
	{
		EluMaterial mtl;
		mtl.cAmbient = { EXPAND_VECTOR(mat.ambient), 1 };
		mtl.cDiffuse = { EXPAND_VECTOR(mat.diffuse), 1 };
		mtl.cSpecular = { EXPAND_VECTOR(mat.specular), 1 };
		mtl.cEmissive = { 0, 0, 0, 0 };
		mtl.shininess = mat.SpecularLevel;
		mtl.roughness = 0.f;
		mtl.AlphaTestValue = mat.AlphaTestValue;
		mtl.TwoSided = mat.TwoSided;

		auto LoadTexture = [&](auto Flag, auto& Texture, auto& Filename) {
			if (Filename.empty())
				return false;

			auto fn = [&](auto& Path) {
				if (!FileExists(Path.c_str()))
					return false;
				Texture = Path;
				return true;
			};
			if (!TryForEachPath(State.Paths, Filename, fn))
				if (!TryForEachPath(State.Paths, Filename + ".dds", fn))
				{
					MLog("loadMaterial -- Failed to load texture %s!\n", Filename.c_str());
					return false;
				}
			return true;
		};

		LoadTexture(FLAG_DIFFUSE, mtl.tDiffuse, mat.DiffuseMap);
		LoadTexture(FLAG_NORMAL, mtl.tNormal, mat.NormalMap);
		LoadTexture(FLAG_SPECULAR, mtl.tSpecular, mat.SpecularMap);
		LoadTexture(FLAG_OPACITY, mtl.tOpacity, mat.OpacityMap);
		LoadTexture(FLAG_SELFILLUM, mtl.tEmissive, mat.SelfIlluminationMap);

		State.Materials.push_back(mtl);
	}
	return true;
}

static bool LoadElu(LoaderState& State, const char* name)
{
	// HACK: Don't load shadowbox
	if (strstr(name, "shadowbox") != nullptr)
		return false;

	State.ObjectData.emplace_back();
	auto& dest = State.ObjectData.back();

	bool ReachedEnd = false;
	DEFER([&] { if (!ReachedEnd) State.ObjectData.pop_back(); });

	MZFile File;

	auto success = TryForEachPath(State.Paths, name, [&](auto& Path) {
		return File.Open(Path.c_str(), RealSpace2::g_pFileSystem);
	});
	if (!success)
	{
		MLog("LoadElu -- Failed to open elu file %s\n", name);
		return false;
	}

	EluHeader hdr;
	File.Read(hdr);

	dest.Meshes.reserve(hdr.meshCount);
	for (u32 i = 0; i < hdr.meshCount; ++i)
	{
		dest.Meshes.emplace_back();
		auto& Mesh = dest.Meshes.back();

		bool success{};

		switch (hdr.Version)
		{
		case 0x5012:
			success = LoadMesh5012(File, Mesh);
			break;
		case 0x5013:
			success = LoadMesh5013(File, Mesh);
			break;
		default:
			DMLog("Unknown mesh version %X\n", hdr.Version);
		}

		if (!success)
		{
			MLog("Failed to load mesh index %d version %x for elu %s\n", i, hdr.Version, name);
			return false;
		}

		dest.VertexCount += Mesh.VertexCount;
		dest.IndexCount += Mesh.IndexCount;

		DMLog("%s mesh %s %d vert %d index\n",
			name, Mesh.Name.c_str(), Mesh.VertexCount, Mesh.IndexCount);
		for (size_t j{}; j < Mesh.DrawProps.size(); ++j)
		{
			auto& dp = Mesh.DrawProps[j];
			DMLog("DrawProp %d: vb: %d, ib: %d, cnt: %d, mat: %d\n",
				j, dp.vertexBase, dp.indexBase, dp.count, dp.material);
		}
	}

	assert(File.Tell() == File.GetLength());

	DMLog("%s -- Meshes.size() = %d\n", name, dest.Meshes.size());

	dest.MaterialStart = State.Materials.size();
	if (!loadMaterial(State, (std::string{ name } +".xml").c_str()))
	{
		MLog("Failed to load material for elu %s\n", name);
		return false;
	}
	dest.Name = name;

	State.EluMap[dest.Name] = State.ObjectData.size() - 1;

	ReachedEnd = true;

	return true;
}

static void MakeEluObject(LoaderState& State, int DataIndex = -1)
{
	if (DataIndex == -1)
		DataIndex = State.ObjectData.size() - 1;
	auto& Data = State.ObjectData[DataIndex];
	State.Objects.emplace_back();
	auto& Obj = State.Objects.back();
	Obj.Data = DataIndex;

	State.ObjectMap[DataIndex].push_back(State.Objects.size() - 1);

	State.TotalVertexCount += Data.VertexCount;
	State.TotalIndexCount += Data.IndexCount;

	DMLog("MakeEluObject -- State.TotalVertexCount = %d, Index = %d\n",
		State.TotalVertexCount, State.TotalIndexCount);
}

static bool load(LoaderState& State, const char * name)
{
	// Check if we've already loaded this elu and duplicate it if so
	{
		auto it = State.EluMap.find(name);
		if (it != State.EluMap.end())
		{
			DMLog("load -- %s is duplicated, cloning\n", name);
			MakeEluObject(State, it->second);

			return true;
		}
	}

	// Load the elu
	if (!LoadElu(State, name))
		return false;

	MakeEluObject(State);

	return true;
}

static rmatrix MakeSaneWorldMatrix(const v3& pos, v3 dir, v3 up)
{
#ifdef _DEBUG
	if (!IS_EQ(MagnitudeSq(dir), 1))
	{
		DMLog("dir %f\n", Magnitude(dir));
		if (IS_EQ(MagnitudeSq(dir), 0))
			dir = { 1, 0, 0 };
		else
			Normalize(dir);
	}
	if (!IS_EQ(MagnitudeSq(up), 1))
	{
		DMLog("up %f\n", Magnitude(up));
		if (IS_EQ(MagnitudeSq(up), 0))
			up = { 1, 0, 0 };
		else
			Normalize(up);
	}
#endif

	auto mat = GetIdentityMatrix();

	auto right = Normalized(CrossProduct(dir, up));
	up = Normalized(CrossProduct(right, dir));
	v3 basis[] = {
		right,
		dir,
		up };
	for (size_t i{}; i < 3; ++i)
		for (size_t j{}; j < 3; ++j)
			mat.m[i][j] = basis[i][j];

	SetTransPos(mat, pos);

	return mat;
}

static bool loadObjectTree(LoaderState& State, const XMLObject& xmlObject, XMLObjectVector &ret)
{
	XMLActor actor;

	auto found = TryForEachPath(State.Paths, xmlObject.SceneXMLFile, [&](auto& Path) {
		return XMLParser::parseScene(Path.c_str(), actor, ret);
	});
	if (!found)
	{
		MLog("loadObjectTree -- Failed to load xmlObject %s\n", xmlObject.Name.c_str());
		return false;
	}

	if (!actor.isValid)
		return false;

	if (!load(State, actor.eluName.c_str()))
	{
		MLog("loadObjectTree(%s) -- Failed to load object\n", xmlObject.Name.c_str());
		return false;
	}

	auto& Obj = State.Objects.back();
	Obj.World = MakeSaneWorldMatrix(xmlObject.Position, xmlObject.Dir, xmlObject.Up);

	return true;
}

static bool ReadTree(LoaderState& State, const XMLObjectVector& vec)
{
	XMLObjectVector ret;
	bool success = true;
	for (auto& obj : vec)
		success &= loadObjectTree(State, obj, ret);
	if (!ret.empty())
		success &= ReadTree(State, ret);
	return success;
}

bool loadTree(LoaderState& State, const char * sceneName, std::vector<RLIGHT>& Lights)
{
	DMLog("loadTree %s\n", sceneName);

	XMLActor actor;
	XMLObjectVector ret;
	XMLLightVector lights;

	auto found = TryForEachPath(State.Paths, sceneName, [&](auto& Path) {
		return XMLParser::parseScene(Path.c_str(), actor, ret, &lights);
	});
	if (!found)
	{
		MLog("loadTree -- Failed to load %s\n", sceneName);
		return false;
	}
	else
		DMLog("loadTree -- loaded %s\n", sceneName);

	for (auto& srcLight : lights)
	{
		Lights.emplace_back();
		auto& dstLight = Lights.back();
		dstLight.Position = srcLight.Position;
		dstLight.Color = srcLight.Diffuse;
		dstLight.fAttnStart = srcLight.AttStart;
		dstLight.fAttnEnd = srcLight.AttEnd;
		dstLight.fIntensity = srcLight.Intensity;
		dstLight.Name = srcLight.Name;
	}

	DMLog("loadTree -- actor.isValid = %d\n", actor.isValid);
	if (actor.isValid)
	{
		if (!load(State, actor.eluName.c_str()))
		{
			MLog("loadTree(%s) -- Failed to load actor thing\n", sceneName);
		}
	}

	ReadTree(State, ret);

	return true;
}

bool loadPropTree(LoaderState& State, const char * propName)
{
	DMLog("loadPropTree %s\n", propName);

	XMLObjectVector ret;

	auto found = TryForEachPath(State.Paths, propName, [&](auto& Path) {
		return XMLParser::parseProp(Path.c_str(), ret);
	});
	if (!found)
	{
		MLog("loadPropTree -- Failed to load %s\n", propName);
		return false;
	}

	ReadTree(State, ret);

	return true;
}

}