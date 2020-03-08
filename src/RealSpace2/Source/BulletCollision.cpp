#include "stdafx.h"
#include "BulletCollision.h"

template <typename T> PHY_ScalarType GetScalarType();
template <> PHY_ScalarType GetScalarType<u16>() { return PHY_SHORT; }
template <> PHY_ScalarType GetScalarType<u32>() { return PHY_INTEGER; }

BulletCollision::BulletCollision() :
	Dispatcher{ &Configuration },
	World{ &Dispatcher, &Broadphase, &Configuration }
{}

void BulletCollision::SetTotalCounts(size_t VertexCount, size_t IndexCount)
{
	Vertices.reserve(VertexCount);
	Indices.reserve(IndexCount);
}

void BulletCollision::AddTriangles(v3* Vertices, size_t VertexCount,
	IndexType* Indices, size_t IndexCount)
{
	// Check if the vectors can hold this many verts and indices without reallocating.
	// Reallocation would invalidate pointers and iterators, making the m_VertexBase
	// and m_triangleIndexBase of meshes we've added invalid.
	auto CanHold = [&](auto& vec, auto count) {
		return vec.capacity() >= vec.size() + count;
	};
	assert(CanHold(this->Vertices, VertexCount) && CanHold(this->Indices, IndexCount));

	btIndexedMesh mesh;
	mesh.m_triangleIndexStride = 3 * sizeof(IndexType);
	mesh.m_vertexStride = sizeof(v3);

	assert(IndexCount % 3 == 0);
	mesh.m_numVertices = VertexCount;
	mesh.m_numTriangles = IndexCount / 3;

	int VertexBase = this->Vertices.size();
	int IndexBase = this->Indices.size();

	auto AddStuff = [&](auto& vec, auto* stuff, size_t count) {
		int base = vec.size();
		vec.resize(vec.size() + count);
		auto* ptr = vec.data() + base;
		memcpy(ptr, stuff, count * sizeof(*stuff));
		return ptr;
	};

	mesh.m_vertexBase = reinterpret_cast<const unsigned char*>(
		AddStuff(this->Vertices, Vertices, VertexCount));
	mesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(
		AddStuff(this->Indices, Indices, IndexCount));

	Triangles.addIndexedMesh(mesh, GetScalarType<IndexType>());
}

void BulletCollision::Build()
{
	Vertices.shrink_to_fit();
	Indices.shrink_to_fit();

	if (Triangles.getIndexedMeshArray().size() == 0)
	{
		MLog("BulletCollision::Build -- No triangles\n");
		return;
	}

	Mesh.emplace(&Triangles, true, false);
	Mesh.value().buildOptimizedBvh();

	Object.setCollisionShape(&Mesh.value());
	btTransform t;
	t.setIdentity();
	Object.setWorldTransform(t);
	Object.setCollisionFlags(Object.getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	World.addCollisionObject(&Object);
}

void BulletCollision::Clear()
{
	Triangles.getIndexedMeshArray().clear();
	Mesh.reset();
	Vertices.clear();
	Indices.clear();
	World.removeCollisionObject(&Object);
}

template <typename T>
static bool SetResults(const T& Result,
	const btVector3& Src, const btVector3& Dest,
	v3* Hit, v3* Normal)
{
	if (!Result.hasHit())
		return false;

	btVector3 point;
	point.setInterpolate3(Src, Dest, Result.m_closestHitFraction);
	if (Hit) *Hit = v3{ point.x(), point.y(), point.z() };
	if (Normal) *Normal = v3{ Result.m_hitNormalWorld.x(), Result.m_hitNormalWorld.y(), Result.m_hitNormalWorld.z() };

	return true;
}

bool BulletCollision::Pick(const v3 & Src, const v3 & Dest,
	v3 * Hit, v3 * Normal)
{
	btVector3 btSrc{ EXPAND_VECTOR(Src) }, btDest{ EXPAND_VECTOR(Dest) };
	btCollisionWorld::ClosestRayResultCallback cr{ btSrc, btDest };

	World.rayTest(btSrc, btDest, cr);

	return SetResults(cr, btSrc, btDest, Hit, Normal);
}

bool BulletCollision::CheckCylinder(const v3 & Src, const v3 & Dest,
	float Radius, float Height,
	v3 * Hit, v3 * Normal)
{
	btVector3 btSrc{ EXPAND_VECTOR(Src) }, btDest{ EXPAND_VECTOR(Dest) };
	btCollisionWorld::ClosestConvexResultCallback cc(btSrc, btDest);

	btTransform tFrom, tTo;
	tFrom.setIdentity();
	tTo.setIdentity();

	btVector3 btCyl{ Radius, Radius, Height * 0.5f };

	btVector3 HalfCylinderHeight{ 0, 0, Height * 0.5f };
	auto btAdjSrc = btSrc + HalfCylinderHeight;
	auto btAdjDest = btDest + HalfCylinderHeight;

	if ((btAdjSrc - btAdjDest).fuzzyZero())
	{
		tFrom.setOrigin(btSrc + btVector3{ 0, 0, Height * 0.75f });
		tTo.setOrigin(btSrc + btVector3{ 0, 0, Height * 0.25f });
		btCyl[2] = Height * 0.25f;
	}
	else
	{
		tFrom.setOrigin(btAdjSrc);
		tTo.setOrigin(btAdjDest);
	}

	btCylinderShapeZ cylinder{ btCyl };

	World.convexSweepTest(&cylinder, tFrom, tTo, cc);

	return SetResults(cc, btSrc, btDest, Hit, Normal);
}

bool BulletCollision::CheckSphere(const v3& Src, const v3& Dest,
	float Radius,
	v3* Hit, v3* Normal)
{
	btSphereShape sphere{ Radius };

	btVector3 btSrc{ EXPAND_VECTOR(Src) }, btDest{ EXPAND_VECTOR(Dest) };
	btCollisionWorld::ClosestConvexResultCallback cc{ btSrc, btDest };
	btTransform tFrom, tTo;
	tFrom.setIdentity();
	tFrom.setOrigin(btSrc);
	tTo.setIdentity();
	tTo.setOrigin(btDest);

	World.convexSweepTest(&sphere, tFrom, tTo, cc);

	return SetResults(cc, btSrc, btDest, Hit, Normal);
}
