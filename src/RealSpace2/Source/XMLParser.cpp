#include "stdafx.h"
#include "XMLParser.h"
#include "rapidxml.hpp"
#include <iostream>
#include <memory>
#include "RealSpace2.h"

using namespace rapidxml;

namespace rsx {

namespace XMLParser {

static std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath)
{
	MZFile File;

	if (!File.Open(szPath, RealSpace2::g_pFileSystem))
	{
		return{ false,{} };
	}

	int FileLength = File.GetLength();

	if (FileLength <= 0)
	{
		return{ false,{} };
	}

	std::vector<unsigned char> InflatedFile;
	InflatedFile.resize(FileLength);

	File.Read(&InflatedFile[0], FileLength);

	return{ true, InflatedFile };
}

struct ParseXMLFileResult
{
	bool success{};
	std::vector<unsigned char> file_memory;
};

static ParseXMLFileResult ParseXMLFile(const char* filename, rapidxml::xml_document<>& doc)
{
	auto file_data = ReadMZFile(filename);

	if (!file_data.first || file_data.second.empty())
		return{};

	try {
		doc.parse<0>(reinterpret_cast<char*>(file_data.second.data()), file_data.second.size());
	}
	catch (rapidxml::parse_error& e) {
		MLog("XMLParser::parseProp -- Parse error on %s\n"
			"e.what() = %s\n",
			filename, e.what());
		return{};
	}

	ParseXMLFileResult ret;
	ret.success = true;
	ret.file_memory = std::move(file_data.second);

	return ret;
}

#define PARSE_XML_FILE(filename) \
xml_document<> doc; \
auto ret = ParseXMLFile(filename, doc); \
if (!ret.success) return false

static float toFloat(const char *str) { return float(atof(str)); }
static int toInt(const char *str) { return atoi(str); }

static bool toFloat3(const char *str, float *fv)
{
	fv[0] = fv[1] = fv[2] = 0.0f;

	auto ret = sscanf_s(str, "%f %f %f", &fv[0], &fv[1], &fv[2]);
	return ret == 3;
}

static const char* GetNodeValue(const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* node = parent->first_node(node_name);
	if (!node) return nullptr;
	auto* value = node->value();
	return value;
}

static bool GetFloat3(float (&dest)[3], const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* value = GetNodeValue(node_name, parent);
	if (!value) return false;
	toFloat3(value, dest);
	return true;
}

static bool GetFloat3XYZ(float(&dest)[3], const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* node = parent->first_node(node_name);
	if (!node) return false;
	auto Get = [&](const char* name) {
		auto* attr = node->first_attribute(name);
		if (!attr) return 0.0f;
		auto* value = attr->value();
		if (!value) return 0.0f;
		return toFloat(value);
	};
	const char* vector_elements[] = { "x", "y", "z" };
	for (size_t i{}; i < std::size(dest); ++i)
		dest[i] = Get(vector_elements[i]);
	return true;
}

static bool GetFloat(float& dest, const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* value = GetNodeValue(node_name, parent);
	if (!value) return false;
	dest = toFloat(value);
	return true;
}

static bool GetInt(int& dest, const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* string_value = GetNodeValue(node_name, parent);
	if (!string_value) return false;
	auto maybe_int_value = StringToInt<int>(string_value);
	if (!maybe_int_value.has_value()) return false;
	dest = maybe_int_value.value();
	return true;
}

static bool GetUShort3(float (&dest)[3], const char* node_name, rapidxml::xml_node<>* parent)
{
	auto* value = GetNodeValue(node_name, parent);
	if (!value) return false;
	int vals[3]{};
	if (sscanf_s(value, "%i %i %i", vals, vals + 1, vals + 2) != 3) return false;
	for (size_t i{}; i < std::size(dest); ++i)
		dest[i] = float(vals[i]);
	return true;
}

bool parseXMLMaterial(const char * name, std::vector<XMLMaterial> &Ret)
{
	PARSE_XML_FILE(name);

	xml_node<> *MatList = doc.first_node("XML")->first_node(MATERIAL_LIST_TAG);
	if (!MatList) return false;

	xml_node<> *CurMaterial = MatList->first_node(MATERIAL_TAG);
	while (CurMaterial)
	{
		unsigned int color = 0;
		XMLMaterial tmp;
		tmp.Name = CurMaterial->first_attribute("name")->value();

		GetUShort3(tmp.diffuse, MAT_DIFFUSE_TAG, CurMaterial);
		GetUShort3(tmp.ambient, MAT_AMBIENT_TAG, CurMaterial);
		GetUShort3(tmp.specular, MAT_SPECULAR_TAG, CurMaterial);
		GetFloat(tmp.SpecularLevel, MAT_SPECULAR_LEVEL_TAG, CurMaterial);
		GetFloat(tmp.glossiness, MAT_GLOSSINESS_TAG, CurMaterial);

		auto* TextureList = CurMaterial->first_node(MAT_TEXTURELIST_TAG);
		if (!TextureList) return false;
		auto* TexLayer = TextureList->first_node(MAT_TEXTURELAYER_TAG);
		while (TexLayer)
		{
			auto GetTexture = [&](const char* node_name, auto& TextureName, u32 flag)
			{
				auto* MapNameNode = TexLayer->first_node(node_name);
				if (!MapNameNode) return;
				auto* MapName = MapNameNode->value();
				if (!MapName) return;
				TextureName = MapName;
				tmp.Flag |= flag;
			};
			
			GetTexture(MAT_DIFFUSEMAP_TAG, tmp.DiffuseMap, FLAG_DIFFUSE);
			GetTexture(MAT_NORMALMAP_TAG, tmp.NormalMap, FLAG_NORMAL);
			GetTexture(MAT_SPECULARMAP_TAG, tmp.SpecularMap, FLAG_SPECULAR);
			GetTexture(MAT_SELFILLUMINATIONMAP_TAG, tmp.SelfIlluminationMap, FLAG_SELFILLUM);
			GetTexture(MAT_OPACITYMAP_TAG, tmp.OpacityMap, FLAG_OPACITY);

			TexLayer = TexLayer->next_sibling(MAT_TEXTURELAYER_TAG);
		}

		tmp.TwoSided = CurMaterial->first_node("TWOSIDED") != nullptr;

		if (auto* node = CurMaterial->first_node("USEALPHATEST"))
		{
			GetInt(tmp.AlphaTestValue, "ALPHATESTVALUE", node);
		}

		Ret.push_back(tmp);
		CurMaterial = CurMaterial->next_sibling(MATERIAL_TAG);
	}

	return true;
}

bool parseScene(const char *name, XMLActor &actor, std::vector<XMLObject> &Ret, std::vector<XMLLight> *Ret2)
{
	PARSE_XML_FILE(name);

	auto* Root = doc.first_node("ROOT");
	if (!Root) return false;
	xml_node<> *Scene = Root->first_node("SCENE");
	if (!Scene) return false;

	xml_node<> *Actor = Scene->first_node("ACTOR");
	if (Actor)
	{
		xml_node<> *Comm = Actor->first_node("COMMON");
		if (!Comm) return false;
		actor.Name = Comm->first_attribute("name")->value();
		xml_node<> *Prop = Actor->first_node("PROPERTY");
		if (!Prop) return false;
		actor.eluName = Prop->first_node("FILENAME")->value();
		actor.isValid = true;
	}

	xml_node<> *Object = Scene->first_node();

	while (Object)
	{
		auto GetCommonProperties = [&](auto& dest)
		{
			auto* Common = Object->first_node(SCENE_COMMON_TAG);
			if (!Common) return false;
			auto* name_attr = Common->first_attribute(SCENE_ATTRIB_NAME);
			if (!name_attr) return false;
			auto* name = name_attr->value();
			if (!name) return false;
			dest.Name = name;

			GetFloat3(dest.Position, SCENE_POSITION_TAG, Common);
			GetFloat3(dest.Dir, SCENE_DIRECTION_TAG, Common);
			GetFloat3(dest.Scale, SCENE_SCALE_TAG, Common);
			GetFloat3(dest.Up, SCENE_UP_TAG, Common);

			return true;
		};

		if (strcmp(Object->name(), SCENE_INST_TAG) == 0)
		{
			XMLObject tmp;
			GetCommonProperties(tmp);

			xml_node<> *Prop = Object->first_node(SCENE_PROPERTY_TAG);
			if (!Prop) return false;
			Prop = Prop->first_node(SCENE_FILENAME_TAG);
			if (!Prop) return false;
			tmp.SceneXMLFile = Prop->value();

			tmp.isDynamic = false;

			Ret.push_back(tmp);
		}
		else if (strcmp(Object->name(), SCENE_LIGHT_TAG) == 0 && Ret2 != nullptr)
		{
			XMLLight tmp2;
			GetCommonProperties(tmp2);

			xml_node<> *Prop = Object->first_node(SCENE_PROPERTY_TAG);
			if (!Prop) return false;

			GetFloat3(tmp2.Diffuse, SCENE_COLOR_TAG, Prop);
			GetFloat(tmp2.Intensity, SCENE_INTENSITY_TAG, Prop);
			GetFloat(tmp2.AttEnd, SCENE_ATTENUATIONEND_TAG, Prop);
			GetFloat(tmp2.AttStart, SCENE_ATTENUATIONSTART_TAG, Prop);

			Ret2->push_back(tmp2);
		}

		Object = Object->next_sibling();
	}

	return true;
}

bool parseProp(const char *name, std::vector<XMLObject> &Ret)
{
	PARSE_XML_FILE(name);

	xml_node<> *Root = doc.first_node("XML");
	if (!Root) return false;

	xml_node<> *Object = Root->first_node();
	if (!Object) return false;

	while (Object)
	{
		if (strcmp(Object->name(), PROP_SCENE_OBJECT_TAG) == 0 ||
			strcmp(Object->name(), PROP_OBJECT_TAG) == 0)
		{
			XMLObject tmp;
			xml_node<> * Common = Object->first_node(SCENE_COMMON_TAG);
			if (!Common) return false;

			GetFloat3XYZ(tmp.Position, SCENE_POSITION_TAG, Common);
			GetFloat3XYZ(tmp.Dir, SCENE_DIRECTION_TAG, Common);
			GetFloat3XYZ(tmp.Up, SCENE_UP_TAG, Common);

			xml_node<> *Prop = Object->first_node(SCENE_PROPERTY_TAG);
			if (!Prop) return false;

			auto* cProp = Prop->first_node(PROP_NAME_TAG);
			if (!cProp) return false;
			tmp.Name = cProp->value();

			cProp = Prop->first_node(PROP_SCENE_FILE_TAG);
			bool isScene = true;
			if (!cProp)
			{
				cProp = Prop->first_node(PROP_DEFINITION_TAG);
				isScene = false;
				tmp.isDynamic = true;
			}

			if (!cProp) return false;
			tmp.SceneXMLFile = cProp->value();
			if (!isScene) tmp.SceneXMLFile.append(".scene.xml");

			Ret.push_back(tmp);
		}

		Object = Object->next_sibling();
	}
	return true;
}
}
}