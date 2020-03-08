#pragma once

#include "GlobalTypes.h"
#pragma warning(push)
#pragma warning(disable: 4305)
#include "../sdk/bullet/include/btBulletCollisionCommon.h"
#pragma warning(pop)
#include "optional.h"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class alignas(16) BulletCollision
{
public:
	using IndexType = u16;

	BulletCollision();
	void SetTotalCounts(size_t VertexCount, size_t IndexCount);
	void AddTriangles(v3* Vertices, size_t VertexCount,
		IndexType* Indices, size_t IndexCount);
	void Build();
	void Clear();

	bool Pick(const v3& Src, const v3& Dest, v3* Hit = nullptr, v3* Normal = nullptr);
	bool CheckCylinder(const v3& Src, const v3& Dest, float Radius, float Height, v3* Hit = nullptr, v3* Normal = nullptr);
	bool CheckSphere(const v3& Src, const v3& Dest, float Radius, v3* Hit = nullptr, v3* Normal = nullptr);

	void* operator new(size_t count) { return _aligned_malloc(count, 16); }
	void operator delete(void* ptr) { _aligned_free(ptr); }

private:
	// The order of these four members is important since they are constructed
	// in the member initializer list and depend on each other.
	btDbvtBroadphase Broadphase;
	btDefaultCollisionConfiguration Configuration;
	btCollisionDispatcher Dispatcher;
	btCollisionWorld World;

	btCollisionObject Object;
	btTriangleIndexVertexArray Triangles;
	optional<btBvhTriangleMeshShape> Mesh;
	std::vector<v3> Vertices;
	std::vector<BulletCollision::IndexType> Indices;
};

_NAMESPACE_REALSPACE2_END
