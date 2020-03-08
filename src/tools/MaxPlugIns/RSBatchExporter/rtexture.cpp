#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <windows.h>		// bitmap 저장 펑션 때문.
#include "rtexture.h"
#include "Dib.h"
#include "Image24.h"
#include "fileinfo.h"

#define ARGB(a,r,g,b) DWORD((((DWORD)(a))<<24) | (((DWORD)(r))<<16) | (((DWORD)(g))<<8) | DWORD(b) )

rtexture::rtexture()
{
	data=NULL;
}

rtexture::~rtexture()
{
	Destroy();
}

void rtexture::Destroy()
{
	if(data) {
		delete []data;
		data=NULL;
	}
}

void rtexture::New(int x,int y,RTEXTUREFORMAT fm)
{
	switch(fm)
	{
	case RTEXTUREFORMAT_24 : data=new BYTE[x*y*3];break;
	case RTEXTUREFORMAT_32 : data=new BYTE[x*y*4];break;
	}
	rtexture::x=x;
	rtexture::y=y;
	format=fm;
}

void rtexture::Create(int x,int y,void* colorbuf,void* alphabuf)
{
	if(alphabuf)
	{
		New(x,y,RTEXTUREFORMAT_32);
		BYTE *dest=(BYTE*)data;
		BYTE *a=(BYTE*)alphabuf,*r=(BYTE*)colorbuf,*g=(BYTE*)colorbuf+1,*b=(BYTE*)colorbuf+2;
		for(int i=0;i<x*y;i++)
		{
			WORD alpha=(WORD(*a)+WORD(*(a+1))+WORD(*(a+2)))/3;if(alpha>255) alpha=255;
			*dest= (BYTE)alpha;dest++;
			*dest=*r;dest++;
			*dest=*g;dest++;
			*dest=*b;dest++;
			a+=3;r+=3;g+=3;b+=3;
		}
	}
	else
	{
		New(x,y,RTEXTUREFORMAT_24);
		memcpy(data,colorbuf,x*y*3);
	}
}

void rtexture::Create(rtexture *source)
{
	New(source->x,source->y,source->format);
	memcpy(data,source->data,x*y*GetBytesPerPixel());
}

bool rtexture::CreateFromBMP(const char *filename)
{

	CDib dib;
	CImage24 image;
	if(dib.Open(NULL,filename))
	{
		if(image.Open(&dib))
		{
			char fname[_MAX_FNAME];
			GetPureFilename(fname,filename);
			if(data) delete []data;
			data=new char[image.GetDataSize()];
			x=image.GetWidth();
			y=image.GetHeight();
			New(x,y,RTEXTUREFORMAT_24);
			memcpy(data,image.GetData(),image.GetDataSize());
		}
		else
			return false;
	}
	else
		return false;
	return true;
}

int rtexture::GetBytesPerPixel()
{
	switch(format)
	{
	case RTEXTUREFORMAT_24 : return 3;
	case RTEXTUREFORMAT_32 : return 4;
	}
	return 0;
}

int rtexture::GetDataSize()
{
	return GetBytesPerPixel()*x*y;
}

bool rtexture::CreateAsHalf(rtexture *source)
{
	if((source->x<2)||(source->y<2)||(source->x%2)||(source->y%2)) return false;
	New(source->x/2,source->y/2,source->format);
	
	int i,j,k,bp=GetBytesPerPixel();
	int y2=source->x;
	BYTE *buf=(BYTE*)data,*m=(BYTE*)source->data;
	for(i=0;i<y;i++)
	{
		for(j=0;j<x;j++)
		{
			for(k=0;k<bp;k++)
			{
				buf[(i*x+j)*bp+k]=((WORD)m[(i*2*y2+j*2)*bp+k]+(WORD)m[(i*2*y2+j*2+1)*bp+k]
										+(WORD)m[((i*2+1)*y2+j*2)*bp+k]+(WORD)m[((i*2+1)*y2+j*2+1)*bp+k])>>2;
			}
		}
	}
	return true;
}

bool rtexture::CreateAsDouble(rtexture *source)
{
	New(source->x*2,source->y*2,source->format);
	
	int i,j,k,bp=GetBytesPerPixel();
	int y2=source->x;
	BYTE *buf=(BYTE*)data,*m=(BYTE*)source->data;
	for(i=0;i<y;i++)
	{
		for(j=0;j<x;j++)
		{
			for(k=0;k<bp;k++)
			{
				buf[(i*x+j)*bp+k]=m[(i/2*y2+j/2)*bp+k];
			}
		}
	}
	return true;
}

bool rtexture::CreateAsCopy(rtexture *source,int sx,int sy,int width,int height,bool bFlipU,bool bFlipV)
{
	if((sx+width>source->GetWidth())||(sy+height>source->GetHeight())) return false;
	int x,y,dx,dy;
	int stepx=bFlipU?-1:1,stepy=bFlipV?-1:1;

	New(width,height,source->GetFormat());
	for(y=0,dy=bFlipV?height-1:0;y<height;y++,dy+=stepy)
	{
		for(x=0,dx=bFlipU?width-1:0;x<width;x++,dx+=stepx)
		{
			SetPixel(dx,dy,source->GetPixel(x+sx,y+sy));
		}
	}

	return true;
}

DWORD rtexture::GetPixel(int x,int y)
{
	if(data==NULL)	return 0;

	BYTE *dest=(BYTE*)data+(rtexture::x*y+x)*GetBytesPerPixel();
	switch(format)
	{
	case RTEXTUREFORMAT_24 : return ARGB(0,*dest,*(dest+1),*(dest+2));
	case RTEXTUREFORMAT_32 : return ARGB(*dest,*(dest+1),*(dest+2),*(dest+3));
	}
	return 0;
}

void rtexture::SetPixel(int x,int y,DWORD color)
{
	if(data==NULL)	return;

	if((x<0)||(x>=rtexture::x)||(y<0)||(y>=rtexture::y)) return;
	BYTE *dest=(BYTE*)data+(rtexture::x*y+x)*GetBytesPerPixel();
	BYTE a=(BYTE)((color & 0xff000000)>>24);
	BYTE r=(BYTE)((color & 0x00ff0000)>>16);
	BYTE g=(BYTE)((color & 0x0000ff00)>>8);
	BYTE b=(BYTE)((color & 0x000000ff));

	switch(format)
	{
	case RTEXTUREFORMAT_24 : *dest=r;*(dest+1)=g;*(dest+2)=b;break;
	case RTEXTUREFORMAT_32 : *dest=a;*(dest+1)=r;*(dest+2)=g;*(dest+3)=b;break;
	}
}

void rtexture::Fill(int dx,int dy,int nDir,rtexture *source)
{
	int sox,soy,ssux,ssuy,ssvx,ssvy;

#define SETUP(a,b,c,d,e,f) { sox=a;soy=b;ssux=c;ssuy=d;ssvx=e;ssvy=f; }
#define XX (source->x-1)
#define YY (source->y-1)

	switch(nDir)
	{
	case 0 : SETUP(0 ,0 , 1, 0, 0, 1);break;
	case 3 : SETUP(XX,0 , 0, 1,-1, 0);break;
	case 2 : SETUP(XX,YY,-1, 0, 0,-1);break;
	case 1 : SETUP(0 ,YY, 0,-1, 1, 0);break;
	}

	int sx,sy;
	int i,j,bp=GetBytesPerPixel();
	for(i=dy;i<min(dy+source->y,y);i++)
	{
		sx=sox+ssvx*(i-dy);
		sy=soy+ssvy*(i-dy);
		for(j=dx;j<min(dx+source->x,x);j++)
		{
			SetPixel(j,i,source->GetPixel(sx,sy));
			sx+=ssux;
			sy+=ssuy;
		}
	}
}

bool rtexture::SaveAsBMP(const char *filename)
{
	if(!data) return false;

	FILE *file;
	file=fopen(filename,"wb+");
	if(!file) return false;

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	
	// SET FILE HEADER : 14 byte
	bmfh.bfType = 0x4D42;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + x*y*3;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// SET INFO HEADER : 40 byte
	ZeroMemory( &bmih, sizeof(BITMAPINFOHEADER) );
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = x;
	bmih.biHeight = y;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	
	fwrite( &bmfh, sizeof(BITMAPFILEHEADER), 1, file );
	fwrite( &bmih, sizeof(BITMAPINFOHEADER), 1, file );
	
	int i,j;

	unsigned char *p=(unsigned char*)data+(y-1)*(x*3);

	for(i=y-1;i>=0;i--)
	{
		for(j=0;j<x;j++)
		{
			fwrite(p+j*3,1,1,file);
			fwrite(p+j*3+1,1,1,file);
			fwrite(p+j*3+2,1,1,file);
		}
		p-=x*3;
	}

	fclose( file );

	return TRUE;
}

void rtexture::Fill(DWORD color)
{
	if(!data) return;
	int i,j;
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			SetPixel(i,j,color);
		}
	}
}

void rtexture::FillBoundary(DWORD color)
{
	if(!data) return;
	int i;
	for(i=0;i<x;i++)
	{
		SetPixel(i,0,color);
		SetPixel(i,y-1,color);
	}
	for(i=0;i<y;i++)
	{
		SetPixel(0,i,color);
		SetPixel(x-1,i,color);
	}
}

void rtexture::FillTriangle(float sx0,float sy0,float sx1,float sy1,float sx2,float sy2,DWORD color)
{
	if(!data) return;

#define FIXDIGIT 16
#define FLOAT2FIX(fl) int((fl)*float(1<<FIXDIGIT))
#define FIX2INT(fx) ((fx)>>FIXDIGIT)

struct RTPOINT
{
	float sx,sy;
};

	// step 1.
	static RTPOINT ver[3],*v[3],*t1,*t2,*m;
	ver[0].sx=sx0;ver[0].sy=sy0;
	ver[1].sx=sx1;ver[1].sy=sy1;
	ver[2].sx=sx2;ver[2].sy=sy2;
	
	if(ver[0].sy<ver[1].sy) { t1=&ver[0];t2=&ver[1]; } else { t1=&ver[1];t2=&ver[0]; }
	if(t1->sy<ver[2].sy) { v[0]=t1;t1=&ver[2]; } else v[0]=&ver[2];
	
	// step 2.
	float a=v[0]->sy-t1->sy,b=t1->sx-v[0]->sx,c=-a*t1->sx-b*t1->sy;
	if(a*t2->sx+b*t2->sy+c<0) { v[1]=t1;v[2]=t2; } else { v[1]=t2;v[2]=t1; }

	// step 3.
	static int y0,y1,y2,ym,yb,i,j;
	m=v[1+(v[2]->sy<v[1]->sy)];
	y0=(int)v[0]->sy;y1=(int)v[1]->sy;y2=(int)v[2]->sy;
	ym=(int)m->sy;yb=(int)v[1+(m==v[1])]->sy;

	// step 4.	:: raster upper triangle
	float oody01=1/(v[1]->sy-v[0]->sy),
			oody02=1/(v[2]->sy-v[0]->sy),
			oody12=1/(v[2]->sy-v[1]->sy);
	float dx01=(v[1]->sx-v[0]->sx)*oody01,
			dx02=(v[2]->sx-v[0]->sx)*oody02,
			dx12=(v[2]->sx-v[1]->sx)*oody12;

	static int x1,x2,dx1,dx2,ix1,ix2;

	x2=x1=FLOAT2FIX(v[0]->sx);
	dx1=FLOAT2FIX(dx01);dx2=FLOAT2FIX(dx02);

	for(i=y0;i<ym;i++)
	{
		ix1=FIX2INT(x1);
		ix2=FIX2INT(x2);

		for(j=ix1;j<=ix2;j++)
			SetPixel(j,i,color);

		x1+=dx1;
		x2+=dx2;
	}

	// step 5.	:: raster lower triangle
	if(m==v[1]) {	// i.e case 1
		x1=FLOAT2FIX(v[1]->sx);dx1=FLOAT2FIX(dx12);
	} 
	else {			// i.e case 2
		x2=FLOAT2FIX(v[2]->sx);dx2=FLOAT2FIX(dx12);
	}
	for(i=ym;i<yb;i++)
	{
		ix1=FIX2INT(x1);
		ix2=FIX2INT(x2);

		if(ix2<ix1) break;
		for(j=ix1;j<ix2;j++)
			SetPixel(j,i,color);

		x1+=dx1;
		x2+=dx2;
	}
}

bool rtexture::Create(FILE *file)
{
	rtexture_header header;
	if(!fread(&header,sizeof(header),1,file)) return false;

	New(header.sx,header.sy,header.fmt);
	if(!fread(GetData(),GetDataSize(),1,file)) return false;

	return true;
}

bool rtexture::Save(FILE *file)
{
	rtexture_header header;
	header.sx=x;
	header.sy=y;
	header.fmt=format;

	if(!fwrite(&header,sizeof(header),1,file)) return false;
	if(!fwrite(GetData(),GetDataSize(),1,file)) return false;

	return true;
}
