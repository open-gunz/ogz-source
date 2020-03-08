// WorldEditView.cpp : implementation of the CWorldEditView class
//

#include "stdafx.h"
#include "WorldEdit.h"
#include "WorldEditDoc.h"
#include "WorldEditView.h"
#include "MainFrm.h"
#include "RealSpace2.h"
#include "RBspObject.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include <algorithm>
#include "defer.h"
#include "MTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

_USING_NAMESPACE_REALSPACE2

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView

IMPLEMENT_DYNCREATE(CWorldEditView, CView)

BEGIN_MESSAGE_MAP(CWorldEditView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_RUNTIMEBOUNDINGBOX, OnRuntimeboundingbox)
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_UPDATE_COMMAND_UI(ID_BOUNDINGBOX, OnUpdateBoundingbox)
	ON_COMMAND(ID_BOUNDINGBOX, OnBoundingbox)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_OCCLUSION, OnOcclusion)
	ON_UPDATE_COMMAND_UI(ID_OCCLUSION, OnUpdateOcclusion)
	ON_COMMAND(ID_SHOWLIGHTMAP, OnShowlightmap)
	ON_UPDATE_COMMAND_UI(ID_SHOWLIGHTMAP, OnUpdateShowlightmap)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView construction/destruction

CWorldEditView::~CWorldEditView()
{
	RCloseDisplay();
}

BOOL CWorldEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

#define DEFAULTSIZE	100.f

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView drawing


static rvector g_LastPickPos;

#include "ProgressDialog.h"
#include ".\worldeditview.h"

static void DoMovement(float DeltaTime)
{
	auto GetKey = [&](auto ch) {
		return (GetAsyncKeyState(ch) & 0x8000) != 0;
	};

	auto Forward = Normalized(RCameraDirection);
	auto Right = Normalized(CrossProduct(Forward, { 0, 0, -1 }));

	v3 dir{ 0, 0, 0 };

	if (GetKey('W'))
		dir += Forward;
	if (GetKey('A'))
		dir += -Right;
	if (GetKey('S'))
		dir += -Forward;
	if (GetKey('D'))
		dir += Right;

	Normalize(dir);

	RCameraPosition += dir * 2000 * DeltaTime;

	RUpdateCamera();
}

void CWorldEditView::OnDraw(CDC* pDC)
{
	CWorldEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	auto Time = GetGlobalTimeMS();
	DEFER(LastTime = Time;);

	if (true)
		DoMovement(min((Time - LastTime) / 1000.0f, 1.0f));

	rmatrix id;
	D3DXMatrixIdentity(&id);
	RGetDevice()->SetTransform(D3DTS_WORLD, &id);

	if (!g_bProgress && pDoc->m_pBspObject)
	{
		if (m_bWireframe) {
			RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		}
		else {
			RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		}

		pDoc->m_pBspObject->SetWireframeMode(m_bWireframe);
		pDoc->m_pBspObject->SetShowLightmapMode(m_bShowLightmap);

		if (m_EditMode != EDITMODE_PATH)
			pDoc->m_pBspObject->Draw();
		if (m_bDrawBoundingBox)
			pDoc->m_pBspObject->DrawBoundingBox();
		if (m_bDrawOcclusion)
			pDoc->m_pBspObject->DrawOcclusions();

		if (m_EditMode == EDITMODE_PATH)
		{
			auto& pBsp = pDoc->m_pBspObject;

			pBsp->Draw();

			RGetDevice()->SetTexture(0, NULL);
			RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
			RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

			RBSPPICKINFO *ppi = &GetDocument()->m_LastPicked;
			if (GetDocument()->m_bLastPicked)
			{
				ppi->pNode->DrawWireFrame(ppi->nIndex, 0xffffffff);
				ppi->pNode->DrawBoundingBox(0xffff0000);

				POINT p;
				GetCursorPos(&p);
				ScreenToClient(&p);

				rvector pos, dir, to;
				RGetScreenLine(p.x, p.y, &pos, &dir);
				to = pos + dir;

				rvector worldpos;
				D3DXPlaneIntersectLine(&worldpos, &ppi->pInfo->plane, &pos, &to);

				rvector normal;
				pBsp->GetNormal(ppi->pInfo->nConvexPolygon, worldpos, &normal);
				RDrawLine(worldpos, worldpos + normal * 100, 0xffff0000);

				RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);

				int nS = ppi->pInfo->nConvexPolygon;
				pBsp->DrawNormal(nS, 100);

			}
		}
	}

	RFlip();

	Sleep(0);
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView diagnostics

#ifdef _DEBUG
void CWorldEditView::AssertValid() const
{
	CView::AssertValid();
}

void CWorldEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWorldEditDoc* CWorldEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWorldEditDoc)));
	return (CWorldEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldEditView message handlers

BOOL CWorldEditView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	BOOL ret = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ModifyStyleEx(0, WS_EX_ACCEPTFILES, 0);

	if (ret)
	{
		RMODEPARAMS mparams = { 1024,768,FullscreenType::Windowed,D3DFMT_R5G6B5 };
		if (!RInitDisplay(m_hWnd, nullptr, &mparams, GraphicsAPI::D3D9))
		{
			AfxMessageBox("Cannot Initialize 3D Engine.");
			return false;
		}
		RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);
	}

	return ret;
}

BOOL CWorldEditView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CWorldEditView::Resize(CSize size)
{
	RMODEPARAMS mparams = { size.cx, size.cy, FullscreenType::Windowed, D3DFMT_R5G6B5 };
	auto& pbsp = GetDocument()->m_pBspObject;
	if (pbsp) pbsp->OnInvalidate();
	RResetDevice(&mparams);
	if (pbsp) pbsp->OnRestore();
}

void CWorldEditView::GetWorldCoordinate(rvector *ret, CPoint pt)
{
	*ret = RGetIntersection(pt.x, pt.y, rplane(0, 0, 1, 0));
}

void CWorldEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);

	m_bLastShiftState = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	m_bLastAltState = (GetKeyState(VK_MENU) & 0x8000) != 0;
	m_LastMatrix = RViewProjectionViewport;
	m_LastCameraPosition = RCameraPosition;
	m_LastCursorPosition = point;
	LastFrameCursorPosition = point;

	if (m_bLastShiftState)
	{
		GetWorldCoordinate(&m_LastWorldPosition, point);
	}

	if (m_bLastAltState)
	{
		GetWorldCoordinate(&m_LastWorldPosition, CPoint(RGetScreenWidth() / 2, RGetScreenHeight() / 2));
	}

	if (m_EditMode == EDITMODE_PATH)
	{
		POINT p = point;

		rvector pos, dir;
		RGetScreenLine(p.x, p.y, &pos, &dir);
		GetDocument()->m_bLastPicked =
			GetDocument()->m_pBspObject->PickOcTree(pos, dir, &GetDocument()->m_LastPicked, RM_FLAG_ADDITIVE | RM_FLAG_HIDE);
	}

}

void CWorldEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CView::OnLButtonUp(nFlags, point);

	// pick test
	RBSPPICKINFO bpi;
	if (GetDocument()->m_pBspObject)
	{
		rvector pos, dir;
		RGetScreenLine(point.x, point.y, &pos, &dir);
		if (GetDocument()->m_pBspObject->Pick(pos, dir, &bpi))
		{
			g_LastPickPos = bpi.PickPos;
		}
	}
}

void CWorldEditView::OnResetCamera()
{
	CWorldEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	rboundingbox *pbb, defaultbb;
	defaultbb.vmin = { 0, 0, 0 };
	defaultbb.vmax = { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };

	if (!pDoc->m_pBspObject || !pDoc->m_pBspObject->GetRootNode())
		pbb = &defaultbb;
	else
		pbb = &pDoc->m_pBspObject->GetRootNode()->bbTree;

	auto size = pbb->vmax - pbb->vmin;

	rvector targetpos = .5f*(pbb->vmax + pbb->vmin);
	targetpos.z = 0;
	rvector sourcepos = targetpos + rvector(0, 100, 100);
	RSetCamera(sourcepos, targetpos, rvector(0, 0, 1));
	RSetProjection(1.f / 3.f * PI_FLOAT, 100, 55000);

	Invalidate();
}

void CWorldEditView::OnMouseMove(UINT nFlags, CPoint point)
{
	CView::OnMouseMove(nFlags, point);

	DEFER(LastFrameCursorPosition = point;);

	if ((MK_LBUTTON & nFlags) != 0)
	{
		if (m_bLastShiftState)
		{
			rvector scrpoint = rvector((float)point.x, (float)point.y, 0.1f);

			rmatrix inv;
			float det;
			D3DXMatrixInverse(&inv, &det, &m_LastMatrix);

			rvector worldpoint;
			D3DXVec3TransformCoord(&worldpoint, &scrpoint, &inv);

			rplane plane = rplane(0, 0, 1, 0);

			rvector intpointdst;
			D3DXPlaneIntersectLine(&intpointdst, &plane, &worldpoint, &m_LastCameraPosition);

			rmatrix cameratm = RView;

			D3DXMatrixTranslation(&cameratm,
				-m_LastCameraPosition.x,
				-m_LastCameraPosition.y,
				-m_LastCameraPosition.z);
			cameratm = cameratm * m_LastMatrix;

			D3DXMatrixInverse(&inv, &det, &cameratm);

			rvector screenpoint;
			D3DXVec3TransformCoord(&screenpoint, &intpointdst, &m_LastMatrix);
			D3DXVec3TransformCoord(&screenpoint, &screenpoint, &inv);

			RCameraPosition = m_LastCameraPosition * 2 - (screenpoint - m_LastWorldPosition);
			RUpdateCamera();
		}
		else if (m_bLastAltState)
		{
			CPoint Diff = point - m_LastCursorPosition;

			rvector relpos = m_LastCameraPosition - m_LastWorldPosition;
			float length = D3DXVec3Length(&relpos);
			Normalize(relpos);

			float anglex, anglez;
			anglex = acos(relpos.z);
			anglez = asin(relpos.x / sin(anglex));
			if (relpos.y < 0)
				anglez = PI_FLOAT - anglez;

			anglex += -0.01f*Diff.y;
			anglex = min(max(anglex, 0.001f), PI_FLOAT - 0.001f);
			anglez += -0.01f*Diff.x;

			relpos = length*rvector(sin(anglez)*sin(anglex), cos(anglez)*sin(anglex), cos(anglex));

			rvector newcamerapos = m_LastWorldPosition + relpos;
			RSetCamera(newcamerapos, m_LastWorldPosition, rvector(0, 0, 1));
		}
		else // No special keys pressed
		{
			CPoint Diff = point - LastFrameCursorPosition;

			auto dir = Normalized(RCameraDirection);

			float anglex{}, anglez{};

			{
				rvector a_dir = dir;

				float fAngleX = 0.0f, fAngleZ = 0.0f;

				fAngleX = acosf(a_dir.z);
				float fSinX = sinf(fAngleX);

				if (fSinX == 0) fAngleZ = 0.0f;
				else
				{
					float fT = (a_dir.x / fSinX);
					if (fT > 1.0f) fT = 1.0f;
					else if (fT < -1.0f) fT = -1.0f;

					float fZ1 = acosf(fT);

					if (IS_EQ((sinf(fZ1) * fSinX), dir.y))
					{
						fAngleZ = fZ1;
					}
					else
					{
						fAngleZ = 2 * PI_FLOAT - fZ1;
					}
				}

				anglex = fAngleX;
				anglez = fAngleZ;
			}

			/*auto anglex = acos(dir.z);
			auto anglez = asin(dir.x / sin(anglex));
			if (dir.y < 0)
				anglez = PI_FLOAT - anglez;*/

			auto clamp = [&](auto&& val, auto&& low, auto&& high) {
				return max(min(val, high), low);
			};

			anglex += 0.005f * Diff.y;
			anglex = clamp(anglex, 0.001f, PI_FLOAT - 0.001f);
			anglez += 0.005f * Diff.x;
			anglez = fmod(anglez, 2 * PI_FLOAT);

			v3 vec{
				cos(anglez) * sin(anglex),
				sin(anglez) * sin(anglex),
				cos(anglex) };

			RCameraDirection = vec;
			RUpdateCamera();
		}
	}
}

#define CAMERA_WHEEL_STEP	100.f

BOOL CWorldEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	GetWorldCoordinate(&m_LastWorldPosition, CPoint(RGetScreenWidth() / 2, RGetScreenHeight() / 2));

	rvector dir = m_LastWorldPosition - RCameraPosition;
	Normalize(dir);

	RCameraPosition += dir*CAMERA_WHEEL_STEP*zDelta / (float)WHEEL_DELTA;
	RUpdateCamera();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
void CWorldEditView::OnRuntimeboundingbox()
{
	auto& pbsp = GetDocument()->m_pBspObject;
	if (!pbsp) return;

	pbsp->OptimizeBoundingBox();
}

void CWorldEditView::OnWireframe()
{
	m_bWireframe = !m_bWireframe;
}

void CWorldEditView::OnUpdateWireframe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bWireframe);
}

void CWorldEditView::OnBoundingbox()
{
	m_bDrawBoundingBox = !m_bDrawBoundingBox;
}

void CWorldEditView::OnUpdateBoundingbox(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawBoundingBox);
}

void CWorldEditView::OnOcclusion()
{
	m_bDrawOcclusion = !m_bDrawOcclusion;
}

void CWorldEditView::OnUpdateOcclusion(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bDrawOcclusion);
}

void CWorldEditView::OnShowlightmap()
{
	m_bShowLightmap = !m_bShowLightmap;
}

void CWorldEditView::OnUpdateShowlightmap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLightmap);
}

void CWorldEditView::OnDropFiles(HDROP hDropInfo)
{
	char szFileName[_MAX_PATH];
	DragQueryFile(hDropInfo, 0, szFileName, sizeof(szFileName));

	int nLen;
	while ((nLen = strlen(szFileName)) > 0)
	{
		if (strnicmp(szFileName + nLen - 3, ".rs", 3) == 0)
		{
			AfxGetApp()->OpenDocumentFile(szFileName);
			return;
		}

		char *lastdot = strrchr(szFileName, '.');
		if (!lastdot) return;

		*lastdot = 0;
	}
}
