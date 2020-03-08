//
//		Cursor
//
//		Cursor, BitmapCursor, Animation Cursor
//
#ifndef MCURSOR_H
#define MCURSOR_H


class MBitmap;
class MAniBitmap;
class MAnimation;
class MDrawContext;

#define MCURSOR_NAME_LENGTH		32

// Abstract Cursor class
class MCursor{
	char	m_szName[MCURSOR_NAME_LENGTH];
protected:
	friend class MCursorSystem;
	virtual void Draw(MDrawContext* pDC, int x, int y){}
public:
	MCursor(const char* szName);
	virtual ~MCursor(void){}
};

// Bitmap Cursor class
class MBitmapCursor : public MCursor{
	MBitmap*	m_pBitmap;
protected:
	virtual void Draw(MDrawContext* pDC, int x, int y);
public:
	MBitmapCursor(const char* szName, MBitmap* pBitmap);
};

// Animation Bitmap Cursor class
class MAniBitmapCursor : public MCursor{
	MAnimation*	m_pAnimation;
protected:
	virtual void Draw(MDrawContext* pDC, int x, int y);
public:
	MAniBitmapCursor(const char* szName, MAniBitmap* pAniBitmap);
	virtual ~MAniBitmapCursor(void);
};



#endif