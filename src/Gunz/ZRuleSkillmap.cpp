#include "stdafx.h"
#include "ZRuleSkillmap.h"
#include "ZMatch.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "rapidxml.hpp"
#include <cctype>

CourseManager ZRuleSkillmap::CourseMgr;

void CourseManager::Init()
{
	MZFile File;

	if (!File.Open("system/skillmaps.xml", g_pFileSystem))
	{
		MLog("failed to open skillmaps.xml\n");
		return;
	}

	int FileLength = File.GetLength();

	if (FileLength <= 0)
	{
		MLog("Invalid skillmaps.xml length\n");
		return;
	}

	std::string SkillmapData;
	SkillmapData.resize(FileLength);

	File.Read(&SkillmapData[0], FileLength);

	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<rapidxml::parse_no_data_nodes>(&SkillmapData[0], SkillmapData.size());
	}
	catch (rapidxml::parse_error &e)
	{
		MLog("RapidXML threw parse_error (%s) on skillmaps.xml at %s\n", e.what(), e.where<char>());
		return;
	}

	for (auto node = doc.first_node("map"); node; node = node->next_sibling("map"))
	{
		auto MapNameAttr = node->first_attribute("name");

		if (!MapNameAttr)
			continue;

		auto* MapNamePtr = MapNameAttr->value();
		if (!MapNamePtr)
			continue;
		char MapName[256];
		// Make lowercase
		std::transform(
			MapNamePtr, MapNamePtr + MapNameAttr->value_size() + 1,
			MapName,
			tolower);

		auto CourseNode = node->first_node("course");

		auto MapInsertion = Courses.insert({ MapName, std::vector<Course>() });

		auto ReadZones = [&](rapidxml::xml_node<> *node, const char *szName)
		{
			auto StartNode = node->first_node("startzone");
			auto EndNode = node->first_node("endzone");

			if (!StartNode || !EndNode)
				return false;

			rboundingbox Start, End;

			auto ReadVector = [](v3_pod &out, rapidxml::xml_attribute<> *attr)
			{
				if (!attr)
					return false;

				if(sscanf(attr->value(), "%f, %f, %f", &out.x, &out.y, &out.z) != 3)
					return false;

				return true;
			};

			bool ret = true;

			ret &= ReadVector(Start.vmin, StartNode->first_attribute("mins"));
			ret &= ReadVector(Start.vmax, StartNode->first_attribute("maxs"));
			ret &= ReadVector(End.vmin, EndNode->first_attribute("mins"));
			ret &= ReadVector(End.vmax, EndNode->first_attribute("maxs"));

			if (!ret)
				return false;

			Start = Union(Start.vmin, Start.vmax);
			End = Union(End.vmax, End.vmin);

			MapInsertion.first->second.push_back({ szName, Start, End });

			return true;
		};

		if (CourseNode)
		{
			for (; CourseNode; CourseNode = CourseNode->next_sibling("course"))
			{
				auto NameAttr = CourseNode->first_attribute("name");
				
				if (!NameAttr)
					continue;

				auto CourseName = NameAttr->value();

				ReadZones(CourseNode, CourseName);
			}
		}
		else
		{
			ReadZones(node, "");
		}

		MapInsertion.first->second.shrink_to_fit();
	}
}

void CourseManager::SetCurrentMap(const char *szMap)
{
	auto it = Courses.find(szMap);

	if (it == Courses.end())
		return;

	pCurrentCourseSet = &it->second;

	PrimitiveCount = 12 * 2 * pCurrentCourseSet->size();

	const int vbsize = PrimitiveCount * sizeof(Line);

	RGetDevice()->CreateVertexBuffer(vbsize, 0, D3DFVF_XYZ, D3DPOOL_MANAGED, MakeWriteProxy(pVB), nullptr);

	Line *pData;
	pVB->Lock(0, vbsize, (void **)&pData, 0);

	for (size_t i = 0; i < pCurrentCourseSet->size(); i++)
	{
		for (int j = 0; j < 2; j++)
		{
			const auto &bb = j ? pCurrentCourseSet->at(i).End : pCurrentCourseSet->at(i).Start;
			const auto &a = bb.vmin;
			const auto &b = bb.vmax;

			int idx = i * 12 * 2 + j * 12;

			auto p = [&](const rvector &a, const rvector &b, const rvector &c,
				const rvector &d, const rvector &e, const rvector &f)
			{
				pData[idx].a = rvector(a.x, b.y, c.z);
				pData[idx].b = rvector(d.x, e.y, f.z);
				idx++;
			};

			p(a, a, a,
				b, a, a);

			p(a, a, a,
				a, b, a);

			p(a, a, a,
				a, a, b);

			p(b, b, b,
				a, b, b);

			p(b, b, b,
				b, a, b);

			p(b, b, b,
				b, b, a);

			//

			p(a, a, b,
				b, a, b);

			p(a, a, b,
				a, b, b);

			p(b, b, a,
				b, a, a);

			p(b, b, a,
				a, b, a);

			p(a, b, a,
				a, b, b);

			p(b, a, a,
				b, a, b);
		}
	}

	pVB->Unlock();
}

const std::string &CourseManager::GetCourseName(int CourseIndex) const
{
	return pCurrentCourseSet->at(CourseIndex).Name;
}

int CourseManager::GetNumCourses() const
{
	if (!pCurrentCourseSet)
		return 0;

	return static_cast<int>(pCurrentCourseSet->size());
}

rvector CourseManager::GetCourseStartPos(int CourseIndex) const
{
	const rboundingbox &bb = pCurrentCourseSet->at(CourseIndex).Start;
	return (bb.vmin + rvector(bb.vmax.x, bb.vmax.y, bb.vmin.z)) / 2;
}

bool CourseManager::IsInStartZone(const rvector &pos, int &CourseIndex) const
{
	if (!pCurrentCourseSet)
		return false;

	rboundingbox Player;
	Player.vmin = pos - rvector(CHARACTER_RADIUS, CHARACTER_RADIUS, 0);
	Player.vmax = pos + rvector(CHARACTER_RADIUS, CHARACTER_RADIUS, CHARACTER_HEIGHT);

	for (size_t i = 0; i < pCurrentCourseSet->size(); i++)
	{
		if (Intersects(Player, pCurrentCourseSet->at(i).Start))
		{
			CourseIndex = i;

			return true;
		}
	}

	return false;
}

bool CourseManager::IsInEndZone(const rvector &pos, int CourseIndex) const
{
	if (!pCurrentCourseSet)
		return false;

	rboundingbox Player;
	Player.vmin = pos - rvector(CHARACTER_RADIUS, CHARACTER_RADIUS, 0);
	Player.vmax = pos + rvector(CHARACTER_RADIUS, CHARACTER_RADIUS, CHARACTER_HEIGHT);

	if (Intersects(Player, pCurrentCourseSet->at(CourseIndex).End))
		return true;

	return false;
}

void CourseManager::Draw()
{
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CONSTANT);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ);
	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	RGetDevice()->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFF0000);

	RGetDevice()->SetStreamSource(0, pVB.get(), 0, sizeof(rvector));

	RSetTransform(D3DTS_WORLD, IdentityMatrix());

	RGetDevice()->DrawPrimitive(D3DPT_LINELIST, 0, PrimitiveCount);
}

ZRuleSkillmap::ZRuleSkillmap(ZMatch* pMatch) : ZRule(pMatch)
{
	char MapName[32];
	strcpy_safe(MapName, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	_strlwr_s(MapName);
	CourseMgr.SetCurrentMap(MapName);
}

void ZRuleSkillmap::OnFall()
{
	if (CourseMgr.GetNumCourses() == 1 || CurrentCourseIndex == -1)
	{
		ZMapSpawnData *pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();

		ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
		ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
	}
	else
	{
		ZGetGame()->m_pMyCharacter->SetPosition(CourseMgr.GetCourseStartPos(CurrentCourseIndex));
	}
}

void ZRuleSkillmap::Draw()
{
	CourseMgr.Draw();
}

void ZRuleSkillmap::OnUpdate(float fDelta)
{
	auto pos = ZGetGame()->m_pMyCharacter->GetPosition();

	if (CourseMgr.IsInStartZone(pos, CurrentCourseIndex))
	{
		fStartTime = ZGetGame()->GetTime();
	}
	else if (CurrentCourseIndex != -1 && CourseMgr.IsInEndZone(pos, CurrentCourseIndex))
	{
		float fNow = ZGetGame()->GetTime();
		ZPostCompletedSkillmap(fNow - fStartTime, CourseMgr.GetCourseName(CurrentCourseIndex).c_str());
		CurrentCourseIndex = -1;
	}
}