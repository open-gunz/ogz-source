#include "stdafx.h"

#include "Mcv.h"

#include "McvDoc.h"
#include "McvView.h"

#include <mmsystem.h>

#include "MDebug.h"
#include "RealSpace2.h"

#include "RMtrl.h"

#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RVisualMeshMgr.h"

#include "RBspObject.h"
#include "RMaterialList.h"

#include "ScaleDlg.h"
#include "MtrlEditDlg.h"
#include "ModelInfoDlg.h"
#include "RCFrameInfoDlg.h"

#include ".\mcvview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

_USING_NAMESPACE_REALSPACE2

/////////////////////////////////////////////////////////////////////////////
// CMcvView

IMPLEMENT_DYNCREATE(CMcvView, CFormView)

BEGIN_MESSAGE_MAP(CMcvView, CFormView)
	//{{AFX_MSG_MAP(CMcvView)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_WM_DROPFILES()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_TEX_ON_OFF, OnTexOnOff)
	ON_COMMAND(ID_VERTEX_NORMAL, OnVertexNormal)
	ON_UPDATE_COMMAND_UI(ID_VERTEX_NORMAL, OnUpdateVertexNormal)
	ON_UPDATE_COMMAND_UI(ID_TEX_ON_OFF, OnUpdateTexOnOff)
	ON_CBN_SELCHANGE(IDC_PARTS_TYPE, OnCbnSelchangePartsType)
	ON_LBN_SELCHANGE(IDC_PARTS_LIST, OnLbnSelchangePartsList)
	ON_LBN_SELCHANGE(IDC_WEAPON_LIST, OnLbnSelchangeWeaponList)
	ON_LBN_SELCHANGE(IDC_ANIMATION_LIST, OnLbnSelchangeAnimationList)
	ON_BN_CLICKED(IDC_PLAY, OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_BN_CLICKED(IDC_PAUSE, OnBnClickedPause)
	ON_COMMAND(ID_FILE_WOPEN, OnFileWopen)
	ON_COMMAND(ID_CAMERA_RESET, OnCameraReset)
	ON_CBN_SELCHANGE(IDC_WEAPON_TYPE, OnCbnSelchangeWeaponType)
	ON_CBN_SELCHANGE(IDC_WEAPON_TYPE2, OnCbnSelchangeWeaponType2)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SLIDER1, OnNMThemeChangedSlider1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	ON_COMMAND(ID_SCALEDLG, OnScaledlg)
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_BKCOLOR, OnBkcolor)
	ON_COMMAND(ID_MAP_OBJ_POS, OnMapObjPos)
	ON_COMMAND(ID_GRID_ONOFF, OnGridOnoff)
	ON_COMMAND(ID_BBOX_ONOFF, OnBboxOnoff)
	ON_UPDATE_COMMAND_UI(ID_BBOX_ONOFF, OnUpdateBboxOnoff)
	ON_UPDATE_COMMAND_UI(ID_GRID_ONOFF, OnUpdateGridOnoff)
	ON_COMMAND(ID_EFFECT_RENDER, OnEffectRender)
	ON_UPDATE_COMMAND_UI(ID_EFFECT_RENDER, OnUpdateEffectRender)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_CHAR_VIEW_MODE, OnCharViewMode)
	ON_UPDATE_COMMAND_UI(ID_CHAR_VIEW_MODE, OnUpdateCharViewMode)
	ON_COMMAND(ID_PARTSCOLOR, OnPartscolor)
	ON_COMMAND(ID_MTRLEDITDLG, OnMtrleditdlg)
	ON_COMMAND(ID_PARTSCOLORALL, OnPartscolorall)
	ON_COMMAND(ID_ANIMATIONINFO, OnAnimationinfo)
	ON_COMMAND(ID_BLEND_COLOR, OnBlendColor)
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
	ON_COMMAND(ID_CARTOON_LIGHT_ONOFF, OnCartoonLightOnoff)
	ON_UPDATE_COMMAND_UI(ID_CARTOON_LIGHT_ONOFF, OnUpdateCartoonLightOnoff)
	ON_COMMAND(ID_MODELINFO, OnModelinfo)
	ON_STN_CLICKED(IDC_CURRENT_FRAME, OnStnClickedCurrentFrame)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcvView construction/destruction

CMcvView::CMcvView() : CFormView(CMcvView::IDD)
, m_SelMeshNodeName(_T(""))
, m_InfoString(_T(""))
{
	//{{AFX_DATA_INIT(CMcvView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nWheel = 0;
}

CMcvView::~CMcvView()
{
}

void CMcvView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMcvView)
	DDX_Control(pDX, IDC_PARTS_LIST, m_PartsListbox);
	DDX_Control(pDX, IDC_PARTS_TYPE, m_PartsTypeCombo);
	DDX_Control(pDX, IDC_WEAPON_TYPE, m_WeaponTypeCombo);
	DDX_Control(pDX, IDC_WEAPON_TYPE2, m_WeaponType2Combo);
	DDX_Control(pDX, IDC_WEAPON_LIST, m_WeaponListBox);
	DDX_Control(pDX, IDC_ANIMATION_LIST, m_AnimationListBox);
	DDX_Control(pDX, IDC_SLIDER1, m_SliderAniSpeed);
	DDX_Text(pDX, IDC_SELMESHNAME, m_SelMeshNodeName);
	DDX_Text(pDX, IDC_STATIC_INFO, m_InfoString);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_CURRENT_FRAME, m_CurrentFrame);
}

BOOL CMcvView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

char			g_last_open_elu_file[256];

CScaleDlg		g_sdlg;
CMtrlEditDlg	g_mtrl_dlg;
CModelInfoDlg	g_model_info_dlg;
CRCFrameInfoDlg	g_frame_info_dlg;

static bool g_TextureRenderOnOff = false;
static bool g_VertexNormalOnOff = false;
static bool g_CartoonLightOnOff = true;

bool g_bScale = false;
bool g_bmtrl_dlg = false;
bool g_bframeinfo_dlg = false;
bool g_bModelInfo_dlg = false;

float g_scale[3];

bool	g_bDrawGrid = true;
bool	g_bDrawBBox = false;
bool	g_effect_model = false;
bool	g_character_view = true;//or map model view 카메라 회전방향이 반대~

////////////////////////////////////////////////////////////////////
// toon Texture 관련..

LPDIRECT3DTEXTURE9 g_toon_tex = NULL;

#define TOONTEXLOAD

/////////////////////////////////////////////////////////////////////

void CMcvView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	g_scale[0] = 1.f;
	g_scale[1] = 1.f;
	g_scale[2] = 1.f;

	Init();

	g_sdlg.Create(IDD_SCALE,this);
	g_mtrl_dlg.Create(IDD_MTRLEDITDLG,this);
	g_model_info_dlg.Create(IDD_MODEL_INFO,this);
	g_frame_info_dlg.Create(IDD_FRAME_INFO,this);
}

/////////////////////////////////////////////////////////////////////////////
// CMcvView diagnostics

#ifdef _DEBUG

void CMcvView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMcvView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMcvDoc* CMcvView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMcvDoc)));
	return (CMcvDoc*)m_pDocument;
}

#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////
// Toon Texture 생성...

HRESULT	CreateToonTextures(char* filename)
{
	REL( g_toon_tex );
	
#ifdef TOONTEXLOAD

	D3DXIMAGE_INFO info;

	D3DXCreateTextureFromFileEx(RGetDevice(),filename,D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR,D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR,0,&info,0,&g_toon_tex);
	
	return S_OK;

#endif

	///////////////////////////////////////////////////////////////////////////////////////
	// 임시 텍스쳐 생성...

	HRESULT	rc;

#define MaxTexUSize 128
#define MaxTexUGap  5		// texture 단계..

//	rc = D3DXCreateTexture( RGetDevice() , 256 , 8 , D3DX_DEFAULT , 0 ,	D3DFMT_L8 , D3DPOOL_MANAGED , &g_toon_tex );
//	rc = D3DXCreateTexture( RGetDevice() , 256 , 8 , D3DX_DEFAULT , 0 ,	D3DFMT_X8R8G8B8 , D3DPOOL_MANAGED , &g_toon_tex );
	rc = D3DXCreateTexture( RGetDevice() , 16  , 1 , D3DX_DEFAULT , 0 ,	D3DFMT_X8R8G8B8 , D3DPOOL_MANAGED , &g_toon_tex );
//	rc = D3DXCreateTexture( RGetDevice() , MaxTexUSize , 1 , D3DX_DEFAULT , 0 ,	D3DFMT_X8R8G8B8 , D3DPOOL_MANAGED , &g_toon_tex );

	if ( FAILED(rc) )
		return rc;

	D3DLOCKED_RECT dataRect;
	memset(&dataRect, 0, sizeof(D3DLOCKED_RECT));

	g_toon_tex->LockRect(0, &dataRect, NULL, 0);

	unsigned char celBrightData[ 16 ] = { 64, 64, 64, 64, 64, 151, 151, 151, 151, 151, 151, 255, 255, 255, 255, 255};

//	unsigned char	byteColor = 0;

//	int				begin_color = 64;
//	int				end_color   = 255;
	
//	int				color_gap = (end_color-begin_color) / MaxTexUGap;//단계당 간격..
//	int				pixel_gap = MaxTexUSize / MaxTexUGap;
 
	unsigned char * dataPtr=(unsigned char *)dataRect.pBits;
	
//	int cnt = 0;

//	for(int i=0; i< MaxTexUSize ; ++i) {//픽셀수만큼...
	for(int i=0; i< 16 ; ++i) {//픽셀수만큼...

//		if(i) cnt = i / (pixel_gap+1);
//		else  cnt = i;

//		byteColor =  min(begin_color + (cnt * color_gap), end_color );

//		dataPtr[i*4+0] = byteColor;
//		dataPtr[i*4+1] = byteColor;
//		dataPtr[i*4+2] = byteColor;
//		dataPtr[i*4+3] = byteColor;

		dataPtr[i*4+0] = celBrightData[i];
		dataPtr[i*4+1] = celBrightData[i];
		dataPtr[i*4+2] = celBrightData[i];
		dataPtr[i*4+3] = celBrightData[i];
	}

	g_toon_tex->UnlockRect(0);

	if ( D3DXSaveTextureToFile("ToonTexture.bmp", D3DXIFF_BMP , g_toon_tex,NULL) != D3D_OK)
		return S_FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CMcvView message handlers

RLVertex	t_grid_vert[1000];

//<---------------------------------------------------------------------

void GetModelTry(RLVertex* pVert,int size,DWORD color,int* face_num)
{
	RLVertex _vert[4];

	RLVertex* t_vert = &_vert[0];

	t_vert->p.x		= 0.f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		= 0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		=-0.5f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		=-0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		= 0.5f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		=-0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		= 0.f*size;
	t_vert->p.y		= 1.5f*size;
	t_vert->p.z		= 0.f*size;
	t_vert->color	= color;

	pVert[0] = _vert[0];
	pVert[1] = _vert[1];
	pVert[2] = _vert[2];

	pVert[3] = _vert[0];
	pVert[4] = _vert[1];
	pVert[5] = _vert[3];

	pVert[6] = _vert[1];
	pVert[7] = _vert[2];
	pVert[8] = _vert[3];

	pVert[9]  = _vert[3];
	pVert[10] = _vert[2];
	pVert[11] = _vert[0];

	// face index num

	*face_num = 4;
}

void draw_try(LPDIRECT3DDEVICE9 dev,rmatrix mat,float size,DWORD color)
{
	int face_num = 0;

	GetModelTry(t_grid_vert,size,color,&face_num);

	dev->SetTransform( D3DTS_WORLD, &mat );
	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, face_num, (LPVOID) t_grid_vert, sizeof(RLVertex));
}

void draw_matrix(LPDIRECT3DDEVICE9 dev,rmatrix mat,float size)
{
	rvector pos   = rvector(0,0,0);
	rvector right = rvector(1,0,0)*size;
	rvector up	  = rvector(0,1,0)*size;
	rvector dir   = rvector(0,0,1)*size;

	dev->SetTransform( D3DTS_WORLD, &mat );
	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	RLVertex* t_vert = &t_grid_vert[0];

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xff00ff00;

	t_vert++;

	t_vert->p.x		= right.x;
	t_vert->p.y		= right.y;
	t_vert->p.z		= right.z;
	t_vert->color	= 0xff00ff00;

	t_vert++;

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xffff0000;

	t_vert++;

	t_vert->p.x		= up.x;
	t_vert->p.y		= up.y;
	t_vert->p.z		= up.z;
	t_vert->color	= 0xffff0000;

	t_vert++;

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xff0000ff;

	t_vert++;

	t_vert->p.x		= dir.x;
	t_vert->p.y		= dir.y;
	t_vert->p.z		= dir.z;
	t_vert->color	= 0xff0000ff;

	t_vert++;

	////////////////////////////////////

	dev->DrawPrimitiveUP(D3DPT_LINELIST, 3, (LPVOID) t_grid_vert, sizeof(RLVertex));
	
	rmatrix _mat = mat;
	rmatrix _tposmat;
	rmatrix _trotmat;

	//right
	D3DXMatrixTranslation(&_tposmat,right.x,right.y,right.z);
	D3DXMatrixRotationZ(&_trotmat,-3.14/2.f);
	_mat = _trotmat*_tposmat*_mat;
	draw_try(dev,_mat,size/10,0xff00ff00);

	//up
	_mat = mat;
	D3DXMatrixTranslation(&_tposmat,up.x,up.y,up.z);
	_mat = _tposmat*_mat;
	draw_try(dev,_mat,size/10,0xffff0000);

	//dir
	_mat = mat;
	D3DXMatrixTranslation(&_tposmat,dir.x,dir.y,dir.z);
	D3DXMatrixRotationX(&_trotmat,3.14/2.f);
	_mat = _trotmat*_tposmat*_mat;
	draw_try(dev,_mat,size/10,0xff0000ff);

}

//<---------------------------------------------------------------------

void draw_grid(LPDIRECT3DDEVICE9 dev,D3DXVECTOR3 pos,float w,float h,int cnt_x,int cnt_y,DWORD color)
{
	if(cnt_x > 500) cnt_x = 500;
	if(cnt_y > 500) cnt_y = 500;

	int size	= 0;

	float x_w	= 0;
	float x_h	= 0;

	float z_add = h * (float)cnt_y;
	float x_add = w * (float)cnt_x;

	D3DXMATRIX  t_mat;
	D3DXMatrixIdentity(&t_mat);
	dev->SetTransform( D3DTS_WORLD, &t_mat );

	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	// 세로 라인

	RLVertex* t_vert = &t_grid_vert[0];

	for(int i=0;i<cnt_x+1;i++) {

		t_vert->p.x		= pos.x + x_w;
		t_vert->p.y		= pos.y;
		t_vert->p.z		= pos.z;
		t_vert->color	= color;

		t_vert++;

		t_vert->p.x		= pos.x + x_w;
		t_vert->p.y		= pos.y;
		t_vert->p.z		= pos.z - z_add;
		t_vert->color	= color;

		t_vert++;

		x_w += w;
		size++;
	}
	
	dev->DrawPrimitiveUP(D3DPT_LINELIST, size, (LPVOID) t_grid_vert, sizeof(RLVertex));

	// 가로라인

	t_vert = &t_grid_vert[0];
	size	= 0;

	for(i=0;i<cnt_y+1;i++) {

		t_vert->p.x		= pos.x;
		t_vert->p.y		= pos.y;
		t_vert->p.z		= pos.z - x_h;
		t_vert->color	= color;

		t_vert++;

		t_vert->p.x		= pos.x + x_add;
		t_vert->p.y		= pos.y;
		t_vert->p.z		= pos.z - x_h;
		t_vert->color	= color;

		t_vert++;

		x_h += h;
		size++;
	}

	dev->DrawPrimitiveUP(D3DPT_LINELIST, size, (LPVOID) t_grid_vert, sizeof(RLVertex));
}


void draw_center(LPDIRECT3DDEVICE9 dev,float size_w,float size_h,DWORD color)
{
	D3DXMATRIX  t_mat;
	D3DXMatrixIdentity(&t_mat);
	dev->SetTransform( D3DTS_WORLD, &t_mat );

	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	// 세로 라인
	RLVertex t_vert[4];
	
	RLVertex* vert = &t_vert[0];

	vert->p.x	= -size_w/2.f;
	vert->p.y	= 0.f;
	vert->p.z	= 0.f;
	vert->color	= color;

	vert++;

	vert->p.x	= size_w/2.f;
	vert->p.y	= 0.f;
	vert->p.z	= 0.f;
	vert->color	= color;

	vert++;

	vert->p.x	= 0.f;
	vert->p.y	= 0.f;
	vert->p.z	= size_h/2.f;
	vert->color	= color;

	vert++;

	vert->p.x	= 0.f;
	vert->p.y	= 0.f;
	vert->p.z	= -size_h/2.f;
	vert->color	= color;

	dev->DrawPrimitiveUP(D3DPT_LINELIST, 2, (LPVOID) t_vert, sizeof(RLVertex));
}


///////////////////////////////////////////////////////////////////////////////////

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

RMODEPARAMS mparams = { 640,480,false,D3DFMT_A1R5G5B5 };

LPDIRECT3DDEVICE9 g_dev = NULL;

RMeshMgr		g_mesh_mgr;
RMeshMgr		g_weapon_mesh_mgr;

RAnimationMgr	g_ani_mgr;
RVisualMeshMgr	g_vmesh_mgr;

int			g_id;

int			g_ArcMode = 0;
CD3DArcBall	g_ArcBall[2];

D3DXMATRIX	g_matWorld;
D3DXMATRIX	g_matView;

D3DXVECTOR3	g_light_pos;

D3DXVECTOR3	g_map_obj_pos;

static float g_rot_y = 0.f;
static float g_pos_h = 0.f;

// 임시로 사용할 카메라

class CTCamera {
public:
	CTCamera() {

		m_rot_x = 0.f;
		m_rot_y = 0.f;
		m_at	= rvector(0.f,0.f,0.f);
		m_dist	= 100.f;

	}

	~CTCamera() {

	}

	void Create(int w,int h) {

		m_dist = 300.f;

		m_at = rvector(0.f,0.f,0.f);//바라보는 위치

		RSetCamera(rvector(0.f, 80.f,-m_dist ),m_at ,rvector(0.f,1.f,0.f));

		RSetProjection(D3DX_PI/4.f,w/(float)h,1.f,50000.f);
	}

	void Render() {

		D3DXMATRIX mat;
		D3DXMatrixTranslation(&mat,m_at.x,m_at.y,m_at.z);

		draw_box(&mat,rvector(5.f,5.f,5.f),rvector(-5.f,-5.f,-5.f),0xffffffff);
	}

	void Update() {

		// 회전값으로 방향 벡터를 만들고~
		// dist 로 거리조절

		rvector pos = RCameraPosition;
		rvector dir = RCameraDirection;	
		rvector up  = RCameraUp;

		rvector v = rvector(0.f,0.f,-1.f);

		rmatrix rot = RGetRotX(m_rot_x) * RGetRotY(m_rot_y);

		dir = v * rot;

		D3DXVec3Normalize(&dir,&dir);

		v = dir * m_dist;

		pos = m_at + v;

		RSetCamera(pos,m_at,up);
	}

	void Reset() {

		m_dist = 300.f;

		m_at = rvector(0.f,80.f,0.f);

		m_rot_x = 0.f;
		m_rot_y = 0.f;

		RSetCamera(rvector(0.f, 80.f,-m_dist ),m_at ,rvector(0.f,1.f,0.f));
	}

	rvector GetPos() {
		return RCameraPosition;
	}

	rvector GetDir() {
		rvector dir = RCameraDirection;	
		D3DXVec3Normalize(&dir,&dir);
		return dir;
	}

	rvector GetRight() {

		rvector right;
		rvector dir = RCameraDirection;
		rvector up = RCameraUp;

		CrossProduct(&right,dir,up);
		
		D3DXVec3Normalize(&right,&right);
		return right;
	}

	void Up(float f) {
		m_at.y += f;
	}

	void Down(float f) {
		m_at.y -= f;
	}

	void Front(float f) {
		rvector dir = GetDir();
		m_at += dir * f;

	}

	void Back(float f) {
		rvector dir = GetDir();
		m_at -= dir * f;
	}

	void Left(float f) {
		rvector r = GetRight();
		m_at += r * f;
	}

	void Right(float f) {
		rvector r = GetRight();
		m_at -= r * f;
	}

	void CenterZoomIn(float f) {
		m_dist += f*2;
	}

	void CenterZoomOut(float f) {
		m_dist -= f*2;
	}

	void CenterRotationLeft(float f) {
		m_rot_y -= f;
	}

	void CenterRotationRight(float f) {
		m_rot_y += f;
	}

	void CenterRotationUp(float f) {
		m_rot_x += f;
	}

	void CenterRotationDown(float f) {
		m_rot_x -= f;
	}

	float m_rot_x;
	float m_rot_y;

	rvector m_at;
	float   m_dist;
};

CTCamera g_camera;

float g_Wheel;

void SetLight(RVisualMesh* pVMesh,int mode,rvector& pos)
{
	D3DLIGHT9 light;

	ZeroMemory( &light, sizeof(D3DLIGHT9) );

	light.Type        = D3DLIGHT_POINT;

	light.Diffuse.r   = 1.0f;
	light.Diffuse.g   = 1.0f;
	light.Diffuse.b   = 1.0f;

	light.Specular.r	= 1.f;
	light.Specular.g	= 1.f;
	light.Specular.b	= 1.f;

	light.Position.x   = pos.x;
	light.Position.y   = pos.y;
	light.Position.z   = pos.z;

	light.Range        = 1000.0f;

	light.Attenuation0 = 0.9f;
	light.Attenuation1 = 0.0f;

//	g_dev->SetLight( mode, &light );
//	g_dev->LightEnable(mode, TRUE);
//	g_dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	pVMesh->SetLight(mode,&light,false);
}

////////////////////////////////////////////////////////////////////////////////
//

int g_win_pos_x = 0;
int g_win_pos_y = 0;

////////////////////////////////////////////////////////////////////////////////

bool CMcvView::Init()
{
	InitLog( MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE );

	CWnd* _hWnd = GetDlgItem(IDC_PIC);
	
	RECT rect,rect2;

	_hWnd->GetClientRect(&rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;

	mparams.nWidth	= w;
	mparams.nHeight = h;

	RInitDisplay(_hWnd->m_hWnd,&mparams);

	_hWnd->GetWindowRect(&rect);//윈도우위치

	GetWindowRect(&rect2);

	g_win_pos_x = rect.left - rect2.left;
	g_win_pos_y = rect.top - rect2.top;

	g_dev = RGetDevice();

	//////////////////////////////////////////////////////////
	// 캐릭터 로딩시 크기에 맞게 위치조정

	g_camera.Create(w,h);

	//////////////////////////////////////////////////////////
	// 캐릭터 로딩시 크기에 맞게 위치조정

	g_light_pos = D3DXVECTOR3(0.f,150.f,-480.f);//캐릭터 바운딩 박스의 중간 앞쪽..


	g_dev->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.8f, 0.8f, 0.8f, 1.0f ) );

	//////////////////////////////////////////////////////////

	g_ArcBall[0].SetWindow( w,h, 0.85f );
	g_ArcBall[0].SetRadius( 2.5f );

	g_ArcBall[1].SetWindow( w,h, 0.85f );
	g_ArcBall[1].SetRadius( 2.5f );

	D3DXMatrixIdentity(&g_matWorld);
	D3DXMatrixIdentity(&g_matView);


	///////////////////////////////////////
	// Parts

	m_PartsTypeCombo.AddString("eq_head");
	m_PartsTypeCombo.AddString("eq_face");
	m_PartsTypeCombo.AddString("eq_chest");
	m_PartsTypeCombo.AddString("eq_hands");
	m_PartsTypeCombo.AddString("eq_legs");
	m_PartsTypeCombo.AddString("eq_feet");

	m_PartsTypeCombo.SetCurSel(0);


	///////////////////////////////////////
	// Weapon Type 

	// 무기목록은 한손 양손 + 종류

	m_WeaponTypeCombo.AddString("eq_single_sword");
	m_WeaponTypeCombo.AddString("eq_single_gun");

	m_WeaponTypeCombo.AddString("eq_double_sword");
	m_WeaponTypeCombo.AddString("eq_double_gun");

	m_WeaponTypeCombo.SetCurSel(0);

	// 장비 위치는 왼손 오른손 + 종류 

	m_WeaponType2Combo.AddString("eq_left_sword");
	m_WeaponType2Combo.AddString("eq_left_gun");
//	m_WeaponType2Combo.AddString("eq_left_bow");
	m_WeaponType2Combo.AddString("eq_right_sword");
	m_WeaponType2Combo.AddString("eq_right_gun");
//	m_WeaponType2Combo.AddString("eq_right_bow");

	m_WeaponType2Combo.SetCurSel(0);

	m_nSelectPartsType  = 0;
	m_nSelectWeaponType = 0;
	m_nSelectWeaponType2 = 0;

	m_SliderAniSpeed.SetRange(0,100);
	m_SliderAniSpeed.SetPos(48);

	SetClearColor(0xffb0b0b0);

	g_map_obj_pos = D3DXVECTOR3(0,0,0);

	OnCameraReset();

	RMesh::SetToolMesh(true);//툴에서 사용될 모델이라는걸 알린다~

	m_SelMeshNodeName = "";

	CreateToonTextures("ToonTexture.bmp");

	return true;
}

void CMcvView::Finish()
{
	RCloseDisplay();
	REL( g_toon_tex );
}

//void AddPartsRot(float x,float y);//temp

// 카메라가 타겟 위치 까지 가지고 움직이기..
// 


void CMcvView::UpdateLightKey(float time)
{
	if(IsKeyDown(VK_NUMPAD4))	g_light_pos.x -= time*2;
	if(IsKeyDown(VK_NUMPAD6))	g_light_pos.x += time*2;

	if(IsKeyDown(VK_NUMPAD8))	g_light_pos.z += time*2;
	if(IsKeyDown(VK_NUMPAD2))	g_light_pos.z -= time*2;

	if(IsKeyDown(VK_NUMPAD7))	g_light_pos.y += time*2;
	if(IsKeyDown(VK_NUMPAD9))	g_light_pos.y -= time*2;

//	SetLight(0, g_light_pos );
//	SetLight(1, g_camera.GetPos() );
}

void CMcvView::UpdateCameraKey(float time)
{
	// 모드에 따라서 작동이 달라져야함
	// 카메라를 중심캐릭터를 비추는 모드
	// 카메라가 마음대로 돌아다니는 모드

	//////////////////////////////////
	// 각도 조정

	if( g_character_view) {

		// 카메라 이동

		if(IsKeyDown('Q'))		g_camera.Up(time);		// 높이조정
		if(IsKeyDown('E'))		g_camera.Down(time);
		if(IsKeyDown('W'))		g_camera.m_dist -= (time*4);	// 줌인 아웃
		if(IsKeyDown('S'))		g_camera.m_dist += (time*4);
		if(IsKeyDown('A'))		g_camera.CenterRotationRight(time);	// 그냥 회전
		if(IsKeyDown('D'))		g_camera.CenterRotationLeft(time);

		if(IsKeyDown(VK_LEFT))	g_camera.CenterRotationRight(time);
		if(IsKeyDown(VK_RIGHT))	g_camera.CenterRotationLeft(time);

	} else {
	
		if(IsKeyDown('Q'))		g_camera.Up(time);
		if(IsKeyDown('E'))		g_camera.Down(time);
		if(IsKeyDown('W'))		g_camera.Front(time*4);
		if(IsKeyDown('S'))		g_camera.Back(time*4);
		if(IsKeyDown('A'))		g_camera.Left(time*4);
		if(IsKeyDown('D'))		g_camera.Right(time*4);

		if(IsKeyDown(VK_LEFT))	g_camera.CenterRotationLeft(time);
		if(IsKeyDown(VK_RIGHT))	g_camera.CenterRotationRight(time);

	}

	if(IsKeyDown(VK_UP))	g_camera.CenterRotationUp(time);
	if(IsKeyDown(VK_DOWN))	g_camera.CenterRotationDown(time);
}

void CMcvView::UpdateKey(float time)
{
	//////////////////////////////////
	// 거리 조정

	// 지금윈도우가 최상위인경우

	bool bWindowTop = true;
	
	if( m_hWnd != ::GetActiveWindow() ) {
		bWindowTop = false;
	}

	if(bWindowTop) {

		UpdateCameraKey(time);
		UpdateLightKey(time);

		if(IsKeyDown(VK_SPACE)) {
			OnCameraReset();
		}

	}

	if(IsKeyDown('G'))
		OnGridOnoff();

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

/*
	if(pVMesh) {

		if(IsKeyDown('I'))//	AddPartsRot(0.f, 1.f);
			pVMesh->m_vRotXYZ.y =  40.f;
		if(IsKeyDown('K'))//	AddPartsRot(0.f,-1.f);
			pVMesh->m_vRotXYZ.y = -40.f;

		if(IsKeyDown('J'))//	AddPartsRot(-1.f,0.f);
			pVMesh->m_vRotXYZ.x =  40.f;
		if(IsKeyDown('L'))//	AddPartsRot( 1.f,0.f);
			pVMesh->m_vRotXYZ.x = -40.f;

	}

	if(IsKeyDown('I'))	AddPartsRot(0.f, 1.f);
	if(IsKeyDown('K'))	AddPartsRot(0.f,-1.f);

	if(IsKeyDown('J'))	AddPartsRot(-1.f,0.f);
	if(IsKeyDown('L'))	AddPartsRot( 1.f,0.f);
*/

	if(g_Wheel != 0) {

		g_camera.m_dist = (-g_Wheel * 0.1f) + g_camera.m_dist;
		g_Wheel = 0;
	}

}

void CMcvView::Update()
{
	static DWORD thistime , lasttime = timeGetTime() , elapsed;

	thistime = timeGetTime();

	elapsed  = (thistime - lasttime) * (IsKeyDown(VK_SHIFT) ? 1:3);

	lasttime = thistime;

	float fsp	  = elapsed * 0.03f;

	UpdateKey(fsp);

	g_camera.Update();

	//////////////////////////////////

/*
	rvector v2,vr;

	v2 = -(dir * fsp);

	D3DXVec3Cross(&vr,&dir,&up);

	if(IsKeyDown('Y'))		g_camera_at += v2;
	if(IsKeyDown('H'))		g_camera_at -= v2;
	if(IsKeyDown('G'))		g_camera_at -= vr;
	if(IsKeyDown('J'))		g_camera_at += vr;
*/

	/////////////////////////////////
}

//////////////////////////////////////////////////////////////////
//

rmatrix g_ms,g_mt;

void SetScale(rvector& s ) {

	float max = -1.f;
	float rs = 1.f;

	if(s.x > max) max = s.x;
	if(s.y > max) max = s.y;
	if(s.z > max) max = s.z;
	
	if( max > 200.f) {
		rs = 200.f/max;
	}

	D3DXMatrixScaling(&g_ms,rs,rs,rs);
}

bool SetPosMat(RMesh* pMesh) {

	if(!pMesh) return false;

//	pMesh->CalcBox();//world mat 회전값 생략
//	2번쨰 프레임 부터는 계산 없어도 되니까.. 그냥사용

	rvector max = pMesh->m_vBBMax;
	rvector	min = pMesh->m_vBBMin;

	rvector s = max - min;
	
	SetScale(s);

	s /= 2.f;
	s += min;

	D3DXMatrixIdentity(&g_mt);

	g_mt._41 -= s.x;
//	g_mt._42 -= s.y;
	g_mt._43 -= s.z;

	return true;
}

rmatrix makematrix(rvector pos,rvector dir,rvector up)
{
	rmatrix m;
	rvector right;
//	D3DXVec3Cross(&right,&dir,&up);
	D3DXVec3Cross(&right,&up,&dir);
	D3DXVec3Normalize(&right,&right);

	D3DXVec3Cross(&up,&right,&dir);
	D3DXVec3Normalize(&up,&up);

	D3DXVec3Normalize(&dir,&dir);

	D3DXMatrixIdentity(&m);

	m._11 = right.x;
	m._12 = right.y;
	m._13 = right.z;

	m._21 = up.x;
	m._22 = up.y;
	m._23 = up.z;

	m._31 = dir.x;
	m._32 = dir.y;
	m._33 = dir.z;

	m._41 = pos.x;
	m._42 = pos.y;
	m._43 = pos.z;

	return m;
}

//<--------------------------------------------------

#define TIME_GAP 5000.f

float GetTimefloat()
{
	static DWORD back_time = timeGetTime();

	DWORD this_time = timeGetTime();

	if(this_time - back_time == 0)
		return 0.f;

	if(this_time > back_time + TIME_GAP) {
		back_time +=TIME_GAP;
	}

	this_time %= (int)TIME_GAP;

	float fhr = (float)this_time / TIME_GAP;

	return fhr;
}

void GetQuatDir(D3DXQUATERNION& q,rvector& right,rvector& up,rvector& dir) {

	float x = q.x;
	float y = q.y;
	float z = q.z;
	float r = q.w;

	right	= rvector(1-2*(y*y+z*z),  2*(x*y+r*z),  2*(x*z-r*y));
	up		= rvector(  2*(x*y-r*z),1-2*(x*x+z*z),  2*(y*z+r*x));
	dir		= rvector(  2*(x*z+r*y),  2*(y*z-r*x),1-2*(x*x+y*y));
}

rmatrix _makealignmatrix(int mode,rmatrix& m)
{
	rmatrix rm = m;
//	D3DXMatrixIdentity(&rm);

	rvector right = rvector(m._11,m._12,m._13);
	rvector up	  = rvector(m._21,m._22,m._23);
	rvector dir   = rvector(m._31,m._32,m._33);
	rvector pos   = rvector(m._41,m._42,m._43);

//	rvector ca_dir	= -RCameraDirection;
	rvector ca_dir	=  RCameraPosition - pos;
	rvector ca_up	=  RCameraUp;
	rvector ca_pos	=  RCameraPosition;

	//////////////////////////////////////////////

	Normalize(ca_dir);
	Normalize(ca_up);

//	scale 고려~

	if( mode==0 ) {

		CrossProduct(&right,up,ca_dir);

		Normalize(right);

		CrossProduct(&up,ca_dir,right);

		Normalize(up);

		rm._11 = right.x;
		rm._12 = right.y;
		rm._13 = right.z;

		rm._21 = up.x;
		rm._22 = up.y;
		rm._23 = up.z;

		rm._31 = ca_dir.x;
		rm._32 = ca_dir.y;
		rm._33 = ca_dir.z;

	} else if( mode==1 ) {

		/*
		CrossProduct(&right,ca_dir,up);

		Normalize(right);

		CrossProduct(&up,ca_dir,right);

		Normalize(up);

		CrossProduct(&right,ca_dir,up);

		Normalize(right);

		rm._11 = right.x;
		rm._12 = right.y;
		rm._13 = right.z;

		rm._21 = up.x;
		rm._22 = up.y;
		rm._23 = up.z;

		rm._31 = ca_dir.x;
		rm._32 = ca_dir.y;
		rm._33 = ca_dir.z;
		*/

		CrossProduct(&right,ca_dir,dir);
		Normalize(right);

		CrossProduct(&right,ca_dir,dir);
		Normalize(right);

	}

	return rm;
}

rmatrix _makematrix(rvector dir,rvector up,rvector pos) {

	rmatrix m;
	D3DXMatrixIdentity(&m);

	rvector right;
	D3DXVec3Normalize(&dir,&dir);
	D3DXVec3Normalize(&up,&up);

	D3DXVec3Cross(&right,&up,&dir);
	D3DXVec3Normalize(&right,&right);

	D3DXVec3Cross(&up,&dir,&right);

	// x-> right
	// y-> up
	// z-> dir

	m._11 = right.x;
	m._12 = right.y;
	m._13 = right.z;

	m._21 = up.x;
	m._22 = up.y;
	m._23 = up.z;

	m._31 = dir.x;
	m._32 = dir.y;
	m._33 = dir.z;

	m._41 = pos.x;
	m._42 = pos.y;
	m._43 = pos.z;

	return m;
}

rmatrix _blendmatrixlimit(rmatrix& m,rvector& vdir,float f)
{
	//dir ,vdir 사이의 dot 의 각도에 따라 f 비율을 조정~ (회전 불가능하게 멀다면)
	rmatrix rm;
	D3DXMatrixIdentity(&rm);
	return rm;
}


rmatrix _blendmatrix(rmatrix& m,rvector& vdir,float f)
{
	// scale 고려 안함~
	rmatrix rm;
	D3DXMatrixIdentity(&rm);

	rvector dir   = rvector(m._11,m._12,m._13);
	rvector up    = rvector(m._21,m._22,m._23);
	rvector right = rvector(m._31,m._32,m._33);
	rvector pos	  = rvector(m._41,m._42,m._43);

	right.x = m._11;
	right.y = m._12;
	right.z = m._13;

	up.x = m._21;
	up.y = m._22;
	up.z = m._23;

	dir.x = m._31;
	dir.y = m._32;
	dir.z = m._33;

	pos.x = m._41;
	pos.y = m._42;
	pos.z = m._43;

	D3DXVec3Lerp(&dir,&dir,&vdir,f);
	Normalize(dir);

	rm = _makematrix(dir,up,pos);

	return rm;
}

//<--------------------------------------------------

using namespace std;

#include "MPDB.h"



////////////////////////////////////////////////////////////////////////////////////

void CMcvView::Idle()
{
	TRACE("TICK:%4u \n", GetTickCount());

	UpdateAnimationSlider();
	ChangeAnimationInfoString();

	static float h = 0.f;

	char a = '`';
	Update();

	if(h == 0.f) {
		RMesh* pMesh = g_mesh_mgr.GetFast(0);

		if(pMesh) {
			rvector p;
			pMesh->CalcBox();
			p = pMesh->m_vBBMax - pMesh->m_vBBMin;

			h = -p.y/2;
		}
		else h = 30.f;
	}

//	RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );

/////////////////////////////////////////////////////////////////



/*
	rvector	viewpos[8];
	
	rmatrix invMat;

	D3DXMatrixInverse(&invMat,NULL,&RViewProjection);
*/
/*
	1    2
 	 |--|
	 |--|
	0    3
*/

/*
	// near

	viewpos[0] = rvector( -1.f,-1.f, 0.f );
	viewpos[1] = rvector( -1.f, 1.f, 0.f );
	viewpos[2] = rvector(  1.f, 1.f, 0.f );
	viewpos[3] = rvector(  1.f,-1.f, 0.f );

	// far

	viewpos[4] = rvector( -1.f,-1.f, 1.f );
	viewpos[5] = rvector( -1.f, 1.f, 1.f );
	viewpos[6] = rvector(  1.f, 1.f, 1.f );
	viewpos[7] = rvector(  1.f,-1.f, 1.f );

	for(int i=0;i<8;i++) {
		viewpos[i] = viewpos[i] * invMat;
	}
*/

/////////////////////////////////////////////////////////////////////

/*	
	face 8개

	0,1,2 ,	0,2,3 ,	
	3,2,6 ,	3,6,7 ,	
	7,6,5 , 7,5,4 ,	
	4,5,1 , 4,1,0 ,	
	1,5,6 ,	1,6,2 ,	
	4,0,3 , 4,3,7
*/	
	
//	pick 되는가 보기..

//	string str;
//	GetCrashInfo(0,str);

	D3DXMATRIX matTemp,rotMat;
	D3DXMatrixTranslation( &g_matWorld, g_map_obj_pos.x, g_map_obj_pos.y, g_map_obj_pos.z );
//	D3DXMatrixMultiply( &g_matWorld, &g_matWorld, g_ArcBall[0].GetTranslationMatrix() );
//	D3DXMatrixMultiply( &g_matWorld, &g_matWorld, g_ArcBall[0].GetRotationMatrix() );
//	rotMat = RGetRotY(g_rot_y);
//	D3DXMatrixMultiply( &g_matWorld, &g_matWorld, &rotMat );

	RRESULT isOK=RIsReadyToRender();

//	g_dev->SetRenderState( D3DRS_ZENABLE ,D3DZB_USEW ); // for w-buffering
	g_dev->SetRenderState( D3DRS_ZENABLE ,TRUE );

	if(isOK==R_NOTREADY)		return;
	else if(isOK==R_RESTORED) {

	} 


	g_vmesh_mgr.Frame();

	g_dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	
	if( g_bDrawGrid ) {
		draw_grid(RGetDevice(),rvector(-200, 0 , 200),10.f,10.f,40,40, 0x228f8f8f );
		draw_center(RGetDevice(),400.f,400.f,0xff88ff88);
	}

	float this_time = GetTimefloat();

	rmatrix axis_mat;
	D3DXMatrixIdentity(&axis_mat);

	rmatrix _mat = axis_mat;
	rmatrix _tposmat;
	rmatrix _trotmat;
	D3DXMatrixIdentity(&_tposmat);
	D3DXMatrixIdentity(&_trotmat);

//	D3DXMatrixTranslation(&_tposmat,0,10,0);
//	D3DXMatrixRotationX(&_trotmat,-3.14/2.f);
//	D3DXMatrixRotationY(&_trotmat,-3.14/2.f);
//	D3DXMatrixRotationZ(&_trotmat,-3.14/4.f);
//	_mat = _trotmat*_tposmat;

	rvector _mdir = rvector(0,0,1);
	rvector _mup  = rvector(0,1,0);
	rvector _mpos = rvector(0,0,0);

	_mat = _makematrix(_mdir,_mup ,_mpos);
//	_mat = _makealignmatrix(0,_mat);

	rvector _dir_vec = rvector(0.0f,0.f,-1.0f);

	rmatrix _dir_rot_y = RGetRotY(90);
	rmatrix _dir_rot_x = RGetRotY(45);

	Normalize(_dir_vec);

	static float _fdir = 0.0f;

	_fdir += 0.01f;

	if(_fdir > 1.f) {
		_fdir = 0.f;
	}

	_mat = _blendmatrix(_mat,_dir_vec,_fdir);

//	draw_matrix(RGetDevice(),_mat,30.f);

	// 두벡터사이의  축과 각으로 나타내기	-> 비중도 둘 수 있다~
		//	-> 이건 쿼터니언으로 바꿀수있고 
		//	-> 메트릭스로 바꿀수있다..  -> 바로 더해줄 메트릭스 나옴
		//	-> 이걸 x,y,z 축에 대한회전으로도 바꿀수 있는가?

	// 두벡터의 cross 벡터가 축이고~~
	// 두벡터의 dot 가 각값~
	// 캐릭터 방향을 카메라 방향에 맞추려면?

/*

	D3DXVECTOR3 _dir[3];
	_dir[0] = D3DXVECTOR3(0,0,1);
	_dir[1] = D3DXVECTOR3(0,1,0);

	D3DXVECTOR3 _up[3];
	_up[0] = D3DXVECTOR3(0,1,0);
	_up[1] = D3DXVECTOR3(-1,0,0);

	D3DXVec3Lerp(&_dir[2],&_dir[0],&_dir[1],this_time);
	D3DXVec3Lerp(&_up[2],&_up[0],&_up[1],this_time);

	D3DXVec3Normalize(&_dir[2],&_dir[2]);
	D3DXVec3Normalize(&_up[2],&_up[2]);

	rmatrix mm = makematrix(rvector(0,0,0),_dir[2],_up[2]);

	draw_matrix(RGetDevice(),mm,20.f);


	D3DXQUATERNION q;

	D3DXVECTOR3 __right;
	D3DXVECTOR3 __up;
	D3DXVECTOR3 __dir;

	rmatrix mq;

	D3DXQuaternionRotationMatrix(&q,&mm);

	GetQuatDir(q,__right,__up,__dir);

	D3DXMatrixRotationQuaternion(&mq,&q);
*/


	//////////////////////////////////////////////////////////

	g_dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {
/*
		if(SetPosMat(pMesh)) {
			g_matWorld = g_ms * g_matWorld;
			g_matWorld = g_mt * g_matWorld;
		}
*/
		//카메라 공간 inverse 구하기

		rmatrix matInvView = RView;

		matInvView._41 = 0; 
		matInvView._42 = 0; 
		matInvView._43 = 0; // position 무시 

		
		D3DXMatrixTranspose( &matInvView, &matInvView ); // scale 없으므로 그냥 transpose 해줘도 됨.
//		float f;
//		D3DXMatrixInverse( &matInvView,&f, &matInvView ); // scale 없으므로 그냥 transpose 해줘도 됨.

		rmatrix matLightDir;
		rvector lightvec;

		memset( &matLightDir, 0, sizeof(rmatrix) );

		D3DXVec3Normalize(&lightvec,&-g_light_pos);

//		lightvec = g_light_pos;
		matLightDir._11 = -0.5 * lightvec.x;
		matLightDir._21 = -0.5 * lightvec.y;
		matLightDir._31 = -0.5 * lightvec.z;

		matLightDir._12 = -0.5 * lightvec.x;
		matLightDir._22 = -0.5 * lightvec.y;
		matLightDir._32 = -0.5 * lightvec.z;

		matLightDir._41 = 0.5f;				// -0.5 곱하고 0.5 더하기. 
		matLightDir._42 = 0.5f;				// -0.5 곱하고 0.5 더하기. 
		matLightDir._44 = 1.00f;

		D3DXMatrixMultiply( &matLightDir, &matInvView, &matLightDir );

		pVMesh->m_bToonLighting			= !g_CartoonLightOnOff;
		pVMesh->m_bToonTextureRender	= g_TextureRenderOnOff;
//		pVMesh->m_bToonColor			= 0xffff00ff;
		pVMesh->m_ToonUVMat				= matLightDir;
		pVMesh->m_ToonTexture			= g_toon_tex;

		SetLight(pVMesh,0,g_light_pos);
//		SetLight(pVMesh,1,g_camera.GetPos());
//		SetLight(pVMesh,2,rvector(0,150,200));

		if(g_effect_model) {
				pVMesh->m_pMesh->m_LitVertexModel = true;
		}
		else {
			if(!pVMesh->m_pMesh->m_bEffectSort)//이펙트모델은 그냥 놔둔다.
				pVMesh->m_pMesh->m_LitVertexModel = false;
		}
/*
		rvector cn = rvector( g_map_obj_pos.x , g_map_obj_pos.y + 160.f , g_map_obj_pos.z );//캐릭터 눈높이 위치
		rvector vn = g_light_pos-cn;
		Normalize( vn );

		pVMesh->SetRotXYZ(vn);
*/
		pVMesh->ChangeChestCloth(1.f,1);
		pVMesh->SetClothForce(rvector( rand()%5 ,-90.f, rand()%5 ));	//cloth
		pVMesh->SetClothValue(true,0.f);								//cloth

		pVMesh->SetScale( rvector( g_scale[0] , g_scale[1] , g_scale[2] ) );
		pVMesh->SetWorldMatrix( g_matWorld );
		pVMesh->Render();

//		rvector pos;
//		pVMesh->GetWeaponDummyPos(weapon_dummy_muzzle_flash,&pos,false);

//		rmatrix m;
//		D3DXMatrixIdentity(&m);

//		RGetDevice()->SetTransform( D3DTS_WORLD, &m );
//		RDrawLine(pos,rvector(0,0,0),0xffff0000);

/*
		// tool 에서 upper blend 는 없다..
		if( pVMesh->GetFrameInfo( ani_mode_upper )->m_pAniSet ) {

		}
*/
		if(g_bDrawBBox)
			draw_box(&g_matWorld,pVMesh->m_vBMax,pVMesh->m_vBMin,0xffffffff);
	}


	//-------------------------------------------------------------------
/*
*/
	//--------------------------------------------------------------------

	rmatrix tm;

	D3DXMatrixTranslation(&tm,g_light_pos.x,g_light_pos.y,g_light_pos.z);
	
	if(g_bDrawBBox)
		draw_box(&tm,rvector(5,5,5),rvector(-5,-5,-5),0xffffffff);

	rvector vOut;
/*
	rvector v1 = rvector( 0.f, 0.f, 0.f);
	rvector v2 = rvector( 5.f, 5.f, 0.f);
	rvector v3 = rvector(10.f, 5.f, 0.f);
	rvector v4 = rvector(15.f, 0.f, 0.f);
*/

/*
	rvector v1 = rvector(  0.f, 0.f,  0.f);
	rvector v2 = rvector( 10.f, 0.f, 10.f);
	rvector v3 = rvector( 20.f, 0.f, 10.f);
	rvector v4 = rvector( 30.f, 0.f,  0.f);

	float s = 0.9f;

//	D3DXVec3Hermite()

	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	static RLVertex t_vert[1000];

	int _size = 400;
	DWORD _color = 0xffff0000;
	
	for(int i=0;i<_size;i++) {

		s = i/(float)_size;
		D3DXVec3CatmullRom(&vOut,&v1,&v2,&v3,&v4,s);

		t_vert[i].p = vOut;
		t_vert[i].color = _color;
	}

	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF( RLVertexType );
	RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP, _size-1, (LPVOID) t_vert, sizeof(RLVertex));
*/

	RFlip();
	//ChangeAnimationInfoString();
}

/////////////////////////////////////////////////////////////////////////////////////


void CMcvView::OnDestroy() 
{
	CFormView::OnDestroy();
	Finish();
}

BOOL CMcvView::PreTranslateMessage(MSG* pMsg) 
{
	g_ArcBall[g_ArcMode].HandleMouseMessages( pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam );

	if( pMsg->message == WM_KEYDOWN ) {//키입력 무시
		return TRUE;
//		SetFocus();

//		CWnd* pWnd = GetDlgItem( IDC_PIC );
//		if(pWnd)
//			pWnd->SetFocus();
	}

	if( pMsg->message == WM_MOUSEWHEEL ) {

		short zDelta = (short)HIWORD(pMsg->wParam);
		g_Wheel += zDelta;
	}

	return CFormView::PreTranslateMessage(pMsg);
}


void CMcvView::OnClose() 
{
	CFormView::OnClose();

	RCloseDisplay();

	PostQuitMessage(0);
}

int CMcvView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CMcvView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

////////////////////////////////////////////////////////////////////////////////
//
// + 에니메이션 상태별 프레임 기술파일 로딩과 관리
// + 장착할 모델 로딩 + 관리

/*
	에니상태파일은
	라인읽기 : 첫라인은 

	ani_filename = ;
	{
		idle begin_frame end_frame
		walk begin_frame end_frame
	}
*/

void CMcvView::ChangePartsListBox()
{
	RMeshPartsType eq_parts = (RMeshPartsType)((int)eq_parts_head + m_nSelectPartsType);

	int cnt = m_PartsListbox.GetCount();

	// clear listbox

	for(int i=0;i<cnt;i++) {
		m_PartsListbox.DeleteString(0);
	}

	m_PartsListbox.AddString("base_parts");

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {

		if(pVMesh->m_pMesh->m_parts_mgr) {
		
		int size = pVMesh->m_pMesh->m_parts_mgr->m_list.size();

		if(	size ) {

			RMeshMgr* pMeshMgr = pVMesh->m_pMesh->m_parts_mgr;
			RMesh* pMesh = NULL;

			for(int j=0;j<size;j++) {

				pMesh = pMeshMgr->GetFast(j);

				int nodesize = pMesh->m_data_num;

				for(int k=0;k<nodesize;k++) {

					RMeshNode* pMNode = pMesh->m_data[k];

					if(pMNode) {

						if( pMNode->m_point_color_num ) {//검색
							int point_color = pMNode->m_point_color_num;
							int debug = 0;
						}

						if(pMNode->m_PartsType == eq_parts) {

							m_PartsListbox.AddString(pMNode->GetName());
						}
					}
				}
			}
		}

		}
	}
}

void CMcvView::ChangeWeaponListBox()
{
	// 한손 무기와 양손무기의 종류로 나눈다.
/*
	RWeaponType eq_weapon = (RWeaponType)(m_nSelectWeaponType + 1);

	int cnt = m_WeaponListBox.GetCount();

	// clear listbox

	for(int i=0;i<cnt;i++) {
		m_WeaponListBox.DeleteString(0);
	}

	m_WeaponListBox.AddString("base_parts");

	int size = g_weapon_mesh_mgr.m_list.size();

	if(	size ) {

		RMeshMgr* pMeshMgr = &g_weapon_mesh_mgr;
		RMesh* pMesh = NULL;

		for(int j=0;j<size;j++) {

			pMesh = pMeshMgr->GetFast(j);

			int nodesize = pMesh->m_data_num;

			for(int k=0;k<nodesize;k++) {

				RMeshNode* pMNode = pMesh->m_data[k];

//				if(pMNode) {
//					if(pMNode->m_WeaponType == eq_weapon) {
//						m_WeaponListBox.AddString(pMNode->m_Name);
//					}
//				}

			}
		}
	}
*/
}

void CMcvView::ChangeAnimationListBox()
{
	int cnt = m_AnimationListBox.GetCount();

	// clear listbox

	for(int i=0;i<cnt;i++) {
		m_AnimationListBox.DeleteString(0);
	}

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {

		RAnimationMgr* pAMgr = &pVMesh->m_pMesh->m_ani_mgr;

		int cnt = pAMgr->m_list.size();

		for(int j=0;j<cnt;j++) {
			if(pAMgr->m_node_table[j]) {
				m_AnimationListBox.AddString( pAMgr->m_node_table[j]->GetName() );
			}
		}
	}
}

void CheckEffectFile(RMesh* pMesh)
{
	if(pMesh) {
		if(pMesh->m_bEffectSort) {
			g_bDrawGrid = false;
			SetClearColor(D3DCOLOR_ARGB(255,111,111,111));
		}
	}
}

void CMcvView::LoadXmlFile(char* FileName)
{
	//모델 크기에 따라 카메라 위치와 라이트 위치를 잡아준다..
	g_mesh_mgr.DelAll();
	g_vmesh_mgr.DelAll();

	g_id = g_mesh_mgr.AddXml(FileName);//load 폴더는? xml 이 있는폴더??

	if(g_id == -1) return;

	RMesh* pMesh = g_mesh_mgr.GetFast(g_id);

	// 이펙트 모델이라면 그리드 꺼준다.+ 배경컬러바꿔준다.

	CheckEffectFile(pMesh);

	RVisualMesh* pVMesh = NULL;
	
	int vid = g_vmesh_mgr.Add(pMesh);

	if(vid == -1) return;

	pVMesh = g_vmesh_mgr.GetFast(vid);

	pVMesh->SetCheckViewFrustum(false);//뷰체크를 안한다.

	// 등록 정보에 따라 list box 채우기

	ChangePartsListBox();
	ChangeWeaponListBox();
	ChangeAnimationListBox();
}

void CMcvView::LoadAniFile(char* FileName)
{
	g_ani_mgr.DelAll();

	g_ani_mgr.Add("idle",FileName,0);

	RAnimation* pAni = g_ani_mgr.GetAnimation(0);
	RAnimation* pAni_ = NULL;

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if( pAni && pVMesh ) {
		pVMesh->m_pMesh->SetAnimation(pAni_);//이전것소거..
		pVMesh->SetAnimation(pAni);
	}
}

void CMcvView::LoadFile(char* FileName)
{
	//모델 크기에 따라 카메라 위치와 라이트 위치를 잡아준다..

	g_vmesh_mgr.DelAll();
	g_mesh_mgr.DelAll();

	g_id = g_mesh_mgr.Add(FileName);

	if(g_id == -1) return;

	RMesh* pMesh = g_mesh_mgr.GetFast(g_id);


	if( pMesh )  {
		if( pMesh->m_isPhysiqueMesh )
			pMesh->m_isCharacterMesh = true;// visualmesh 떄문에~ xml 은 캐릭터 식별된다..
	}

	// 이펙트 모델이라면 그리드 꺼준다.+ 배경컬러바꿔준다.

	CheckEffectFile(pMesh);
	
	RVisualMesh* pVMesh = NULL;

	int vid = g_vmesh_mgr.Add(pMesh);

	if(vid == -1) return;

	pVMesh = g_vmesh_mgr.GetFast(vid);

	pVMesh->SetCheckViewFrustum(false);//뷰체크를 안한다.
}

void CMcvView::FileOpen(char* filename)
{
	int len = strlen(filename);

	if(len <= 0) return;

	if(strncmp(&filename[len-4],".ani",4)==0) {

		if(g_bframeinfo_dlg) {
			g_bframeinfo_dlg = false;
			OnAnimationinfo();
		}

		LoadAniFile(filename);
	}
	else if(strncmp(&filename[len-4],".elu",4)==0) {

		strcpy( g_last_open_elu_file , filename );

		LoadFile(filename);

		if(g_bmtrl_dlg) {// 보이고 있는 상태라면 갱신이 필요
			g_bmtrl_dlg = false;
			OnMtrleditdlg();
		}
	}
	else if(strncmp(&filename[len-4],".xml",4)==0) {

		g_last_open_elu_file[0] = 0;

		LoadXmlFile(filename);

		if(g_bmtrl_dlg) {// 보이고 있는 상태라면 갱신이 필요
			g_bmtrl_dlg = false;
			OnMtrleditdlg();
		}
	}
	else if( (strncmp(&filename[len-4],".bmp",4)==0 ) || 
			 (strncmp(&filename[len-4],".png",4)==0 ) || 
			 (strncmp(&filename[len-4],".tga",4)==0 ) ||
			 (strncmp(&filename[len-4],".jpg",4)==0 ) ||
			 (strncmp(&filename[len-4],".dds",4)==0 ) ) {
		{
			CreateToonTextures(filename);
			return;
		}

	}
	else {
		::MessageBox(NULL,"알수없는 확장자를 가진 파일입니다~","info",MB_OK);
		return;
	}

	//ChangeAnimationInfoString();
	m_pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_ANIMATION);
}

void CMcvView::OnFileOpen()
{
	CString t_str;
	char	t_filename[256];

	CFileDialog dlg( TRUE,_T("elu"),_T("*.elu"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("Model File (test.elu) |*.elu|Animation File (test.ani)|*.ani|XML File (test.xml)|*.xml|"));

	if( dlg.DoModal()==IDOK ) {

		t_str = dlg.GetPathName();

		lstrcpy((LPSTR)t_filename,(LPSTR)t_str.operator const char*());

		FileOpen(t_filename);
	}
}

// 무기파일 열기 elu 나 xml -> weapon mesh list 와 연결
// weapon file open

void CMcvView::LoadWeaponFile(char* filename)
{
//	g_weapon_mesh_mgr.DelAll();
	g_weapon_mesh_mgr.Add(filename);

	ChangeWeaponListBox();
}

void CMcvView::LoadWeaponXmlFile(char* filename)
{

}

void CMcvView::WFileOpen(char* filename)
{
	int len = strlen(filename);

	if(len <= 0) return;

	if(strncmp(&filename[len-4],".elu",4)==0) {
		LoadWeaponFile(filename);
	}
	else if(strncmp(&filename[len-4],".xml",4)==0) {
		LoadWeaponXmlFile(filename);
	}
	else
		::MessageBox(NULL,"알수없는 확장자를 가진 파일입니다~","info",MB_OK);
}

void CMcvView::OnFileWopen()
{
	CString t_str;
	char	t_filename[256];

	CFileDialog dlg( TRUE,_T("elu"),_T("*.elu"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("Model File (test.elu) |*.elu|XML File (test.xml)|*.xml|"));

	if( dlg.DoModal()==IDOK ) {

		t_str = dlg.GetPathName();

		lstrcpy((LPSTR)t_filename,(LPSTR)t_str.operator const char*());

		WFileOpen(t_filename);
	}
}

void CMcvView::OnDropFiles(HDROP hDropInfo)
{
	char filename[256];

	int cnt = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if(cnt > 1)
		::MessageBox(NULL,"한번에 하나씩~","info",MB_OK);

//	for(int i=0;i<cnt;i++) {
	for(int i=0;i<1;i++) {
		DragQueryFile(hDropInfo,i,filename,sizeof(filename));
	}

	FileOpen(filename);
}

BOOL CMcvView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_nWheel += zDelta;

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMcvView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMcvView::OnTexOnOff()
{
	RMesh::SetTextureRenderOnOff(g_TextureRenderOnOff);
	g_TextureRenderOnOff = !g_TextureRenderOnOff;
}

void CMcvView::OnVertexNormal()
{
	RMesh::SetVertexNormalOnOff(g_VertexNormalOnOff);
	g_VertexNormalOnOff = !g_VertexNormalOnOff;
}

void CMcvView::OnCartoonLightOnoff()
{
	g_CartoonLightOnOff = !g_CartoonLightOnOff;
}

void CMcvView::OnUpdateVertexNormal(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)!g_VertexNormalOnOff);
}

void CMcvView::OnUpdateTexOnOff(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)!g_TextureRenderOnOff);
}

void CMcvView::OnUpdateCartoonLightOnoff(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)!g_CartoonLightOnOff);
}

///////////////////////////////////////////////////////

void CMcvView::OnCbnSelchangePartsType()
{
	m_nSelectPartsType = m_PartsTypeCombo.GetCurSel();

	ChangePartsListBox();
}

void CMcvView::OnCbnSelchangeWeaponType()
{
	m_nSelectWeaponType = m_WeaponTypeCombo.GetCurSel();

	ChangeWeaponListBox();
}
// 장비될 위치 선택
void CMcvView::OnCbnSelchangeWeaponType2()
{
	m_nSelectWeaponType2 = m_WeaponType2Combo.GetCurSel();
}

void CMcvView::OnLbnSelchangePartsList()
{
	int sel = m_PartsListbox.GetCurSel();

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	RMeshPartsType _parts = (RMeshPartsType)((int)eq_parts_head + m_nSelectPartsType);

	CString str;
	m_PartsListbox.GetText(sel,str);

	if(pVMesh) {

	//	base parts

		if(sel==0) {
			pVMesh->SetBaseParts(_parts);
		}
		else {
			pVMesh->SetParts(_parts,(char*)str.operator const char*() );
		}
	}
}

void CMcvView::OnLbnSelchangeWeaponList()
{
	int sel = m_WeaponListBox.GetCurSel();

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	// 장비위치

//	RMeshPartsType _weapon = (RMeshPartsType)((int)eq_parts_left_blade + m_nSelectWeaponType2);
	RWeaponMotionType type = eq_ws_pistol;

	CString str;

	m_WeaponListBox.GetText(sel,str);

	if(pVMesh) {

		//	base weapon

		if(sel==0) {
//			pVMesh->SetBaseParts(_weapon);//weapon 은 따로 관리 될수도..
		}
		else {

			RMesh* pMesh = g_weapon_mesh_mgr.GetFast(0);

//			RMeshNode* pPartsNode = g_weapon_mesh_mgr.GetPartsNode((char*)str.operator const char*());
//			pVMesh->SetParts(_weapon,pPartsNode);//NULL 이라도 관계 없음..
			pVMesh->AddWeapon(type,pMesh);
		}
	}
}

void CMcvView::OnLbnSelchangeAnimationList()
{
	int sel = m_AnimationListBox.GetCurSel();

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh) {
	
		RAnimation* pAni = pVMesh->m_pMesh->m_ani_mgr.m_node_table[sel];

		if( pAni && pVMesh ) {

			pVMesh->SetAnimation(pAni);
		//	ChangeAnimationInfoString();

			if(g_bframeinfo_dlg) {
				g_bframeinfo_dlg = false;
				OnAnimationinfo();
			}
		}
	}
}

void CMcvView::OnBnClickedPlay()
{
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh)
		pVMesh->Play();
}

void CMcvView::OnBnClickedStop()
{
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh)
		pVMesh->Stop();
}

void CMcvView::OnBnClickedPause()
{
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(pVMesh)
		pVMesh->Pause();
}

// 카메라와 라이트 위치값 초기화 

void CMcvView::OnCameraReset()
{
	g_camera.Reset();

	g_light_pos = D3DXVECTOR3(0.f,150.f,-380.f);//캐릭터 바운딩 박스의 중간 앞쪽..
	g_map_obj_pos = D3DXVECTOR3(0,0,0);
}

void CMcvView::OnNMThemeChangedSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

void CMcvView::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int pos = m_SliderAniSpeed.GetPos();

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	float s = pos * 0.1f;

	if(pVMesh)
		pVMesh->SetSpeed(s);//4.8 이 1배속..

	*pResult = 0;
}

void CMcvView::OnScaledlg()
{
	if(!g_bScale) {
		g_sdlg.ShowWindow(SW_SHOW);
	}
	else {
		g_sdlg.ShowWindow(SW_HIDE);
	}

	g_bScale = !g_bScale;
}

void CMcvView::OnModelinfo()
{
	if(!g_bModelInfo_dlg) {

		// 노드 정보와 메터리얼 정보를 담아준다..
		g_model_info_dlg.Begin();		
		g_model_info_dlg.ShowWindow(SW_SHOW);
	}
	else {
		g_model_info_dlg.ShowWindow(SW_HIDE);
	}

	g_bModelInfo_dlg = !g_bModelInfo_dlg;
}

void CMcvView::OnMtrleditdlg()
{
	if(!g_bmtrl_dlg) {
		// 지금 읽은 모델이 있다면..
		// 이전값은 clear..

		g_mtrl_dlg.Begin();

		g_mtrl_dlg.ShowWindow(SW_SHOW);
	}
	else {

		g_mtrl_dlg.ShowWindow(SW_HIDE);
	}

	g_bmtrl_dlg = !g_bmtrl_dlg;
}

void CMcvView::OnAnimationinfo()
{
	if(!g_bframeinfo_dlg) {
		g_frame_info_dlg.Begin();
		g_frame_info_dlg.ShowWindow(SW_SHOW);

	}
	else {
		g_frame_info_dlg.ShowWindow(SW_HIDE);
	}

	g_bframeinfo_dlg = !g_bframeinfo_dlg;
}


//////////////////////////////////////////////////////////////
// mouse event

float GetTimer()//원할때
{
	static DWORD time_back = timeGetTime();

	DWORD this_time = timeGetTime();

	float s = (float)(this_time - time_back)/1000.f;

	time_back = this_time;

	return s;
}

static bool l_drag = false;
static bool r_drag = false;


void CMcvView::OnMouseMove(UINT nFlags, CPoint point)
{
	static CPoint old_pos;

	CPoint pos = point - old_pos;

	float t = GetTimer() * g_camera.m_dist * 0.05f;

	if(l_drag) {// pan
		
		if(pos.x > 0)
			g_camera.Left((float)pos.x * t);
		else 
			g_camera.Right((float)-pos.x * t);

		if(pos.y > 0)
			g_camera.Up((float)pos.y * t);
		else 
			g_camera.Down((float)-pos.y * t);
	}
	else if(r_drag) {//rot

		if(pos.x > 0)
			g_camera.CenterRotationLeft((float)pos.x * t);
		else 
			g_camera.CenterRotationRight((float)-pos.x * t);

		if(pos.y > 0)
			g_camera.CenterRotationUp((float)pos.y * t);
		else 
			g_camera.CenterRotationDown((float)-pos.y * t);

	}

	old_pos = point;

	CFormView::OnMouseMove(nFlags, point);
}

// 마우스캡춰 써야하나?

void CMcvView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	l_drag = true;
	CFormView::OnLButtonDown(nFlags, point);
}

void CMcvView::OnLButtonUp(UINT nFlags, CPoint point)
{
	l_drag = false;

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	rvector v;
	bool g_bPick = false;

	RPickInfo info;

	if(pVMesh&&pVMesh->m_pMesh) {
/*
		pVMesh->m_pMesh->SetPickingType(pick_real_mesh);

		if(pVMesh->Pick ( point.x - g_win_pos_x, point.y - g_win_pos_y, &info ) ) {

//			if(info.pMNode) {
//				m_SelMeshNodeName = info.pMNode->m_Name;
//				UpdateData(FALSE);
//			}

			g_bPick = true;
		}
		else {
			m_SelMeshNodeName = "";
			UpdateData(FALSE);
			g_bPick = false;
		}
*/
	}

	CFormView::OnLButtonUp(nFlags, point);

	ReleaseCapture();
}

void CMcvView::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();

	r_drag = true;
	CFormView::OnRButtonDown(nFlags, point);
}

void CMcvView::OnRButtonUp(UINT nFlags, CPoint point)
{
	r_drag = false;
	CFormView::OnRButtonUp(nFlags, point);
	ReleaseCapture();
}

void CMcvView::OnBkcolor()
{
	CColorDialog dlg;
	COLORREF col;

	BYTE a,r,g,b;

	if( dlg.DoModal()==IDOK ) {
		col = dlg.GetColor();
		
		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		SetClearColor((DWORD)col);
	}
}

void CMcvView::OnMapObjPos()
{
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);
	
	if(pVMesh) {
		g_map_obj_pos = pVMesh->m_vBMin + (pVMesh->m_vBMax - pVMesh->m_vBMin)/2.f;
		g_map_obj_pos.y = pVMesh->m_vBMin.y;
	}
}

void CMcvView::OnGridOnoff()
{
	static DWORD backuptime;

	if( backuptime + 200 < timeGetTime() ) {
		g_bDrawGrid = !g_bDrawGrid;
		backuptime = timeGetTime();
	}	
}

void CMcvView::OnBboxOnoff()
{
	g_bDrawBBox = !g_bDrawBBox;
}

void CMcvView::OnEffectRender()
{
	g_effect_model = !g_effect_model;
}

void CMcvView::OnUpdateBboxOnoff(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)g_bDrawBBox);
}

void CMcvView::OnUpdateGridOnoff(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)g_bDrawGrid);
}

void CMcvView::OnUpdateEffectRender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)g_effect_model);
}

void CMcvView::OnCharViewMode()
{
	g_character_view = !g_character_view;
}

void CMcvView::OnUpdateCharViewMode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((int)g_character_view);
}

void CMcvView::OnPartscolor()
{
	CColorDialog dlg;
	COLORREF col;

	BYTE a,r,g,b;

	if( dlg.DoModal()==IDOK ) {
		col = dlg.GetColor();

		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		// 지금 선택되어있는 파츠 노드를 구한다..xml 이 있을 경우와 아닌경우? visual mesh 에서 구하기..

		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

		if(pVMesh) {

			int sel = m_PartsTypeCombo.GetCurSel();

			RMeshPartsType type = eq_parts_etc;

				 if(sel==0) type = eq_parts_head;
			else if(sel==1) type = eq_parts_face;
			else if(sel==2) type = eq_parts_chest;
			else if(sel==3) type = eq_parts_hands;
			else if(sel==4) type = eq_parts_legs;
			else if(sel==5) type = eq_parts_feet;

			if(pVMesh->m_pTMesh) {
			
			RealSpace2::RMeshNode* pMNode = pVMesh->m_pTMesh[type];

			if(pMNode==NULL)//xml 이 아닌경우...
				pMNode = pVMesh->m_pMesh->GetMeshData(type);

			if(pMNode) {
				pMNode->SetTColor((DWORD)col);
			}

			}
			
		}
		// 장비위치
	}
}


void CMcvView::OnPartscolorall()
{
	CColorDialog dlg;
	COLORREF col;

	BYTE a,r,g,b;

	if( dlg.DoModal()==IDOK ) {
		col = dlg.GetColor();

		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		// 지금 선택되어있는 파츠 노드를 구한다..xml 이 있을 경우와 아닌경우? visual mesh 에서 구하기..

		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

		if(pVMesh) {

			RMeshPartsType type[6];

			type[0] = eq_parts_head;
			type[1] = eq_parts_face;
			type[2] = eq_parts_chest;
			type[3] = eq_parts_hands;
			type[4] = eq_parts_legs;
			type[5] = eq_parts_feet;

			if(pVMesh->m_pMesh) {

				RealSpace2::RMeshNode* pMNode = NULL;

				for(int i=0;i<pVMesh->m_pMesh->m_data_num;i++) {
					pMNode = pVMesh->m_pMesh->m_data[i];
//					if(!pMNode->m_isDummy&&pMNode->m_isDummyMesh&&pMNode->m_isb)
						pMNode->SetTColor((DWORD)col);
				}
/*
				for(int i=0;i<6;i++) {
				
					pMNode = pVMesh->m_pTMesh[type[i]];

					if(pMNode==NULL)
						pMNode = pVMesh->m_pMesh->GetMeshData(type[i]);

					if(pMNode) {
						pMNode->SetTColor((DWORD)col);
					}

				}
*/
			}

		}
		// 장비위치
	}
}

void CMcvView::OnBlendColor()
{
	CColorDialog dlg;
	COLORREF col;
	D3DCOLORVALUE color;
	BYTE a,r,g,b;

	if( dlg.DoModal()==IDOK ) {
		col = dlg.GetColor();

		a = (col>>24)&0xff;
		b = (col>>16)&0xff;
		g = (col>> 8)&0xff;
		r = (col    )&0xff;

		col = D3DCOLOR_ARGB(255,r,g,b);

		color.r = r/255.f;
		color.g = g/255.f;
		color.b = b/255.f;
		color.a = 1.f;

		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

		if(pVMesh) {

			pVMesh->SetNPCBlendColor(color);
		}
	}	
}

void CMcvView::UpdateAnimationSlider()
{
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ANIMATION);
	if(!pSlider) return;
	
	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);
	if(pVMesh)
	{
		AniFrameInfo* animInfo = pVMesh->GetFrameInfo(ani_mode_lower);
		int maxRange = pVMesh->GetMaxFrame(ani_mode_lower);
		if(animInfo)
		{
			pSlider->SetRange(0, maxRange);
			pSlider->SetPos(animInfo->m_nFrame);
		}
	}
}

void CMcvView::ChangeAnimationInfoString()
{

	RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);

	if(!pVMesh) return;

	RAnimation* pAniSet = pVMesh->GetFrameInfo( ani_mode_lower )->m_pAniSet;

	char strTemp[1024];

	if( pAniSet ) {
		if( pAniSet->m_pAniData ) {
			
			DWORD _ms  = pAniSet->m_pAniData->m_max_frame / 4.8f;
			float _sec = _ms / 1000.f;
			int currentFrame = pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame;

			sprintf(strTemp,"(%d  / %d frame) ", currentFrame, pAniSet->m_pAniData->m_max_frame);
		 	m_CurrentFrame.SetWindowText(strTemp);
			UpdateData(FALSE);
		}
	}
}


void CMcvView::OnStnClickedCurrentFrame()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}



void CMcvView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_ANIMATION);
	if((void*)pScrollBar==(void*)pSlider)
	{
			// 애니메이션 프레임 업데이트
		RVisualMesh* pVMesh = g_vmesh_mgr.GetFast(0);
		if(pVMesh)
		{
			AniFrameInfo* animInfo = pVMesh->GetFrameInfo(ani_mode_lower);
			if(animInfo)
			{
				pVMesh->Pause();
				animInfo->m_nFrame = pSlider->GetPos();
			}
		}
	}
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}
