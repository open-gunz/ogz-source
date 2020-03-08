#pragma once
#include <vector>
#include "RTypes.h"

class HitboxManager
{
public:
	HitboxManager() { }

	void Create();
	void Draw();
	void Update();
	void OnSlash(const rvector &Pos, const rvector &Dir);
	void OnMassive(const rvector &Pos);
	void AddEnemyPositions();

private:
	enum Type
	{
		SLASH,
		MASSIVE,
		POS,
		TYPE_END,
	};
	struct Info
	{
		rvector Pos, Dir;
		Type type;
		float Time;
		rmatrix World;
	};
	std::vector<Info> List;

	struct Vertex
	{
		float x, y, z;
	};
	D3DPtr<IDirect3DVertexBuffer9> pVB;

	bool Enabled() const;
};