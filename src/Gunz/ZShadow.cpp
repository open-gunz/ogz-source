#include "stdafx.h"
#include "ZShadow.h"
#include "RBspObject.h"
#include "MDebug.h"

// Maximum distance to the floor before the shadow disappears
constexpr float MaxShadowDistance = 250;
constexpr float MaxShadowDistanceSquared = Square(MaxShadowDistance);

static int GetAlpha(float DistanceToFloorSquared)
{
	if (DistanceToFloorSquared > MaxShadowDistanceSquared) {
		// Out of range. 0% alpha.
		return 0;
	}
	else if (DistanceToFloorSquared <= 0) {
		// Shadow is above the foot position?
		// Strange but whatever, 100% alpha.
		return 255;
	}
	else {
		// Scale the alpha based on distance.
		const auto BlendFactor = 1 - (DistanceToFloorSquared / MaxShadowDistanceSquared);
		return static_cast<int>(BlendFactor * 255);
	}
}

void ZShadow::Draw()
{
	for (auto&& Shadow : Shadows)
	{
		if (!Shadow.Visible)
			continue;

		const auto Alpha = GetAlpha(Shadow.DistanceToFloorSquared);
		if (Alpha == 0)
			continue;

		const auto Color = ARGB(Alpha, 255, 255, 255);
		ZGetEffectManager()->AddShadowEffect(Shadow.World, Color);
	}
}

bool ZShadow::SetMatrices(RVisualMesh& VisualMesh, RBspObject& Map, float Size)
{
	const v3 FootPositions[] = {
		VisualMesh.GetLFootPosition(),
		VisualMesh.GetRFootPosition(),
	};

	for (int ShadowIndex = 0; ShadowIndex < int(std::size(Shadows)); ++ShadowIndex)
	{
		auto&& Shadow = Shadows[ShadowIndex];
		Shadow.Visible = true;
		const auto& FootPosition = FootPositions[ShadowIndex];

		const v3 Down{ 0, 0, -1 };
		v3 Dir, FloorPosition;
		if (!Map.GetShadowPosition(FootPosition, Down, &Dir, &FloorPosition))
		{
			if (ZGetGame()) {
				FloorPosition = ZGetGame()->GetFloor(FootPosition);
			}
			else {
				Shadow.Visible = false;
			}
		}

		Shadow.DistanceToFloorSquared = MagnitudeSq(FootPosition - FloorPosition);
		Shadow.Visible = Shadow.DistanceToFloorSquared <= MaxShadowDistanceSquared &&
			FloorPosition.z < FootPosition.z;

		if (!Shadow.Visible) {
			continue;
		}

		Shadow.World = ScalingMatrix(Size) * TranslationMatrix(FloorPosition + v3{ 0, 0, 1 });
	}

	return Shadows[0].Visible || Shadows[1].Visible;
}
