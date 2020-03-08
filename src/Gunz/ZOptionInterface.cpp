#include "StdAfx.h"
#include "ZOptionInterface.h"
#include "MSlider.h"
#include "ZConfiguration.h"
#include "ZActionKey.h"
#include "FileInfo.h"
#include "ZCanvas.h"
#include "ZInput.h"
#include "ZRoomListBox.h"
#include "RGMain.h"
#include "RBspObject.h"
#include "RS2.h"
#include "FunctionalListener.h"

#define DEFAULT_SLIDER_MAX			10000

#define DEFAULT_GAMMA_SLIDER_MIN	50
#define DEFAULT_GAMMA_SLIDER_MAX	800

static	map< int, D3DDISPLAYMODE> gDisplayMode;
auto find_ddm(const D3DDISPLAYMODE& ddm)
{
	return std::find_if(gDisplayMode.begin(), gDisplayMode.end(),
		[&](auto& val) { return val.second == ddm; });
}

#define DEFAULT_REFRESHRATE			0

bool operator == ( D3DDISPLAYMODE lhs, D3DDISPLAYMODE rhs )
{
	return( lhs.Width == rhs.Width && lhs.Height == rhs.Height && lhs.Format == rhs.Format );
}

static int widths[]={ 640,800,1024,1280,1600 };
static int heights[]={ 480,600,768,960,1200 };


ZOptionInterface::ZOptionInterface(void)
{
	mbTimer = false;

	mnOldBpp = D3DFMT_A8R8G8B8;
	mTimerTime = 0;

	mOldScreenWidth = 800;
	mOldScreenHeight = 600;

}

static auto SliderClamp(int Value)
{
	return max(0, min(DEFAULT_SLIDER_MAX, Value));
}

void ZOptionInterface::SetListeners()
{
	Listen("MouseSensitivitySlider", [this](MWidget* Widget, const char* Message) {
		if (!MWidget::IsMsg(Message, MLIST_VALUE_CHANGED))
			return false;

		auto* Slider = static_cast<MSlider*>(Widget);

		Sensitivity = Slider->GetValue();
		
		if (auto&& Edit = ZFindWidgetAs<MEdit>("MouseSensitivityEdit"))
		{
			char String[64];
			sprintf_safe(String, "%d", Sensitivity);
			Edit->SetText(String);
		}

		return true;
	});

	Listen("MouseSensitivityEdit", [this](MWidget* Widget, const char* Message) {
		if (!MWidget::IsMsg(Message, MEDIT_CHAR_MSG))
			return false;

		auto IsIllegalChar = [](char c) {
			return !isdigit(c);
		};
		StringView Text = Widget->GetText();
		auto it = std::find_if(Text.begin(), Text.end(), IsIllegalChar);
		if (it != Text.end())
		{
			char FilteredText[512];
			size_t i = 0;
			for (char c : Text)
			{
				if (IsIllegalChar(c))
					continue;

				FilteredText[i] = c;
				++i;
				if (i == std::size(FilteredText) - 1)
				{
					break;
				}
			}
			FilteredText[i] = 0;

			Widget->SetText(FilteredText);
		}

		Sensitivity = StringToInt<int>(Text).value_or(Sensitivity);

		if (auto&& Slider = ZFindWidgetAs<MSlider>("MouseSensitivitySlider"))
		{
			Slider->SetValue(SliderClamp(Sensitivity));
		}

		return true;
	});
}

void ZOptionInterface::InitInterfaceOption(void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	mlog("ZOptionInterface::InitInterfaceOption\n");

	Sensitivity = int(round(Z_MOUSE_SENSITIVITY * DEFAULT_SLIDER_MAX));

	MSlider* pWidget = (MSlider*)pResource->FindWidget("MouseSensitivitySlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(SliderClamp(Sensitivity));
	}

	if (auto&& Widget = ZFindWidgetAs<MEdit>("MouseSensitivityEdit"))
	{
		char buf[64];
		sprintf_safe(buf, "%d", Sensitivity);
		Widget->SetText(buf);
	}

	pWidget = (MSlider*)pResource->FindWidget("JoystickSensitivitySlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(ZGetConfiguration()->GetJoystick()->fSensitivity * DEFAULT_SLIDER_MAX);
	}

	pWidget = (MSlider*)pResource->FindWidget("BGMVolumeSlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(Z_AUDIO_BGM_VOLUME*DEFAULT_SLIDER_MAX);
	}

	pWidget = (MSlider*)pResource->FindWidget("EffectVolumeSlider");
	if(pWidget)
	{
		pWidget->SetMinMax(0, DEFAULT_SLIDER_MAX);
		pWidget->SetValue(Z_AUDIO_EFFECT_VOLUME*DEFAULT_SLIDER_MAX);
	}

	pWidget = (MSlider*)pResource->FindWidget("VideoGamma");
	if(pWidget)
	{
		pWidget->SetMinMax(DEFAULT_GAMMA_SLIDER_MIN, DEFAULT_GAMMA_SLIDER_MAX);
		pWidget->SetValue(Z_VIDEO_GAMMA_VALUE*DEFAULT_GAMMA_SLIDER_MAX);
		pWidget->SetValue(Z_VIDEO_GAMMA_VALUE);
	}

	// Action Key
	for(int i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf_safe(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);

		BEGIN_WIDGETLIST(szItemName, pResource, ZActionKey*, pWidget);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt);
		END_WIDGETLIST();
	}

	//	ComboBox
	{
		MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
		if(pWidget)
		{
			pWidget->RemoveAll();
			gDisplayMode.clear();

			int dmIndex = 0;
			char szBuf[256];

			D3DDISPLAYMODE ddm;

			D3DFORMAT Formats[] = {
				D3DFMT_X8R8G8B8,
				//D3DFMT_R5G6B5
			};

			for (auto& Format : Formats)
			{
				int nDM = RGetAdapterModeCount(Format);

				mlog("Number of display mode for format %d: %d\n", Format, nDM );

				for( int idm = 0 ; idm < nDM; ++idm )
				{
					if (REnumAdapterMode( D3DADAPTER_DEFAULT, Format, idm, &ddm))
					{
						ddm.RefreshRate = DEFAULT_REFRESHRATE;

						if(ddm.Format == D3DFMT_X8R8G8B8 || ddm.Format == D3DFMT_R5G6B5)
						{
							auto iter_ = find_ddm(ddm);
							if( iter_ == gDisplayMode.end() )
							{
								gDisplayMode.insert({ dmIndex++, ddm });
								sprintf_safe(szBuf, "%d x %d %dbpp", ddm.Width, ddm.Height,
									ddm.Format == D3DFMT_X8R8G8B8 ? 32 : 16);
								pWidget->Add(szBuf);
							}
						}
					}
				}
			}

			if (gDisplayMode.size() == 0)
			{
				for (int i = 0; i < 10; ++i)
				{
					ddm.Width = widths[i / 2];
					ddm.Height = heights[i / 2];
					ddm.RefreshRate = DEFAULT_REFRESHRATE;
					ddm.Format = ((i % 2 == 1) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5);

					int bpp = (i % 2 == 1) ? 32 : 16;
					gDisplayMode.insert(map<int, D3DDISPLAYMODE>::value_type(i, ddm));
					sprintf_safe(szBuf, "%dx%d  %d bpp", ddm.Width, ddm.Height, bpp);
					pWidget->Add(szBuf);
				}
			}
			ddm.Width = RGetScreenWidth();
			ddm.Height = RGetScreenHeight();
			ddm.RefreshRate = DEFAULT_REFRESHRATE;
			ddm.Format	= RGetPixelFormat();
			auto iter = find_ddm(ddm);
			pWidget->SetSelIndex( iter->first );
		}

		pWidget = ZFindWidgetAs<MComboBox>("FullscreenMode");
		if (pWidget)
		{
			pWidget->SetSelIndex(int(Z_VIDEO_FULLSCREEN));
		}

		pWidget = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nCharTexLevel == 8 ? 3 : ZGetConfiguration()->GetVideo()->nCharTexLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nMapTexLevel == 8 ? 3 : ZGetConfiguration()->GetVideo()->nMapTexLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nEffectLevel);
		}

		pWidget = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pWidget)	{
			pWidget->SetSelIndex(ZGetConfiguration()->GetVideo()->nTextureFormat);
		}

	}

	//	Button
	{
		MButton* pWidget = (MButton*)pResource->FindWidget("Reflection");
		if( pWidget )
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bReflection);
		}

		pWidget = (MButton*)pResource->FindWidget("LightMap");
		if (pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bLightMap);

			if (ZGetGame()) {
				ZGetGame()->GetWorld()->GetBsp()->LightMapOnOff(ZGetConfiguration()->GetVideo()->bLightMap);
			}
			else {
				RBspObject::SetDrawLightMap(ZGetConfiguration()->GetVideo()->bLightMap);
			}
		}

		pWidget = (MButton*)pResource->FindWidget("DynamicLight");
		if( pWidget )
		{
			pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bDynamicLight);
		}

		pWidget = (MButton*)pResource->FindWidget("Shader");
		if( pWidget )
		{
			//if( !RShaderMgr::shader_enabled )
			if(!RIsSupportVS())
			{
				pWidget->SetCheck(false);
				pWidget->Enable(false);
			}
			else
			{
				pWidget->SetCheck(ZGetConfiguration()->GetVideo()->bShader);
			}
		}

		pWidget = (MButton*)pResource->FindWidget("BGMMute");
		if(pWidget)
		{
			pWidget->SetCheck( !ZGetConfiguration()->GetAudio()->bBGMMute );
		}

		pWidget = (MButton*)pResource->FindWidget("EffectMute");
		if(pWidget)
		{
			pWidget->SetCheck( !ZGetConfiguration()->GetAudio()->bEffectMute );
		}

		pWidget	= (MButton*)pResource->FindWidget("Effect3D");
		if(pWidget)
		{
			pWidget->SetCheck(ZGetConfiguration()->GetAudio()->b3DSound);
		}

		pWidget = (MButton*)pResource->FindWidget("8BitSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_8BITSOUND);
			pWidget = (MButton*)pResource->FindWidget("16BitSound");
			if(pWidget) pWidget->SetCheck(!Z_AUDIO_8BITSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("InverseSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_INVERSE);
		}
		pWidget = (MButton*)pResource->FindWidget("HWMixing");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_HWMIXING);
		}
		pWidget = (MButton*)pResource->FindWidget("HitSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_HITSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("NarrationSound");
		if(pWidget)
		{
			pWidget->SetCheck(Z_AUDIO_NARRATIONSOUND);
		}
		pWidget = (MButton*)pResource->FindWidget("InvertMouse");
		if(pWidget)
		{
			pWidget->SetCheck(Z_MOUSE_INVERT);
		}
	}

	// Etc
	{	
		MEdit* pEdit = (MEdit*)pResource->FindWidget("NetworkPort1");
		if (pEdit)
		{
			char szBuf[64];
			sprintf_safe(szBuf, "%d", Z_ETC_NETWORKPORT1);
			pEdit->SetText(szBuf);
		}

		pEdit = (MEdit*)pResource->FindWidget("NetworkPort2");
		if (pEdit)
		{
			char szBuf[64];
			sprintf_safe(szBuf, "%d", Z_ETC_NETWORKPORT2);
			pEdit->SetText(szBuf);
		}

		MButton* pBtnBoost = (MButton*)pResource->FindWidget("BoostOption");
		if (pBtnBoost)
		{
			pBtnBoost->SetCheck(Z_ETC_BOOST);
		}

		MButton* pBtnNormalChat = (MButton*)pResource->FindWidget("NormalChatOption");
		if (pBtnNormalChat)
		{
			pBtnNormalChat->SetCheck( Z_ETC_REJECT_NORMALCHAT);
		}

		MButton* pBtnTeamChat = (MButton*)pResource->FindWidget("TeamChatOption");
		if (pBtnTeamChat)
		{
			pBtnTeamChat->SetCheck(Z_ETC_REJECT_TEAMCHAT);
		}

		MButton* pBtnClanChat = (MButton*)pResource->FindWidget("ClanChatOption");
		if (pBtnClanChat)
		{
			pBtnClanChat->SetCheck(Z_ETC_REJECT_CLANCHAT);
		}

		MButton* pBtnWhisper = (MButton*)pResource->FindWidget("WhisperOption");
		if (pBtnWhisper)
		{
			pBtnWhisper->SetCheck(Z_ETC_REJECT_WHISPER);
		}

		MButton* pBtnInvite = (MButton*)pResource->FindWidget("InviteOption");
		if (pBtnInvite)
		{
			pBtnInvite->SetCheck(Z_ETC_REJECT_INVITE);
		}

		MComboBox *pComboBox = (MComboBox*)pResource->FindWidget("CrossHairComboBox");
		if(pComboBox)
		{
			pComboBox->RemoveAll();

			// 기본 크로스 헤어 하드코딩으로 입력

			for (int i = 0; i < ZCSP_CUSTOM; i++)
			{
				char szText[256];
				sprintf_safe(szText, "%s %d", ZMsg(MSG_WORD_TYPE), i+1);
				pComboBox->Add(szText);
			}
			char szCustomFile[256];
			sprintf_safe(szCustomFile, "%s%s%s", PATH_CUSTOM_CROSSHAIR, FN_CROSSHAIR_HEADER, FN_CROSSHAIR_TAILER);
			if (IsExist(szCustomFile)) pComboBox->Add("Custom");

			if (Z_ETC_CROSSHAIR >= pComboBox->GetCount())	// 사용자지정이였는데 사용자지정이 없어졌을 경우
			{
				Z_ETC_CROSSHAIR = 0;
			}
			pComboBox->SetSelIndex(Z_ETC_CROSSHAIR);			
		}

		ZCanvas* pCrossHairPreview = (ZCanvas*)pResource->FindWidget("CrossHairPreviewCanvas");
		if (pCrossHairPreview)
		{
			pCrossHairPreview->SetOnDrawCallback(ZCrossHair::OnDrawOptionCrossHairPreview);
		}
	}

	//Macro
	{
		static char stemp_str[ZCONFIG_MACRO_MAX][80] = {
			"MacroF1",
			"MacroF2",
			"MacroF3",
			"MacroF4",
			"MacroF5",
			"MacroF6",
			"MacroF7",
			"MacroF8"
		};

		ZCONFIG_MACRO* pMacro = ZGetConfiguration()->GetMacro();

		if(pMacro) {

			MEdit* pEdit = NULL;

			for(int i=0;i<ZCONFIG_MACRO_MAX;i++) {

				pEdit = (MEdit*) pResource->FindWidget(stemp_str[i]);

				if (pEdit) {

					pEdit->SetText( pMacro->GetString(i) );

				}
			}
		}
	}

	{
		auto&& Cfg = *ZGetConfiguration();
		std::pair<const char*, bool> Buttons[] = {
			{"HitboxOption",             Cfg.GetShowHitboxes()},
			{"DrawTrailsOption",         Cfg.GetDrawTrails()},
			{"CamFixOption",             Cfg.GetCamFix()},
			{"InterfaceFixOption",       Cfg.GetInterfaceFix()},
			{"SlashEffectOption",        Cfg.GetSlashEffect()},
			{"UnlockedDirOption",        Cfg.GetUnlockedDir()},
			{"ShowDebugInfoOption",      Cfg.GetShowDebugInfo()},
			{"ChatFontBoldOption",       Cfg.GetChat()->BoldFont},
			{"ColorInvertOption",        Cfg.GetColorInvert()},
			{"MonochromeOption",         Cfg.GetMonochrome()},
			{"AsyncScreenshotsOption",   Cfg.AsyncScreenshots},
			{"FastWeaponCycleOption",    Cfg.FastWeaponCycle},
		};

		for (auto&& ButtonInfo : Buttons)
		{
			auto ButtonWidget = ZFindWidgetAs<MButton>(ButtonInfo.first);
			if (ButtonWidget)
				ButtonWidget->SetCheck(ButtonInfo.second);
		}

		char StringFOV[64];
		sprintf_safe(StringFOV, "%.2f", Cfg.GetFOV());
		char StringFontSize[64];
		sprintf_safe(StringFontSize, "%d", Cfg.GetChat()->FontSize);
		char StringBackgroundColor[64];
		sprintf_safe(StringBackgroundColor, "%08X", Cfg.GetChat()->BackgroundColor);
		char StringLogicalFPSLimit[64];
		sprintf_safe(StringLogicalFPSLimit, "%d", Cfg.GetLogicalFPSLimit());
		char StringVisualFPSLimit[64];
		sprintf_safe(StringVisualFPSLimit, "%d", Cfg.GetVisualFPSLimit());

		std::pair<const char*, const char*> Edits[] = {
			{"FOVOption",                 StringFOV},
			{"ChatFontOption",            Cfg.GetChat()->Font.c_str()},
			{"ChatFontSizeOption",        StringFontSize},
			{"ChatBackgroundColorOption", StringBackgroundColor},
			{"LogicalFPSLimitOption",     StringLogicalFPSLimit},
			{"VisualFPSLimitOption",      StringVisualFPSLimit},
		};

		for (auto&& EditInfo : Edits)
		{
			auto EditWidget = ZFindWidgetAs<MEdit>(EditInfo.first);
			if (EditWidget)
				EditWidget->SetText(EditInfo.second);
		}

		std::pair<const char*, int> ComboBoxes[] = {
			{"ScreenshotFormatOption", int(Cfg.ScreenshotFormat)},
		};

		for (auto&& ComboBoxInfo : ComboBoxes)
		{
			auto Widget = ZFindWidgetAs<MComboBox>(ComboBoxInfo.first);
			if (Widget)
				Widget->SetSelIndex(ComboBoxInfo.second);
		}

#ifndef ENABLE_FOV_OPTION
		if (auto&& Widget = ZFindWidget("FOVOption"))
		{
			Widget->Enable(false);
		}
#endif
	}

	//mlog("ZGameInterface::InitInterfaceOption ok\n");
}

bool ZOptionInterface::SaveInterfaceOption(void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	{ // 슬라이더
		Z_MOUSE_SENSITIVITY = float(Sensitivity) / DEFAULT_SLIDER_MAX;;

		auto&& pWidget = (MSlider*)pResource->FindWidget("JoystickSensitivitySlider");
		Z_JOYSTICK_SENSITIVITY = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;

		pWidget = (MSlider*)pResource->FindWidget("BGMVolumeSlider");
		if(pWidget)
		{
			Z_AUDIO_BGM_VOLUME = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;
			ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME) ;
		}

		pWidget = (MSlider*)pResource->FindWidget("EffectVolumeSlider");
		if(pWidget)
		{
			Z_AUDIO_EFFECT_VOLUME = (float) ((MSlider*)pWidget)->GetValue() / (float)DEFAULT_SLIDER_MAX;

			ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
		}

	}

	int i=0;

	//for(i=0; i<ZACTION_COUNT; i++) {
	//	ZGetInput()->UnregisterActionKey(i);
	//}

	// 모두 클리어후 재등록
	ZGetInput()->ClearActionKey();

	for(i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf_safe(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)pResource->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		int nKey = 0;
		pWidget->GetActionKey(&nKey);
		//		Mint::GetInstance()->UnregisterActionKey(i);
		//		Mint::GetInstance()->RegisterActionKey(i, nKey);	// 키 등록

//		ZGetInput()->UnregisterActionKey(i);
		ZGetInput()->RegisterActionKey(i,nKey);

		ZVIRTUALKEY altKey;
		pWidget->GetActionAltKey(&altKey);
		if(altKey!=-1)
			ZGetInput()->RegisterActionKey(i,altKey);

		ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey = nKey;
		ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt = altKey;
	}

	{
		Z_VIDEO_WIDTH = RGetScreenWidth();
		Z_VIDEO_HEIGHT	= RGetScreenHeight();
		Z_VIDEO_BPP	= RGetPixelFormat()==D3DFMT_X8R8G8B8 ? 32:16 ;

		[&] {
			auto FullscreenModeWidget = ZFindWidgetAs<MComboBox>("FullscreenMode");
			if (!FullscreenModeWidget)
				return;

			auto SelIndex = FullscreenModeWidget->GetSelIndex();
			if (SelIndex < 0 || SelIndex > 2)
				return;

			auto NewFullscreenMode = static_cast<FullscreenType>(SelIndex);
			if (Z_VIDEO_FULLSCREEN == NewFullscreenMode)
				return;

			Z_VIDEO_FULLSCREEN = NewFullscreenMode;

			RMODEPARAMS ModeParams;
			ModeParams.nWidth = Z_VIDEO_WIDTH;
			ModeParams.nHeight = Z_VIDEO_HEIGHT;
			ModeParams.FullscreenMode = Z_VIDEO_FULLSCREEN;
			ModeParams.PixelFormat = RGetPixelFormat();

			RResetDevice(&ModeParams);
		}();

		MComboBox*	pWidget = (MComboBox*)pResource->FindWidget("CharTexLevel");

		int TexLevel = 0;
		u32 flag = 0;
		int EffectLevel = 0;
		int nTextureFormat = 0;

		if(pWidget)	{

			TexLevel = pWidget->GetSelIndex();
			if (!strcmp(pWidget->GetSelItemString(), "Archetype's"))
				TexLevel = 8;

			if( ZGetConfiguration()->GetVideo()->bTerrible ){
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				if( TexLevel == 2 )
					SetObjectTextureLevel(TexLevel+2);
				else
					SetObjectTextureLevel(TexLevel);

				flag |= static_cast<u32>(RTextureType::Object);
			}
			else if( ZGetConfiguration()->GetVideo()->nCharTexLevel != TexLevel ) {
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				SetObjectTextureLevel(TexLevel);
				flag |= static_cast<u32>(RTextureType::Object);
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("MapTexLevel");

		if(pWidget)	{

			TexLevel = pWidget->GetSelIndex();
			if (!strcmp(pWidget->GetSelItemString(), "Archetype's"))
				TexLevel = 8;

			if( ZGetConfiguration()->GetVideo()->bTerrible ){
				ZGetConfiguration()->GetVideo()->nCharTexLevel = TexLevel;
				if( TexLevel == 2 )
					SetObjectTextureLevel(TexLevel+2);
				else
					SetObjectTextureLevel(TexLevel);

				flag |= static_cast<u32>(RTextureType::Object);
			}
			if( ZGetConfiguration()->GetVideo()->nMapTexLevel != TexLevel ) {
				ZGetConfiguration()->GetVideo()->nMapTexLevel = TexLevel;
				SetMapTextureLevel(TexLevel);
				flag |= static_cast<u32>(RTextureType::Map);
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("EffectLevel");

		if(pWidget)	{

			EffectLevel = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->nEffectLevel != EffectLevel ) {
				ZGetConfiguration()->GetVideo()->nEffectLevel = EffectLevel;
				SetEffectLevel(EffectLevel);
			}
		}

		pWidget = (MComboBox*)pResource->FindWidget("TextureFormat");

		if(pWidget)	{

			nTextureFormat = pWidget->GetSelIndex();

			if( ZGetConfiguration()->GetVideo()->nTextureFormat != nTextureFormat ) {
				ZGetConfiguration()->GetVideo()->nTextureFormat = nTextureFormat;
				SetTextureFormat(nTextureFormat);
				flag = static_cast<u32>(RTextureType::All);
			}
		}

		if(flag) {
			RChangeBaseTextureLevel(static_cast<RTextureType>(flag));
		}
	}
	{
		MButton* pWidget = (MButton*)pResource->FindWidget("Reflection");
		if(pWidget)
		{
			Z_VIDEO_REFLECTION = pWidget->GetCheck();
		}

		pWidget = (MButton*)pResource->FindWidget("LightMap");
		if(pWidget)
		{
			Z_VIDEO_LIGHTMAP = pWidget->GetCheck();

			if(ZGetGame()) {
				ZGetGame()->GetWorld()->GetBsp()->LightMapOnOff(Z_VIDEO_LIGHTMAP);
			}
			else {
				RBspObject::SetDrawLightMap(Z_VIDEO_LIGHTMAP);
			}
		}

		pWidget = (MButton*)pResource->FindWidget("DynamicLight");
		if(pWidget)
		{
			Z_VIDEO_DYNAMICLIGHT = pWidget->GetCheck();
		}

		pWidget = (MButton*)pResource->FindWidget("Shader");
		if(pWidget)
		{
			Z_VIDEO_SHADER = pWidget->GetCheck();

			if( Z_VIDEO_SHADER )
			{
				RGetShaderMgr()->SetEnable();
			}
			else
			{
				RGetShaderMgr()->SetDisable();
			}
			//*/
		}

		pWidget	= (MButton*)pResource->FindWidget("BGMMute");
		if( pWidget )
		{
			Z_AUDIO_BGM_MUTE	= !(pWidget->GetCheck());

			ZGetSoundEngine()->SetMusicMute( Z_AUDIO_BGM_MUTE );
		}
		pWidget	= (MButton*)pResource->FindWidget("EffectMute");
		if(pWidget)
		{
			Z_AUDIO_EFFECT_MUTE = !(pWidget->GetCheck());
			ZGetSoundEngine()->SetEffectMute( Z_AUDIO_EFFECT_MUTE );

		}
		//pWidget	= (MButton*)pResource->FindWidget("Effect3D");
		//if(pWidget)
		//{
		//	Z_AUDIO_3D_SOUND = pWidget->GetCheck();
		//	ZGetSoundEngine()->Set3DSound( Z_AUDIO_3D_SOUND );
		//}
		pWidget = (MButton*)pResource->FindWidget("8BitSound");
		if(pWidget)
		{
			Z_AUDIO_8BITSOUND = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->SetSamplingBits(Z_AUDIO_8BITSOUND);
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("InverseSound");
		if(pWidget)
		{
			Z_AUDIO_INVERSE = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->SetInverseSound( Z_AUDIO_INVERSE );
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("HWMixing");
		if(pWidget)
		{
			Z_AUDIO_HWMIXING = pWidget->GetCheck();
#ifdef _BIRDSOUND

#else
			ZGetSoundEngine()->Reset(g_hWnd, Z_AUDIO_HWMIXING);
#endif
		}
		pWidget = (MButton*)pResource->FindWidget("HitSound");
		if(pWidget)
		{
			Z_AUDIO_HITSOUND = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("NarrationSound");
		if(pWidget)
		{
			Z_AUDIO_NARRATIONSOUND = pWidget->GetCheck();
		}
		pWidget = (MButton*)pResource->FindWidget("InvertMouse");
		if(pWidget)
		{
			Z_MOUSE_INVERT = pWidget->GetCheck();
		}
	}
	{	// Etc
		MEdit* pEdit = (MEdit*)pResource->FindWidget("NetworkPort1");
		if (pEdit)
		{
			int nPreviousPort = Z_ETC_NETWORKPORT1;
			Z_ETC_NETWORKPORT1 = atoi(pEdit->GetText());
		}

		pEdit = (MEdit*)pResource->FindWidget("NetworkPort2");
		if (pEdit)
		{
			int nPreviousPort = Z_ETC_NETWORKPORT2;
			Z_ETC_NETWORKPORT2 = atoi(pEdit->GetText());
		}

		MButton* pBoost = (MButton*)pResource->FindWidget("BoostOption");
		if(pBoost)
		{
			if (Z_ETC_BOOST != pBoost->GetCheck()) {
				Z_ETC_BOOST = pBoost->GetCheck();
				if (Z_ETC_BOOST)
					ZGetGameClient()->PriorityBoost(true);
				else
					ZGetGameClient()->PriorityBoost(false);
			}
		}

		MButton* pNormalChat = (MButton*)pResource->FindWidget("NormalChatOption");
		if(pNormalChat)
		{
			if (Z_ETC_REJECT_NORMALCHAT != pNormalChat->GetCheck()) {
				Z_ETC_REJECT_NORMALCHAT = pNormalChat->GetCheck();
				if (Z_ETC_REJECT_NORMALCHAT)
					ZGetGameClient()->SetRejectNormalChat(true);
				else
					ZGetGameClient()->SetRejectNormalChat(false);
			}
		}

		MButton* pTeamChat = (MButton*)pResource->FindWidget("TeamChatOption");
		if(pTeamChat)
		{
			if (Z_ETC_REJECT_TEAMCHAT != pTeamChat->GetCheck()) {
				Z_ETC_REJECT_TEAMCHAT = pTeamChat->GetCheck();
				if (Z_ETC_REJECT_TEAMCHAT)
					ZGetGameClient()->SetRejectTeamChat(true);
				else
					ZGetGameClient()->SetRejectTeamChat(false);
			}
		}

		MButton* pClanChat = (MButton*)pResource->FindWidget("ClanChatOption");
		if(pClanChat)
		{
			if (Z_ETC_REJECT_CLANCHAT != pClanChat->GetCheck()) {
				Z_ETC_REJECT_CLANCHAT = pClanChat->GetCheck();
				if (Z_ETC_REJECT_CLANCHAT)
					ZGetGameClient()->SetRejectClanChat(true);
				else
					ZGetGameClient()->SetRejectClanChat(false);
			}
		}

		MButton* pWhisper = (MButton*)pResource->FindWidget("WhisperOption");
		if(pWhisper)
		{
			if (Z_ETC_REJECT_WHISPER != pWhisper->GetCheck()) {
				Z_ETC_REJECT_WHISPER = pWhisper->GetCheck();
				if (Z_ETC_REJECT_WHISPER)
					ZGetGameClient()->SetRejectWhisper(true);
				else
					ZGetGameClient()->SetRejectWhisper(false);
				ZPostUserOption();
			}
		}

		MButton* pInvite = (MButton*)pResource->FindWidget("InviteOption");
		if(pInvite)
		{
			if (Z_ETC_REJECT_INVITE != pInvite->GetCheck()) {
				Z_ETC_REJECT_INVITE = pInvite->GetCheck();
				if (Z_ETC_REJECT_INVITE)
					ZGetGameClient()->SetRejectInvite(true);
				else
					ZGetGameClient()->SetRejectInvite(false);
				ZPostUserOption();
			}
		}

		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("CrossHairComboBox");
		if (pComboBox)
		{
			Z_ETC_CROSSHAIR = pComboBox->GetSelIndex();
		}
	}

	{
		auto&& Cfg = *ZGetConfiguration();

		const auto OldFont = Cfg.GetChat()->Font;
		const auto OldBoldFont = Cfg.GetChat()->BoldFont;
		const auto OldFontSize = Cfg.GetChat()->FontSize;

		std::pair<const char*, bool*> Buttons[] = {
			{ "HitboxOption",             &Cfg.bShowHitboxes },
			{ "DrawTrailsOption",         &Cfg.bDrawTrails },
			{ "CamFixOption",             &Cfg.bCamFix },
			{ "InterfaceFixOption",       &Cfg.InterfaceFix },
			{ "SlashEffectOption",        &Cfg.SlashEffect },
			{ "UnlockedDirOption",        &Cfg.UnlockedDir },
			{ "ShowDebugInfoOption",      &Cfg.ShowDebugInfo },
			{ "ChatFontBoldOption",       &Cfg.GetChat()->BoldFont },
			{ "ColorInvertOption",        &Cfg.ColorInvert },
			{ "MonochromeOption",         &Cfg.Monochrome },
			{ "AsyncScreenshotsOption",   &Cfg.AsyncScreenshots },
			{ "FastWeaponCycleOption",    &Cfg.FastWeaponCycle },
		};

		for (auto&& ButtonInfo : Buttons)
		{
			auto ButtonWidget = ZFindWidgetAs<MButton>(ButtonInfo.first);
			if (ButtonWidget)
				*ButtonInfo.second = ButtonWidget->GetCheck();
		}
		
		Mint::GetInstance()->SetStretch(!Cfg.InterfaceFix);

		GetRenderer().PostProcess.EnableEffect("ColorInvert", Cfg.ColorInvert);
		GetRenderer().PostProcess.EnableEffect("Monochrome", Cfg.Monochrome);

		using SetEditFunction = void(*)(const char*);
		std::pair<const char*, SetEditFunction> Edits[] = {
#ifdef ENABLE_FOV_OPTION
			{ "FOVOption",      [](const char* Value) {
				ZGetConfiguration()->FOV = atof(Value); } },
#endif
			{ "ChatFontOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->Font = Value; } },
			{ "ChatFontSizeOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->FontSize = StringToInt<int>(Value).value_or(16); } },
			{ "ChatBackgroundColorOption", [](const char* Value) {
				ZGetConfiguration()->GetChat()->BackgroundColor =
					StringToInt<u32, 16>(Value).value_or(0x80000000); } },
			{ "LogicalFPSLimitOption", [](const char* Value) {
				ZGetConfiguration()->LogicalFPSLimit = StringToInt<int>(Value).value_or(250); } },
			{ "VisualFPSLimitOption", [](const char* Value) {
				ZGetConfiguration()->VisualFPSLimit = StringToInt<int>(Value).value_or(0); } },
		};

		for (auto&& EditInfo : Edits)
		{
			auto EditWidget = ZFindWidgetAs<MEdit>(EditInfo.first);
			if (EditWidget)
				EditInfo.second(EditWidget->GetText());
		}

		int ScreenshotFormat = 2;
		std::pair<const char*, int*> ComboBoxes[] = {
			{ "ScreenshotFormatOption", &ScreenshotFormat },
		};

		for (auto&& ComboBoxInfo : ComboBoxes)
		{
			auto Widget = ZFindWidgetAs<MComboBox>(ComboBoxInfo.first);
			if (Widget)
				*ComboBoxInfo.second = Widget->GetSelIndex();
		}

		Cfg.ScreenshotFormat = ScreenshotFormatType(ScreenshotFormat);

		const auto& CurFont = Cfg.GetChat()->Font;
		const auto CurBoldFont = Cfg.GetChat()->BoldFont;
		const auto CurFontSize = Cfg.GetChat()->FontSize;
		if (!iequals(OldFont, CurFont) || OldBoldFont != CurBoldFont)
		{
			GetRGMain().GetChat().SetFont(CurFont, CurBoldFont);
		}
		if (OldFontSize != CurFontSize)
		{
			GetRGMain().GetChat().SetFontSize(CurFontSize);
		}

		GetRGMain().GetChat().SetBackgroundColor(Cfg.GetChat()->BackgroundColor);

		SetFOV(ToRadian(Cfg.FOV));
	}

	{
		// Macro

		static char stemp_str[ZCONFIG_MACRO_MAX][80] = {
			"MacroF1",
			"MacroF2",
			"MacroF3",
			"MacroF4",
			"MacroF5",
			"MacroF6",
			"MacroF7",
			"MacroF8"
		};

		ZCONFIG_MACRO* pMacro = ZGetConfiguration()->GetMacro();

		if(pMacro) {

			MEdit* pEdit = NULL;

			for(int i=0;i<ZCONFIG_MACRO_MAX;i++) {

				pEdit = (MEdit*) pResource->FindWidget(stemp_str[i]);

				if (pEdit) {

					pMacro->SetString(i,(char*)pEdit->GetText());
				}
			}
		}

	}

	// 감마값 저장
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		Z_VIDEO_GAMMA_VALUE = pSlider->GetValue();
	}

	ZGetConfiguration()->Save( Z_LOCALE_XML_HEADER);

	return true;
}



void ZOptionInterface::ShowResizeConfirmDialog( bool Resized )
{
	if( Resized )
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("ViewConfirm");
		if(pWidget!= 0)
			pWidget->Show( true, true );
	}
	else
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
		if(pWidget!= 0)
			pWidget->Show( true, true );
	}
}

bool ZOptionInterface::SetTimer( bool b, float time /* = 0.f  */ )
{
	static DWORD DeadTime = 0;

	if( !b )
	{
		mbTimer = b;
		return false;
	}

	if( !mbTimer )
	{
		mTimerTime	= GetGlobalTimeMS();
		mbTimer		= true;
		DeadTime		= time*1000;
	}

	if(( GetGlobalTimeMS() - mTimerTime ) > DeadTime )
	{
		DeadTime = 0;
		mbTimer	= false;
		return true;
	}
	else
	{
		char szBuf[128];
		sprintf_safe(szBuf, "%d", min(max( (10 - (int)(( GetGlobalTimeMS() - mTimerTime ) * 0.001)),0),10));

		char szText[ 128];
		ZTransMsg( szText, MSG_BACKTOTHEPREV, 1, szBuf);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MLabel* Countdown = (MLabel*)pResource->FindWidget( "ViewConfirm_CountDown" );
		if ( Countdown)
			Countdown->SetText( szText);
	}
	return false;
}

void ZOptionInterface::ShowNetworkPortConfirmDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("NetworkPortConfirm");
	if(pWidget!= 0) pWidget->Show( true, true );
}

bool ZOptionInterface::IsDiffNetworkPort()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	int nCurrPort = ntohs( ZGetGameClient()->GetSafeUDP()->GetLocalPort() );

	int nNewPort1, nNewPort2;

	MEdit* pEdit1 = (MEdit*)pResource->FindWidget("NetworkPort1");
	if ( pEdit1)
		nNewPort1 = atoi( pEdit1->GetText());
	else
		return false;


	MEdit* pEdit2 = (MEdit*)pResource->FindWidget("NetworkPort2");
	if ( pEdit2)
		nNewPort2 = atoi( pEdit2->GetText());
	else
		return false;


	if (nNewPort1 > nNewPort2)
	{
		char szStr[25];
		itoa_safe(Z_ETC_NETWORKPORT1, szStr, 10);
		pEdit1->SetText(szStr);
		itoa_safe(Z_ETC_NETWORKPORT2, szStr, 10);
		pEdit2->SetText(szStr);

		return false;
	}


	if ( ( nNewPort1 != Z_ETC_NETWORKPORT1) || ( nNewPort2 != Z_ETC_NETWORKPORT2))
		return true;

	return false;
}

void ZOptionInterface::OptimizationVideoOption()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pButton = 0;
	MComboBox* pCombo = 0;
	MLabel* pLabel = 0;

	ZGetConfiguration()->SetForceOptimization( true );

	if(!RIsHardwareTNL())
	{	
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(0); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("LightMap");
		if(pButton!=0) pButton->SetCheck(false);		
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(false);

		pCombo	= (MComboBox*)pResource->FindWidget("ScreenResolution");
		if( pCombo != 0)
		{
			D3DDISPLAYMODE ddm;
			ddm.Width = 640;
			ddm.Height = 480;
			ddm.Format = D3DFMT_R5G6B5;
			ddm.RefreshRate = DEFAULT_REFRESHRATE;
			map<int, D3DDISPLAYMODE>::iterator iter_ = find_ddm(ddm);
			if( iter_ != gDisplayMode.end() )
			{
				int n = iter_->first;
				pCombo->SetSelIndex( n );
			}
		}

		ZGetConfiguration()->GetVideo()->bTerrible = true;
		return;
	}	

	ZGetConfiguration()->GetVideo()->bTerrible = false;

	int nVMem = RGetApproxVMem() /1024 /1024;
	if( nVMem < 32 )
	{		
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(0); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(false);		
	}
	else if( nVMem < 64 )
	{
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 보통
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(2); // 나쁨
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 보통
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(0); // 16 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(false);
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(true);
	}
	else // nVMem > 64
	{
		pCombo = (MComboBox*)pResource->FindWidget("CharTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 보통
		pCombo = (MComboBox*)pResource->FindWidget("MapTexLevel");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 보통
		pCombo = (MComboBox*)pResource->FindWidget("EffectLevel");
		if(pCombo!=0) pCombo->SetSelIndex(0); // 좋음
		pCombo = (MComboBox*)pResource->FindWidget("TextureFormat");
		if(pCombo!=0) pCombo->SetSelIndex(1); // 32 bit

		pButton = (MButton*)pResource->FindWidget("Reflection");
		if(pButton!=0) pButton->SetCheck(true);		
		pButton = (MButton*)pResource->FindWidget("DynamicLight");
		if(pButton!=0) pButton->SetCheck(true);
	}

	//pLabel = (MLabel*)pResource->FindWidget("Lightmap Label");
	//if(pLabel!=0) pLabel->SetTextColor(MCOLOR(64,64,64));
	pButton = (MButton*)pResource->FindWidget("LightMap");
	if(pButton!=0) {
		pButton->SetCheck(true);		
		//pButton->Enable(false);
	}

	if(RIsSupportVS())
	{
		pButton = (MButton*)pResource->FindWidget("Shader");
		if(pButton!=0) pButton->SetCheck(true);
	}
	else
	{		
		pButton = (MButton*)pResource->FindWidget("Shader");
		if(pButton!=0) pButton->SetCheck(false);
	}
}

bool ZOptionInterface::ResizeWidgetRecursive( MWidget* pWidget, int w, int h)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	//MWidget* pWidget = pResource->FindWidget(szName);
	if(pWidget==NULL) return false;
	int n = pWidget->GetChildCount();
	for( int i = 0; i < n; ++i)
	{
		MWidget* pChildWidget = pWidget->GetChild(i);
		ResizeWidgetRecursive( pChildWidget, w, h );
	}

	// idl에서 읽어서 적절한 값을 가지고 있는 위젯이라면 그값으로 리사이즈한다
	if(pWidget->GetIDLRect().w>0 && pWidget->GetIDLRect().h>0)	
	{
		// idl 에서는 800x600 기준으로 기술되어있다
		const float tempWidth = ((float)RGetScreenWidth()) / 800;
		const float tempHeight = ((float)RGetScreenHeight()) / 600;

		MRECT r=pWidget->GetIDLRect();
		r.x*=tempWidth;
		r.w*=tempWidth;
		r.y*=tempHeight;
		r.h*=tempHeight;
		pWidget->SetBounds(r);
	}
	else 
	{
		const float tempWidth = ((float)RGetScreenWidth()) / mOldScreenWidth;
		const float tempHeight = ((float)RGetScreenHeight()) / mOldScreenHeight;
		MPOINT p = pWidget->GetPosition();
		p.Scale( tempWidth, tempHeight );
		pWidget->SetPosition( p );
		MRECT r=pWidget->GetRect();
		pWidget->SetSize( r.w*tempWidth, r.h*tempHeight );
	}
	return true;
}


bool ZOptionInterface::ResizeWidget(const char* szName, int w, int h)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget(szName);
	if(pWidget==NULL) return false;

	if ( _stricmp( szName, "Login") == 0)
	{
		//		// Resize Login background img
		//		pWidget = pResource->FindWidget( "Login_BackgrdImg");
		//		if ( pWidget)
		//			pWidget->SetSize( w, h);
		//		else
		//			return true;


		// Resize frame
		pWidget = pResource->FindWidget( "Login");
		if ( pWidget)
			pWidget->SetSize( w, h);

		// Resize background image
		pWidget = pResource->FindWidget( "Login_BackgrdImg");
		if ( pWidget)
			pWidget->SetSize( w, h);

		// Reposition login frame
		pWidget = pResource->FindWidget( "LoginFrame");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();

			rect.x = (w / 2) - (rect.w / 2) + 5;
			rect.y = h - rect.h;

			pWidget->SetBounds( rect);
		}

		// REposition connecting message
		pWidget = pResource->FindWidget( "Login_ConnectingMsg");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = 0;
			rect.y = (int)(h * 0.66f);
			rect.w = w;
			pWidget->SetBounds( rect);
		}

		return true;
	}

	if( _stricmp( szName, "Shop") == 0)
	{
		// Resize frame
		ResizeWidgetRecursive( pWidget, w, h);

		// Resize components
		/*
		pListBox = (MListBox*)pResource->FindWidget( "AllEquipmentList" );
		pPicture = (MPicture*)pResource->FindWidget( "Shop_ListLabel1" );
		if ( pPicture && pListBox)
		{
		rectList = pListBox->GetRect();
		rectList.x++;
		rectList.y++;
		rectList.w -= 24;
		rectList.h = 35;
		pPicture->SetBounds( rectList);
		for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
		pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
		}

		pListBox = (MListBox*)pResource->FindWidget( "MyAllEquipmentList" );

		pPicture = (MPicture*)pResource->FindWidget( "Shop_ListLabel" );
		if ( pPicture && pListBox)
		{
		rectList = pListBox->GetRect();
		rectList.x++;
		rectList.y++;
		rectList.w -= 24;
		rectList.h = 35;
		pPicture->SetBounds( rectList);
		for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
		pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
		}
		*/
		MListBox* pListBox1 = (MListBox*)pResource->FindWidget( "AllEquipmentList" );
		MListBox* pListBox2 = (MListBox*)pResource->FindWidget( "MyAllEquipmentList" );
		MListBox* pListBox3 = (MListBox*)pResource->FindWidget( "CashEquipmentList" );

		MPicture* pPicture = (MPicture*)pResource->FindWidget( "Shop_ListLabel" );
		float flSize = (float)RGetScreenWidth() / 800.0f;

		if ( pPicture)
		{
			MRECT rectList;
			rectList = pListBox1->GetRect();
			rectList.x++;
			rectList.y++;
			rectList.w -= 24;
			rectList.h = 35;
			pPicture->SetBounds( rectList);

			int cField, nFieldSize[] = { 32, 160, 35, 45};
			if ( pListBox1)
			{
				for ( cField = 0; cField < pListBox1->GetFieldCount(); cField++)
					pListBox1->GetField( cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
			}
			if ( pListBox2)
			{
				for ( cField = 0; cField < pListBox2->GetFieldCount(); cField++)
					pListBox2->GetField( cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
			}
			if ( pListBox3)
			{
				for ( cField = 0; cField < pListBox3->GetFieldCount(); cField++)
					pListBox3->GetField( cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
			}
		}


		ZGetGameInterface()->SelectEquipmentFrameList( "Shop", false);

		return true;
	}
	else if ( _stricmp( szName, "Equipment") == 0 )
	{
		// Resize frame
		ResizeWidgetRecursive( pWidget, w, h);

		// Resize components
		MListBox* pListBox;
		MRECT     rectList;
		int cField, nFieldSize[] = { 32, 160, 35, 45};
		float flSize = (float)RGetScreenWidth() / 800.0f;

		pListBox = (MListBox*)pResource->FindWidget( "EquipmentList");
		for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
			pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);

		pListBox = (MListBox*)pResource->FindWidget( "AccountItemList");
		for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
			pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);

		MPicture* pPicture;
		pListBox = (MListBox*)pResource->FindWidget( "EquipmentList" );
		pPicture = (MPicture*)pResource->FindWidget( "Equip_ListLabel1" );
		if ( pPicture && pListBox)
		{
			rectList = pListBox->GetRect();
			rectList.x++;
			rectList.y++;
			rectList.w -= 24;
			rectList.h = 35;
			pPicture->SetBounds( rectList);
			for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
				pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
		}

		pListBox = (MListBox*)pResource->FindWidget( "AccountItemList" );
		pPicture = (MPicture*)pResource->FindWidget( "Equip_ListLabel2" );
		if ( pPicture && pListBox)
		{
			rectList = pListBox->GetRect();
			rectList.x++;
			rectList.y++;
			rectList.w -= 24;
			rectList.h = 35;
			pPicture->SetBounds( rectList);
			for ( cField = 0; cField < pListBox->GetFieldCount(); cField++)
				pListBox->GetField(cField)->nTabSize = (int)( (float)nFieldSize[ cField] * flSize);
		}

		ZGetGameInterface()->SelectEquipmentFrameList( "Equip", true);

		return true;
	}
	else if( _stricmp( szName, "Stage" )== 0 )
	{
		ResizeWidgetRecursive(pWidget, w, h);

		ZApplication::GetStageInterface()->GetSacrificeItemBoxPos();

		return true;
	}
	else if(_stricmp( szName, "Lobby" )==0)
	{
		ResizeWidgetRecursive( pWidget, w, h );
		ZRoomListBox* pRoomList;

		pRoomList = (ZRoomListBox*)pResource->FindWidget( "Lobby_StageList" );
		if( pRoomList != 0 )
		{
			const float tempWidth = ((float)RGetScreenWidth()) / mOldScreenWidth;
			const float tempHeight = ((float)RGetScreenHeight()) / mOldScreenHeight;
			pRoomList->Resize( tempWidth, tempHeight );
		}

		return true;
	}
	else if(_stricmp( szName, "CharSelection")==0)
	{
		ResizeWidgetRecursive( pWidget, w, h );
	}

	else if(_stricmp( szName, "CharCreation")==0)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = 50 * ( RGetScreenWidth() / 800.0f);
		rect.y = (int)((RGetScreenHeight() - rect.h) / 2.0f);
		pWidget->SetBounds( rect);

		return true;
	}
	else if(_stricmp( szName, "GameResult")==0)
		ResizeWidgetRecursive( pWidget, w, h );
	else if(_stricmp( szName, "MonsterBook")==0)
		ResizeWidgetRecursive( pWidget, w, h );
	else if(_stricmp( szName, "CombatTDMInfo")==0)
		ResizeWidgetRecursive( pWidget, w, h );

	pWidget->SetSize(w, h);
	return true;
}

void ZOptionInterface::Resize(int w, int h)
{
	ZGetGameInterface()->SetSize(w, h);
	ResizeWidget("Login", w, h);
	ResizeWidget("Shop", w, h);
	ResizeWidget("Equipment", w, h);
	ResizeWidget("Lobby", w, h);
	ResizeWidget("Stage", w, h);
	ResizeWidget("Game", w, h);
	ResizeWidget("Greeter", w, h);
	ResizeWidget("Option", w, h);
	ResizeWidget("CharSelection", w, h);
	ResizeWidget("CharCreation", w, h);
	ResizeWidget("GameResult", w, h);
	ResizeWidget("MonsterBook", w, h);
	ResizeWidget("CombatTDMInfo", w, h);

	if (ZGetCombatInterface())
		ZGetCombatInterface()->Resize(w, h);

	GetRGMain().Resize(w, h);
}

void ZOptionInterface::GetOldScreenResolution()
{
	RMODEPARAMS ModeParams;
	ModeParams.nWidth	= mOldScreenWidth;
	ModeParams.nHeight	= mOldScreenHeight;
	ModeParams.FullscreenMode	= RGetFullscreenMode();
	ModeParams.PixelFormat	= mnOldBpp;

	mOldScreenWidth = RGetScreenWidth();
	mOldScreenHeight = RGetScreenHeight();
	mnOldBpp = RGetPixelFormat();

	RResetDevice( &ModeParams );
	Mint::GetInstance()->SetWorkspaceSize(ModeParams.nWidth, ModeParams.nHeight);
	Mint::GetInstance()->GetMainFrame()->SetSize(ModeParams.nWidth, ModeParams.nHeight);
	Resize(ModeParams.nWidth, ModeParams.nHeight);

	D3DDISPLAYMODE ddm;
	ddm.Width = ModeParams.nWidth;
	ddm.Height = ModeParams.nHeight;
	ddm.Format = ModeParams.PixelFormat;
	ddm.RefreshRate = DEFAULT_REFRESHRATE;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox *pWidget = (MComboBox*)pResource->FindWidget( "ScreenResolution" );

	auto iter = find_ddm(ddm);
	if( iter != gDisplayMode.end() )
		pWidget->SetSelIndex( iter->first );
}

bool ZOptionInterface::IsDiffScreenResolution()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
	if(pWidget)
	{
		int nSel = pWidget->GetSelIndex();
		D3DDISPLAYMODE ddm = (gDisplayMode.find( nSel ))->second;
		if( ddm.Width == RGetScreenWidth() && ddm.Height == RGetScreenHeight() && ddm.Format == RGetPixelFormat() )
			return false;
#ifdef _DEBUG
		mlog( "%d/%d , %d/%d, %d/%d\n", ddm.Width, RGetScreenWidth(), ddm.Height, RGetScreenHeight(), ddm.Format == D3DFMT_X8R8G8B8 ? 32 : 16, RGetPixelFormat() == D3DFMT_X8R8G8B8 ? 32 : 16 );
#endif
	}
	return true;
}

bool ZOptionInterface::TestScreenResolution()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MComboBox *pWidget = (MComboBox*)pResource->FindWidget("ScreenResolution");
	if(pWidget)
	{
		RMODEPARAMS	ModeParams;

		map<int, D3DDISPLAYMODE>::iterator iter = gDisplayMode.find( pWidget->GetSelIndex() );
		if( iter == gDisplayMode.end() )
		{
			mlog("선택한 해상도가 존재하지 않아서 해상도 변경에 실패하였습니다..\n" );
			return false;
		}

		D3DDISPLAYMODE ddm = iter->second;

		mOldScreenWidth	= RGetScreenWidth();
		mOldScreenHeight	= RGetScreenHeight();
		mnOldBpp				= RGetPixelFormat();

		ModeParams.nWidth = ddm.Width;
		ModeParams.nHeight = ddm.Height;
		ModeParams.FullscreenMode = RGetFullscreenMode();
		ModeParams.PixelFormat = ddm.Format;

		RResetDevice(&ModeParams);

		Mint::GetInstance()->SetWorkspaceSize(ModeParams.nWidth, ModeParams.nHeight);
		Mint::GetInstance()->GetMainFrame()->SetSize(ModeParams.nWidth, ModeParams.nHeight);
		Resize(ModeParams.nWidth, ModeParams.nHeight);
	}
	return true;
}

void ZOptionInterface::Update()
{
	if( mbTimer )
	{
		if( SetTimer(true) )
		{
			GetOldScreenResolution();
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pResource->FindWidget("ViewConfirm");
			if(pWidget!= 0) pWidget->Show( false );
		}
	}
}

void ZOptionInterface::OnActionKeySet(ZActionKey* pActionKey, ZVIRTUALKEY key)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	for(int i=0; i<ZACTION_COUNT; i++){
		char szItemName[256];
		sprintf_safe(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)pResource->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		if(pWidget==pActionKey) continue;

		if(pWidget->DeleteActionKey(key))
			pWidget->UpdateText();
	}
}








///////////////////// 이하 interface listener
BEGIN_IMPLEMENT_LISTENER(ZGetOptionFrameButtonListener, MBTN_CLK_MSG)
	// 옵션 프레임 보여주기
	ZGetOptionInterface()->InitInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("Option");
	pWidget->Show(true, true);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetSaveOptionButtonListener, MBTN_CLK_MSG)
	// Save & Close
	/*
	ZApplication::GetGameInterface()->SaveInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("Option");
	if(pWidget!=NULL) pWidget->Show(false);
	//*/

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if( pWidget->m_bEventAcceleratorCall ) {

		MTabCtrl* pTab = (MTabCtrl*)pResource->FindWidget("OptionTabControl");

		if(pTab) {//키보드 옵션은 키입력을 막아준다.
			if(pTab->GetSelIndex()==3)//이름을 찾아 조사
				return true;
		}
	}

	if( ZGetOptionInterface()->IsDiffNetworkPort() )
	{
		static bool bRestartAsk = false;
		if (bRestartAsk == false) {
			ZGetOptionInterface()->ShowNetworkPortConfirmDialog();
			bRestartAsk = true;
			return true;
		}
	}

	if( ZGetOptionInterface()->IsDiffScreenResolution() )
	{
		ZGetOptionInterface()->ShowResizeConfirmDialog( false );
	}
	else
	{
		ZGetOptionInterface()->SaveInterfaceOption();
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("Option");
		if(pWidget!=NULL) pWidget->Show(false);
	}

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		if (ZApplication::GetGameInterface()->GetCombatInterface())
		{
			ZApplication::GetGameInterface()->GetCombatInterface()->GetCrossHair()->ChangeFromOption();
		}
	}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelOptionButtonListener, MBTN_CLK_MSG)
//	ZApplication::GetGameInterface()->SaveInterfaceOption();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// TODO: 이게 필요한가 ? 테스트 요망
	if( pWidget->m_bEventAcceleratorCall ) {

		MTabCtrl* pTab = (MTabCtrl*)pResource->FindWidget("OptionTabControl");

		if(pTab) {//키보드 옵션은 키입력을 막아준다.
			if(pTab->GetSelIndex()==3)//이름을 찾아 조사
				return true;
		}
	}

	MWidget* pWidget = pResource->FindWidget("Option");
	
	if(pWidget!=NULL) pWidget->Show(false);

	// 원래 감마값으로 돌리기
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		if (pSlider->GetValue() != Z_VIDEO_GAMMA_VALUE)
		{
			RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
		}
	}

	// Close Only
//	ZApplication::GetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()


///////////////////////////////////////////////////
/// control

BEGIN_IMPLEMENT_LISTENER( ZGetLoadDefaultKeySettingListener, MBTN_CLK_MSG)
	ZGetConfiguration()->LoadDefaultKeySetting( );
	for(int i=0; i<ZACTION_COUNT; i++)
	{
		char szItemName[256];
		sprintf_safe(szItemName, "%sActionKey", ZGetConfiguration()->GetKeyboard()->ActionKeys[i].szName);
		ZActionKey* pWidget = (ZActionKey*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szItemName);
		if(pWidget==NULL) continue;
		//unsigned int nKey = 0;
		//pWidget->GetActionKey(&nKey);
		//Mint::GetInstance()->UnregisterActionKey(i);
		//Mint::GetInstance()->RegisterActionKey(i, nKey);	// 키 등록
		//m_Keyboard.ActionKeys[i].nScanCode = nKey;	// 옵션 저장
		pWidget->ClearActionKey();
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKey);
		pWidget->SetActionKey(ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nVirtualKeyAlt);
	}
END_IMPLEMENT_LISTENER()


//////////////////////////////////////////////////
/// video

BEGIN_IMPLEMENT_LISTENER(ZGetOptionGammaSliderChangeListener, MLIST_VALUE_CHANGED)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MSlider* pSlider = (MSlider*)pResource->FindWidget("VideoGamma");
	if (pSlider != NULL) 
	{
		unsigned short nGamma = (unsigned short)pSlider->GetValue();
		if (nGamma < 50) nGamma = 50;
		RSetGammaRamp(nGamma);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetRequestResizeListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
	if(pWidget!= 0) pWidget->Show( false );
	//해상도 변경후
	ZGetOptionInterface()->TestScreenResolution();
	ZGetOptionInterface()->SetTimer( true, 10 );
	ZGetOptionInterface()->ShowResizeConfirmDialog( true );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetViewConfirmCancelListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ViewConfirm");
	if(pWidget!= 0) pWidget->Show( false );
	// 해상도 원래대로 변경
 	ZGetOptionInterface()->SetTimer( false );
	ZGetOptionInterface()->GetOldScreenResolution();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetViewConfrimAcceptListener, MBTN_CLK_MSG )

	ZGetOptionInterface()->SetTimer( false );

	ZGetOptionInterface()->SaveInterfaceOption();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ViewConfirm");
	if(pWidget!= 0) pWidget->Show( false );
		
	pWidget = pResource->FindWidget("Option");
	if(pWidget!=NULL) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelResizeConfirmListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ResizeConfirm");
	if(pWidget!=NULL) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZSetOptimizationListener, MBTN_CLK_MSG )
	ZGetOptionInterface()->OptimizationVideoOption();
END_IMPLEMENT_LISTENER()

//////////////////////////////////////////////////
/// sound

BEGIN_IMPLEMENT_LISTENER( ZGet8BitSoundListener, MBTN_CLK_MSG )
	MButton* pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("8BitSound");
	if(pWidget)
	{
		pWidget->SetCheck(true);
		pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("16BitSound");
		if(pWidget) pWidget->SetCheck(false);
	}
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER( ZGet16BitSoundListener, MBTN_CLK_MSG )
	MButton* pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("16BitSound");
	if(pWidget)
	{
		pWidget->SetCheck(true);
		pWidget = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("8BitSound");
		if(pWidget) pWidget->SetCheck(false);
	}
END_IMPLEMENT_LISTENER()


/////////////////////////////////////////////////
//// network
BEGIN_IMPLEMENT_LISTENER( ZGetNetworkPortChangeRestartListener, MBTN_CLK_MSG )
	ZGetOptionInterface()->SaveInterfaceOption();
	ZChangeGameState(GUNZ_SHUTDOWN);
	ZApplication* pApp = ZApplication::GetInstance();
	pApp->Exit();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetNetworkPortChangeCancelListener, MBTN_CLK_MSG )
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("NetworkPortConfirm");
	if(pWidget!= 0) pWidget->Show(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBGMVolumeSizeSliderListener, MLIST_VALUE_CHANGED)
MSlider* pWidget = (MSlider*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("BGMVolumeSlider");
if (pWidget)
{
	Z_AUDIO_BGM_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)10000;
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEffectVolumeSizeSliderListener, MLIST_VALUE_CHANGED)
MSlider* pWidget = (MSlider*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("EffectVolumeSlider");
if (pWidget)
{
	Z_AUDIO_EFFECT_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)10000;
	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
}
END_IMPLEMENT_LISTENER()