#include "RAniEventInfo.h"
#include "MXml.h"

const RAniNameEventSet* RAniIDEventSet::GetAniNameEventSet(const char* AnimationName) const
{
	for (auto& e : AniIDEventSet)
		if (iequals(AnimationName, e.AnimationName))
			return &e;
	return nullptr;
}

const RAniIDEventSet* RAniEventMgr::GetAniIDEventSet(int ID) const
{
	for (auto& e : AniEventMgr)
		if (ID == e.ID)
			return &e;
	return nullptr;
}

bool RAniEventMgr::ReadXml(MZFileSystem* FileSystem, const char* Filename)
{
	MXmlDocument xmlIniData;
	if (!xmlIniData.LoadFromFile(Filename, FileSystem))
		return false;

	for (auto&& NPC : xmlIniData.GetDocumentElement().Children())
	{
		if (!iequals(NPC.GetTagName(), "NPC"))
			return false;

		auto& IDSet = emplace_back(AniEventMgr);
		if (!NPC.GetAttribute(&IDSet.ID, "id"))
			return false;

		for (auto&& Animation : NPC.Children())
		{
			if (!iequals(Animation.GetTagName(), "Animation"))
				return false;

			auto& NameSet = emplace_back(IDSet.AniIDEventSet);
			if (!Animation.GetAttribute(NameSet.AnimationName, "name"))
				return false;

			for (auto&& AddAnimEvent : Animation.Children())
			{
				if (!iequals(AddAnimEvent.GetNodeName(), "AddAnimEvent") ||
					!iequals(AddAnimEvent.GetAttribute("eventtype").value_or(""), "sound"))
					return false;

				auto& Info = emplace_back(NameSet.AniNameEventSet);
				if (!AddAnimEvent.GetAttribute(Info.Filename, "filename") ||
					!AddAnimEvent.GetAttribute(&Info.BeginFrame, "beginframe"))
					return false;
			}
		}
	}

	return true;
}
