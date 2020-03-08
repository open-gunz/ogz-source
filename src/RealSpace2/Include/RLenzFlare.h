#pragma once

#include "RBaseTexture.h"

_NAMESPACE_REALSPACE2_BEGIN

enum FLARE_ELEMENT_TYPE
{
	FLARE_ELEMENT_SPHERE = 0,
	FLARE_ELEMENT_RING,
	FLARE_ELEMENT_SPOT,
	FLARE_ELEMENT_POLYGON,
	FLARE_ELEMENT_ETC,
	FLARE_ELEMENT_GROW,
	MAX_NUMBER_ELEMENT = 10,
};

#define MAX_LENZFLARE_NUMBER 1
#define MAX_NUMBER_TEXTURE MAX_NUMBER_ELEMENT

struct sFlareElement
{
	int iType;
	float width, height;
	u32 color;
	int iTextureIndex;
};

class RLenzFlare
{
public:
	RLenzFlare();
	~RLenzFlare();

	void Initialize();
	bool Render(rvector& light_pos,
		rvector& centre_,
		class RBspObject* pbsp_);
	bool Render(rvector& centre_, RBspObject* pbsp_);
	bool SetLight(rvector& pos_);
	void Clear() { miNumLight = 0; }
	auto GetNumLight() const { return miNumLight; }
	auto& GetLightPos(int i) const { return mLightList[i]; }

	static bool	Create(char* filename_);
	static bool	Destroy();
	static bool	IsReady();
	static RLenzFlare* GetInstance() { return &msInstance; }

private:
	bool open(const char* pFileName_, MZFileSystem* pfs_);
	bool draw(float x_, float y_,
		float width_, float height_,
		float alpha,
		u32 color_,
		int	textureIndex_);

	static bool ReadXmlElement(MXmlElement* PNode, char* Path);

	int	miNumFlareElement;
	int* miElementOrder;
	rvector mLightList[MAX_LENZFLARE_NUMBER];
	int miNumLight;

	static bool mbIsReady;
	static RLenzFlare msInstance;
	static sFlareElement msElements[MAX_NUMBER_ELEMENT];
	static RBaseTexture *msTextures[MAX_NUMBER_TEXTURE];
};

bool	RCreateLenzFlare(char* filename_);
bool	RDestroyLenzFlare();
bool	RReadyLenzFlare();
RLenzFlare* RGetLenzFlare();

_NAMESPACE_REALSPACE2_END