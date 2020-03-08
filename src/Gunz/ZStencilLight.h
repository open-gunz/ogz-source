#pragma	 once

#include "RTypes.h"
#include "RBaseTexture.h"
#include "RMeshUtil.h"
#include "map"
#include "MemPool.h"
#include "SimpleMesh.h"

struct LightSource : public CMemPoolSm<LightSource>
{
	bool bAttenuation;
	rvector pos;
	float power; /* 0~1 */
	DWORD attenuationTime;
	DWORD deadTime;
};

class ZStencilLight
{
protected:
	rvector m_Position;
	float m_Radius;
	RealSpace2::SphereMesh Mesh;
	RealSpace2::RBaseTexture* m_pTex;
	RTLVertex m_VBuffer[4];
	int m_id;
	std::map<int, LightSource*> m_LightSource;
    
public:
	void Destroy();

	void PreRender();
	void RenderStencil();
	void RenderStencil(const rvector& p, float radius);
	void RenderLight();	
	void PostRender();

	void Render();

	void Update();
	
	void SetPosition( const rvector& p) { m_Position = p; }
	void SetRadius( float r )	{ m_Radius = r; }
	
	int AddLightSource( const rvector& p, float power );
	int AddLightSource(const rvector& p, float power, DWORD lastTime );
	bool SetLightSourcePosition( int id, const rvector& p );
	bool DeleteLightSource( int id );
	bool DeleteLightSource( int id, DWORD lastTime );

	static ZStencilLight* GetInstance() { static ZStencilLight inst; return &inst; }

	size_t GetCount() { return m_LightSource.size(); }

public:
	ZStencilLight();
	ZStencilLight(const ZStencilLight&) = delete;
	~ZStencilLight();
};

ZStencilLight* ZGetStencilLight();