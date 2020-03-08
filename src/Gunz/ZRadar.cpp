#include "stdafx.h"

#include "ZRadar.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZApplication.h"

ZRadar::ZRadar(const char* szName, MWidget* pParent, MListener* pListener)
: ZInterface(szName, pParent, pListener)
{
	m_pBitmap = NULL;
}

ZRadar::~ZRadar()
{

}

void ZRadar::RotatePixel(int* poutX, int* poutY, int sx, int sy, int nHotSpotX, int nHotSpotY, float fAngle)
{
	float fSinq = sinf(fAngle);
	float fCosq = cosf(fAngle);

	int rx = sx - nHotSpotX;
	int ry = sy - nHotSpotY;

	*poutX = (ry * fSinq + rx * fCosq) + nHotSpotX;
	*poutY = (ry * fCosq - rx * fSinq) + nHotSpotY;

}

bool ZRadar::OnCreate()
{
	m_pBitmap = (MBitmapR2*)MBitmapManager::Get("radar01.bmp");
	if(!m_pBitmap) return false;

	float fRot = 0.0f;
	MRECT r = GetRect();
	int nDelta = (GetRect().h /2) - 90;

	m_Nodes[0].x[0] = (r.w - m_pBitmap->GetWidth())  / 2;
	m_Nodes[0].y[0] = (r.h - m_pBitmap->GetHeight())  / 2 - nDelta;

	m_Nodes[0].x[1] = m_Nodes[0].x[0] + m_pBitmap->GetWidth() - 1;
	m_Nodes[0].y[1] = m_Nodes[0].y[0];

	m_Nodes[0].x[2] = m_Nodes[0].x[0];
	m_Nodes[0].y[2] = m_Nodes[0].y[0] + m_pBitmap->GetHeight() - 1;

	m_Nodes[0].x[3] = m_Nodes[0].x[0] + m_pBitmap->GetWidth() - 1;
	m_Nodes[0].y[3] = m_Nodes[0].y[0] + m_pBitmap->GetHeight() - 1;
	m_Nodes[0].bShoted = false;

	for (int i = 1; i < 8; i++)
	{
		m_Nodes[i].bShoted = false;
		fRot = (PI_FLOAT / 4.0f) * i;

		for (int j = 0; j < 4; j++)
		{
            RotatePixel(&m_Nodes[i].x[j], &m_Nodes[i].y[j], m_Nodes[0].x[j], m_Nodes[0].y[j],
				r.w / 2, r.h / 2, fRot);
		}
	}

	return true;
}

void ZRadar::OnDestroy()
{
}


void ZRadar::OnDraw(MDrawContext* pDC)
{
	bool ret = false;
	for (int i = 0; i < 8; i++)
	{
		if (m_Nodes[i].bShoted == true) ret = true;
	}
	if (ret == false) return;


	u32 currentTime = GetGlobalTimeMS();
	for (int i = 0; i < 8; i++)
	{
		if (m_Nodes[i].bShoted)
		{
			if (currentTime - m_Nodes[i].nLastShotTime < 800)
			{
				u32 nTime = currentTime - m_Nodes[i].nLastShotTime;

				if (currentTime % 200 < 120)
				{
					pDC->SetBitmap(m_pBitmap);
					pDC->SetOpacity(0xFF);

					pDC->SetEffect(MDE_ADD);
					pDC->DrawEx(m_Nodes[i].x[0], m_Nodes[i].y[0], 
						m_Nodes[i].x[1], m_Nodes[i].y[1], 
						m_Nodes[i].x[2], m_Nodes[i].y[2], 
						m_Nodes[i].x[3], m_Nodes[i].y[3]);
					pDC->SetEffect(MDE_NORMAL);
				}
			}
			else
			{
				m_Nodes[i].bShoted = false;
			}
		}
	}
}

bool ZRadar::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}


void ZRadar::SetMaxDistance(float fDist)
{
	m_fMaxDistance = fDist;
}

void ZRadar::OnAttack(rvector& pAttackerPos)
{
	rvector my_dir = ZApplication::GetGame()->m_pMyCharacter->m_Direction;
	rvector my_pos = ZApplication::GetGame()->m_pMyCharacter->GetPosition();
	rvector attackPos = pAttackerPos;

//	my_dir.z = 0.0f;
	my_pos.z = 0.0f;
	attackPos.z = 0.0f;

	Normalize(my_dir);

	rvector dir = attackPos - my_pos;
	Normalize(dir);


	rvector vector1 = my_dir, vector2 = dir;
	vector1.y = -vector1.y;
	vector2.y = -vector2.y;
	float cosAng1 = DotProduct(vector1, vector2); 
	
	float r;
	if (-vector1.y*vector2.x + vector1.x*vector2.y > 0.0f)
	{
		r = (float)(acos(cosAng1));
	}
	else
	{
		r = -(float)(acos(cosAng1)); 
	}

	float t = (PI_FLOAT / 4.0f);

	int nIndex = -1;
	if ((r > 0) && (r < t))	nIndex = 1;
	else if ((r > t) && (r < t*2))nIndex = 2;
	else if ((r > t*2) && (r < t*3)) nIndex = 3;
	else if ((r > t*3) && (r < t*4)) nIndex = 4;
	else if ((r < 0) && (r > -t)) nIndex = 8;
	else if ((r < -t) && (r > -t*2)) nIndex = 7;
	else if ((r < -t*2) && (r > -t*3)) nIndex = 6;
	else if ((r < -t*3) && (r > -t*4)) nIndex = 5;





	if (nIndex > 0)
	{
		m_Nodes[nIndex-1].nLastShotTime = GetGlobalTimeMS();
		m_Nodes[nIndex-1].bShoted = true;
	}
}