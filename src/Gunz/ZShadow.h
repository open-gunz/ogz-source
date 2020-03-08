#pragma once

#include "RTypes.h"

class ZShadow
{
public:
	bool SetMatrices(RealSpace2::RVisualMesh& VisualMesh,
		RealSpace2::RBspObject& Map,
		float Size = DefaultSize);
	void Draw();

	static constexpr float DefaultSize = 100;

private:
	struct ShadowType {
		bool Visible{};
		float DistanceToFloorSquared{};
		rmatrix World = IdentityMatrix();
	} Shadows[2]; // 0 = left, 1 = right
};
