#include "stdafx.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "MDebug.h"
#include "MZFileSystem.h"
#include "TextureLoader.h"
#include <numeric>

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

#define _MANAGED
#define LOAD_FROM_DDS

#ifdef _DEBUG
#define	LOAD_TEST
#endif

#ifdef LOAD_TEST
#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);
#else
#define __BP(i,n);
#define __EP(i);
#endif

static int g_nObjectTextureLevel;
static int g_nMapTextureLevel;
static int g_nTextureFormat; // 0 = 16 bit, 1 = 32bit
static RTextureManager* g_pTextureManager;

void SetObjectTextureLevel(int nLevel) {
	g_nObjectTextureLevel = nLevel == 3 ? 8 : nLevel; // HACK: Archetype's
}
void SetMapTextureLevel(int nLevel) {
	g_nMapTextureLevel = nLevel == 3 ? 8 : nLevel;
}
void SetTextureFormat(int nLevel) { g_nTextureFormat = nLevel; }
int GetObjectTextureLevel() { return g_nObjectTextureLevel; }
int GetMapTextureLevel() { return g_nMapTextureLevel; }
int GetTextureFormat() { return g_nTextureFormat; }

static int SubGetTexLevel(RTextureType tex_type)
{
	if (tex_type == RTextureType::Etc)
		return 0;
	else if (tex_type == RTextureType::Map)
		return GetMapTextureLevel();
	else if (tex_type == RTextureType::Object)
		return GetObjectTextureLevel();

	return 0;
}

void RBaseTexture::Resize()
{
	if (filename.empty())
		return;

	MZFile mzf;
	if (!mzf.Open(filename.c_str(), m_bUseFileSystem ? g_pFileSystem : nullptr))
		return;
	auto buf = mzf.Release();

	SubCreateTexture(buf.get(), mzf.GetLength());
}

LPDIRECT3DTEXTURE9 RBaseTexture::GetTexture()
{
	return m_pTex.get();
}

BOOL IsCompressedTextureFormatOk(D3DFORMAT TextureFormat);

bool RBaseTexture::SubCreateTexture(const void* data, size_t size)
{
	m_pTex = LoadTexture(data, size,
		1.0f / (1 << m_nTexLevel),
		m_Info);

	return m_pTex != nullptr;
}

void RBaseTexture_Create()
{
	g_pTextureManager = new RTextureManager;
}

void RBaseTexture_Destroy()
{
	SAFE_DELETE(g_pTextureManager);
}

RTextureManager* RGetTextureManager() {
	return g_pTextureManager;
}

RTextureManager::~RTextureManager() {
	Destroy();
	g_pTextureManager = NULL;
}

void RTextureManager::Destroy() {
	for (auto&& tex : Textures)
	{
		if (tex.m_nRefCount > 0)
		{
			mlog("RTextureManager::Destroy -- \"%s\" texture not destroyed. ( ref count = %d )\n",
				tex.filename.c_str(), tex.m_nRefCount);
		}
	}
}

void RTextureManager::OnInvalidate() {}
void RTextureManager::OnRestore() {}

void RTextureManager::OnChangeTextureLevel(RTextureType flag)
{
	RBaseTexture* pTex = NULL;

	for (auto&& tex : Textures)
	{
		if (flag == RTextureType::All || (static_cast<u32>(flag) & static_cast<u32>(tex.m_nTexType))) {
			tex.m_nTexLevel = SubGetTexLevel(flag);
			tex.Resize();
		}
	}
}

int RTextureManager::PrintUsedTexture()
{
	int cnt = 0;
	int nUse = 0;

	for (auto&& tex : Textures)
	{
		nUse = 0;

		if (tex.m_pTex)
			nUse = 1;

		mlog("texture : \"%s\" Used %d RefCnt %d \n", tex.filename.c_str(), nUse, tex.m_nRefCount);
		cnt++;
	}

	return cnt;
}

int RTextureManager::CalcUsedCount()
{
	return std::accumulate(Textures.begin(), Textures.end(), 0,
		[&](auto counter, auto&& tex) {
		return counter + static_cast<int>(tex.m_pTex != nullptr); });
}

int RTextureManager::CalcUsedSize()
{
	int return_size = 0;

	RBaseTexture* pTex = NULL;

	int add_size = 0;

	for (auto&& tex : Textures)
	{
		if (tex.m_pTex)
		{
			add_size = 0;

			add_size = NextPowerOfTwo(pTex->GetWidth()) / (1 << pTex->GetTexLevel()) *
				NextPowerOfTwo(pTex->GetHeight()) / (1 << pTex->GetTexLevel()) *
				(g_nTextureFormat == 0 ? 2 : 4);

			return_size += add_size;

		}
	}

	return return_size;
}

RBaseTexture *RTextureManager::CreateBaseTexture(const StringView& filename,
	RTextureType tex_type,
	bool bUseMipmap,
	bool bUseFileSystem)
{
	return CreateBaseTextureSub(false, filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture *RTextureManager::CreateBaseTextureMg(const StringView& filename,
	RTextureType tex_type,
	bool bUseMipmap,
	bool bUseFileSystem)
{
	return CreateBaseTextureSub(true, filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture * RTextureManager::CreateBaseTextureFromMemory(const void * data, size_t size,
	RTextureType tex_type, bool bUseMipmap, bool bUseFileSystem)
{
	Textures.emplace_back();
	auto&& new_tex = Textures.back();
	new_tex.m_bUseFileSystem = bUseFileSystem;
	new_tex.m_nRefCount = 1;
	new_tex.m_bUseMipmap = bUseMipmap;
	new_tex.m_nTexType = tex_type;
	new_tex.m_nTexLevel = SubGetTexLevel(tex_type);
	new_tex.SubCreateTexture(data, size);
	if (!new_tex.m_pTex)
	{
		Textures.pop_back();
		return nullptr;
	}
	return &new_tex;
}

RBaseTexture *RTextureManager::CreateBaseTextureSub(bool Managed, const StringView& _filename,
	RTextureType tex_type, bool UseMipmap, bool UseFileSystem)
{
	if (_filename.empty())
		return nullptr;

	auto it = FilenameToTexture.find(_filename);

	if (it != FilenameToTexture.end())
	{
		++it->second->m_nRefCount;
		return it->second;
	}

	auto filename_str = _filename.str();

	MZFile mzf;
	MZFileSystem* pfs = UseFileSystem ? g_pFileSystem : nullptr;

	// TODO: Fix this silly stuff. Add .dds to the filenames
	// in the xmls if there is actually one available
#ifdef LOAD_FROM_DDS
	char ddstexturefile[MAX_PATH];
	sprintf_safe(ddstexturefile, "%s.dds", filename_str.c_str());
	if (mzf.Open(ddstexturefile, pfs))
	{
		filename_str.append(".dds");
	}
	else
#endif
	{
		if (!mzf.Open(filename_str.c_str(), pfs))
			return nullptr;
	}

	auto buf = mzf.Release();

	auto* new_tex = CreateBaseTextureFromMemory(buf.get(), mzf.GetLength(),
		tex_type, UseMipmap, UseFileSystem);

	if (!new_tex)
	{
		MLog("Failed to create texture from file %s\n",
			filename_str.c_str());
		return nullptr;
	}

	// The filename string memory is stored within the texture.
	new_tex->filename = std::move(filename_str);
	
	StringView filename_view = new_tex->filename;
	FilenameToTexture.insert({ filename_view, new_tex });

	return new_tex;
}

void RTextureManager::DestroyBaseTexture(RBaseTexture* pTex)
{
	--pTex->m_nRefCount;
	if (pTex->m_nRefCount > 0)
		return;

	auto map_it = FilenameToTexture.find(pTex->filename.c_str());
	if (map_it != FilenameToTexture.end())
		FilenameToTexture.erase(map_it);

	auto it = std::find_if(Textures.begin(), Textures.end(), [&](auto&& x) { return &x == pTex; });
	if (it == Textures.end())
	{
		assert(false);
		return;
	}
	Textures.erase(it);
}

RBaseTexture* RCreateBaseTexture(const StringView& filename, RTextureType tex_type,
	bool bUseMipmap, bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTexture(filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture* RCreateBaseTextureMg(const StringView& filename, RTextureType tex_type,
	bool bUseMipmap, bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTextureMg(filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture * RCreateBaseTextureFromMemory(const void * data, size_t size,
	RTextureType TexType, bool UseMipmap)
{
	return g_pTextureManager->CreateBaseTextureFromMemory(data, size, TexType, UseMipmap);
}

void RDestroyBaseTexture(RBaseTexture *pTex)
{
	if (g_pTextureManager == NULL || pTex == NULL) return;
	g_pTextureManager->DestroyBaseTexture(pTex);
}

void RBaseTexture_Invalidate()
{
	_RPT0(_CRT_WARN, "begin invalidate\n");
	g_pTextureManager->OnInvalidate();
	_RPT0(_CRT_WARN, "invalidate complete\n");
}

void RBaseTexture_Restore()
{
	_RPT0(_CRT_WARN, "begin restore\n");
	g_pTextureManager->OnRestore();
	_RPT0(_CRT_WARN, "restore complete\n");
}

void RChangeBaseTextureLevel(RTextureType flag)
{
	_RPT0(_CRT_WARN, "begin change texture level\n");
	g_pTextureManager->OnChangeTextureLevel(flag);
	_RPT0(_CRT_WARN, "change texture level complete\n");
}

#undef __BP
#undef __EP

_NAMESPACE_REALSPACE2_END