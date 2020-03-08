#include "stdafx.h"
#include "MeshManager.h"
#include "rapidxml.hpp"
#include <fstream>
#include "defer.h"

TaskManager::TaskManager()
{
	thr = std::thread([this](){ while (true) ThreadLoop(); });
	thr.detach();
}

void TaskManager::Update(float Elapsed)
{
	std::unique_lock<std::mutex> lock(QueueMutex[1]);

	while (Invokations.size())
	{
		Invokations.front()();
		Invokations.pop();
	}
}

void TaskManager::ThreadLoop()
{
	std::unique_lock<std::mutex> lock(QueueMutex[0]);
	cv.wait(lock, [this]() { return Notified; });

	while (Tasks.size())
	{
		Tasks.front()();
		Tasks.pop();
	}

	Notified = false;
}

#ifdef _DEBUG
#define LOG(...) DMLog(__VA_ARGS__)
#else
static void nop(...) {}
// The arguments are passed to a function that does nothing so that they are still evaluated.
// If the macro were empty, it would hide compilation errors in Release mode.
#define LOG(...) nop(__VA_ARGS__)
#endif

bool MeshManager::LoadParts(std::vector<unsigned char>& File)
{
	// TODO: Parse elu files that aren't in parts_index.xml

	rapidxml::xml_document<> parts;

	try
	{
		parts.parse<rapidxml::parse_non_destructive>(reinterpret_cast<char*>(&File[0]), File.size());
	}
	catch (rapidxml::parse_error &e)
	{
		MLog("RapidXML threw parse_error (%s) on parts_index.xml at %s\n", e.what(), e.where<char>());
		return false;
	}

#define LOOP_NODES(var_name, parent, tag) \
for (auto var_name = (parent)->first_node(tag); var_name; var_name = var_name->next_sibling(tag))
#define LOOP_ATTRIBUTES(var_name, parent, tag) \
for (auto var_name = (parent)->first_attribute(tag); var_name; var_name = var_name->next_attribute(tag))

	LOOP_NODES(listnode, &parts, "partslisting")
	{
		auto meshattr = listnode->first_attribute("mesh");
		if (!meshattr || !meshattr->value())
			continue;

		// The strings aren't zero-terminated in RapidXML's non-destructive mode,
		// so we need to carefully construct an std::string with the size in mind.
		auto AttributeToString = [&](auto* attr) {
			return std::string{ attr->value(), attr->value_size() };
		};

		auto MeshName = AttributeToString(meshattr);

		auto MeshEmplaceResult = BaseMeshMap.emplace(MeshName, BaseMeshData{});
		auto&& PartsToEluMap = MeshEmplaceResult.first->second.PartsToEluMap;

		// parts tag example:
		// <parts file="Model/woman/woman-parts11.elu" part="eq_chest_05" part="eq_legs_005"
		//     part="eq_feet_005" part="eq_hands_05" part="eq_head_02" part="eq_face_004"/>
		LOOP_NODES(node, listnode, "parts")
		{
			auto fileattr = node->first_attribute("file");
			if (!fileattr || !fileattr->value())
				continue;

			LOOP_ATTRIBUTES(attr, node, "part")
			{
				if (!attr->value())
					continue;

				auto PartsName = AttributeToString(attr);
				auto EluPath = AttributeToString(fileattr);
				PartsToEluMap.emplace(PartsName, EluPath);

				LOG("Added %s -> %s\n", PartsName.c_str(), EluPath.c_str());
			}
		}
	}

#undef LOOP_NODES
#undef LOOP_ATTRIBUTES

	return true;
}

void MeshManager::Destroy()
{
	LOG("MeshManager::Destroy()\n");

	for (auto&& Pair : BaseMeshMap)
	{
		auto&& BaseMeshName = Pair.first;
		auto&& BaseMesh = Pair.second;
		LOG("BaseMesh %s\n"
			"PartsToEluMap.size() = %zu\n"
			"AllocatedMeshes.size() = %zu\n"
			"AllocatedNodes.size() = %zu\n"
			"\n",
			BaseMeshName.c_str(),
			BaseMesh.PartsToEluMap.size(),
			BaseMesh.AllocatedMeshes.size(),
			BaseMesh.AllocatedNodes.size());
	}

	BaseMeshMap.clear();
}

MeshManager::GetResult MeshManager::GetCached(const char *szMeshName, const char *szNodeName,
	RMeshNodePtr& NodeOutput, LoadInfoType& LoadInfoOutput)
{
	auto Prof = MBeginProfile("MeshManager::Get");

	std::lock_guard<std::mutex> lock{ mutex };

	LOG("Get mesh: %s, node: %s\n", szMeshName, szNodeName);

	auto mesh_it = BaseMeshMap.find(szMeshName);
	if (mesh_it == BaseMeshMap.end())
	{
		LOG("Couldn't find mesh %s in MeshMap!\n", szMeshName);
		return GetResult::NotFound;
	}

	auto&& BaseMesh = mesh_it->second;

	{
		auto alloc_node_it = BaseMesh.AllocatedNodes.find(szNodeName);

		if (alloc_node_it != BaseMesh.AllocatedNodes.end())
		{
			auto* meshnode = alloc_node_it->second.Node;

			LOG("Found already allocated node %s %p\n",
				meshnode->GetName(), static_cast<void*>(meshnode));

			auto* mesh = meshnode->m_pParentMesh;
			auto alloc_mesh_it = BaseMesh.AllocatedMeshes.find(mesh->GetFileName());
			if (alloc_mesh_it == BaseMesh.AllocatedMeshes.end())
			{
				LOG("Couldn't find mesh %s %p in AllocatedMeshes\n",
					mesh->GetFileName(), static_cast<void*>(mesh));
				return GetResult::NotFound;
			}

			LOG("Returning already allocated node %s %p in mesh %s %p\n",
				meshnode->GetName(), static_cast<void*>(meshnode),
				mesh->GetFileName(), static_cast<void*>(mesh));

			alloc_mesh_it->second.References.fetch_add(1, std::memory_order_relaxed);
			alloc_node_it->second.References++;
			NodeOutput = RMeshNodePtr{alloc_node_it->second.Node};
			return GetResult::Found;
		}
	}

	auto nodeit = BaseMesh.PartsToEluMap.find(szNodeName);

	if (nodeit == BaseMesh.PartsToEluMap.end())
	{ 
		LOG("Couldn't find node %s in PartsToEluMap element\n", szNodeName);

		return GetResult::NotFound;
	}

	RMesh *pMesh = nullptr;

	{
		auto AllocMeshEmplaceRes = BaseMesh.AllocatedMeshes.try_emplace(nodeit->second);
		auto& MeshAlloc = AllocMeshEmplaceRes.first->second;

		if (!AllocMeshEmplaceRes.second)
		{
			if (MeshAlloc.References == -1)
			{
				LOG("Mesh %s for %s being loaded\n", nodeit->second.c_str(), szNodeName);
				LoadInfoOutput = {nodeit->first.c_str(), nodeit->second.c_str(), &MeshAlloc,
					&BaseMesh};
				return GetResult::MeshBeingLoaded;
			}
			pMesh = &MeshAlloc.Mesh;
			MeshAlloc.References.fetch_add(1, std::memory_order_relaxed);
			LOG("Found already allocated mesh %s %p, new ref count %d\n",
				pMesh->GetFileName(), static_cast<void*>(pMesh),
				MeshAlloc.References.load(std::memory_order_relaxed));
		}
		else
		{
			LOG("Loading mesh %s for %s\n", nodeit->second.c_str(), szNodeName);
			MeshAlloc.References.store(-1, std::memory_order_relaxed);
			LoadInfoOutput = {nodeit->first.c_str(), nodeit->second.c_str(), &MeshAlloc,
				&BaseMesh};
			return GetResult::LoadMesh;
		}
	}

	auto node = pMesh->GetMeshData(szNodeName);

	if (!node)
	{
		LOG("Failed to find node %s\n", szNodeName);

		return GetResult::NotFound;
	}

	LOG("Placing %p -> %p in MeshNodeToMeshMap\n",
		static_cast<void*>(node), static_cast<void*>(pMesh));

	BaseMesh.AllocatedNodes.emplace(szNodeName, RMeshNodeAllocation{ node, 1 });

	NodeOutput = RMeshNodePtr{node};
	return GetResult::Found;
}

RMeshNode* MeshManager::Load(const LoadInfoType& LoadInfo)
{
	if (!LoadInfo.MeshAlloc->Mesh.ReadElu(LoadInfo.EluFilename))
	{
		MLog("Couldn't load elu %s\n", LoadInfo.EluFilename);
		return nullptr;
	}

	LOG("Loaded mesh %s, %s, %p\n",
		LoadInfo.EluFilename, LoadInfo.MeshAlloc->Mesh.GetFileName(),
		static_cast<void*>(&LoadInfo.MeshAlloc->Mesh));

	LoadInfo.MeshAlloc->References.store(1, std::memory_order_release);
	auto Node = LoadInfo.MeshAlloc->Mesh.GetMeshData(LoadInfo.NodeName);
	{
		std::lock_guard<std::mutex> lock{mutex};
		auto emplace_ret = LoadInfo.BaseMesh->AllocatedNodes.emplace(LoadInfo.NodeName,
			RMeshNodeAllocation{Node, 0});
		emplace_ret.first->second.References++;
	}
	LOG("Load -- Placed node %s -> %p from mesh %s, %p\n", LoadInfo.NodeName, Node,
		LoadInfo.MeshAlloc->Mesh.GetFileName(), &LoadInfo.MeshAlloc->Mesh);
	return Node;
}

void MeshManager::LoadAsync(const LoadInfoType& LoadInfo, void* Obj,
	std::function<void(RMeshNodePtr, const char*)> Callback)
{
	{
		std::lock_guard<std::mutex> lock(ObjQueueMutex);
		QueuedObjs.push_back(Obj);
	}

	auto Task = [this, LoadInfo, Obj, Callback = std::move(Callback)]() mutable
	{
		auto Node = Load(LoadInfo);
		InvokeCallback(Obj, Node, LoadInfo.NodeName, std::move(Callback));
	};
	TaskManager::GetInstance().AddTask(std::move(Task));
}

void MeshManager::AwaitMeshLoad(const LoadInfoType& LoadInfo, void* Obj,
	CallbackType Callback)
{
	{
		std::lock_guard<std::mutex> lock(ObjQueueMutex);
		QueuedObjs.push_back(Obj);
	}

	auto Task = [this, LoadInfo, Obj, Callback = std::move(Callback)]() mutable
	{
		while (LoadInfo.MeshAlloc->References.load(std::memory_order_acquire) == -1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		auto Node = LoadInfo.MeshAlloc->Mesh.GetMeshData(LoadInfo.NodeName);
		{
			std::lock_guard<std::mutex> lock{mutex};
			LoadInfo.MeshAlloc->References.fetch_add(1, std::memory_order_relaxed);
			auto emplace_ret = LoadInfo.BaseMesh->AllocatedNodes.try_emplace(LoadInfo.NodeName,
				RMeshNodeAllocation{Node, 0});
			emplace_ret.first->second.References++;
		}
		LOG("AwaitMeshLoad -- Placed node %s -> %p from mesh %s, %p\n", LoadInfo.NodeName, Node,
			LoadInfo.MeshAlloc->Mesh.GetFileName(), &LoadInfo.MeshAlloc->Mesh);
		InvokeCallback(Obj, Node, LoadInfo.NodeName, std::move(Callback));
	};
	TaskManager::GetInstance().AddTask(std::move(Task));
}

void MeshManager::InvokeCallback(void* Obj, RMeshNode* Node, const char* NodeName,
	CallbackType&& Callback)
{
	auto Invokation = [this, Obj, Node, NodeName,
		Callback = std::move(Callback)]() mutable
	{
		// We want to make sure the object hasn't been destroyed
		// between the GetAsync call and the invokation.
		if (RemoveObject(Obj, false))
			Callback(RMeshNodePtr{Node}, NodeName);
	};
	TaskManager::GetInstance().Invoke(std::move(Invokation));
}

void MeshManager::Release(RMeshNode *pNode)
{
	auto Prof = MBeginProfile("MeshManager::Release");

	std::lock_guard<std::mutex> lock(mutex);

	LOG("Release %s %p, parts type %d\n",
		pNode->GetName(), static_cast<void*>(pNode),
		pNode->m_PartsType);

	switch (pNode->m_PartsType)
	{
	case eq_parts_head:
	case eq_parts_face:
	case eq_parts_chest:
	case eq_parts_hands:
	case eq_parts_legs:
	case eq_parts_feet:
		break;
	default:
		LOG("Unsuitable parts type %d, returning\n",
			pNode->m_PartsType);
		return;
	};

	auto* NodeBaseMesh = pNode->m_pBaseMesh;
	auto* BaseMeshName = NodeBaseMesh->GetName();
	LOG("Base mesh %s\n", BaseMeshName);

	auto&& BaseMeshIt = BaseMeshMap.find(BaseMeshName);
	if (BaseMeshIt == BaseMeshMap.end())
	{
		LOG("Failed to find base mesh %s in base mesh map!\n", BaseMeshName);
		return;
	}

	auto&& BaseMesh = BaseMeshIt->second;

	auto AllocIt = BaseMesh.AllocatedNodes.find(pNode->GetName());
	if (AllocIt == BaseMesh.AllocatedNodes.end())
	{
		LOG("Couldn't find allocated node %s\n", pNode->GetName());
		return;
	}

	auto* pMesh = AllocIt->second.Node->m_pParentMesh;
	DecrementRefCount(BaseMesh.AllocatedNodes, AllocIt);

	LOG("Filename %s, %d\n", pMesh->m_FileName.c_str(), pMesh->m_FileName.length());

	auto allocmesh = BaseMesh.AllocatedMeshes.find(pMesh->GetFileName());
	if (allocmesh == BaseMesh.AllocatedMeshes.end())
	{
		LOG("Couldn't find mesh %s to release\n", pMesh->GetFileName());
		return;
	}

	DecrementRefCount(BaseMesh.AllocatedMeshes, allocmesh);
}

void MeshManager::DecrementRefCount(AllocatedMeshesType& AllocatedMeshes,
	AllocatedMeshesType::iterator AllocIt)
{
	auto& Alloc = AllocIt->second;
	Alloc.References.fetch_sub(1, std::memory_order_relaxed);

	LOG("DecRefCount mesh %s %p, new ref count %d\n",
		Alloc.Mesh.GetFileName(), static_cast<void*>(&Alloc.Mesh),
		Alloc.References.load());

	if (Alloc.References <= 0)
	{
		LOG("Releasing mesh %s %p\n",
			Alloc.Mesh.GetFileName(), static_cast<void*>(&Alloc.Mesh));

		AllocatedMeshes.erase(AllocIt);
	}
}

void MeshManager::DecrementRefCount(AllocatedNodesType& AllocatedNodes,
	AllocatedNodesType::iterator AllocIt)
{
	auto& Alloc = AllocIt->second;
	Alloc.References--;

	LOG("DecRefCount node %s %p, new ref count %d\n",
		Alloc.Node->GetName(), static_cast<void*>(Alloc.Node),
		Alloc.References);

	if (Alloc.References <= 0)
	{
		LOG("Erasing node allocation %s %p\n",
			Alloc.Node->GetName(), static_cast<void*>(Alloc.Node));

		AllocatedNodes.erase(AllocIt);
	}
}

bool MeshManager::RemoveObject(void *Obj, bool All)
{
	std::lock_guard<std::mutex> lock(ObjQueueMutex);

	if (All)
	{
		auto it = std::remove(QueuedObjs.begin(), QueuedObjs.end(), Obj);

		if (it == QueuedObjs.end())
			return false;

		QueuedObjs.erase(it, QueuedObjs.end());
	}
	else
	{
		auto it = std::find(QueuedObjs.begin(), QueuedObjs.end(), Obj);

		if (it == QueuedObjs.end())
			return false;

		QueuedObjs.erase(it);
	}

	return true;
}

MeshManager* GetMeshManager()
{
	static MeshManager Instance;
	return &Instance;
}
