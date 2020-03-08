//
//	24Bit Image Class
//
//											written by lee jang ho
//
//////////////////////////////////////////////////////////////////////
#include "Image24.h"
#include "CMError.h"

CImage24::CImage24()
{
	m_pBitmapData=NULL;
}

CImage24::~CImage24()
{
	Close();
}

#define BYTE_PER_PIXEL	3

BOOL CImage24::Open(int nWidth,int nHeight)
{
	m_nWidth =nWidth;
	m_nHeight=nHeight;

	m_pBitmapData=new BYTE[m_nWidth*m_nHeight*BYTE_PER_PIXEL];
	if(m_pBitmapData==NULL){
//		SetError(CMERR_OUT_OF_MEMORY);
		return FALSE;
	}
	return TRUE;
}

BOOL CImage24::Open(CDib *pDib)
{
	m_nWidth =(WORD)pDib->GetWidth();
	m_nHeight=(WORD)pDib->GetHeight();

	m_pBitmapData=new BYTE[m_nWidth*m_nHeight*BYTE_PER_PIXEL];
	if(m_pBitmapData==NULL){
		//_RPT0(_CRT_WARN,"Memory Allocation Error");
//		SetError(CMERR_OUT_OF_MEMORY);
		return FALSE;
	}

	// 위에서 아래로 찍히는 비트맵인가? 아닌가?
	// 여기서 4바이트 정렬의 가비지를 제거함
	if(pDib->IsTopDown()==TRUE){
		memcpy(m_pBitmapData,pDib->GetData(),m_nWidth*m_nHeight*BYTE_PER_PIXEL);
	}
	else{
		for(int y=0;y<m_nHeight;y++){
			int nAlign=(m_nWidth*BYTE_PER_PIXEL)%4;
			if(nAlign!=0)nAlign=4-nAlign;
			memcpy((m_pBitmapData+(m_nHeight-1)*(m_nWidth*BYTE_PER_PIXEL))-y*m_nWidth*BYTE_PER_PIXEL,
				pDib->GetData()+y*(m_nWidth*BYTE_PER_PIXEL+nAlign),
				m_nWidth*BYTE_PER_PIXEL);
		}
	}

	return TRUE;
}

void CImage24::Close(void)
{
	if(m_pBitmapData!=NULL){
		delete[] m_pBitmapData;
		m_pBitmapData=NULL;
	}
}

BOOL CImage24::IsOpen(void)
{
	if(m_pBitmapData!=NULL)
		return TRUE;
	else return FALSE;
}

LPBYTE CImage24::GetData(void)
{
	return m_pBitmapData;
}

// 데이타 블럭 크기 얻기
int CImage24::GetDataSize(void)
{
	return (m_nWidth*m_nHeight*BYTE_PER_PIXEL);
}

void CImage24::BitBlt(LPBYTE pDst,WORD nDstX,WORD nDstY,WORD nDstWidth,WORD nSrcX,WORD nSrcY,WORD nSrcWidth,WORD nSrcHeight)
{
	// 찍을 것이 없으면...
	if(nSrcWidth<=0 || nSrcHeight<=0)return;

	// Destination Address
	DWORD pDestAddress=(DWORD)pDst+nDstX*BYTE_PER_PIXEL+nDstY*nDstWidth*BYTE_PER_PIXEL;
	// Source Address
	DWORD pSrcAddress=(DWORD)m_pBitmapData+nSrcX*BYTE_PER_PIXEL+nSrcY*m_nWidth*BYTE_PER_PIXEL;
	// 가로를 찍고 건너띄어야 할값
	DWORD nDestGap=(nDstWidth-nSrcWidth)*BYTE_PER_PIXEL;
	// 가로를 찍고 소스 이미지에서 건너띄어야 할값
	DWORD nSrcGap=(m_nWidth-nSrcWidth)*BYTE_PER_PIXEL;
	// 4바이트씩 찍을때 찍어야 할 횟수
	DWORD nWidthCount=(nSrcWidth*BYTE_PER_PIXEL)/4;
	// 4바이트씩 찍을때 4로 나눠 떨어지는지?
	DWORD nLeft=(nSrcWidth*BYTE_PER_PIXEL)%4;

	DWORD dwSrcHeight=(DWORD)nSrcHeight;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress
		//mov ecx, DWORD PTR nSrcHeight
		mov ecx, dwSrcHeight
LOOP1:
		push ecx
		mov ecx, nLeft
		rep movsb

		mov ecx, nWidthCount
		rep movsd
		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOP1
	}
}

// BitBlt with Pitch width(byte value)
void CImage24::BitBltwp(LPBYTE pDst,WORD nDstX,WORD nDstY,WORD nPitchByte,WORD nSrcX,WORD nSrcY,WORD nSrcWidth,WORD nSrcHeight)
{
	// 찍을 것이 없으면...
	if(nSrcWidth<=0 || nSrcHeight<=0)return;

	// Destination Address
	DWORD pDestAddress=(DWORD)pDst+nDstX*BYTE_PER_PIXEL+nDstY*nPitchByte;
	// Source Address
	DWORD pSrcAddress=(DWORD)m_pBitmapData+nSrcX*BYTE_PER_PIXEL+nSrcY*m_nWidth*BYTE_PER_PIXEL;
	// 가로를 찍고 건너띄어야 할값
	DWORD nDestGap=nPitchByte-nSrcWidth*BYTE_PER_PIXEL;
	// 가로를 찍고 소스 이미지에서 건너띄어야 할값
	DWORD nSrcGap=(m_nWidth-nSrcWidth)*BYTE_PER_PIXEL;
	// 4바이트씩 찍을때 찍어야 할 횟수
	DWORD nWidthCount=(nSrcWidth*BYTE_PER_PIXEL)/4;
	// 4바이트씩 찍을때 4로 나눠 떨어지는지?
	DWORD nLeft=(nSrcWidth*BYTE_PER_PIXEL)%4;

	DWORD dwSrcHeight=(DWORD)nSrcHeight;

	__asm{
		mov edi, pDestAddress
		mov esi, pSrcAddress
		//mov ecx, DWORD PTR nSrcHeight
		mov ecx, dwSrcHeight
LOOP2:
		push ecx
		mov ecx, nLeft
		rep movsb

		mov ecx, nWidthCount
		rep movsd
		add edi, nDestGap
		add esi, nSrcGap
		pop ecx
		dec ecx
		jnz LOOP2
	}
}

WORD CImage24::GetWidth(void)
{
	return m_nWidth;
}

WORD CImage24::GetHeight(void)
{
	return m_nHeight;
}

WORD CImage24::GetPitch(void)
{
	return m_nWidth*BYTE_PER_PIXEL;
}
/*
void CImage24::Copy(WORD nX,WORD nY,MImage *pSrcImage,WORD nSrcX,WORD nSrcY,WORD nSrcWidth,WORD nSrcHeight)
{
}
*/
