// MeshManager is a poorly named type that manages dynamically loading and unloading meshes as they
// are requested by RVisualMeshes.
//
// To understand how it handles this, we have to look at the structure of the Gunz mesh system.
// At the first level, we have base meshes (e.g. "heroman1" for male models, and "herowoman1" for
// female models).
// Each base mesh has a number of elu files, containing meshes that are designed for it.
// Each elu file contains one or more parts in the form of mesh nodes.
//
// "Part" here means a particular physical part of the base mesh. These are switched out from the
// default naked nodes as the player equips new clothing items.
// The parts are enumerated by the RMeshPartsType enum, and the relevant enumerators (the ones
// related to clothing) are:
// * eq_parts_head
// * eq_parts_face
// * eq_parts_chest
// * eq_parts_hands
// * eq_parts_legs
// * eq_parts_feet
//
// That adds up to six total parts, and each clothing elu file can contain one of each of these.
// Typically, each elu file contains all the parts for a matching set. (That is, all of the above,
// except the face.)
//
// Since players are not forced to wear all the parts of a matching set, but can instead mix and
// match different items, the result is that we usually have to reach into a number of different
// elu files to get all the mesh nodes that form a player's uniquely composed outfit, and often
// end up only using a small part of the file, since it contains multiple parts.
// Since elu files can't really be partially loaded, we load the entire elu file, even if we don't
// use all the parts.
//
// The elu files do not outwardly signal in any way which parts they contain, so the only way to
// find out is to load them. To reduce loading time, we do this beforehand, storing the
// preprocessed data into a file called parts_index.xml. This is loaded when the MeshManager loads
// (in MeshManager::LoadParts) and processed into a list of parts for each base mesh, containing
// which elu file they are stored in.
//
// When a player equips a particular clothing item, we look up their base mesh, and then look up
// the parts name in the base mesh's parts list. If the elu file it resides in is not already
// loaded, we load it and grab the mesh node corresponding to the requested part. However, 
// since elu files often contain the mesh nodes for several parts, it might already be loaded
// for a different part. If it is already loaded, we find the existing mesh, and grab the mesh node
// from that.
//
// To this end, the BaseMeshData struct contains two maps: AllocatedMeshes and AllocatedNodes.
// AllocatedMeshes contains all the currently loaded meshes, or elu files. AllocatedNodes contains
// all the mesh nodes in use. As long as at least one mesh node from a particular mesh is in use,
// the file will stay loaded. When all mesh nodes have been released, the file is unloaded.
//
// Each value of the AllocatedNodes map contains reference counting information for the mesh node.
// The reference count is the number of RVisualMeshes that are using it.
// AllocatedMeshes also contains refcount info; the count is the sum of the counts of all the nodes
// within it. When it reaches zero, there must be no nodes used, and the mesh is released.
//
// The lookups and mesh loading are done asynchronously, in a different thread, in order to not
// interrupt gameplay (since new players could be joining with new items while a game is in
// progress).

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

struct MeshNodeDeleter { bool Managed = true; void operator()(RMeshNode* ptr) const; };
using RMeshNodePtr = std::unique_ptr<RMeshNode, MeshNodeDeleter>;

class MeshManager
{
public:
	bool LoadParts(std::vector<unsigned char>& File);
	void Destroy();

	// Callback should have the signature `void(RMeshNodePtr Node, const char* NodeName)`.
	template <typename T>
	void Get(const char *MeshName, const char *NodeName, void* Obj, T&& Callback);
	void Release(RMeshNode *pNode);

	bool RemoveObject(void* Obj, bool All = true);

private:
	struct RMeshAllocation
	{
		RMesh Mesh;
		// -1 indicates it's being loaded.
		std::atomic<int> References;
		~RMeshAllocation()
		{
			Mesh.ClearMtrl();
		}
	};

	struct RMeshNodeAllocation
	{
		RMeshNode* Node;
		int References;
	};

	using PartsToEluMapType = std::unordered_map<std::string, std::string>;
	using AllocatedMeshesType = std::unordered_map<std::string, RMeshAllocation>;
	using AllocatedNodesType = std::unordered_map<std::string, RMeshNodeAllocation>;

	struct BaseMeshData
	{
		// Maps parts names to elu filenames for that mesh.
		// E.g., "eq_chest_05" -> "Model/woman/woman-parts11.elu".
		PartsToEluMapType PartsToEluMap;

		// The two below maps are for already loaded meshes.
		// AllocatedMeshes maps a mesh filename (e.g. "Mode/woman/woman-parts11.elu") to refcount
		// info for that RMesh.
		// AllocatedNodes does the same, but with parts names (e.g. "eq_chest_05") to the
		// RMeshNode refcount info.
		AllocatedMeshesType AllocatedMeshes;
		AllocatedNodesType AllocatedNodes;
	};
	
	// Maps a base mesh name (e.g. "heroman1" for the normal male mesh) to a BaseMeshData.
	// This is necessary because parts can have the same names for different base meshes.
	// E.g., both the female and male black dragon headpieces are called "eq_head_blackdragon".
	std::unordered_map<std::string, BaseMeshData> BaseMeshMap;

	struct LoadInfoType
	{
		// These three pointers' lifetime are all tied to the MeshManager instance.
		const char* NodeName;
		const char* EluFilename;
		RMeshAllocation* MeshAlloc;
		BaseMeshData* BaseMesh;
	};
	enum class GetResult
	{
		Found,
		MeshBeingLoaded,
		LoadMesh,
		NotFound,
	};
	GetResult GetCached(const char *szMeshName, const char *szNodeName,
		RMeshNodePtr& NodeOutput, LoadInfoType& LoadInfoOutput);
	RMeshNode* Load(const LoadInfoType& LoadInfo);
	using CallbackType = std::function<void(RMeshNodePtr, const char*)>;
	void LoadAsync(const LoadInfoType& LoadInfo, void* Obj, CallbackType Callback);
	void AwaitMeshLoad(const LoadInfoType& LoadInfo, void* Obj,
		CallbackType Callback);
	void InvokeCallback(void* Obj, RMeshNode* Node, const char* NodeName,
		CallbackType&& Callback);

	// Decrements the reference count of an allocation, performing the appropriate deletions if it
	// has hit zero.
	void DecrementRefCount(AllocatedMeshesType& AllocatedMeshes, AllocatedMeshesType::iterator);
	void DecrementRefCount(AllocatedNodesType& AllocatedNodes, AllocatedNodesType::iterator);

	// Mutex for accessing any part of the class, except QueuedObjs.
	std::mutex mutex;

	std::vector<void*> QueuedObjs;
	// Mutex only for accessing QueuedObjs.
	std::mutex ObjQueueMutex;
};

MeshManager* GetMeshManager();

template <typename T>
void MeshManager::Get(const char *MeshName, const char *NodeName, void* Obj,
	T&& Callback)
{
	RMeshNodePtr Node;
	LoadInfoType LoadInfo;
	auto Res = GetCached(MeshName, NodeName, Node, LoadInfo);
	switch (Res)
	{
	case GetResult::Found:
		Callback(std::move(Node), NodeName);
		break;
	case GetResult::NotFound:
		Callback(nullptr, NodeName);
		break;
	case GetResult::LoadMesh:
		LoadAsync(LoadInfo, Obj, std::forward<T>(Callback));
		break;
	case GetResult::MeshBeingLoaded:
		AwaitMeshLoad(LoadInfo, Obj, std::forward<T>(Callback));
		break;
	}
}

inline void MeshNodeDeleter::operator()(RMeshNode* ptr) const {
	if (ptr && Managed)
		GetMeshManager()->Release(ptr);
}

// TODO: Move this awkward thing or something
class TaskManager
{
public:
	TaskManager();

	template<typename T>
	void AddTask(T&& Task)
	{
		{
			std::lock_guard<std::mutex> lock(QueueMutex[0]);
			Tasks.emplace(std::forward<T>(Task));
			Notified = true;
		}

		cv.notify_one();
	}

	template<typename T>
	void Invoke(T&& Task)
	{
		std::lock_guard<std::mutex> lock(QueueMutex[1]);
		Invokations.emplace(std::forward<T>(Task));
	}

	void Update(float Elapsed);
	void ThreadLoop();

	static TaskManager& GetInstance()
	{
		static TaskManager Instance;

		return Instance;
	}

private:
	std::mutex QueueMutex[2];
	std::queue<std::function<void()>> Tasks;
	std::queue<std::function<void()>> Invokations;
	std::condition_variable cv;
	bool Notified = false;

	std::thread thr;
};
