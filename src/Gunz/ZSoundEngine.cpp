#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZSoundEngine.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "MMatchItem.h"
#include "ZConfiguration.h"
#include "Physics.h"
#include "Fileinfo.h"
#include "ZApplication.h"
#include "ZSoundFMod.h"
#include "MMath.h"
#include "ZInitialLoading.h"
#include "RBspObject.h"

_USING_NAMESPACE_REALSPACE2

#ifdef _DEBUG
#define _SOUND_LOG
#endif

extern ZGame* m_gGame;

#define ZDEF_MINDISTANCE	300.0f						//	3m 
#define ZDEF_MAXDISTANCE	4000.f						//	40m
#define ZDEF_MAXDISTANCESQ	16000000.F
#define ZDEF_MAX_DISTANCE_PRIORITY	100
#define ZDEF_ROLLFACTOR		1.0f

// Sound FX
#define SOUNDEFFECT_DIR	"sound/effect/"
#define SOUNDEFFECT_EXT	"*.wav"
#define SOUND_OPTION_FILE_NAME	"SoundOption.xml"
#define SOUNDEFFECT_XML "sound/effect/effect.xml"
#define SOUNDNPC_DIR	"sound/npc/"

#define DEFAULT_SOUND_FRAME	30


#ifdef _BIRDSOUND

void ZSoundEngine::OnCreate()
{
	SetRollFactor(1.0f);
	SetDistanceFactor(100.0f);
	SetDopplerFactor(1.0f);	

	if( !LoadResource( SOUNDEFFECT_XML ) )
	{
		mlog("fail to load sound effect\n");
	}

}

bool ZSoundEngine::LoadResource(char* pFileName)
{
	MXmlDocument Data;

	MZFile mzf;
	if(!mzf.Open(pFileName, m_pZFileSystem)) return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	Data.Create();
	if(!Data.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}
	delete buffer;
	mzf.Close();


	MXmlElement root, chr, attr;

	char szSoundName[256];
	char szSoundFileName[256];


	root		= Data.GetDocumentElement();
	int iCount	= root.GetChildNodeCount();

	for( int i = 0 ; i < iCount; ++i )
	{
		chr		= root.GetChildNode(i);
		chr.GetTagName( szSoundName );
		if( szSoundName[0] == '#' ) continue;

		chr.GetAttribute( szSoundName, "name" );
		strcpy_safe( szSoundFileName, SOUNDEFFECT_DIR );
		strcat( szSoundFileName, szSoundName );
		strcat( szSoundFileName, ".wav" );

		char szType[64] = "";
		chr.GetAttribute(szType, "type", "3d");


		float min = ZDEF_MINDISTANCE;
		float max = ZDEF_MAXDISTANCE;
		float fTemp;
		if( chr.GetAttribute( &fTemp, "MINDISTANCE" )) min = fTemp;
		if( chr.GetAttribute( &fTemp, "MAXDISTANCE" )) max = fTemp;

		bool bLoop = false;
		chr.GetAttribute(&bLoop, "loop");

		float fDefaultVolume = 1.0f;
		chr.GetAttribute(&fDefaultVolume, "volume", 1.0f);

		u32 nFlags=0;

		if (bLoop) nFlags |= RSOUND_LOOP_NORMAL;
		else nFlags |= RSOUND_LOOP_OFF;

		if (!_stricmp(szType, "2d"))
		{
			OpenSound(szSoundFileName, RSOUND_2D | nFlags);

			if (!IS_EQ(fDefaultVolume, 1.0f))
			{
				RBaseSoundSource* pSoundSource = GetSoundSource(szSoundFileName, RSOUND_]2D);
				if (pSoundSource)
				{
					SetDefaultVolume(pSoundSource, fDefaultVolume);
				}
			}
		}
		else
		{
			OpenSound(szSoundFileName, RSOUND_3D | nFlags);

			RBaseSoundSource* pSoundSource = GetSoundSource(szSoundFileName, RSOUND_3D);
			if (pSoundSource) 
			{
				SetMinMaxDistance( pSoundSource, min, max);

				if (!IS_EQ(fDefaultVolume, 1.0f))
				{
					SetDefaultVolume(pSoundSource, fDefaultVolume);
				}
			}
		}

	}

	return true;

}

void ZSoundEngine::GetSoundName(const char* szSrcSoundName, char* out)
{
	strcpy_safe(out, SOUNDEFFECT_DIR);
	strcat(out, szSrcSoundName);
	strcat(out, ".wav");
}

bool ZSoundEngine::OpenMusic(int nBgmIndex)
{
//	if( !m_bSoundEnable ) return false;

	// 가짜
	static char m_stSndFileName[MAX_BGM][64] = {"Intro Retake2(D-R).ogg", 
												"Theme Rock(D).ogg", 
												"HardBgm3 Vanessa Retake(D).ogg", 
												"HardTech(D).ogg", 
												"HardCore(D).ogg",
												"Ryswick style.ogg",
												"El-tracaz.ogg",
												"Industrial technolism.ogg",
												"Fin.ogg" };
												
	char szFileName[256] = "";
#define BGM_FOLDER		"Sound/BGM/"

	int nRealBgmIndex = nBgmIndex;
	if ((nBgmIndex >= BGMID_BATTLE) && (nBgmIndex < BGMID_FIN)) nRealBgmIndex = RandomNumber(BGMID_BATTLE, BGMID_BATTLE+2);

	sprintf_safe(szFileName, "%s%s", BGM_FOLDER, m_stSndFileName[nRealBgmIndex]);

	return RealSound2::OpenMusic((const char*)szFileName);
}


class ZPlayingChannels : public list<int>
{

} g_Channels;

int ZSoundEngine::PlaySoundCharacter(const char* szSoundName, rvector& pos, bool bHero, int nPriority)
{
	char key[256] = "";
	GetSoundName(szSoundName, key);

	int nChannel = -1;

	if (bHero)
	{
		//nChannel= RealSound2::PlaySound(key, nPriority);
		return RealSound2::PlaySound(key, nPriority);
	}
	else
	{
		float p[3];
		p[0] = pos.x;
		p[1] = pos.y;
		p[2] = pos.z;

		nChannel= RealSound2::PlaySound(key, p, NULL, nPriority);
		g_Channels.push_back(nChannel);
	}
/*
	char temp[256];
	sprintf_safe(temp, "Play Channel:%4d\n", nChannel);
	mlog(temp);
*/


	return nChannel;
}

/*
int ZSoundEngine::PlaySound(const char* szSoundName, rvector& pos, int nPriority, DWORD dwDelay)
{
	float p[3];
	p[0] = pos.x;
	p[1] = pos.y;
	p[2] = pos.z;

	return RealSound2::PlaySound(szSoundName, p, NULL, nPriority);
}
*/
// 이건 나중에 삭제될 것
void ZSoundEngine::PlaySound(char* Name,rvector& pos,bool bHero, bool bLoop, DWORD dwDelay)
{
//	if( !m_bSoundEnable )	return;
//	if( !m_b3DSoundUpdate ) return;
	
	//Culling

	// SetPriority
	if( dwDelay > 0 )
	{
		_ASSERT(0);
/*
		DelaySound DS;
		DS.dwDelay = dwDelay + GetGlobalTimeMS();
		DS.pSS = pSS;
		DS.pos = pos;
		DS.priority = priority;
		DS.bPlayer = bHero;
		m_DelaySoundList.push_back(DS);
		return;
*/
	}

//	char key[256] = "";
//	GetSoundName(Name, key);

	PlaySoundCharacter((const char*)Name, pos, bHero);
}

int ZSoundEngine::PlaySound(const char* szSoundName, int nPriority)
{
	char key[256] = "";
	GetSoundName(szSoundName, key);

	return RealSound2::PlaySound(key, nPriority);
}

int ZSoundEngine::PlaySound(const char* szSoundName, float* pos, float* vel, int nPriority)
{
	char key[256] = "";
	GetSoundName(szSoundName, key);

	return RealSound2::PlaySound(key, pos, vel, nPriority);
}

#include "fmod.h"

void ZSoundEngine::OnUpdate()
{
/*
	DWORD currentTime = GetGlobalTimeMS();
	if( (currentTime - m_Time) < m_DelayTime ) return;
	m_Time = currentTime;
*/
	if(g_pGame)
	{
		rvector Pos = RCameraPosition;
		ZCharacter* pInterestCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		if(pInterestCharacter != NULL)
		{
			Pos = pInterestCharacter->GetPosition();
			Pos.z += 170.f;
		}

		rvector Orientation = Pos - RCameraPosition;
		D3DXVec3Normalize(&Orientation, &Orientation);

		rvector right;
		D3DXVec3Cross(&right, &Orientation, &RCameraUp);

//		UpdateAmbSound(Pos, right);

		rvector m_ListenerPos = Pos;
		
		float pos[3];
		float forward[3];
		float vel[3] = {0.0f, 0.0f, 0.0f};
		float top[3] = {0.0f, 0.0f, 1.0f};

		pos[0] = Pos.x;	pos[1] = Pos.y;	pos[2] = Pos.z;
		forward[0] = Orientation.x;	forward[1] = Orientation.y;	forward[2] = Orientation.z;

		for (ZPlayingChannels::iterator itor=g_Channels.begin(); itor != g_Channels.end(); )
		{
			int nChannel = (*itor);
			if (FSOUND_IsPlaying(nChannel) == FALSE)
			{
				itor = g_Channels.erase(itor);
			}
			else
			{
				FSOUND_3D_SetAttributes(nChannel, pos, vel);
				++itor;
			}
		}

		m_pAudio->SetListener( pos, vel, forward, top);
	}
}

void ZSoundEngine::SetEffectVolume(float fVolume)
{
	m_pAudio->SetVolume(fVolume);
}

void ZSoundEngine::SetEffectMute(bool bMute)
{
	m_pAudio->SetMute(bMute);
}


void ZSoundEngine::PlaySEFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero)
{
	if( !pDesc ) return;

	if( pDesc->m_nType == MMIT_RANGE || pDesc->m_nType == MMIT_CUSTOM )
	{
		char* szSndName = pDesc->m_szFireSndName;

		rvector pos = rvector(x,y,z);
		PlaySoundCharacter(szSndName, pos, bHero, 200);
		return;

		// 자기 자신이면 총소리는 2d로 낸다.
		if (bHero)
		{
			char szFireSndName[256];
			sprintf_safe(szFireSndName, "%s%s", szSndName, "_2d");

			char key[256] = "";
			GetSoundName(szFireSndName, key);
			
			RealSound2::PlaySound(key, 200);
		}
		else
		{
			rvector pos = rvector(x,y,z);
			PlaySoundCharacter(szSndName, pos, bHero, 200);
		}
	}

}

void ZSoundEngine::PlaySEDryFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero)
{

}

void ZSoundEngine::PlaySEReload(MMatchItemDesc *pDesc, float x, float y, float z, bool bHero)
{
	if(!pDesc )	return;

	if(pDesc->m_nType == MMIT_RANGE)
	{
		char* szSndName = pDesc->m_szReloadSndName;
		if(bHero)
		{
			char szBuffer[64];
			sprintf_safe( szBuffer, "%s_2d", szSndName );

			char key[256] = "";
			GetSoundName(szBuffer, key);
			
			RealSound2::PlaySound(key, 200);
		}
		else
		{
			rvector pos = rvector(x,y,z);
			PlaySoundCharacter(szSndName, pos, bHero, 200);
		}
		//PlaySoundElseDefault(szSndName,"we_rifle_reload",rvector(x,y,z),bPlayer);
	}

}

void ZSoundEngine::PlaySERicochet(float x, float y, float z)
{

}

void ZSoundEngine::PlaySEHitObject( float x, float y, float z, RBSPPICKINFO& info_ )
{

}

void ZSoundEngine::PlaySEHitBody(float x, float y, float z)
{

}





////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#else // _BIRDSOUND

FSOUND_SAMPLE* ZSoundEngine::GetFS( const char* szName, bool bHero )
{
	SoundSource* pSS = GetSoundSource( szName, bHero );	
	if(pSS != NULL ) return pSS->pFS;
	return NULL;
}

SoundSource* ZSoundEngine::GetSoundSource( const char* szName, bool bHero )
{
	SESMAP::iterator i;
	if( !bHero )
	{
		i = m_SoundEffectSource.find(string(szName));
		if( i == m_SoundEffectSource.end() )
		{
			i = m_SoundEffectSource2D.find(string(szName));
			if( i == m_SoundEffectSource2D.end() ) return NULL;
		}
	}
	else
	{
		i = m_SoundEffectSource2D.find(string(szName));
		if( i == m_SoundEffectSource2D.end() ) 
		{
			i = m_SoundEffectSource.find(string(szName));
			if( i == m_SoundEffectSource.end() ) return NULL;
		}
	}


	SoundSource* pRetSource = (SoundSource*)(i->second);
	return pRetSource;
}

ZSoundEngine::ZSoundEngine()
{
	m_pMusicBuffer = NULL;
	
	m_fEffectVolume = 1.0f;
	m_fMusicVolume = 0.0f;

	m_bEffectMute = false;
	m_szOpenedMusicName[0] = 0;
	m_bSoundEnable	= false;
	m_b3DSound	= true;
	m_b3DSoundUpdate = false;
	m_bHWMixing = true;

	m_ListenerPos = rvector(0,0,0);

	m_bEffectVolControl = false;
	m_bBGMVolControl = false;
	m_fEffectVolFactor = 0;
	m_fEffectVolEnd = 0;
	m_fBGMVolFactor = 0;
	m_fBGMVolEnd = 0;

	m_bBattleMusic=false;
	m_pfs = NULL;
}

ZSoundEngine::~ZSoundEngine()
{

}


void ZSoundEngine::SetEffectVolume(float fVolume)
{
	m_fEffectVolume=fVolume;
	ZGetSoundFMod()->SetVolume( m_fEffectVolume * 255 );
}

void ZSoundEngine::SetEffectVolume( int iChnnel, float fVolume )
{
	ZGetSoundFMod()->SetVolume( iChnnel, (int)(fVolume * 255) );
}

bool ZSoundEngine::Create(HWND hwnd, bool bHWMixing, ZLoadingProgress *pLoadingProgress )
{
	m_bSoundEnable = ZGetSoundFMod()->Create( hwnd, FSOUND_OUTPUT_DSOUND, 44100, bHWMixing?16:1024, bHWMixing?16:0, bHWMixing?16:32, 0 );
	if(!m_bSoundEnable && bHWMixing)
	{
		m_bSoundEnable = ZGetSoundFMod()->Create( hwnd, FSOUND_OUTPUT_DSOUND, 44100, 0, 0, 32, 0 ); // try software mode
		if( !m_bSoundEnable )
		{
			mlog("Fail to Create Sound Engine..\n");
			return false;
		}
		else
		{
			mlog("Create SoundEngine with Software mode after failed with hardware mode..\n");
			m_bHWMixing = false;
			Z_AUDIO_HWMIXING = false;
		}
	}
	m_b8Bits = Z_AUDIO_8BITSOUND;

	if( !LoadResource( SOUNDEFFECT_XML, pLoadingProgress ) )
	{
		mlog("fail to load sound effect\n");
	}

	ZGetSoundFMod()->SetRollFactor(3.f);
	ZGetSoundFMod()->SetDistanceFactor(100.f);
	ZGetSoundFMod()->SetDopplerFactor(1.f);	

	ZGetSoundFMod()->SetMusicEndCallback(MusicEndCallback, this);

	SetEffectVolume(m_fEffectVolume);
	SetMusicVolume(m_fMusicVolume);
	ZGetSoundFMod()->SetMute( Z_AUDIO_EFFECT_MUTE );
	ZGetSoundFMod()->SetMusicMute( Z_AUDIO_BGM_MUTE );

	SetFramePerSecond( DEFAULT_SOUND_FRAME );
	SetInverseSound( Z_AUDIO_INVERSE );

	m_bHWMixing = Z_AUDIO_HWMIXING;

	return true;
}

bool ZSoundEngine::Reset( HWND hwnd, bool bHWMixing )
{
	if(m_bHWMixing == bHWMixing ) return false;
	m_bHWMixing = bHWMixing;

	char szBuffer[128];
	strcpy_safe(szBuffer, m_szOpenedMusicName );

	Destroy();
	m_bSoundEnable = Create(hwnd, bHWMixing);
	if(!m_bSoundEnable)	return false;

	if(g_pGame)
	{
		for(auto& AS : ZGetGame()->GetWorld()->GetBsp()->GetAmbSndList())
		{
			if(AS.itype & AS_AABB)
				ZGetSoundEngine()->SetAmbientSoundBox(AS.szSoundName,
					AS.min, AS.max,
					(AS.itype&AS_2D)?true:false );
			else if(AS.itype & AS_SPHERE )
				ZGetSoundEngine()->SetAmbientSoundSphere(AS.szSoundName,
					AS.center, AS.radius,
					(AS.itype&AS_2D)?true:false );
		}
	}

	OpenMusic(szBuffer, ZGetFileSystem());
	PlayMusic();
	return true;
}

void ZSoundEngine::Destroy()
{
	mlog("ZSoundEngine::Destroy\n");
	for( SESMAP::iterator iter = m_SoundEffectSource.begin(); iter != m_SoundEffectSource.end(); ++iter )
	{
		SoundSource* pSS = iter->second;
		if( pSS != NULL && pSS->pFS != NULL )
			FSOUND_Sample_Free(pSS->pFS);
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource.clear();

	for( SESMAP::iterator iter = m_SoundEffectSource2D.begin(); iter != m_SoundEffectSource2D.end(); ++iter )
	{
		SoundSource* pSS = iter->second;
		if( pSS != NULL && pSS->pFS != NULL )
			FSOUND_Sample_Free(pSS->pFS);
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource2D.clear();
	m_DelaySoundList.clear();
	ClearAmbientSound();

	StopMusic();
	CloseMusic();

	mlog("ZSoundEngine::Destroy() : Close FMod\n");
	ZGetSoundFMod()->Close();
}

bool ZSoundEngine::SetSamplingBits( bool b8Bits )
{
	if( b8Bits == m_b8Bits ) return true; 
	m_b8Bits = b8Bits;
	return Reload();
}

bool ZSoundEngine::OpenMusic(const char* szFileName, MZFileSystem* pfs)
{
	if( !m_bSoundEnable ) return false;

	m_pfs = pfs;
	if (!strcmp(m_szOpenedMusicName, szFileName)) return false;
	if (m_szOpenedMusicName[0] != 0) CloseMusic();
	strcpy_safe(m_szOpenedMusicName, szFileName);
	
	MZFile mzf;
	if(!mzf.Open(szFileName, pfs)) return false;
	m_pMusicBuffer = new char[mzf.GetLength()+1];
	mzf.Read(m_pMusicBuffer, mzf.GetLength());
	m_pMusicBuffer[mzf.GetLength()] = 0;

	int len = mzf.GetLength();

	return ZGetSoundFMod()->OpenStream( m_pMusicBuffer, len );
}

void ZSoundEngine::CloseMusic()
{	
	if( !m_bSoundEnable ) return;

	if (m_szOpenedMusicName[0] == 0) return;
	m_szOpenedMusicName[0] = 0;

	ZGetSoundFMod()->CloseMusic();
    
	if (m_pMusicBuffer) 
	{
		delete m_pMusicBuffer; m_pMusicBuffer = NULL;
	}
}

void ZSoundEngine::SetMusicVolume(float fVolume)
{
	if( !m_bSoundEnable ) return;

	m_fMusicVolume = fVolume;
	ZGetSoundFMod()->SetMusicVolume( fVolume * 255 );
}

float ZSoundEngine::GetMusicVolume( void)
{
	return m_fMusicVolume;
}

void ZSoundEngine::MusicEndCallback(void* pCallbackContext)
{
	ZSoundEngine* pSoundEngine = (ZSoundEngine*)pCallbackContext;
	if (pSoundEngine->m_bBattleMusic)
	{
		pSoundEngine->OpenMusic(BGMID_BATTLE, pSoundEngine->m_pfs);
		pSoundEngine->PlayMusic();
	}
}

void ZSoundEngine::PlayMusic(bool bLoop)
{
	if( !m_bSoundEnable || m_bMusicMute ) return;

	if (m_bBattleMusic)
	{
		// 전투중에는 배경음악이 루핑되지 않고 다음노래로 넘어간다.
		ZGetSoundFMod()->PlayMusic( false );
	}
	else
	{
		ZGetSoundFMod()->PlayMusic( bLoop );
	}



	SetMusicVolume( m_fMusicVolume );
}

void ZSoundEngine::StopMusic()
{
	if( !m_bSoundEnable ) return;
	ZGetSoundFMod()->StopMusic();
}


void ZSoundEngine::PlaySEFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer)
{
	if( !m_bSoundEnable || !pDesc )	return;

	if( pDesc->m_nType == MMIT_RANGE || pDesc->m_nType == MMIT_CUSTOM )
	{
		char* szSndName = pDesc->m_szFireSndName;
		if(bPlayer)
		{
			char szBuffer[64];
			sprintf_safe( szBuffer, "%s_2d", szSndName );
#ifdef _SOUND_LOG
			mlog("%s stereo 2d sound is played..\n",szBuffer);
#endif
			PlaySoundElseDefault(szBuffer,"we_rifle_fire_2d",rvector(x,y,z),bPlayer);
		}
		PlaySoundElseDefault(szSndName,"we_rifle_fire",rvector(x,y,z),bPlayer);
	}
}

void ZSoundEngine::PlaySEDryFire(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer)
{
	if( !m_bSoundEnable || !pDesc )	return;
	PlaySound("762arifle_dryfire", rvector(x,y,z), bPlayer, false );
}

void ZSoundEngine::PlaySEReload(MMatchItemDesc *pDesc, float x, float y, float z, bool bPlayer)
{
	if( !m_bSoundEnable || !pDesc )	return;

	if(pDesc->m_nType == MMIT_RANGE)
	{
		char* szSndName = pDesc->m_szReloadSndName;
		if(bPlayer)
		{
			char szBuffer[64];
			sprintf_safe( szBuffer, "%s_2d", szSndName );
#ifdef _SOUND_LOG
			mlog("%s stereo 2d sound is played..\n",szBuffer);
#endif
			PlaySoundElseDefault(szBuffer,"we_rifle_reload_2d",rvector(x,y,z),bPlayer);
		}
		PlaySoundElseDefault(szSndName,"we_rifle_reload",rvector(x,y,z),bPlayer);
	}
}

void ZSoundEngine::PlaySERicochet(float x, float y, float z)
{
	if( !m_bSoundEnable)	return;
	PlaySound("ricochet_concrete01",rvector(x,y,z), false, false );
}

void ZSoundEngine::PlaySEHitObject( float x, float y, float z, RBSPPICKINFO& info_ )
{
	if( !m_bSoundEnable )	return;

	static const char* base_snd_name	= "fx_bullethit_mt_";
	FSOUND_SAMPLE* pFS = NULL;
	static char	buffer[256];

	if( info_.pNode==NULL ) {
		OutputDebugString("ZSoundEngine::PlaySEHitObject 엉뚱한곳이 picking ?\n");
		return;
	}
	RMATERIAL*	material_info = ZGetGame()->GetWorld()->GetBsp()->GetMaterial( info_.pNode, info_.nIndex );

	if(material_info==NULL) {
		OutputDebugString("ZSoundEngine::PlaySEHitObject ( material_info==NULL ) ?\n");
		return;
	}

	const char* temp			= material_info->Name.c_str();
	size_t	size				= strlen(temp);

	// Compare string...
	int index		= (int) (size - 1);
	while( index >= 2 )
	{
		if( temp[index--] == 't' && temp[index--] == 'm' )
		{
			index += 4;
			break;
		}
	}

	if( index <= 2 )
	{
		PlaySound("fx_bullethit_mt_con", rvector(x,y,z), false, false );
		return;
	}

	strcpy_safe(buffer, base_snd_name);
	strncat_safe(buffer, temp + index, size - index);

	PlaySoundElseDefault(buffer, "fx_bullethit_mt_con", rvector(x,y,z) );
}

void ZSoundEngine::PlaySEHitBody(float x, float y, float z)
{
	if( !m_bSoundEnable )	return;
	PlaySound("fx_bullethit_mt_fsh", rvector(x,y,z), false, false );
}

bool ZSoundEngine::isPlayAble(char* name)
{
	if( !m_bSoundEnable )	return false;

	SESMAP::iterator i = m_SoundEffectSource.find(name);

	if(i==m_SoundEffectSource.end())
	{
		i = m_SoundEffectSource2D.find(name);
		if( i == m_SoundEffectSource2D.end() )
			return false;
	}
	return true;
}

bool ZSoundEngine::isPlayAbleMtrl(char* name)
{
	if( !m_bSoundEnable )	return false;
	
	if(!name)		return false;
	if(!name[0])	return false;

	int len = (int)strlen(name);

	SESMAP::iterator node;
	FSOUND_SAMPLE* pFS = NULL;
	char filename[256];

	for(node = m_SoundEffectSource.begin(); node != m_SoundEffectSource.end(); ++node) 
	{
		strcpy_safe( filename, ((string)((*node).first)).c_str());

		if(strncmp(filename,name,len)==0)
			return true;
	}
	
	for(node = m_SoundEffectSource2D.begin(); node != m_SoundEffectSource2D.end(); ++node) 
	{
		strcpy_safe( filename, ((string)((*node).first)).c_str());

		if(strncmp(filename,name,len)==0)
			return true;
	}

	return false;
}

int ZSoundEngine::PlaySound(const char* Name, const rvector& pos, bool bHero, bool bLoop, DWORD dwDelay ) 
{
	if( !m_bSoundEnable )	return 0;
	if( !m_b3DSoundUpdate ) return 0;
	
	// Find Sound Source
	SoundSource* pSS = GetSoundSource(Name, bHero);
	if(pSS == 0 )
	{
#ifdef _SOUND_LOG
		mlog("No %sSound Source[%s]\n", bHero?"2d":"3d", Name);
#endif
		return 0;
	}

	//Culling
	int priority=0;
	if (!CheckCulling(Name, pSS, pos, bHero, &priority)) return 0;

	if( dwDelay > 0 )
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + GetGlobalTimeMS();
		DS.pSS = pSS;
		DS.pos = pos;
		DS.priority = priority;
		DS.bPlayer = bHero;
		m_DelaySoundList.push_back(DS);
		return 0;
	}

	//Play
	FSOUND_SAMPLE* pFS = pSS->pFS;
	if(pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return 0;
	}
	return PlaySE( pFS, pos, priority, bHero, bLoop );	
}

void ZSoundEngine::PlaySoundElseDefault(const char* Name, const char* NameDefault, const rvector& pos,bool bHero,bool bLoop, DWORD dwDelay )
{
	if( !m_bSoundEnable )	return;
 	if( !m_b3DSoundUpdate ) return;
	
	SoundSource* pSS = GetSoundSource(Name, bHero);
	if(pSS == 0 )
	{
		pSS = GetSoundSource(NameDefault, bHero);
		if(pSS == 0)
		{
#ifdef _SOUND_LOG
			mlog("No %sSound Source[%s] even Default Sound Source[%s]\n", bHero?"2d":"3d", Name, NameDefault);
#endif
			return;
		}
#ifdef _SOUND_LOG
		mlog("No %sSound Source[%s] so Use Default Sound Source[%s]\n", bHero?"2d":"3d", Name, NameDefault);
#endif
	}

	//Culling
	int priority=0;
	if (!CheckCulling(Name, pSS, pos, bHero, &priority)) return;

	if( dwDelay > 0 )
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + GetGlobalTimeMS();
		DS.pSS = pSS;
		DS.pos = pos;
		DS.priority = priority;
		DS.bPlayer = bHero;
		m_DelaySoundList.push_back(DS);
		return;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if(pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return;
	}

	PlaySE( pFS, pos, priority, bHero, bLoop );	
}

int ZSoundEngine::PlaySound( const char* Name, bool bLoop, DWORD dwDelay )
{
	if( !m_bSoundEnable )	return 0;

	SoundSource* pSS = GetSoundSource(Name, true);
	if(pSS == 0 )
	{
#ifdef _SOUND_LOG
		mlog("No 2DSound Source[%s]\n", Name);
#endif
		return 0;
	}

	if( dwDelay > 0 )
	{
		DelaySound DS;
		DS.dwDelay = dwDelay + GetGlobalTimeMS();
		DS.pSS = pSS;
		DS.priority = 200;
		DS.bPlayer = true;
		m_DelaySoundList.push_back(DS);
		return 0;
	}

	FSOUND_SAMPLE* pFS = pSS->pFS;
	if(pFS == NULL)
	{
#ifdef _SOUND_LOG
		mlog("FSOUND_SAMPLE is Null for Sound Source[%s]\n", Name);
#endif
		return 0;
	}
	return PlaySE( pFS, rvector(0,0,0), 200, true, bLoop );
}

void ZSoundEngine::Run(void)
{
	DWORD currentTime = GetGlobalTimeMS();
	if( (currentTime - m_Time) < m_DelayTime ) return;
	m_Time = currentTime;

	auto ZSoundEngineRun = MBeginProfile("ZSoundEngine::Run");

	if( !m_bSoundEnable )	return;
	if( !m_b3DSoundUpdate )	return;

	if(g_pGame)
	{
		rvector Pos = RCameraPosition;
		ZCharacter* pInterestCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		if(pInterestCharacter != NULL)
		{
			Pos = pInterestCharacter->GetPosition();
			Pos.z += 170.f;
		}

		rvector Orientation = Normalized(Pos - RCameraPosition);

		rvector right = CrossProduct(Orientation, RCameraUp);

		UpdateAmbSound(Pos, right);

		for( DSLIST::iterator iter = m_DelaySoundList.begin(); iter != m_DelaySoundList.end();)
		{
			DelaySound DS = *iter;
			if( DS.dwDelay < m_Time )
			{
				PlaySE( DS.pSS->pFS, DS.pos, DS.priority, DS.bPlayer );
				iter = m_DelaySoundList.erase( iter );
				continue;
			}
			++iter;
		}

		m_ListenerPos = Pos;
		
		MBeginProfile(1004, "ZSoundEngine::Run : SetListener");
		if(m_bInverse)
			ZGetSoundFMod()->SetListener( &Pos, NULL, -Orientation.x, -Orientation.y, Orientation.z, 0, 0, 1 );
		else
			ZGetSoundFMod()->SetListener( &Pos, NULL, Orientation.x, Orientation.y, Orientation.z, 0, 0, 1 );
	
		MEndProfile(1004);
		MBeginProfile(33, "ZSoundEngine::Run : Update");
		ZGetSoundFMod()->Update();
		MEndProfile(33);
	}
	MEndProfile(ZSoundEngineRun);
}


const char* ZSoundEngine::GetBGMFileName(int nBgmIndex)
{
	static char m_stSndFileName[MAX_BGM][64] = {"Intro Retake2(D-R).ogg", 
		"Theme Rock(D).ogg", 
		"HardBgm3 Vanessa Retake(D).ogg", 
		"HardBgm(D).ogg",
		"HardTech(D).ogg", 
		"HardCore(D).ogg",
		"Ryswick style.ogg", 
		"El-tracaz.ogg", 
		"Industrial technolism.ogg",
		"Fin.ogg" };

	static char szFileName[256] = "";
#define BGM_FOLDER		"Sound/BGM/"

	int nRealBgmIndex = nBgmIndex;
	if ((nBgmIndex >= BGMID_BATTLE) && (nBgmIndex < BGMID_FIN)) nRealBgmIndex = RandomNumber(BGMID_BATTLE, BGMID_FIN-1);
	sprintf_safe(szFileName, "%s%s", BGM_FOLDER, m_stSndFileName[nRealBgmIndex]);

	return szFileName;
}

bool ZSoundEngine::OpenMusic(int nBgmIndex, MZFileSystem* pfs)
{
	if( !m_bSoundEnable ) return false;

	m_pfs=pfs;
	if (nBgmIndex == BGMID_BATTLE) m_bBattleMusic = true;
	else m_bBattleMusic = false;

	char szFileName[256];
	strcpy_safe(szFileName, GetBGMFileName(nBgmIndex));

	return OpenMusic(szFileName, pfs);
}

bool ZSoundEngine::LoadResource( char* pFileName_ ,ZLoadingProgress *pLoading )
{
	if( !m_bSoundEnable )
 	{
		return false;
	}

	MXmlDocument Data;
	if (!Data.LoadFromFile(pFileName_, ZGetFileSystem()))
	{
		return false;
	}

	MXmlElement root, chr, attr;

	float fTemp;
	char szSoundName[256];
	char szSoundFileName[256];
	int iType = 0;

	root		= Data.GetDocumentElement();
	int iCount	= root.GetChildNodeCount();

	for( int i = 0 ; i < iCount; ++i )
	{
		if(pLoading && (i%10==0)) pLoading->UpdateAndDraw(float(i)/float(iCount));
		chr		= root.GetChildNode(i);
		chr.GetTagName( szSoundName );
		if( szSoundName[0] == '#' )
		{
			continue;
		}
		chr.GetAttribute( szSoundName, "NAME" );
		strcpy_safe( szSoundFileName, SOUNDEFFECT_DIR );
		strcat_safe( szSoundFileName, szSoundName );
		strcat_safe( szSoundFileName, ".wav" );

		chr.GetAttribute( &iType, "type", 0 );

		FSOUND_SAMPLE* pFS = NULL;
		FSOUND_SAMPLE* pFS2 = NULL;

		int flag = FSOUND_SIGNED|FSOUND_MONO;
		
		flag |= FSOUND_16BITS;

		switch( iType ) 
		{
		case 0:
			
			if(m_bHWMixing)
				flag |= FSOUND_HW3D;
			pFS = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );
			
			break;
		
		case 1:
			
			if(m_bHWMixing)
				flag |= FSOUND_HW2D; 
			else flag |= FSOUND_2D;			
			pFS = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );
			
			break;
		
		case 2:
						
			if(m_bHWMixing) 
				flag |= FSOUND_HW3D;
			pFS = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );
			
			if(m_bHWMixing) 
			{
				flag &= ~FSOUND_HW3D;
				flag |= FSOUND_HW2D; 
			}
			else flag |= FSOUND_2D;		
			pFS2 = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );
			
			break;
		
		case 3:

			flag &= ~FSOUND_MONO;
			flag |= FSOUND_STEREO;
			
			if(m_bHWMixing) flag |= FSOUND_HW2D;
			else flag |= FSOUND_2D;
			
			pFS2 = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );
			
			break;
		
		case 4:

  			pFS2 = ZGetSoundFMod()->LoadWave( szSoundFileName, FSOUND_LOOP_NORMAL|FSOUND_NORMAL|FSOUND_2D );
			break;

		case 5:
			pFS2 = ZGetSoundFMod()->LoadWave( szSoundFileName, FSOUND_LOOP_NORMAL|FSOUND_SIGNED|FSOUND_STEREO|FSOUND_16BITS|FSOUND_2D );
			break;

		case 6:
			pFS = ZGetSoundFMod()->LoadWave( szSoundFileName, FSOUND_LOOP_NORMAL|FSOUND_NORMAL );
			break;
		}

		SoundSource* pSS = NULL;
		
		if( pFS != NULL )
		{			
			float min = ZDEF_MINDISTANCE;
			float max = ZDEF_MAXDISTANCE;
			if( chr.GetAttribute( &fTemp, "MINDISTANCE" ))
				min = fTemp;
			pSS = new SoundSource;
			pSS->pFS = pFS;
			pSS->fMaxDistance = max;
			ZGetSoundFMod()->SetMinMaxDistance( pFS, min, 1000000000.0f );
			
#ifdef _DEBUG
			_ASSERT(m_SoundEffectSource.find(szSoundName)==m_SoundEffectSource.end());
#endif
			m_SoundEffectSource.insert(SESMAP::value_type(szSoundName, pSS ));			
		}
		if( pFS2 != NULL )
		{
			pSS = new SoundSource;
			pSS->pFS = pFS2;
#ifdef _DEBUG
			_ASSERT(m_SoundEffectSource2D.find(szSoundName)==m_SoundEffectSource2D.end());
#endif
			m_SoundEffectSource2D.insert(SESMAP::value_type(szSoundName, pSS ) );
		}

#ifdef _DEBUG
		if(pFS==NULL && pFS2==NULL) {
			mlog("cannot create sound : %s\n",szSoundName);
		}
#endif
	}
	strcpy_safe( m_SoundFileName, pFileName_ );
	return true;
}

int ZSoundEngine::GetEnumDeviceCount()
{
	return ZGetSoundFMod()->GetNumDriver();
}

const char* ZSoundEngine::GetDeviceDescription( int index )
{
	return ZGetSoundFMod()->GetDriverName( index );
}

void ZSoundEngine::SetMusicMute( bool b )
{
 	if( b == m_bMusicMute ) return;

	m_bMusicMute = b;

	if( !m_bSoundEnable ) return;

	ZGetSoundFMod()->SetMusicMute( b );
	if( !b )	SetMusicVolume( m_fMusicVolume );
}

int ZSoundEngine::PlaySE(FSOUND_SAMPLE* pFS, const rvector& pos, int Priority, bool bPlayer, bool bLoop)
{
	if(!m_bSoundEnable||m_bEffectMute||pFS==NULL) return -1;
	return ZGetSoundFMod()->Play(pFS, &pos, NULL, m_fEffectVolume*255,Priority,bPlayer,bLoop);
}

void ZSoundEngine::StopLoopSound()
{

}

void ZSoundEngine::StopSound( int iChannel )
{
	if(!m_bSoundEnable) return;
	ZGetSoundFMod()->StopSound( iChannel );
}

void ZSoundEngine::SetEffectMute( bool b )
{
	m_bEffectMute = b;
}

void ZSoundEngine::Set3DSoundUpdate(bool b) 
{
	m_b3DSoundUpdate = b; 
	if( !b )
	{
		ZGetSoundFMod()->StopSound();
	}
}

bool ZSoundEngine::Reload()
{
	mlog("Reload Sound Sources...\n");
	for( SESMAP::iterator iter = m_SoundEffectSource.begin(); iter != m_SoundEffectSource.end(); ++iter )
	{
		SoundSource* pSS = iter->second;
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource.clear();
	for( SESMAP::iterator iter = m_SoundEffectSource2D.begin(); iter != m_SoundEffectSource2D.end(); ++iter )
	{
		SoundSource* pSS = iter->second;
		SAFE_DELETE(pSS);
	}
	m_SoundEffectSource2D.clear();
	return LoadResource( m_SoundFileName );
}

void ZSoundEngine::SetAmbientSoundBox( char* Name, rvector& pos1, rvector& pos2, bool b2d )
{
	AmbSound AS;
	AS.type = AS_AABB;
	if(b2d) AS.type |= AS_2D;
	else AS.type |= AS_3D;
	AS.pSS = GetSoundSource(Name, b2d);
	if(AS.pSS == NULL)
	{
		return;
	}
	strcpy_safe(AS.szSoundName, Name);
	AS.iChannel = -1;
	AS.pos[0] = pos1;
	AS.pos[1] = pos2;
	AS.center = ( pos1 + pos2 ) * 0.5f;
	AS.dx = AS.pos[1].x - AS.center.x;
	AS.dy = AS.pos[1].y - AS.center.y;
	AS.dz = AS.pos[1].z - AS.center.z;
	m_AmbientSoundList.push_back(AS);
	if(!b2d) 
	{
		auto vec = rvector(AS.dx, AS.dy, AS.dz);
		float length = Magnitude(vec);
		FSOUND_Sample_SetMinMaxDistance( AS.pSS->pFS, length * 0.1f, length );
	}
}

void ZSoundEngine::SetAmbientSoundSphere( char* Name, rvector& pos, float radius, bool b2d )
{
	AmbSound AS;
	AS.type = AS_SPHERE;
	if(b2d) AS.type |= AS_2D;
	else AS.type |= AS_3D;
	AS.pSS = GetSoundSource(Name, b2d);
	if(AS.pSS == NULL)
	{
		return;
	}
	strcpy_safe(AS.szSoundName, Name);
	AS.iChannel = -1;
	AS.radius = radius;
	AS.center = pos;
	m_AmbientSoundList.push_back(AS);
	if(!b2d) FSOUND_Sample_SetMinMaxDistance( AS.pSS->pFS, radius * 0.1f, radius );
}

void ZSoundEngine::ClearAmbientSound()
{
	for(ASLIST::iterator iter = m_AmbientSoundList.begin(); iter != m_AmbientSoundList.end(); )
	{
		AmbSound* AS = &(*iter);
		
		if(AS->iChannel != -1) 
		{
			ZGetSoundFMod()->StopSound(AS->iChannel);
			SetEffectVolume( AS->iChannel, m_fEffectVolume );
			AS->iChannel = -1;
		}
		iter = m_AmbientSoundList.erase(iter);
	}	
}

void ZSoundEngine::UpdateAmbSound(rvector& Pos,	rvector& Ori)
{
	// 환경 사운드 처리
	for( ASLIST::iterator iter = m_AmbientSoundList.begin(); iter != m_AmbientSoundList.end(); ++iter )
	{
 		AmbSound* AS = &(*iter);

		if(AS == NULL) continue;

		float t = GetArea(Pos, *AS );

		if( t <=0 ) 
		{
			if( AS->iChannel != -1 )
			{
				SetEffectVolume(AS->iChannel,m_fEffectVolume);
				StopSound(AS->iChannel);
				AS->iChannel = -1;
			}
			continue;
		}

		if(AS->iChannel == -1)
		{
			AS->iChannel = PlaySE(AS->pSS->pFS, AS->center, 150, true );
		}

		if(AS->iChannel != -1 )
		{
			float vol = m_fEffectVolume * t;
			SetEffectVolume(AS->iChannel,vol);
		}
	}
}

#define AS_TA_ATTENUATION_RATIO_SQ 0.7f
#define AS_TB_ATTENUATION_RATIO_SQ 0.1f // 10 percent

#define AS_TA_AMP_COEFFICIENT 5.0f
#define AS_TB_AMP_COEFFICIENT 1.5f

float ZSoundEngine::GetArea( rvector& Pos, AmbSound& a )
{
	// box
 	if(a.type & AS_AABB)
	{
		float dX = fabs(Pos.x - a.center.x);
		float dY = fabs(Pos.y - a.center.y);
		float dZ = fabs(Pos.z - a.center.z);
		if(dX < a.dx && dY < a.dy && dZ < a.dz )
		{
			return min((a.dx-dX)/a.dx * (a.dy-dY)/a.dy * (a.dz-dZ)/a.dz * ((a.type&AS_2D)?AS_TA_AMP_COEFFICIENT:AS_TB_AMP_COEFFICIENT), 1.0f );
		}
		return -1;
	}
	// sphere 
	else if( a.type & AS_SPHERE )
	{
		auto vec = a.center - Pos;
 		float length = Magnitude(vec);
		float radius = a.radius;
		if( length >= radius ) return -1;
		float sacred = radius*((a.type&AS_2D)?AS_TA_ATTENUATION_RATIO_SQ:AS_TB_ATTENUATION_RATIO_SQ);
		if( length <= sacred ) return 1;
		return (radius - length)/(radius-sacred);
	}
	return -1;
}

void ZSoundEngine::SetVolumeControlwithDuration( float fStartPercent, float fEndPercent, DWORD dwDuration, bool bEffect, bool bBGM )
{
	m_bEffectVolControl = bEffect;
	m_bBGMVolControl = bBGM;

	DWORD currentTime = GetGlobalTimeMS();
	DWORD endTime = currentTime + dwDuration;
	int nUpdate = ( endTime - currentTime ) / m_DelayTime;
	
	float startEffectVol, startBGMVol;

	if( bEffect && !m_bEffectMute )
	{
		startEffectVol = fStartPercent*m_fEffectVolume;
		m_fEffectVolEnd = fEndPercent*m_fEffectVolume;
		m_fEffectVolFactor = ( m_fEffectVolEnd - startEffectVol ) / nUpdate;
		m_fEffectVolume = startEffectVol;
	}	

	if( bBGM && !m_bMusicMute )
	{
		startBGMVol = fStartPercent*m_fMusicVolume;
		m_fBGMVolEnd = fEndPercent*m_fMusicVolume;
		m_fBGMVolFactor = ( m_fBGMVolEnd - startBGMVol ) / nUpdate;
		m_fMusicVolume = startBGMVol;
	}
}

bool ZSoundEngine::LoadNPCResource(MQUEST_NPC nNPC, ZLoadingProgress* pLoading)
{
	FSOUND_SAMPLE* pFS = NULL;
	FSOUND_SAMPLE* pFS2 = NULL;

	int flag = FSOUND_SIGNED|FSOUND_MONO | FSOUND_16BITS;
	if (m_bHWMixing) flag |= FSOUND_HW3D;

	for (int i = 0; i < NPC_SOUND_END; i++)
	{
		MQuestNPCInfo* pNPCInfo = ZGetQuest()->GetNPCCatalogue()->GetInfo(nNPC);
		if (pNPCInfo == NULL) return false;

		if (pNPCInfo->szSoundName[i][0] == 0) continue;
		char szSoundFileName[256] = "";
		
		strcpy_safe(szSoundFileName, SOUNDNPC_DIR);
		strcat_safe(szSoundFileName, pNPCInfo->szSoundName[i] );
		strcat_safe(szSoundFileName, ".wav" );

		pFS = ZGetSoundFMod()->LoadWave( szSoundFileName, flag );

		if( pFS != NULL )
		{			
			float min = 500.0f;
			float max = ZDEF_MAXDISTANCE;

			SoundSource* pSS = new SoundSource;
			pSS->pFS = pFS;
			pSS->fMaxDistance = max;
			ZGetSoundFMod()->SetMinMaxDistance( pFS, min, 1000000000.0f );
			
			m_SoundEffectSource.insert(SESMAP::value_type(szSoundFileName, pSS ));
		}
	}

	m_ASManager.insert(nNPC);

	return true;
}

void ZSoundEngine::ReleaseNPCResources()
{

}

void ZSoundEngine::PlayNPCSound(MQUEST_NPC nNPC, MQUEST_NPC_SOUND nSound, rvector& pos, bool bMyKill)
{
	MQuestNPCInfo* pNPCInfo = ZGetQuest()->GetNPCCatalogue()->GetInfo(nNPC);
	if (pNPCInfo == NULL) return;

	if (pNPCInfo->szSoundName[nSound][0] != 0)
	{
		char szSoundFileName[256] = "";
		strcpy_safe(szSoundFileName, SOUNDNPC_DIR);
		strcat_safe(szSoundFileName, pNPCInfo->szSoundName[nSound] );
		strcat_safe(szSoundFileName, ".wav" );


		int nChannel = PlaySound(szSoundFileName, pos, false, false);
		if (nChannel != 0)
		{
			if (bMyKill)
			{
				ZGetSoundFMod()->SetMinMaxDistance(nChannel, 3500.0f, ZDEF_MAXDISTANCE);
			}
			else
			{
				ZGetSoundFMod()->SetMinMaxDistance(nChannel, 500.0f, ZDEF_MAXDISTANCE);
			}
		}
	}
}

bool ZSoundEngine::CheckCulling(const char* szName, SoundSource* pSS, const rvector& vSoundPos, bool bHero, int* pnoutPriority)
{
	auto vec = vSoundPos - m_ListenerPos;
	float fDistSq = MagnitudeSq(vec);

	if(!bHero)
	{
		if( fDistSq > (pSS->fMaxDistance*pSS->fMaxDistance) ) 
		{
#ifdef _SOUND_LOG
			mlog("Cull by Distance[%s]\n", szName);
#endif
			return false;
		}
	}


	u32 nNowTime = GetGlobalTimeMS();

	if ((nNowTime - pSS->nLastPlayedTime) < 10)
	{
		if (strncmp("fx_dash", szName, 7))
		{
#ifdef _DEBUG
			mlog("--------- 복수 sound 출력(%s, %u)\n", szName, nNowTime - pSS->nLastPlayedTime);
#endif
			return false;
		}
	}

	pSS->nLastPlayedTime = nNowTime;



	if (pnoutPriority)
	{
		*pnoutPriority = ((1-fDistSq/pSS->fMaxDistance)*ZDEF_MAX_DISTANCE_PRIORITY ); // 0~100
	}

	return true;
}


#endif