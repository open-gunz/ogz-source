#include "stdafx.h"
#include "ZGameDrawD3D9.h"
#include "ShaderGlobals.h"
#include "RealSpace2.h"
#include "Config.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZConfiguration.h"
#include "RDynamicLight.h"
#include "ZGameClient.h"
#include "ZWorldItem.h"
#include "RLenzFlare.h"
#include "ZEffectFlashBang.h"
#include "ZStencilLight.h"
#include "RGMain.h"
#include "Portal.h"
#include <fstream>
#include "defer.h"
#include "RUtil.h"
#include "RS2.h"
#include "Renderer.inl"
#include "RBspObject.h"
#include "Monochrome.h"
#include "ShaderUtil.h"

using namespace RealSpace2;

ZGameDrawD3D9::ZGameDrawD3D9(ZGame& Game) : Game(Game) {}

static void SetProjection()
{
#ifdef PORTAL
	if (g_pPortal->ForcingProjection())
		return;
#endif
	
	RSetProjection(GetFOV(), DEFAULT_NEAR_Z, g_fFarZ);
}

static void SetStatesPreDraw(ZGame& Game)
{
	auto profPreDraw = MBeginProfile("ZGame::OnPreDraw");

	SetProjection();

	bool bTrilinear = RIsTrilinear();

	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	if (Game.m_bShowWireframe) {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		Game.GetWorld()->SetFog(false);
	}
	else {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		Game.GetWorld()->SetFog(true);
	}

	Game.GetWorld()->GetBsp()->SetWireframeMode(Game.m_bShowWireframe);

	auto Identity = GetIdentityMatrix();
	RSetTransform(D3DTS_WORLD, Identity);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);

	RGetDevice()->SetTexture(0, nullptr);
	RGetDevice()->SetTexture(1, nullptr);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	RGetDevice()->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	if (Game.m_pMyCharacter && ZGetConfiguration()->GetVideo()->bDynamicLight)
	{
		auto pos = Game.m_pMyCharacter->GetPosition();
		RGetDynamicLightManager()->SetPosition(pos);
	}

	MEndProfile(profPreDraw);
}

#define PROFILE_IMPL(expr, name) [&]{ auto name = MBeginProfile(#expr); { expr; } MEndProfile(name); }()
#define PROFILE(expr) PROFILE_IMPL(expr, TOKENIZE(prof, __COUNTER__))

void ZGameDrawD3D9::DrawScene()
{
	auto profZGameDraw = MBeginProfile("ZGame::Draw");

	// Draw portals. This must happen before the rest of the draw
	// so that there are no depth values overriding the render
	// of the world from the portals' perspectives.
#ifdef PORTAL
	g_pPortal->PreDraw();
#endif

	GetRGMain().OnPreDrawGame();

	SetStatesPreDraw(Game);

	GetRenderer().PostProcess.Begin();

	{
		// Save the world matrix because, apparently, something in this block mutates it.
		rmatrix OrigWorld = RGetTransform(D3DTS_WORLD);

		// Draw the world, i.e. the
		// - Skybox
		// - Static parts of the map
		// - Emblem flags (e.g. the red flags from Mansion).
		PROFILE(Game.GetWorld()->Draw());

		if (Game.GetMapDesc())
			Game.GetMapDesc()->DrawMapDesc();

		if (Game.GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
		{
			// Draw all ZObjects, such as players and quest monsters.
			PROFILE(Game.m_ObjectManager.Draw());
			IF_DEBUG(Game.m_render_poly_cnt = RealSpace2::g_poly_render_cnt;);
		}

		RSetTransform(D3DTS_WORLD, OrigWorld);
	}

	// Draw world items, such as medkits on the ground and HP, AP, and ammo spawns.
	// This pass draws world items that are below water, since we haven't rendered it yet.
	ZGetWorldItemManager()->Draw(0, Game.GetWorld()->GetWaterHeight(), Game.GetWorld()->IsWaterMap());

	// Draw "weapons," such as grenades and medkits in flight.
	PROFILE(Game.m_WeaponManager.Render());
	// Draw map objects. Not sure what an example of these would be?
	PROFILE(Game.GetWorld()->GetBsp()->DrawObjects());
	// Draw water
	PROFILE(Game.GetWorld()->GetWaters()->Render());

	// Draw effects
	// TODO: Remove whatever code is using the time.
	if (Game.GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
		PROFILE(ZGetEffectManager()->Draw(GetGlobalTimeMS()));

	// Draw world items above water.
	PROFILE(ZGetWorldItemManager()->Draw(1, Game.GetWorld()->GetWaterHeight(), Game.GetWorld()->IsWaterMap()));

	// Draw particles
	PROFILE(RGetParticleSystem()->Draw());

	// Draw lens flare from the sun.
	if (RReadyLenzFlare())
		PROFILE(RGetLenzFlare()->Render(RCameraPosition, Game.GetWorld()->GetBsp()));

	PROFILE(GetRGMain().OnDrawGame());

	SetProjection();
	RSetFog(FALSE);

	// Draw the blinding effect of flashbangs
	if (IsActivatedFlashBangEffect())
		PROFILE(ShowFlashBangEffect());

	// Draw dynamic lighting
	if (Z_VIDEO_DYNAMICLIGHT)
		PROFILE(ZGetStencilLight()->Render());

	PROFILE(Game.GetMatch()->OnDrawGameMessage());

	PROFILE(g_pPortal->PostDraw());

	GetRenderer().PostProcess.End();

	MEndProfile(profZGameDraw);
}
#undef PROFILE

void ZGameDrawD3D9::Draw()
{
	// Don't render if the device is lost
	if (RIsReadyToRender() == R_NOTREADY)
		return;

	DrawScene();
}