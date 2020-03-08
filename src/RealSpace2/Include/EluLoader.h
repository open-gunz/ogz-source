#pragma once

#include "XMLFileStructs.h"
#include "XMLParser.h"
#include <vector>

namespace RealSpace2
{
struct RLIGHT;
class RSolidBspNode;
}

namespace rsx
{

struct EluHeader
{
	u32 Signature;
	u32 Version;
	u32 matCount;
	u32 meshCount;
};

struct DrawProp
{
	u32 vertexBase;
	u32 indexBase;
	// Triangle count
	u32 count;
	// Index into the material vector in its EluObjectData instance
	int material;
};

struct EluMesh
{
	std::unique_ptr<v3[]> Positions;
	std::unique_ptr<v3[]> Normals;
	std::unique_ptr<v2[]> TexCoords;
	std::unique_ptr<v4[]> Tangents;
	std::unique_ptr<u16[]> Indices;

	u32 VertexCount;
	u32 IndexCount;

	std::vector<DrawProp> DrawProps;

	rmatrix World;

	std::string Name;
};

using TextureType = std::string;

struct EluMaterial
{
	v4 cDiffuse;
	v4 cAmbient;
	v4 cSpecular;
	v4 cEmissive;

	float shininess;
	float roughness;

	TextureType tDiffuse;
	TextureType tNormal;
	TextureType tSpecular;
	TextureType tOpacity;
	TextureType tEmissive;

	int AlphaTestValue = -1;
	bool TwoSided{};
};

// The data comprising a particular elu.
struct EluObjectData
{
	std::string Name;
	std::vector<EluMesh> Meshes;
	// Total of the vertex and index counts of each mesh this elu owns.
	size_t VertexCount{}, IndexCount{};
	// The beginning of the continuous range of materials this elu uses
	// in LoaderState.Materials
	int MaterialStart = -1;
};

// An object in the map, comprised of an elu and the transform that situates it into world space.
struct EluObject
{
	// Index into LoaderState.ObjectData to retrieve the elu it uses.
	int Data = -1;
	rmatrix World;
};

struct LoaderState
{
	// Paths the loader searches for stuff in.
	std::vector<std::string> Paths;
	std::vector<EluObjectData> ObjectData;
	std::vector<EluObject> Objects;
	std::vector<EluMaterial> Materials;
	// Maps elu names to indices into ObjectData
	std::unordered_map<std::string, int> EluMap;
	// Maps indices into ObjectData to a list of indices into Objects
	std::unordered_map<int, std::vector<int>> ObjectMap;
	// The vertex and index counts of every single object in the map.
	// NOTE: OBJECTS, not elus, i.e. elus have their counts
	// multiplied by the amount of objects that use them.
	size_t TotalVertexCount{}, TotalIndexCount{};
};

bool loadTree(LoaderState& State, const char * sceneName, std::vector<RealSpace2::RLIGHT>& Lights);
bool loadPropTree(LoaderState& State, const char * propName);

}