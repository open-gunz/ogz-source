#include "stdafx.h"
#include "RCloth.h"
#include "MDebug.h"


//////////////////////////////////////////////////////////////////////////
//	생성자
//////////////////////////////////////////////////////////////////////////
RCloth::RCloth(void) : 
m_pX(0), 
m_pOldX(0), 
m_pForce(0), 
m_pConst(0), 
m_pHolds(0), 
m_pWeights(0), 
m_pNormal(0)
{
}


//////////////////////////////////////////////////////////////////////////
//	소멸자
//////////////////////////////////////////////////////////////////////////
RCloth::~RCloth(void)
{
	SAFE_DELETE_ARRAY( m_pX );
	SAFE_DELETE_ARRAY( m_pOldX );
	SAFE_DELETE_ARRAY( m_pForce );
	SAFE_DELETE_ARRAY( m_pConst );
	SAFE_DELETE_ARRAY( m_pHolds );
	SAFE_DELETE_ARRAY( m_pWeights );
	SAFE_DELETE_ARRAY( m_pNormal );
}

//////////////////////////////////////////////////////////////////////////
// Update - 매 프레임 혹은 일정간격으로 호출
//////////////////////////////////////////////////////////////////////////
void RCloth::update()	
{
	accumulateForces();
	varlet();
	satisfyConstraints();
}

//////////////////////////////////////////////////////////////////////////
// AccumulateForces
//	바람.. 등
//////////////////////////////////////////////////////////////////////////
void RCloth::accumulateForces()
{
}

//////////////////////////////////////////////////////////////////////////
// Varlet Interpolation
//////////////////////////////////////////////////////////////////////////
void RCloth::varlet()
{
	rvector* swapTemp;

	for( int i = 0 ; i < m_nCntP; ++i )
	{
		if( m_pHolds[i] != CLOTH_HOLD )
		{
			m_pOldX[i] = m_pX[i] + m_AccelationRatio * ( m_pX[i] - m_pOldX[i] ) + m_pForce[i] * m_fTimeStep * m_fTimeStep; 
		}
	}

	swapTemp = m_pX;
	m_pX = m_pOldX;
	m_pOldX = swapTemp;

	// 초기화
	memset( m_pForce, 0, sizeof(rvector) * m_nCntP );
}


//////////////////////////////////////////////////////////////////////////
// Satisfy Constraints
//////////////////////////////////////////////////////////////////////////
void RCloth::satisfyConstraints()
{
	sConstraint* c;
	rvector* x1;
	rvector* x2;
	rvector delta;
	float deltaLegth;
	float diff;
	int i, j;

	for( i = 0 ; i < m_nCntIter; ++i )
	{
		// TODO : Do Collision Check Here
		
		float w1, w2;
		for( j = 0 ; j < m_nCntC; ++j )
		{
			c = &m_pConst[j];

			x1 = &m_pX[ c->refA ];
			x2 = &m_pX[ c->refB ];

			w1 = m_pWeights[c->refA];
			w2 = m_pWeights[c->refB];
		
			if( w1 == 0 && w2 == 0 )
			{
				continue;
			}

			delta = *x2 - *x1;
			deltaLegth = Magnitude(delta);
			diff = (float) ( ( deltaLegth - c->restLength ) / (deltaLegth *  (w1 + w2) ));

			*x1 += delta * w1 * diff;
			*x2 -= delta * w2 * diff; 
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Render
//////////////////////////////////////////////////////////////////////////
void RCloth::render()
{
}

//////////////////////////////////////////////////////////////////////////
//	UpdateNormal
//////////////////////////////////////////////////////////////////////////
void RCloth::UpdateNormal()
{
}


//////////////////////////////////////////////////////////////////////////



void RWindGenerator::Update( DWORD time )
{
	_ASSERT( m_Time <= time );
	DWORD	ElapsedTime = m_Time	- time;
	int factor = 0;

	//하드코디드 바람들..
	switch( m_WindType )
	{
	case RANDOM_WIND:		
		if( m_WindPower > 0 )
			factor	= rand() % (int)(m_WindPower);
		m_ResultWind.x = m_WindDirection.x * factor;
		m_ResultWind.y = m_WindDirection.y * factor;
		m_ResultWind.z = 0;
		break;
	case GENTLE_BREEZE_WIND:		// 나뭇잎과 가는 가지가 쉴새없이 흔들리고 깃발이 가볍게 휘날린다
		{
#define PEACE_TIME	1000		//1초
#define POWER_INC		1.0f
			if( m_bFlag )
			{
				if( m_fTemp2 == 0 ) m_fTemp2 = 1;
				if( m_fTemp1 > m_WindPower ) m_fTemp2 = -1;
				m_fTemp1	+= POWER_INC * m_fTemp2;
				m_Time	= time;
				if( m_fTemp1 < 0 ) 
				{
					m_bFlag		= false;
					m_fTemp1		= 0;
					m_fTemp2		= 0;
				}
			}
			else if( ElapsedTime > m_DelayTime )
			{
				m_Time	= time;
				m_bFlag	= true;			// 여기서 m_bFlag는 '바람이 분다'라는 것을 의미한다
			}
		}
		m_ResultWind = m_WindDirection * m_fTemp1;
		break;
	default:
		m_WindDirection	= rvector(0,0,0);
		m_WindPower		= 0.f;
	}
}

