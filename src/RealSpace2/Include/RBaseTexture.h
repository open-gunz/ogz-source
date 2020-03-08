#pragma once

#include <string>
#include <map>
#include <algorithm>
#include "MUtil.h"
#include "RNameSpace.h"
#include "TextureLoader.h"
#include "StringView.h"
#include "MHash.h"
#include "RTypes.h"

typedef struct IDirect3DTexture9 *LPDIRECT3DTEXTURE9;

_NAMESPACE_REALSPACE2_BEGIN

enum class RTextureType
{
	Etc = 1 << 0,
	Map = 1 << 1,
	Object = 1 << 2,
	All = 1 << 3,
};

class RTextureManager;

class RBaseTexture final
{
public:
	RBaseTexture() = default;
	RBaseTexture(RBaseTexture&&) = default;
	RBaseTexture& operator =(RBaseTexture&&) = default;
	~RBaseTexture() = default;

	void Resize();

	auto GetWidth() const { return m_Info.Width; }
	auto GetHeight() const { return m_Info.Height; }
	auto GetMipLevels() const { return m_Info.MipLevels; }

	auto GetTexLevel() const { return m_nTexLevel; }
	void SetTexLevel(int level) { m_nTexLevel = level; }

	auto GetTexType() const { return m_nTexType; }
	void SetTexType(RTextureType type) { m_nTexType = type; }

	auto GetFormat() const { return static_cast<RPIXELFORMAT>(m_Info.Format); }

	auto& GetFileName() const { return filename; }

	LPDIRECT3DTEXTURE9 GetTexture();

private:
	friend RTextureManager;

	bool SubCreateTexture(const void* data, size_t size);

	bool	m_bManaged{};
	int		m_nRefCount{};
	bool	m_bUseMipmap{};
	bool	m_bUseFileSystem = true;

	int		m_nTexLevel{};
	RTextureType m_nTexType = RTextureType::Etc;

	TextureInfo m_Info{};
	D3DPtr<IDirect3DTexture9> m_pTex;

	std::string filename;
};

class RTextureManager final
{
public:
	RTextureManager() = default;
	RTextureManager(const RTextureManager&) = delete;
	~RTextureManager();

	void Destroy();

	RBaseTexture *CreateBaseTexture(const StringView& filename, RTextureType tex_type,
		bool bUseMipmap = false, bool bUseFileSystem = true);
	RBaseTexture *CreateBaseTextureMg(const StringView& filename, RTextureType tex_type,
		bool bUseMipmap = false, bool bUseFileSystem = true);
	RBaseTexture *CreateBaseTextureFromMemory(const void* data, size_t size, RTextureType texlevel,
		bool bUseMipmap = false, bool bUseFileSystem = true);

	void DestroyBaseTexture(RBaseTexture*);

	void OnInvalidate();
	void OnRestore();
	void OnChangeTextureLevel(RTextureType flag);

	int CalcUsedSize();
	int PrintUsedTexture();
	int CalcUsedCount();

private:
	RBaseTexture *CreateBaseTextureSub(bool Managed, const StringView& filename, RTextureType tex_type,
		bool bUseMipmap = false, bool bUseFileSystem = true);

	std::list<RBaseTexture> Textures;
	// String memory is stored within the textures.
	std::unordered_map<StringView, RBaseTexture*, PathHasher, PathComparer> FilenameToTexture;
};

void RBaseTexture_Create();
void RBaseTexture_Destroy();

void RBaseTexture_Invalidate();
void RBaseTexture_Restore();

RTextureManager* RGetTextureManager();

void SetObjectTextureLevel(int nLevel);
void SetMapTextureLevel(int nLevel);
void SetTextureFormat(int nLevel);	// 0 = 16 bit , 1 = 32bit

int GetObjectTextureLevel();
int GetMapTextureLevel();
int GetTextureFormat();

RBaseTexture* RCreateBaseTexture(const StringView& filename, RTextureType TexType = RTextureType::Etc,
	bool UseMipmap = false, bool bUseFileSystem = true);
RBaseTexture* RCreateBaseTextureMg(const StringView& filename, RTextureType TexType = RTextureType::Etc,
	bool UseMipmap = false, bool bUseFileSystem = true);
RBaseTexture* RCreateBaseTextureFromMemory(const void* data, size_t size,
	RTextureType TexType = RTextureType::Etc, bool UseMipmap = false);
void RDestroyBaseTexture(RBaseTexture*);
void RChangeBaseTextureLevel(RTextureType flag);

struct RBaseTextureDeleter {
	void operator()(RBaseTexture* ptr) const {
		RDestroyBaseTexture(ptr);
	}
};

using RBaseTexturePtr = std::unique_ptr<RBaseTexture, RBaseTextureDeleter>;

_NAMESPACE_REALSPACE2_END