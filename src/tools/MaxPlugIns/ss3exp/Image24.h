//
//	24Bit Image Class
//
//											written by lee jang ho
//
//////////////////////////////////////////////////////////////////////
#ifndef _IMAGE24_H
#define _IMAGE24_H

#include "Dib.h"

//	24Bit Image Buffer Class
class CImage24
{
private:
	WORD	m_nWidth;			// Width
	WORD	m_nHeight;			// Height
	LPBYTE	m_pBitmapData;		// Buffer Pointer
public:
	CImage24();
	virtual ~CImage24();

	// 지정한 크기로 버퍼만 할당
	BOOL Open(int nWidth,int nHeight);
	// 로드된 이미지를 이 오브젝트에 복사함
	BOOL Open(CDib *pDib);
	// 해제
	void Close();

	// Open 되었는가?
	BOOL IsOpen(void);

	// 내부 버퍼의 포인터 얻기
	LPBYTE GetData(void);
	// 데이타 블럭 크기 얻기
	int GetDataSize(void);
	// BitBlt
	void BitBlt(LPBYTE pDst,WORD nDstX,WORD nDstY,WORD nDstPitch,WORD nSrcX,WORD nSrcY,WORD nSrcWidth,WORD nSrcHeight);
	// BitBlt with Pitch width(byte value)
	void CImage24::BitBltwp(LPBYTE pDst,WORD nDstX,WORD nDstY,WORD nPitchByte,WORD nSrcX,WORD nSrcY,WORD nSrcWidth,WORD nSrcHeight);

	// 가로 폭 얻기
	WORD GetWidth(void);
	// 셀로 폭 얻기
	WORD GetHeight(void);

	// 가로 피치 얻기(Byte)
	WORD GetPitch(void);
};

#endif
