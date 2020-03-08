#pragma once

class ZMessageNameGroup;

class ZMessageManager
{
public :
	ZMessageManager();
	~ZMessageManager();

	bool Init( const ZMessageNameGroup& rfMessageNameGroup );

private :
	bool LoadMessageGroup( const ZMessageNameGroup& rfMessageNameGroup );
};