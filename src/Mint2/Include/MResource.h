#ifndef MRESOURCE_H
#define MRESOURCE_H

#include <map>
#include <string>
#include <list>

#include "MTypes.h"

struct MWIDGETINFO{
	char	szWidgetClass[256];
	char	szWidgetName[256];
	int		nResourceID;
};

struct MWIDGETRESOURCE{
	MRECT				Bounds;	
	bool				bBounds;

	class MWIDGETINFOLIST : public list<MWIDGETINFO*>{
	public:
		virtual ~MWIDGETINFOLIST(void){
			// Delete Automatically
			while(empty()==false){
				delete (*begin());
				erase(begin());
			}
		}
	} Children;
};

class MResourceMap : public map<string, MWIDGETRESOURCE*>{
public:
	virtual ~MResourceMap(void){
		// Delete Automatically
		while(empty()==false){
			delete (*begin()).second;
			erase(begin());
		}
	}
};


#endif