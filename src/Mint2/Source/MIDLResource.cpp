#include "stdafx.h"
#include "MIDLResource.h"
#include "Mint.h"
#include "MFrame.h"
#include "MLabel.h"
#include "MButton.h"
#include "MEdit.h"
#include "MListBox.h"
#include "MPicture.h"
#include "MScrollBar.h"
#include "MSlider.h"
#include "MGroup.h"
#include "MComboBox.h"
#include "MToolTip.h"
#include "MBLabelLook.h"
#include "MBButtonLook.h"
#include "MBGroupLook.h"
#include "MBFrameLook.h"
#include "MBEditLook.h"
#include "MBListBoxLook.h"
#include "MBScrollBarLook.h"
#include "MBSliderLook.h"
#include "MBGroupLook.h"
#include "MBitmap.h"
#include "MPopupMenu.h"
#include "MAnimation.h"
#include "MCursor.h"
#include "MBmButton.h"
#include "MBmLabel.h"
#include "MHotKey.h"
#include "MMsgBox.h"
#include "MTextArea.h"
#include "MBTextAreaLook.h"
#include "MBSliderLook.h"
#include "MTabCtrl.h"
#include "MPanel.h"
#include "MDebug.h"
#include "MBTabCtrlLook.h"

MIDLResource::MIDLResource()
{
	m_pParent = Mint::GetInstance()->GetMainFrame();
}
MIDLResource::~MIDLResource()
{
	Clear();
}

MPOINT MIDLResource::GetPoint(MXmlElement& element)
{
	MPOINT point;
	element.GetChildContents(&point.x, "X");
	element.GetChildContents(&point.y, "Y");
	return point;
}
MRECT  MIDLResource::GetRect(MXmlElement& element)
{
	MRECT rect;
	element.GetChildContents(&rect.w, "W");
	element.GetChildContents(&rect.h, "H");
	element.GetChildContents(&rect.x, "X");
	element.GetChildContents(&rect.y, "Y");
	return rect;
}
MSIZE  MIDLResource::GetSize(MXmlElement& element)
{
	MSIZE size;
	element.GetChildContents(&size.w, "W");
	element.GetChildContents(&size.h, "H");
	return size;
}
MCOLOR MIDLResource::GetColor(MXmlElement& element)
{
	MCOLOR color;

	int r = 0xff, g = 0xff, b = 0xff, a = 0xff;
	element.GetChildContents(&r, "R");
	element.GetChildContents(&g, "G");
	element.GetChildContents(&b, "B");
	element.GetChildContents(&a, "ALPHA");

	color.r = (unsigned char)r;
	color.g = (unsigned char)g;
	color.b = (unsigned char)b;
	color.a = (unsigned char)a;

	return color;
}
MAnchors MIDLResource::GetAnchors(MXmlElement& element)
{
	MAnchors ret;

	element.GetChildContents(&ret.m_bLeft, "LEFT");
	element.GetChildContents(&ret.m_bTop, "TOP");
	element.GetChildContents(&ret.m_bRight, "RIGHT");
	element.GetChildContents(&ret.m_bBottom, "BOTTOM");

	return ret;
}

MAlignmentMode MIDLResource::GetAlignmentMode(MXmlElement& element)
{
	MAlignmentMode am = MAM_NOTALIGN;

	char szValue[256] = "";

	if(element.GetChildContents(szValue, "HALIGN")==true){
		if(_stricmp(szValue, "LEFT")==0) am |= MAM_LEFT;
		else if(_stricmp(szValue, "CENTER")==0) am |= MAM_HCENTER;
		else if(_stricmp(szValue, "RIGHT")==0) am |= MAM_RIGHT;
	}

	if(element.GetChildContents(szValue, "VALIGN")==true){
		if(_stricmp(szValue, "TOP")==0) am |= MAM_TOP;
		else if(_stricmp(szValue, "CENTER")==0) am |= MAM_VCENTER;
		else if(_stricmp(szValue, "BOTTOM")==0) am |= MAM_BOTTOM;
	}

	return am;
}

void MIDLResource::GetFrameBtn(MFrameBtn* pFrameBtn, MBFrameLook* pFrameLook, MXmlElement& element)
{
	if (pFrameLook == NULL) return;

	char szTagName[256];
	MXmlElement childElement;

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BOUNDS"))
		{
			MRECT rect = GetRect(childElement);
			pFrameBtn->m_Rect = rect;
		}
		else if (!strcmp(szTagName, "ANCHORS"))
		{
			pFrameBtn->m_Anchors = GetAnchors(childElement);
		}
	}

	pFrameBtn->m_bVisible = true;
}

void MIDLResource::GetBmButtonBitmaps(MBitmap** ppBitmaps, MXmlElement& element)
{
	char szTagName[256];
	MXmlElement bitmapElement;

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		bitmapElement = element.GetChildNode(i);
		bitmapElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BITMAP"))
		{
			char szType[256];
			bitmapElement.GetAttribute(szType, "type", "");
			MBitmap* pBitmap = GetBitmap(bitmapElement);

			if (pBitmap != NULL)
			{
				if (!strcmp(szType, "up"))
				{
					ppBitmaps[0] = pBitmap;
				}
				else if (!strcmp(szType, "down"))
				{
					ppBitmaps[1] = pBitmap;
				}
				else if (!strcmp(szType, "disable"))
				{
					ppBitmaps[2] = pBitmap;
				}
			}
		}
	}
}

MBmButton* MIDLResource::GetBmButton(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[1024];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MBmButton* pBmButton = (MBmButton*)Mint::GetInstance()->NewWidget(MINT_BMBUTTON, "", pParentWidget, pListener);
	InsertWidget(element, pBmButton);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (GetCommonWidgetProperty(pBmButton, childElement, szTagName)) continue;
		
		if (!strcmp(szTagName, "BUTTONLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBButtonLook*>::iterator itor = m_ButtonLookMap.find(szItem);
			if (itor != m_ButtonLookMap.end())
			{
				pBmButton->ChangeCustomLook((MBButtonLook*)(*itor).second);
			}
		}

		if (!strcmp(szTagName, "BITMAP"))
		{
			char szType[256];
			childElement.GetAttribute(szType, "type", "");
			MBitmap* pBitmap = GetBitmap(childElement);

			if (pBitmap != NULL)
			{
				if (!strcmp(szType, "up"))
				{
					pBmButton->SetUpBitmap(pBitmap);
				}
				else if (!strcmp(szType, "down"))
				{
					pBmButton->SetDownBitmap(pBitmap);
				}
				else if (!strcmp(szType, "disable"))
				{
					pBmButton->SetDisableBitmap(pBitmap);
				}
				else if (!strcmp(szType, "over"))
				{
					pBmButton->SetOverBitmap(pBitmap);
				}
			}
		}
		else if(!strcmp(szTagName, "CONFIRMMESSAGE"))
		{
			char szContents[256] = {0, };
			char szItem[256] = {0,};
			childElement.GetContents(szContents);
			TransText(szContents, szItem, sizeof(szItem));
			pBmButton->SetConfirmMessageBox(szItem);
		}
		else if(!strcmp(szTagName, "CONFIRMLOOK")){
			if (pBmButton->m_pMsgBox != NULL)
			{
				char szItem[256];
				memset(szItem, 0, sizeof(szItem));
				childElement.GetContents(szItem);

				map<string, MBFrameLook*>::iterator itor = m_FrameLookMap.find(szItem);
				if (itor != m_FrameLookMap.end())
				{
					MBFrameLook* pFrameLook = (MBFrameLook*)(*itor).second;
					pBmButton->m_pMsgBox->ChangeCustomLook(pFrameLook);
					pBmButton->m_pMsgBox->SetTitle(pFrameLook->m_szDefaultTitle);
				}
			}
		}
		else if( !strcmp(szTagName,"STRETCH"))
		{
			pBmButton->SetStretch(true);
		}
		else if( !strcmp(szTagName,"BMTEXTCOLOR"))
		{
			pBmButton->m_bTextColor = true;
			float r,g,b,a;
			childElement.GetAttribute( &r, "r", 0);
			childElement.GetAttribute( &g, "g", 0);
			childElement.GetAttribute( &b, "b", 0);
			childElement.GetAttribute( &a, "a", 255);
			pBmButton->m_BmTextColor = MCOLOR( (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
		}
		else if(!strcmp(szTagName, "PUSHBUTTON")){
			pBmButton->SetType(MBT_PUSH);
		}
		else if(!strcmp(szTagName, "SETCHECK")){
			pBmButton->SetCheck(true);
		}
		else if(!strcmp(szTagName, "GROUP")){
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MButtonGroup*>::iterator itor = m_ButtonGroupMap.find(szItem);

			MButtonGroup *pButtonGroup;
			if(itor != m_ButtonGroupMap.end()) {
				pButtonGroup = itor->second;
			} else {
				pButtonGroup = new MButtonGroup;
				m_ButtonGroupMap.insert(map<string, MButtonGroup*>::value_type(szItem,pButtonGroup));
			}

			pBmButton->SetButtonGroup(pButtonGroup);
		}
	}

	return pBmButton;
}

MBitmap* MIDLResource::GetBitmap(MXmlElement& element)
{
	bool bSourceFound = false;
	char szFileName[256];

	memset(szFileName, 0, sizeof(szFileName));

	bool bBoundsFound = false;
	MRECT rt;

	char szTagName[256];
	MXmlElement childElement;

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BOUNDS"))
		{
			bBoundsFound = true;
			rt = GetRect(childElement);
		}else
		if (!strcmp(szTagName, "SOURCE"))
		{
			bSourceFound = true;
			childElement.GetContents(szFileName);
		}
	}

	if(bSourceFound && bBoundsFound) {

		if(szFileName[0]==0 || _stricmp(szFileName,"NULL")==0) return NULL;
		MBitmap *pBitmap = MBitmapManager::Get(szFileName);
		if(pBitmap==NULL) {
			mlog("warning : bitmap %s not found.\n",szFileName);
			return NULL;
		}

		MPartialBitmap *pNewBitmap = new MPartialBitmap(pBitmap,rt);

		MBitmapManager::Add(pNewBitmap);

		return pNewBitmap;
	}

	element.GetContents(szFileName);
	if(szFileName[0]==0 || _stricmp(szFileName,"NULL")==0) return NULL;

	MBitmap *pBitmap = MBitmapManager::Get(szFileName);
	if(pBitmap==NULL) {
		mlog("warning : bitmap %s not found.\n",szFileName);
	}

	return pBitmap;
}

MBitmap* MIDLResource::GetBitmapAlias(MXmlElement& element)
{
	bool bSourceFound = false;
	char szSourceFileName[256];

	memset(szSourceFileName, 0, sizeof(szSourceFileName));

	bool bBoundsFound = false;
	MRECT rt;

	char szTagName[256];
	MXmlElement childElement;

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BOUNDS"))
		{
			bBoundsFound = true;
			rt = GetRect(childElement);
		}else
		if (!strcmp(szTagName, "SOURCE"))
		{
			bSourceFound = true;
			childElement.GetContents(szSourceFileName);
		}
	}

	char szAliasName[256];
	element.GetAttribute(szAliasName, "name");

	if(bSourceFound && bBoundsFound) {

		if(szSourceFileName[0]==0 || _stricmp(szSourceFileName,"NULL")==0) return NULL;
		MBitmap *pBitmap = MBitmapManager::Get(szSourceFileName);
		if(pBitmap==NULL) {
			mlog("warning : bitmap %s not found.\n",szSourceFileName);
			return NULL;
		}

		MPartialBitmap *pNewBitmap = new MPartialBitmap(pBitmap,rt);
		strcpy_safe(pNewBitmap->m_szName,szAliasName);

		MBitmapManager::Add(pNewBitmap);

		return pNewBitmap;
	}

	return NULL;
}

void MIDLResource::GetBitmaps(MBitmap** ppBitmaps, MXmlElement& element, const int nBitmapCount)
{
	char szTagName[256];
	MXmlElement bitmapElement;

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		bitmapElement = element.GetChildNode(i);
		bitmapElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BITMAP"))
		{
			int t = -1;
			bitmapElement.GetAttribute(&t, "type", -1);
			MBitmap* pBitmap = GetBitmap(bitmapElement);

			if ((0 <= t) && (t < nBitmapCount))
			{
				ppBitmaps[t] = pBitmap;
			}
		}
	}
}

MBGroupLook* MIDLResource::GetGroupLook(MXmlElement& element)
{
	MXmlElement childElement, bitmapElement;
	char szTagName[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));
	bool bDefaultLook = false;

	MBGroupLook* pGroupLook = new MBGroupLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "FONT"))
		{
			childElement.GetContents(szFontName);			
		} 
		if (!strcmp(szTagName, "TEXTPOSITION"))
		{
			pGroupLook->m_TitlePosition = GetPoint(childElement);
		} 
		if (!strcmp(szTagName, "STRETCH"))
		{
			bool bStretch = true;
			childElement.GetContents(&bStretch);
			pGroupLook->m_bStretch = bStretch;
		} 
		else if (!strcmp(szTagName, "TEXTCOLOR"))
		{
			pGroupLook->m_FontColor = GetColor(childElement);
		} 
		else if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pGroupLook->m_pFrameBitmaps, childElement, FRAME_BITMAP_COUNT);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}
	pGroupLook->m_pFont = MFontManager::Get(szFontName);

	// Default Look
	if(bDefaultLook==true) MGroup::ChangeLook(pGroupLook);

	// FrameLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_GroupLookMap.insert(map<string, MBGroupLook*>::value_type(string(szItem), pGroupLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pGroupLook;
}

MBFrameLook* MIDLResource::GetFrameLook(MXmlElement& element)
{
	MXmlElement childElement, bitmapElement;
	char szTagName[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));
	bool bDefaultLook = false;

	MBFrameLook* pFrameLook = new MBFrameLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "FONT"))
		{
			childElement.GetContents(szFontName);			
		} 
		if (!strcmp(szTagName, "TEXTPOSITION"))
		{
			pFrameLook->m_TitlePosition = GetPoint(childElement);
		} 
		if (!strcmp(szTagName, "STRETCH"))
		{
			bool bStretch = true;
			childElement.GetContents(&bStretch);
			pFrameLook->m_bStretch = bStretch;
		} 
		else if (!strcmp(szTagName, "TEXTCOLOR"))
		{
			pFrameLook->m_FontColor = GetColor(childElement);
		} 
		else if( !strcmp(szTagName,"BGCOLOR"))
		{
			pFrameLook->m_BGColor = GetColor(childElement);
		}
		else if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pFrameLook->m_pFrameBitmaps, childElement, FRAME_BITMAP_COUNT);
		}
		else if (!strcmp(szTagName, "CLOSE"))
		{
			GetBmButtonBitmaps(pFrameLook->m_pCloseButtonBitmaps, childElement);
		}
		else if (!strcmp(szTagName, "MINIMIZE"))
		{
			GetBmButtonBitmaps(pFrameLook->m_pMinimizeButtonBitmaps, childElement);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
		else if(!strcmp(szTagName,"CUSTOMLOOK"))
		{
			int iTempBuf;
			childElement.GetContents( &iTempBuf );
			pFrameLook->SetCustomLook( iTempBuf );
		}
		else if (!strcmp(szTagName, "DEFAULTTITLE"))
		{
			char szText[256];
			childElement.GetContents(szText);
			strcpy_safe(pFrameLook->m_szDefaultTitle,szText);
		}
		else if (!strcmp(szTagName, "SCALABLE"))
		{
			pFrameLook->SetScaleEnable(true);
		} 
	}
	pFrameLook->m_pFont = MFontManager::Get(szFontName);

	// Default Look
	if(bDefaultLook==true) MFrame::ChangeLook(pFrameLook);

	// FrameLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_FrameLookMap.insert(map<string, MBFrameLook*>::value_type(string(szItem), pFrameLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pFrameLook;
}


MBTextAreaLook*	MIDLResource::GetTextAreaLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));
	bool bDefaultLook = false;

	MBTextAreaLook* pTextAreaLook = new MBTextAreaLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;
		if (!strcmp(szTagName, "FONT"))
		{
			childElement.GetContents(szFontName);			
		} 
		else if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pTextAreaLook->m_pFrameBitmaps, childElement, 9);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}
	pTextAreaLook->m_pFont = MFontManager::Get(szFontName);

	// Default Look
	if(bDefaultLook==true) MTextArea::ChangeLook(pTextAreaLook);

	// TextAreaLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_TextAreaLookMap.insert(map<string, MBTextAreaLook*>::value_type(string(szItem), pTextAreaLook)).second)
		OutputDebugString("insert widget failed.\n");


	return pTextAreaLook;
}

MBLabelLook* MIDLResource::GetLabelLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));
	bool bDefaultLook = false;

	MBLabelLook* pLabelLook = new MBLabelLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "FONT"))
		{
			childElement.GetContents(szFontName);			
		} 
		else if (!strcmp(szTagName, "TEXTCOLOR"))
		{
			pLabelLook->m_FontColor = GetColor(childElement);
		} 
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}
	pLabelLook->m_pFont = MFontManager::Get(szFontName);

	// Default Look
	if(bDefaultLook==true) MLabel::ChangeLook(pLabelLook);

	// LabelLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_LabelLookMap.insert(map<string, MBLabelLook*>::value_type(string(szItem), pLabelLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pLabelLook;
}


MBButtonLook* MIDLResource::GetButtonLook(MXmlElement& element)
{
	MXmlElement childElement, bitmapElement;
	char szBuf[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));

	bool bDefaultLook = false;

	MBButtonLook* pButtonLook = new MBButtonLook();
	
	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (szBuf[0] == '#') continue;

		if (!strcmp(szBuf, "FONT"))
		{
			childElement.GetContents(szFontName);
		} 
		else if (!strcmp(szBuf, "TEXTCOLOR"))
		{
			pButtonLook->m_FontColor = GetColor(childElement);
		} 
		else if (!strcmp(szBuf, "TEXTDOWNCOLOR"))
		{
			pButtonLook->m_FontDownColor = GetColor(childElement);
		} 
		else if (!strcmp(szBuf, "TEXTDOWNOFFSET"))
		{
			pButtonLook->m_FontDownOffset = GetPoint(childElement);
		}
		else if (!strcmp(szBuf, "TEXTHIGHLIGHTCOLOR"))
		{
			pButtonLook->m_FontHighlightColor = GetColor(childElement);
		} 
		else if (!strcmp(szBuf, "TEXTDISABLECOLOR"))
		{
			pButtonLook->m_FontDisableColor = GetColor(childElement);
		} 
		else if (!strcmp(szBuf, "UP"))
		{
			GetBitmaps(pButtonLook->m_pUpBitmaps, childElement, 9);
		}
		else if (!strcmp(szBuf, "DOWN"))
		{
			GetBitmaps(pButtonLook->m_pDownBitmaps, childElement, 9);
		}
		else if (!strcmp(szBuf, "OVER"))
		{
			GetBitmaps(pButtonLook->m_pOverBitmaps, childElement, 9);
		}
		else if (!strcmp(szBuf, "FOCUS"))
		{
			GetBitmaps(pButtonLook->m_pFocusBitmaps, childElement, 4);
		}
		else if (!strcmp(szBuf, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
		else if (!strcmp(szBuf, "STRETCH"))
		{
			bool bStretch = true;
			childElement.GetContents(&bStretch);
			pButtonLook->m_bStretch = bStretch;
		} 
		else if(!strcmp(szBuf, "CUSTOMLOOK"))
		{
			pButtonLook->SetCustomLook(true);
		}
		else if(!strcmp(szBuf, "WIRELOOK"))
		{
			pButtonLook->SetWireLook(true);
		}
		else if (!strcmp(szBuf, "SCALABLE"))
		{
			pButtonLook->SetScaleEnable(true);
		} 
	}

	pButtonLook->m_pFont = MFontManager::Get(szFontName);

	// Default Look
	if(bDefaultLook==true) MButton::ChangeLook(pButtonLook);

	// ButtonLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ButtonLookMap.insert(map<string, MBButtonLook*>::value_type(string(szItem), pButtonLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pButtonLook;
}


MBEditLook* MIDLResource::GetEditLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256];

	MBEditLook* pEditLook = new MBEditLook();

	int iCount = element.GetChildNodeCount();
	bool bDefaultLook = false;

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;
		else if (!strcmp(szTagName, "FONT"))
		{
			char szFontName[256];
			childElement.GetContents(szFontName);
			pEditLook->m_pFont = MFontManager::Get(szFontName);
		} 
		else if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pEditLook->m_pFrameBitmaps, childElement, 9);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
		else	if(!strcmp(szTagName,"CUSTOMLOOK"))
		{
			pEditLook->SetCustomLook(true);
		}
	}

	// Default Look
	if(bDefaultLook==true){
		MEdit::ChangeLook(pEditLook);
		MHotKey::ChangeLook(pEditLook);
	}

	// EditLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_EditLookMap.insert(map<string, MBEditLook*>::value_type(string(szItem), pEditLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pEditLook;
}

MBListBoxLook* MIDLResource::GetListBoxLook(MXmlElement& element, int nType)
{
	MXmlElement childElement;
	char szTagName[256];

	MBListBoxLook* pListBoxLook = new MBListBoxLook();

	bool bDefaultLook = false;

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		else if (!strcmp(szTagName, "FONT"))
		{
			char szFontName[256];
			childElement.GetContents(szFontName);
			pListBoxLook->m_pFont = MFontManager::Get(szFontName);
		} 
		else if (!strcmp(szTagName, "BITMAPS")) 
		{
			GetBitmaps(pListBoxLook->m_pFrameBitmaps, childElement, 9);
		}
		else if (!strcmp(szTagName, "SELECTEDPLANECOLOR"))
		{
			pListBoxLook->m_SelectedPlaneColor = GetColor(childElement);
		} 
		else if (!strcmp(szTagName, "SELECTEDTEXTCOLOR"))
		{
			pListBoxLook->m_SelectedTextColor = GetColor(childElement);
		} 
		else if (!strcmp(szTagName, "UNFOCUSEDSELECTEDPLANECOLOR"))
		{
			pListBoxLook->m_UnfocusedSelectedPlaneColor = GetColor(childElement);
		} 
		else	if( !strcmp(szTagName, "ITEMTEXTMULTILINE"))
		{
			pListBoxLook->m_bItemTextMultiLine	= true;
		}
		else if( !strcmp(szTagName, "ITEMTEXTHCENTER"))
		{
			pListBoxLook->m_ItemTextAlignmentMode |= MAM_HCENTER;
		}
 		else if( !strcmp(szTagName, "ITEMTEXTVCENTER"))
		{
			pListBoxLook->m_ItemTextAlignmentMode |= MAM_VCENTER;
		}
		else if( !strcmp(szTagName, "ITEMBITMAP"))
		{
			pListBoxLook->m_pItemSlotBitmap	= GetBitmap( childElement );
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}

	// Default Look
	if(bDefaultLook==true){
		if(nType==0)	MListBox::ChangeLook(pListBoxLook);
		else			MComboListBox::ChangeLook(pListBoxLook);
	}

	// ListBoxLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ListBoxLookMap.insert(map<string, MBListBoxLook*>::value_type(string(szItem), pListBoxLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pListBoxLook;
}

MBArrowLook* MIDLResource::GetArrowLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256];

	MBArrowLook* pArrowLook = new MBArrowLook();
	bool bDefaultLook = false;

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pArrowLook->m_pArrowBitmaps, childElement, 8);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}

	// Default Look
	if(bDefaultLook==true) MArrow::ChangeLook(pArrowLook);

	// ArrowLook
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ArrowLookMap.insert(map<string, MBArrowLook*>::value_type(string(szItem), pArrowLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pArrowLook;
}

MBThumbLook* MIDLResource::GetThumbLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256];

	bool bDefaultLook = false;

	MBThumbLook* pThumbLook = new MBThumbLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "VERTICAL"))
		{
			char szType[256];
			childElement.GetAttribute(szType, "type", "normal");
			if (!strcmp(szType, "normal"))
			{
				GetBitmaps(pThumbLook->m_pVBitmaps, childElement, 3);
			}
			else if (!strcmp(szType, "pressed"))
			{
				GetBitmaps(pThumbLook->m_pVPressedBitmaps, childElement, 3);
			}
		}
		else if (!strcmp(szTagName, "HORIZONTAL"))
		{
			char szType[256];
			childElement.GetAttribute(szType, "type", "normal");
			if (!strcmp(szType, "normal"))
			{
				GetBitmaps(pThumbLook->m_pHBitmaps, childElement, 3);
			}
			else if (!strcmp(szType, "pressed"))
			{
				GetBitmaps(pThumbLook->m_pHPressedBitmaps, childElement, 3);
			}
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}

	// Default Look
	if(bDefaultLook==true) MThumb::ChangeLook(pThumbLook);

	// ThumbLook 등록
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ThumbLookMap.insert(map<string, MBThumbLook*>::value_type(string(szItem), pThumbLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pThumbLook;
}

MBScrollBarLook* MIDLResource::GetScrollBarLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256];

	bool bDefaultLook = false;

	MBScrollBarLook* pScrollBarLook = new MBScrollBarLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "VERTICAL"))
		{
			GetBitmaps(pScrollBarLook->m_pVFrameBitmaps, childElement, 3);
		}
		else if (!strcmp(szTagName, "HORIZONTAL"))
		{
			GetBitmaps(pScrollBarLook->m_pHFrameBitmaps, childElement, 3);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}

	// Default Look
	if(bDefaultLook==true) MScrollBar::ChangeLook(pScrollBarLook);

	// ScrollBarLook 등록
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ScrollBarLookMap.insert(map<string, MBScrollBarLook*>::value_type(string(szItem), pScrollBarLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pScrollBarLook;
}

MBSliderLook* MIDLResource::GetSliderLook(MXmlElement& element)
{
	return NULL;
}

MBSliderThumbLook* MIDLResource::GetSliderThumbLook(MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[256];

	bool bDefaultLook = false;

	MBSliderThumbLook* pSliderThumbLook = new MBSliderThumbLook();

	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if(!strcmp(szTagName, "BITMAP") )
		{
			pSliderThumbLook->m_pBitmap = GetBitmap(childElement);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
		// else if BITMAP...
	}

	// Default Look
	if(bDefaultLook==true) MSliderThumb::ChangeLook((MSliderThumbLook*)pSliderThumbLook);

	// ThumbLook 등록
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_ThumbLookMap.insert(map<string, MBThumbLook*>::value_type(string(szItem), (MBThumbLook*)pSliderThumbLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pSliderThumbLook;
}

MBTabCtrlLook* MIDLResource::GetTabCtrlLook(MXmlElement& element)
{
	MXmlElement childElement, bitmapElement;
	char szTagName[256], szFontName[256];
	memset(szFontName, 0, sizeof(szFontName));
	bool bDefaultLook = false;

	MBTabCtrlLook* pTabCtrlLook = new MBTabCtrlLook();

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szTagName, 0, sizeof(szTagName));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "BITMAPS"))
		{
			GetBitmaps(pTabCtrlLook->m_pFrameBitmaps, childElement, FRAME_BITMAP_COUNT);
		}
		else if (!strcmp(szTagName, "DEFAULT"))
		{
			childElement.GetContents(&bDefaultLook);
		}
	}

	// Default Look
	if(bDefaultLook==true) MTabCtrl::ChangeLook(pTabCtrlLook);

	// MTabCtrlLook 등록
	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_TabCtrlLookMap.insert(map<string, MBTabCtrlLook*>::value_type(string(szItem), pTabCtrlLook)).second)
		OutputDebugString("insert widget failed.\n");

	return pTabCtrlLook;
}

MWidget* MIDLResource::GetParentWidget(MXmlElement& element)
{
	char szBuf[4096];
	element.GetAttribute(szBuf, IDL_ATTR_PARENT, "");

	MWidgetMMap::iterator itor;
	itor = m_WidgetMap.find(szBuf);
	if (itor != m_WidgetMap.end())
	{
		return (MWidget*)(*itor).second;
	}
	else
	{
		return (MWidget*)m_pParent;
	}
}

void MIDLResource::InsertWidget(MXmlElement& element, MWidget* pWidget)
{
	char szItem[256];
	element.GetAttribute(szItem, "item", "unnamed widget");
	strcpy_safe(pWidget->m_szIDLName,szItem);
	m_WidgetMap.insert(MWidgetMMap::value_type(string(szItem), pWidget));
}

bool MIDLResource::GetCommonWidgetProperty(MWidget* pWidget, MXmlElement& element, const char* szTagName)
{
	if (szTagName[0] == '#') return true;

	bool bRet = false;
	if (!strcmp(szTagName, "BOUNDS"))
	{
		int w = MGetWorkspaceWidth();
		int h = MGetWorkspaceHeight();
		if(pWidget->GetParent()!=NULL){
			MRECT r = pWidget->GetParent()->GetRect();
			w = r.w;
			h = r.h;
		}
		MRECT rect = GetRect(element);
		if(rect.w<0) rect.w = w-abs(rect.x);
		if(rect.h<0) rect.h = h-abs(rect.y);

		pWidget->SetBounds(rect);
		pWidget->m_IDLRect = rect;
		bRet = true;
	}
	else if (!strcmp(szTagName, "TEXT"))
	{
		char szText[16384], szTar[16384];
		element.GetContents(szText);
		TransText(szText, szTar, sizeof(szText));
		pWidget->SetText(szTar);
		bRet = true;
	}
	else if (!strcmp(szTagName, "ANCHORS"))
	{	
		pWidget->m_Anchors = GetAnchors(element);
		bRet = true;
	}
	else if (!strcmp(szTagName, "CLIP"))
	{
		bool bClip = false;
		element.GetContents(&bClip);
		pWidget->SetClipByParent( bClip );
	}
	else if (!strcmp(szTagName, "TOOLTIP"))
	{
		char szContents[1024];
		char szToolTip[1024];
		element.GetContents(szContents);
		TransText(szContents, szToolTip, sizeof(szToolTip));
		pWidget->AttachToolTip(szToolTip);
	}
	else if (!strcmp(szTagName, "ALIGN"))
	{
		pWidget->SetBoundsAlignment(GetAlignmentMode(element), -1, -1);
	}
	else if (!strcmp(szTagName, "VISIBLE"))
	{
		bool bValue = true;
		element.GetContents(&bValue);
		pWidget->Show(bValue);
	}
	else if (!strcmp(szTagName, "ENABLE"))
	{
		bool bValue = true;
		element.GetContents(&bValue);
		pWidget->Enable(bValue);
	}
	else if (!strcmp(szTagName, "FOCUSABLE")) 
	{
		bool bValue = true;
		element.GetContents(&bValue);
		pWidget->SetFocusEnable(bValue);
	}

	return bRet;
}

MFrame*	MIDLResource::GetFrame(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	MBFrameLook* pFrameLook = NULL;

	pListener = pParentWidget = GetParentWidget(element);
	MFrame* pFrame = CreateFrame("", pParentWidget, pListener);
	InsertWidget(element, pFrame);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pFrame, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "FRAMELOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBFrameLook*>::iterator itor = m_FrameLookMap.find(szItem);
			if (itor != m_FrameLookMap.end())
			{
				pFrameLook = (MBFrameLook*)(*itor).second;
				pFrame->ChangeCustomLook(pFrameLook);
			}else {
				mlog("warning : FrameLook %s not found.\n",szItem);
			}
		}
		else if (!strcmp(szBuf, "CLOSE_BUTTON"))
		{
			GetFrameBtn(pFrame->GetCloseButton(), pFrameLook, childElement);
		}
		else if (!strcmp(szBuf, "MINIMIZE_BUTTON"))
		{
			GetFrameBtn(pFrame->GetMinimizeButton(), pFrameLook, childElement);
		}
		else if (!strcmp(szBuf, "TITLEBAR"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pFrame->m_bTitleBar = bValue;
		}
		else if (!strcmp(szBuf, "RESIZABLE"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pFrame->SetResizable(bValue);
		}
		else if (!strcmp(szBuf, "MINWIDTH"))
		{
			int nValue = 300;
			childElement.GetContents(&nValue);
			pFrame->m_nMinWidth = nValue;
		}
		else if (!strcmp(szBuf, "MINHEIGHT"))
		{
			int nValue = 300;
			childElement.GetContents(&nValue);
			pFrame->m_nMinHeight = nValue;
		}
		else if (!strcmp(szBuf, "MOVABLE"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pFrame->SetMovable(bValue);
		}
	}


	return pFrame;
}

MLabel* MIDLResource::GetLabel(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	char szFontName[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MLabel* pLabel = (MLabel*)Mint::GetInstance()->NewWidget(MINT_LABEL, "", pParentWidget, pListener);
	InsertWidget(element, pLabel);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pLabel, childElement, szBuf)) continue;

		if (!strcmp(szBuf, IDL_TEXTCOLOR))
		{
			MCOLOR color = GetColor(childElement);
			pLabel->SetTextColor(color);
		}
		else if (!strcmp(szBuf, "LABELLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBLabelLook*>::iterator itor = m_LabelLookMap.find(szItem);
			if (itor != m_LabelLookMap.end())
			{
				pLabel->ChangeCustomLook((MBLabelLook*)(*itor).second);
			}
		}
		else if(!strcmp(szBuf, "FONT"))
		{
			childElement.GetContents(szFontName);
			pLabel->SetFont( MFontManager::Get(szFontName) );
		}

	}
	return pLabel;
}

MButton* MIDLResource::GetButton(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MButton* pButton = (MButton*)Mint::GetInstance()->NewWidget(MINT_BUTTON, "", pParentWidget, pListener);
	InsertWidget(element, pButton);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pButton, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "BUTTONLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);
			
			map<string, MBButtonLook*>::iterator itor = m_ButtonLookMap.find(szItem);
			if (itor != m_ButtonLookMap.end())
			{
				pButton->ChangeCustomLook((MBButtonLook*)(*itor).second);
			}
		}
		else if(!strcmp(szBuf, "DEFAULTKEY")){
			char szItem[256] = {0,};
			childElement.GetContents(szItem);
			if(_stricmp(szItem, "ENTER")==0){
				pButton->m_nKeyAssigned = MBKA_ENTER;
			}
			else if(_stricmp(szItem, "ESC")==0){
				pButton->m_nKeyAssigned = MBKA_ESC;
			}
		}
		else if(!strcmp(szBuf, "CONFIRMMESSAGE")){

			char szContents[256] = {0, };
			char szItem[256] = {0,};
			childElement.GetContents(szContents);
			TransText(szContents, szItem, sizeof(szItem));
			pButton->SetConfirmMessageBox(szItem);
		}
		else if(!strcmp(szBuf, "CONFIRMLOOK")){
			if (pButton->m_pMsgBox != NULL)
			{
				char szItem[256];
				memset(szItem, 0, sizeof(szItem));
				childElement.GetContents(szItem);

				map<string, MBFrameLook*>::iterator itor = m_FrameLookMap.find(szItem);
				if (itor != m_FrameLookMap.end())
				{
					MBFrameLook* pFrameLook = (MBFrameLook*)(*itor).second;
					pButton->m_pMsgBox->ChangeCustomLook(pFrameLook);
					pButton->m_pMsgBox->SetTitle(pFrameLook->m_szDefaultTitle);
				}
			}
		}
		else if(!strcmp(szBuf, "PUSHBUTTON")){
			pButton->SetType(MBT_PUSH);
		}
		else if(!strcmp(szBuf, "PUSHBUTTON2")){
			pButton->SetType(MBT_PUSH2);
		}
		else if(!strcmp(szBuf, "SETCHECK")){
			pButton->SetCheck(true);
		}
		else if(!strcmp(szBuf, "GROUP")){
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MButtonGroup*>::iterator itor = m_ButtonGroupMap.find(szItem);
			
			MButtonGroup *pButtonGroup;
			if(itor != m_ButtonGroupMap.end()) {
				pButtonGroup = itor->second;
			} else {
                pButtonGroup = new MButtonGroup;
				m_ButtonGroupMap.insert(map<string, MButtonGroup*>::value_type(szItem,pButtonGroup));
			}

			pButton->SetButtonGroup(pButtonGroup);
		}
	}

	return pButton;
}

MEdit* MIDLResource::GetEdit(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MEdit* pEdit = (MEdit*)Mint::GetInstance()->NewWidget(MINT_EDIT, "", pParentWidget, pListener);
	InsertWidget(element, pEdit);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pEdit, childElement, szBuf)) continue;


		if (!strcmp(szBuf, "EDITLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBEditLook*>::iterator itor = m_EditLookMap.find(szItem);
			if (itor != m_EditLookMap.end())
			{
				pEdit->ChangeCustomLook((MBEditLook*)(*itor).second);
			}
		}
		else if (!strcmp(szBuf, "PASSWORD"))
		{
			bool bPassword = false;
			childElement.GetContents(&bPassword);
			if (bPassword == true)
			{
				pEdit->SetPasswordField(true);
			}
		}
		else if (!strcmp(szBuf, "MAXLENGTH"))
		{
			int nMaxLength;
			childElement.GetContents(&nMaxLength);
			pEdit->SetMaxLength(nMaxLength);
		}
	}

	return pEdit;
}

MListBox* MIDLResource::GetListBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	MListBoxLook* pListBoxLook = NULL;

	pListener = pParentWidget = GetParentWidget(element);
	MListBox* pListBox = (MListBox*)Mint::GetInstance()->NewWidget(MINT_LISTBOX, "", pParentWidget, pListener);
	InsertWidget(element, pListBox);

	int iCount = element.GetChildNodeCount();

	char szItem[256];

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pListBox, childElement, szBuf)) continue;
		else if (!strcmp(szBuf, "LISTBOXLOOK"))
		{
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBListBoxLook*>::iterator itor = m_ListBoxLookMap.find(szItem);
			if (itor != m_ListBoxLookMap.end())
			{
				pListBoxLook = (MBListBoxLook*)(*itor).second;
				pListBox->ChangeCustomLook(pListBoxLook);
			}
		}
		else if (!strcmp(szBuf, "SELECTED"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pListBox->SetSelected(bValue);
		}
		else if( !strcmp(szBuf, "ITEMHEIGHT" ))
		{
			int iValue = -1;
			childElement.GetContents(&iValue);
			pListBox->SetItemHeight( iValue );
		}
		else if(!strcmp(szBuf, "FONT" ))
		{
			childElement.GetContents( szItem );
			pListBox->SetFont( MFontManager::Get(szItem) );
		}
		else if(!strcmp(szBuf, "TEXTCOLOR" ))
		{
			pListBox->m_FontColor = GetColor( childElement );
		}
		else if(!strcmp(szBuf, "TEXTALIGN" ))
		{
			pListBox->m_FontAlign = GetAlignmentMode(childElement);
		}
		else if(!strcmp(szBuf, "NULLFRAME"))
		{
			pListBox->m_bNullFrame = true;
		}
		else if(!strcmp(szBuf, "MULTISELECT"))
		{
			pListBox->m_bMultiSelect = true;
		}
	}

	return pListBox;
}

MPicture* MIDLResource::GetPicture(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MPicture* pPicture = (MPicture*)Mint::GetInstance()->NewWidget(MINT_PICTURE, "", pParentWidget, pListener);
	InsertWidget(element, pPicture);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pPicture, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "BITMAP"))
		{
			MBitmap* pBitmap = GetBitmap(childElement);
			pPicture->SetBitmap(pBitmap);
		}
		else if (!strcmp(szBuf, "STRETCH"))
		{
			char ctemp[16];
			childElement.GetContents( ctemp );
			if( ctemp[0] == 'x' )
				pPicture->SetStretch(1);
			else if( ctemp[0] == 'y' )
				pPicture->SetStretch(2);
			else
				pPicture->SetStretch(3);
		}
		else if( !strcmp(szBuf, "DRAWMODE" ))
		{
			int mode;
			childElement.GetContents(&mode);
			DWORD cmode = pPicture->GetDrawMode();
			if( mode == 0 ) pPicture->SetDrawMode( cmode | MBM_Normal );
			else if( mode == 1 ) pPicture->SetDrawMode( cmode | MBM_FlipLR );
			else if( mode == 2 ) pPicture->SetDrawMode( cmode | MBM_FlipUD );
			else if( mode == 3 ) pPicture->SetDrawMode( cmode | MBM_RotL90 );
			else if( mode == 4 ) pPicture->SetDrawMode( cmode | MBM_RotR90 );
		}
	}

	return pPicture;
}

MScrollBar* MIDLResource::GetScrollBar(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	MScrollBarTypes ScrollBarType = MSBT_VERTICAL;
	int nMin = 0, nMax = 0;

	element.GetAttribute(szBuf, "type", "v");
	if (!strcmp(szBuf, "h"))
	{
		ScrollBarType = MSBT_HORIZONTAL;
	}

	pListener = pParentWidget = GetParentWidget(element);
	MScrollBar* pScrollBar = (MScrollBar*)Mint::GetInstance()->NewWidget(MINT_SCROLLBAR, "", pParentWidget, pListener);
	pScrollBar->SetType(ScrollBarType);
	InsertWidget(element, pScrollBar);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pScrollBar, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "MIN"))
		{
			childElement.GetContents(&nMin);
		}
		else if (!strcmp(szBuf, "MAX"))
		{
			childElement.GetContents(&nMax);
		}
		else if (!strcmp(szBuf, "VALUE"))
		{
			int nValue = 0;
			childElement.GetContents(&nValue);
			pScrollBar->SetValue(nValue);
		}
		else if (!strcmp(szBuf, "SCROLLBARLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBScrollBarLook*>::iterator itor = m_ScrollBarLookMap.find(szItem);
			if (itor != m_ScrollBarLookMap.end())
			{
				pScrollBar->ChangeCustomLook((MBScrollBarLook*)(*itor).second);
			}
		}
		else if (!strcmp(szBuf, "ARROWLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBArrowLook*>::iterator itor = m_ArrowLookMap.find(szItem);
			if (itor != m_ArrowLookMap.end())
				pScrollBar->ChangeCustomArrowLook((MBArrowLook*)(*itor).second);
		}else if (!strcmp(szBuf, "THUMBLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBThumbLook*>::iterator itor = m_ThumbLookMap.find(szItem);
			if (itor != m_ThumbLookMap.end())
				pScrollBar->ChangeCustomThumbLook((*itor).second);
		}


	}
	pScrollBar->SetMinMax(nMin, nMax);

	return pScrollBar;
}

MSlider* MIDLResource::GetSlider(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	int nMin = 0, nMax = 0;

	pListener = pParentWidget = GetParentWidget(element);
	MSlider* pSlider = (MSlider*)Mint::GetInstance()->NewWidget(MINT_SLIDER, "", pParentWidget, pListener);
	InsertWidget(element, pSlider);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pSlider, childElement, szBuf))
			continue;
		else if (!strcmp(szBuf, "MIN"))
		{
			childElement.GetContents(&nMin);
		}
		else if (!strcmp(szBuf, "MAX"))
		{
			childElement.GetContents(&nMax);
		}
		else if (!strcmp(szBuf, "VALUE"))
		{
			int nValue = 0;
			childElement.GetContents(&nValue);
			pSlider->SetValue(nValue);
		}
		else if (!strcmp(szBuf, "SLIDERLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBSliderLook*>::iterator itor = m_SliderLookMap.find(szItem);
			if (itor != m_SliderLookMap.end())
			{
				pSlider->ChangeCustomLook((MBScrollBarLook*)(*itor).second);
			}
		}
		else if(!strcmp(szBuf, "ARROWLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBArrowLook*>::iterator itor = m_ArrowLookMap.find(szItem);
			if (itor != m_ArrowLookMap.end())
			{
				pSlider->m_pUp->ChangeCustomLook((MBArrowLook*)(*itor).second);
				pSlider->m_pDown->ChangeCustomLook((MBArrowLook*)(*itor).second);
			}
		}
		else if(!strcmp(szBuf, "THUMBLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBThumbLook*>::iterator itor = m_ThumbLookMap.find(szItem);
			if (itor != m_ThumbLookMap.end())
			{
				pSlider->m_pThumb->ChangeCustomLook((MBThumbLook*)(*itor).second);				
			}
		}
	}

	pSlider->SetMinMax(nMin, nMax);

	return pSlider;

}

MGroup* MIDLResource::GetGroup(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	MBGroupLook* pGroupLook = NULL;

	pListener = pParentWidget = GetParentWidget(element);
	MGroup* pGroup = (MGroup*)Mint::GetInstance()->NewWidget(MINT_GROUP, "", pParentWidget, pListener);
	InsertWidget(element, pGroup);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pGroup, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "GROUPLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBGroupLook*>::iterator itor = m_GroupLookMap.find(szItem);
			if (itor != m_GroupLookMap.end())
			{
				pGroupLook = (MBGroupLook*)(*itor).second;
				pGroup->ChangeCustomLook(pGroupLook);
			}
		}
	}

	return pGroup;
}

MComboBox* MIDLResource::GetComboBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MComboBox* pComboBox = (MComboBox*)Mint::GetInstance()->NewWidget(MINT_COMBOBOX, "", pParentWidget, pListener);
	InsertWidget(element, pComboBox);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pComboBox, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "BUTTONLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBButtonLook*>::iterator itor = m_ButtonLookMap.find(szItem);
			if (itor != m_ButtonLookMap.end())
			{
				pComboBox->ChangeCustomLook((MBButtonLook*)(*itor).second);
			}
		}
		else if (!strcmp(szBuf, "LISTBOXLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBListBoxLook*>::iterator itor = m_ListBoxLookMap.find(szItem);
			if (itor != m_ListBoxLookMap.end())
			{
				pComboBox->m_pListBox->ChangeCustomLook((MBListBoxLook*)(*itor).second);
			}
		}
		else if (!strcmp(szBuf, "LISTITEM"))
		{
			char szItem[4096], szContents[4096];
			childElement.GetContents(szContents);
			TransText(szContents, szItem, sizeof(szItem));
			pComboBox->Add(szItem);

			bool bSelected = false;
			childElement.GetAttribute(&bSelected, "selected", false);
			if (bSelected)
			{
				pComboBox->SetSelIndex(pComboBox->GetCount()-1);
			}
		}
		else if(!strcmp(szBuf, "DROPSIZE"))
		{
			int nDropSize = 100;
			childElement.GetContents(&nDropSize);
			pComboBox->SetDropSize(nDropSize);
		}
		else if(!strcmp(szBuf, "COMBOTYPE"))
		{
			int nType = 0;
			childElement.GetContents(&nType);
			pComboBox->SetComboType(nType);
		}
		else if(!strcmp(szBuf, "COMBOFIRSTSIZE"))
		{
			int nComboSize = 0;
			childElement.GetContents(&nComboSize);
			pComboBox->SetNextComboBoxTypeSize(nComboSize);
		}
		else if(!strcmp(szBuf, "TEXTALIGN" ))
		{
			MAlignmentMode mode = GetAlignmentMode( childElement );
 			pComboBox->m_pListBox->m_FontAlign = mode;
			pComboBox->SetAlignment(mode);
		}
		else if(!strcmp(szBuf, "FONT" ))
		{
			childElement.GetContents( szBuf );
			MFont* pFont = MFontManager::Get(szBuf);
			pComboBox->m_pListBox->	SetFont( pFont );
			pComboBox->SetFont( pFont );
		}
		else if(!strcmp(szBuf, "TEXTCOLOR" ))
		{
			MCOLOR color = GetColor( childElement );
			pComboBox->m_pListBox->m_FontColor = color;
            pComboBox->SetTextColor( color );
		}
		else if( !strcmp(szBuf, "ITEMHEIGHT" ))
		{
			int iValue = -1;
			childElement.GetContents(&iValue);
			pComboBox->m_pListBox->SetItemHeight( iValue );
		}
		else if(!strcmp(szBuf,"DROPUNDER"))
		{
			bool bValue;
			childElement.GetContents(&bValue);
			pComboBox->m_bAutoDrop = false;
			pComboBox->m_bDropUnder = bValue;
		}
	}

	return pComboBox;
}

MMenuItem* MIDLResource::GetMenuItem(MPopupMenu* pPopupMenu, MXmlElement& element)
{
	MXmlElement childElement;
	char szTagName[1024];
	MMenuItem* pMenuItem = (MMenuItem*)Mint::GetInstance()->NewWidget(MINT_MENUITEM, "", NULL, NULL);
	pPopupMenu->AddMenuItem(pMenuItem);
	
	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;
		else if (!strcmp(szTagName, "TEXT"))
		{
			char szItem[1024];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);
			pMenuItem->SetText(szItem);
		}
		else if (!strcmp(szTagName, "SUBMENU"))
		{
			GetSubMenu(pMenuItem, childElement);
		}
	}

	return pMenuItem;	
}

MPopupMenu* MIDLResource::GetSubMenu(MMenuItem* pParentMenuItem, MXmlElement& element)
{
	MPopupMenu* pSubMenu = pParentMenuItem->CreateSubMenu();
	MXmlElement childElement;
	char szTagName[1024];
	int iCount = element.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;
		else if (!strcmp(szTagName, "MENUITEM"))
		{
			GetMenuItem(pSubMenu, childElement);
		}
	}

	return pSubMenu;
}

MPopupMenu* MIDLResource::GetPopupMenu(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);

	char szType[256];
	MPopupMenuTypes t = MPMT_VERTICAL;

	element.GetAttribute(szType, "type", "v");
	if (!strcmp(szType, "h")) t = MPMT_HORIZONTAL;

	MPopupMenu* pPopupMenu = (MPopupMenu*)Mint::GetInstance()->NewWidget(MINT_POPUPMENU, "", pParentWidget, pListener);
	pPopupMenu->SetType(t);
	InsertWidget(element, pPopupMenu);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pPopupMenu, childElement, szBuf)) continue;

		else if (!strcmp(szBuf, "MENUITEM"))
		{
			GetMenuItem(pPopupMenu, childElement);
		}
	}

	pPopupMenu->Show(false);
	return pPopupMenu;
}

MAniBitmap* MIDLResource::GetAniBitmap(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MAniBitmap* pAniBitmap = new MAniBitmap;

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (!strcmp(szBuf, "BITMAP"))
		{
			MBitmap* pBitmap = GetBitmap(childElement);
			if(pBitmap)
				pAniBitmap->Add(pBitmap);
		}
		else if (!strcmp(szBuf, "DELAY"))
		{
			int nDelay = 0;
			childElement.GetContents(&nDelay);
			pAniBitmap->SetDelay(nDelay);
		}
	}

	char szItem[256];
	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");
	if(!m_AniBitmapMap.insert(map<string, MAniBitmap*>::value_type(string(szItem), pAniBitmap)).second)
		OutputDebugString("insert widget failed.\n");

	return pAniBitmap;
}

MAnimation* MIDLResource::GetAnimation(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	char szAniBitmap[256];
	MAniBitmap* pAniBitmap = NULL;

	memset(szAniBitmap, 0, sizeof(szAniBitmap));
	MWidget* pParentWidget = GetParentWidget(element);

	MAnimation* pAnimation = (MAnimation*)Mint::GetInstance()->NewWidget(MINT_ANIMATION, "", pParentWidget, NULL);
	InsertWidget(element, pAnimation);


	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pAnimation, childElement, szBuf)) continue;

		else if (!strcmp(szBuf, "ANIMTEMPLATE"))
		{
			childElement.GetContents(szAniBitmap);
		}
		else if (!strcmp(szBuf, "PLAYMODE"))
		{
			char szPlayMode[256];
			childElement.GetContents(szPlayMode);
			if (!strcmp(szPlayMode, "forwardonce"))
			{
				pAnimation->m_nPlayMode = MAPM_FORWARDONCE;
			}
			else if (!strcmp(szPlayMode, "forwardbackward"))
			{
				pAnimation->m_nPlayMode = MAPM_FORWARDNBACKWARD;
			}
			else if (!strcmp(szPlayMode, "repetition"))
			{
				pAnimation->m_nPlayMode = MAPM_REPETITION;
			}
			else if (!strcmp(szPlayMode, "stop"))
			{
				pAnimation->m_nPlayMode = MAPM_FORWARDONCE;
				pAnimation->m_bRunAnimation = false;
			}
		}
		else if (!strcmp(szBuf, "RUN"))
		{
			bool bValue = true;
			childElement.GetContents( &bValue);
			pAnimation->m_bRunAnimation = bValue;
		}
	}

	map<string, MAniBitmap*>::iterator itor = m_AniBitmapMap.find(szAniBitmap);
	if (itor != m_AniBitmapMap.end())
	{
		pAniBitmap = ((MAniBitmap*)(*itor).second);
		pAnimation->SetAniBitmap(pAniBitmap);
	}

	
	return pAnimation;
}

MCursor* MIDLResource::GetCursor(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	char szItem[256];

	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");

	MWidget* pParentWidget = GetParentWidget(element);
	MCursor* pCursor = NULL;

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (!strcmp(szBuf, "ANIMTEMPLATE"))
		{
			char szAniBitmap[256];
			MAniBitmap* pAniBitmap = NULL;
			childElement.GetContents(szAniBitmap);
			pAniBitmap = (m_AniBitmapMap.find(szAniBitmap))->second;
			if (pCursor == NULL) pCursor = new MAniBitmapCursor(szItem, pAniBitmap);
		}
		else if (!strcmp(szBuf, "BITMAP"))
		{
			char szBitmap[256];
			MBitmap* pBitmap = NULL;
			childElement.GetContents(szBitmap);
			pBitmap = MBitmapManager::Get(szBitmap);
			if (pCursor == NULL) pCursor = new MBitmapCursor(szItem, pBitmap);
		}
	}

	if (pCursor != NULL) 
	{
		if (pParentWidget!=NULL) pParentWidget->SetCursor(pCursor);
		else (MWidget*)m_pParent->SetCursor(pCursor);

		MCursorSystem::Add(pCursor);
	}
	return pCursor;
}

MBmLabel* MIDLResource::GetBmLabel(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[1024];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MBmLabel* pBmLabel = (MBmLabel*)Mint::GetInstance()->NewWidget(MINT_BMLABEL, "", pParentWidget, pListener);
	InsertWidget(element, pBmLabel);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pBmLabel, childElement, szBuf)) continue;
		else if (!strcmp(szBuf, "FONTSIZE"))
		{
			pBmLabel->SetCharSize(GetSize(childElement));
		}
		else if (!strcmp(szBuf, "BITMAP"))
		{
			MBitmap* pBitmap = GetBitmap(childElement);

			if (pBitmap != NULL)
			{
				pBmLabel->SetLabelBitmap(pBitmap);
			}
		}
	}
	return pBmLabel;
}

MFont* MIDLResource::CreateFont(char* szAliasName, char* szFontName, int nHeight,
								bool bBold, bool bItalic, int nOutlineStyle, bool bAntialiasing, DWORD nColorArg1, DWORD nColorArg2)
{
	return NULL;
}

MFont* MIDLResource::GetFont(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	char szItem[256];
	char szName[256];
	int nHeight = 10;
	bool bBold = false;
	bool bItalic = false;
	int nOutlineStyle = 0;
	DWORD nColorArg1 = 0;
	DWORD nColorArg2 = 0;
	int a,r,g,b; a=r=g=b=0;
	bool bAntialiasing = false;

	element.GetAttribute(szItem, IDL_ATTR_ITEM, "");

	MFont* pFont = NULL;

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (!strcmp(szBuf, "FONTSET"))
		{
			childElement.GetContents(szName);
		}
		else if (!strcmp(szBuf, "FONTHEIGHT"))
		{
			childElement.GetContents(&nHeight);
		}
		else if (!strcmp(szBuf, "BOLD"))
		{
			bBold = true;
		}
		else if (!strcmp(szBuf, "ITALIC"))
		{
			bItalic = true;
		}
		else if (!strcmp(szBuf, "OUTLINESTYLE"))
		{
			childElement.GetContents(&nOutlineStyle);
		}
		else if (!strcmp(szBuf, "ANTIALIASING"))
		{
			bAntialiasing = true;
		}
		else if (!strcmp(szBuf, "COLORARG1"))
		{
			childElement.GetContents((int*)&nColorArg1);
			childElement.GetChildContents(&a, "A");
			childElement.GetChildContents(&r, "R");
			childElement.GetChildContents(&g, "G");
			childElement.GetChildContents(&b, "B");
			nColorArg1 = MINT_ARGB(a,r,g,b);
		}
		else if (!strcmp(szBuf, "COLORARG2"))
		{
			childElement.GetContents((int*)&nColorArg2);
			childElement.GetChildContents(&a, "A");
			childElement.GetChildContents(&r, "R");
			childElement.GetChildContents(&g, "G");
			childElement.GetChildContents(&b, "B");
			nColorArg2 = MINT_ARGB(a,r,g,b);
		}
	}

	pFont = CreateFont(szItem, szName, nHeight, bBold, bItalic, nOutlineStyle, bAntialiasing, nColorArg1, nColorArg2);
	if (pFont != NULL) 
	{
		MFontManager::Add(pFont);
	}
	return pFont;
}

MHotKey* MIDLResource::GetHotKey(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MHotKey* pHotKey = (MHotKey*)Mint::GetInstance()->NewWidget(MINT_HOTKEY, "", pParentWidget, pListener);
	InsertWidget(element, pHotKey);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pHotKey, childElement, szBuf)) continue;


		if (!strcmp(szBuf, "EDITLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBEditLook*>::iterator itor = m_EditLookMap.find(szItem);
			if (itor != m_EditLookMap.end())
			{
				pHotKey->ChangeCustomLook((MBEditLook*)(*itor).second);
			}
		}

	}

	return pHotKey;
}

MTextArea* MIDLResource::GetTextArea(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MTextArea* pTextArea = (MTextArea*)Mint::GetInstance()->NewWidget(MINT_TEXTAREA, "", pParentWidget, pListener);
	InsertWidget(element, pTextArea);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pTextArea, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "EDITABLE"))
		{
			bool bEditable = true;
			childElement.GetContents(&bEditable);
			pTextArea->SetEditable(bEditable);
		} 
		else if (!strcmp(szBuf, "RESIZABLE"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pTextArea->SetResizable(bValue);
		}
		else if (!strcmp(szBuf, "TEXTOFFSET"))
		{
			pTextArea->SetTextOffset(GetPoint(childElement));
		} 
		else if (!strcmp(szBuf, "TEXTCOLOR"))
		{
			pTextArea->SetTextColor(GetColor(childElement));
		} 
		else if (!strcmp(szBuf, "TEXTAREALOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBTextAreaLook*>::iterator itor = m_TextAreaLookMap.find(szItem);
			if (itor != m_TextAreaLookMap.end())
			{
				MBTextAreaLook* pLook = (MBTextAreaLook*)(*itor).second;
				pTextArea->ChangeCustomLook(pLook);
				pTextArea->SetFont(pLook->m_pFont);
			}
		}
		else if (!strcmp(szBuf, "MAXLENGTH"))
		{
			int nMaxLength;
			childElement.GetContents(&nMaxLength);
			pTextArea->SetMaxLen(nMaxLength);
		}
		else if (!strcmp(szBuf, "INDENTATION"))
		{
			int nIndentation;
			childElement.GetContents(&nIndentation);
			pTextArea->SetIndentation(nIndentation);
		}
		else if (!strcmp(szBuf, "SCROLLBAR"))
		{
			bool bValue = true;
			childElement.GetContents(&bValue);
			pTextArea->SetScrollBarEnable(bValue);
		}
	}
	return pTextArea;
}

MTabCtrl* MIDLResource::GetTabCtrl(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MTabCtrl* pTabCtrl= (MTabCtrl*)Mint::GetInstance()->NewWidget(MINT_TABCTRL, "", pParentWidget, pListener);
	InsertWidget(element, pTabCtrl);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pTabCtrl, childElement, szBuf)) continue;

		if (!strcmp(szBuf, "TAB"))
		{
			MWidget *pButton = NULL;
			MWidget *pFrame = NULL;

			char szWidgetName[256];
			childElement.GetAttribute(szWidgetName, "button");
			pButton = FindWidget(szWidgetName);
			if(pButton != NULL && 
				(strcmp(pButton->GetClassName(),MINT_BUTTON)==0 ||
				strcmp(pButton->GetClassName(),MINT_BMBUTTON)==0))
			{
				childElement.GetAttribute(szWidgetName, "widget");
				pFrame = FindWidget(szWidgetName);
				pTabCtrl->Add((MButton*)pButton,pFrame);
			}
		}
		else if (!strcmp(szBuf, "TABCONTROLLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBTabCtrlLook*>::iterator itor = m_TabCtrlLookMap.find(szItem);
			if (itor != m_TabCtrlLookMap.end())
			{
				pTabCtrl->ChangeCustomLook((MBTabCtrlLook*)(*itor).second);
			}
		}
	}
	return pTabCtrl;
}

MPanel* MIDLResource::GetPanel(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	MPanel* pPanel = (MPanel*)Mint::GetInstance()->NewWidget(MINT_PANEL, "", pParentWidget, pListener);
	InsertWidget(element, pPanel);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pPanel, childElement, szBuf)) continue;

		if (!_stricmp(szBuf, "BORDERSTYLE"))
		{
			char szBorderStyle[256];
			childElement.GetContents(szBorderStyle);
			if (!_stricmp(szBorderStyle, "none")) pPanel->SetBorderStyle(MBS_NONE);
			else if (!_stricmp(szBorderStyle, "single")) pPanel->SetBorderStyle(MBS_SINGLE);
		}
		else if(!_stricmp(szBuf, "BORDERCOLOR"))
		{
			MCOLOR color = GetColor(childElement);
			pPanel->SetBorderColor(color);
		}
		else if(!_stricmp(szBuf, "BACKGROUND"))
		{
			MCOLOR color = GetColor(childElement);
			pPanel->SetBackgroundColor(color);
		}
	}
	return pPanel;
}

void MIDLResource::Parse(MXmlElement& element)
{
	auto MIDLResourceParse = MBeginProfile("MIDLResource::Parse");
	char szTagName[256];
	element.GetTagName(szTagName);

	if (!_stricmp(szTagName, "BUTTONLOOKTEMPLATE"))
	{
		GetButtonLook(element);
	}
	else if (!_stricmp(szTagName, "GROUPLOOKTEMPLATE"))
	{
		GetGroupLook(element);
	}
	else if (!_stricmp(szTagName, "FRAMELOOKTEMPLATE"))
	{
		GetFrameLook(element);
	}
	else if (!_stricmp(szTagName, "LABELLOOKTEMPLATE"))
	{
		GetLabelLook(element);
	}
	else if (!_stricmp(szTagName, "EDITLOOKTEMPLATE"))
	{
		GetEditLook(element);
	}
	else if (!_stricmp(szTagName, "LISTBOXLOOKTEMPLATE"))
	{
		GetListBoxLook(element, 0);
	}
	else if (!_stricmp(szTagName, "COMBOLISTBOXLOOKTEMPLATE"))
	{
		GetListBoxLook(element, 1);
	}
	else if (!_stricmp(szTagName, "ARROWLOOKTEMPLATE"))
	{
		GetArrowLook(element);
	}
	else if (!_stricmp(szTagName, "THUMBLOOKTEMPLATE"))
	{
		GetThumbLook(element);
	}
	else if (!_stricmp(szTagName, "SCROLLBARLOOKTEMPLATE"))
	{
		GetScrollBarLook(element);
	}
	else if (!_stricmp(szTagName, "SLIDERLOOKTEMPLATE"))
	{
		GetSliderLook(element);
	}
	else if (!_stricmp(szTagName, "TEXTAREALOOKTEMPLATE"))
	{
		GetTextAreaLook(element);
	}
	else if( !_stricmp(szTagName, "SLIDERTHUMBLOOKTEMPLATE" ))
	{
		GetSliderThumbLook( element );
	}
	else if( !_stricmp(szTagName, "TABCTRLLOOKTEMPLATE" ))
	{
		GetTabCtrlLook( element );
	}
	else if (!_stricmp(szTagName, "FONTTEMPLATE"))
	{
		GetFont(element);
	}
	else if (!_stricmp(szTagName, "FRAME"))
	{
		GetFrame(element);
	}
	else if (!_stricmp(szTagName, "LABEL"))
	{
		GetLabel(element);
	}
	else if (!_stricmp(szTagName, "BMLABEL"))
	{
		GetBmLabel(element);
	}
	else if (!_stricmp(szTagName, "BUTTON"))
	{
		GetButton(element);
	}
	else if (!_stricmp(szTagName, "BMBUTTON"))
	{
		GetBmButton(element);
	}
	else if (!_stricmp(szTagName, "EDIT"))
	{
		GetEdit(element);
	}
	else if (!_stricmp(szTagName, "LISTBOX"))
	{
		GetListBox(element);
	}
	else if (!_stricmp(szTagName, "PICTURE"))
	{
		GetPicture(element);
	}
	else if (!_stricmp(szTagName, "SCROLLBAR"))
	{
		GetScrollBar(element);
	}
	else if (!_stricmp(szTagName, "SLIDER"))
	{
		GetSlider(element);
	}
	else if (!_stricmp(szTagName, "GROUP"))
	{
		GetGroup(element);
	}
	else if (!_stricmp(szTagName, "COMBOBOX"))
	{
		GetComboBox(element);
	}
	else if (!_stricmp(szTagName, "POPUPMENU"))
	{
		GetPopupMenu(element);
	}
	else if (!_stricmp(szTagName, "ANIMATIONTEMPLATE"))
	{
		GetAniBitmap(element);
	}
	else if (!_stricmp(szTagName, "ANIMATION"))
	{
		GetAnimation(element);
	}
	else if (!_stricmp(szTagName, "CURSOR"))
	{
		GetCursor(element);
	}
	else if (!_stricmp(szTagName, "HOTKEY"))
	{
		GetHotKey(element);
	}
	else if (!_stricmp(szTagName, "TEXTAREA"))
	{
		GetTextArea(element);
	}
	else if (!_stricmp(szTagName, "TABCONTROL"))
	{
		GetTabCtrl(element);
	}
	else if (!_stricmp(szTagName, "PANEL"))
	{
		GetPanel(element);
	}
	else if (!_stricmp(szTagName, "BITMAPALIAS"))
	{
		GetBitmapAlias(element);
	}
	else if (!_stricmp(szTagName, "REBOUNDS"))
	{
		GetRebounds(element);
	}
	MEndProfile(MIDLResourceParse);
}

bool MIDLResource::LoadFromFile(const char* szFileName, MWidget* pParent, MZFileSystem *pfs)
{
	m_pParent = pParent;
	if (m_pParent == NULL) m_pParent = Mint::GetInstance()->GetMainFrame();

	MXmlDocument	xmlDocument;
	MXmlElement		rootElement, childElement;
	char			szBuf[4096];

	if (!xmlDocument.LoadFromFile(szFileName, pfs))
		return false;

	rootElement = xmlDocument.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);

		childElement.GetTagName(szBuf);
		if (szBuf[0] == '#') {
			continue;
		}
		else if (!strcmp(szBuf, "INCLUDE"))
		{
			auto pdest = strrchr(szFileName, '\\');
			if (pdest == NULL) {
				pdest = strrchr(szFileName, '/');
			}

			char szContents[256], szFileName2[256];
			childElement.GetContents(szContents);

			memset(szFileName2, 0, sizeof(szFileName2));
			if (pdest != NULL)
			{
				int t = pdest - szFileName + 1;
				strncpy_s(szFileName2, szFileName, t);
			}
			strcat_s(szFileName2, szContents);

			LoadFromFile(szFileName2, m_pParent, pfs);
		}

		Parse(childElement);
	}

	xmlDocument.Destroy();

	return true;
}

bool MIDLResource::SaveToFile(const char* szFileName)
{
	MXmlDocument	xmlDocument;
	MXmlElement		rootElement, childElement;
	if (!xmlDocument.Create()) return false;
	xmlDocument.CreateProcessingInstruction();
	rootElement = xmlDocument.CreateElement(IDL_ROOT);
	rootElement.AppendText("\n\t");
	xmlDocument.AppendChild(rootElement);

	for (MWidgetMMap::iterator itor = m_WidgetMap.begin(); itor != m_WidgetMap.end(); ++itor)
	{
		MWidget* pWidget = (*itor).second;
		if (!strcmp(pWidget->GetClassName(), MINT_LABEL))
		{
			SetLabel(rootElement, (MLabel*)pWidget);
		}
	}

	rootElement.AppendText("\n");
	if (!xmlDocument.SaveToFile(szFileName)) 
	{
		xmlDocument.Destroy();
		return false;
	}
	xmlDocument.Destroy();
	return true;
}

void MIDLResource::ClearLooks()
{
#define CLEAR(x) while(!x.empty()) { delete (*x.begin()).second; x.erase(x.begin()); }

	CLEAR(m_LabelLookMap);
	CLEAR(m_ButtonLookMap);
	CLEAR(m_GroupLookMap);
	CLEAR(m_FrameLookMap);
	CLEAR(m_EditLookMap);
	CLEAR(m_ListBoxLookMap);
	CLEAR(m_ScrollBarLookMap);
	CLEAR(m_ArrowLookMap);
	CLEAR(m_ThumbLookMap);
	CLEAR(m_AniBitmapMap);
	CLEAR(m_TabCtrlLookMap);
	CLEAR(m_TextAreaLookMap);
	CLEAR(m_ButtonGroupMap);
}

void MIDLResource::Clear()
{
	ClearLooks();

	mlog("ClearLooks() End : \n");

	m_WidgetMap.Clear();

	mlog("m_WidgetMap.Clear() End : \n");

	MCursorSystem::Destroy();

	mlog("MCursorSystem::Destroy() End : \n");
}

void MIDLResource::SetLabel(MXmlElement& element, MLabel* pLabel)
{
	MXmlElement childElement, labelElement;
	labelElement = element.CreateChildElement(IDL_LABEL);

	MRECT rect = pLabel->GetRect();
	SetRect(labelElement, &rect, "BOUNDS");
	pLabel->m_IDLRect = rect;

	MCOLOR color = pLabel->GetTextColor();
	SetColor(labelElement, &color, "TEXTCOLOR");

	childElement = labelElement.CreateChildElement(IDL_TEXT);
	childElement.SetContents(pLabel->GetText());
}

void MIDLResource::SetPoint(MXmlElement& element, MPOINT* pPoint, const char* szTagName)
{
	MXmlElement pointElement, childElement;
	pointElement = element.CreateChildElement(szTagName);
	childElement = pointElement.CreateChildElement("X");
	childElement.SetContents(pPoint->x);
	childElement = pointElement.CreateChildElement("Y");
	childElement.SetContents(pPoint->y);
}

void MIDLResource::SetRect(MXmlElement& element, MRECT* pRect, const char* szTagName)
{
	MXmlElement rectElement, childElement;
	rectElement = element.CreateChildElement(szTagName);
	childElement = rectElement.CreateChildElement("X");
	childElement.SetContents(pRect->x);
	childElement = rectElement.CreateChildElement("Y");
	childElement.SetContents(pRect->y);
	childElement = rectElement.CreateChildElement("W");
	childElement.SetContents(pRect->w);
	childElement = rectElement.CreateChildElement("H");
	childElement.SetContents(pRect->h);
}

void MIDLResource::SetSize(MXmlElement& element, MSIZE* pSize, const char* szTagName)
{
	MXmlElement sizeElement, childElement;
	sizeElement = element.CreateChildElement(szTagName);
	childElement = sizeElement.CreateChildElement("W");
	childElement.SetContents(pSize->w);
	childElement = sizeElement.CreateChildElement("H");
	childElement.SetContents(pSize->h);
}

void MIDLResource::SetColor(MXmlElement& element, MCOLOR* pColor, const char* szTagName)
{
	MXmlElement colorElement, childElement;
	colorElement = element.CreateChildElement(szTagName);
	childElement = colorElement.CreateChildElement("R");
	childElement.SetContents(pColor->r);
	childElement = colorElement.CreateChildElement("G");
	childElement.SetContents(pColor->g);
	childElement = colorElement.CreateChildElement("B");
	childElement.SetContents(pColor->b);
	childElement = colorElement.CreateChildElement("ALPHA");
	childElement.SetContents(pColor->a);
}

MWidget* MIDLResource::FindWidget(string szItem)
{
	MWidgetMMap::iterator itor = m_WidgetMap.find(szItem);
	if (itor != m_WidgetMap.end())
	{
		return (MWidget*)(*itor).second;
	}
	else
	{
		return NULL;
	}
}

void MIDLResource::FindWidgets(MWidgetList& widgetList, string szItem)
{
	pair<MWidgetMMap::iterator, MWidgetMMap::iterator> p = m_WidgetMap.equal_range(szItem);

	for (MWidgetMMap::iterator itor = p.first; itor != p.second; ++itor)
	{
		MWidget* pWidget = (*itor).second;
		widgetList.push_back(pWidget);
	}
}

MFrame*	MIDLResource::CreateFrame(const char* szName, MWidget* pParent, MListener* pListener)
{
	MFrame* pFrame = (MFrame*)Mint::GetInstance()->NewWidget(MINT_FRAME, szName, pParent, pListener);
	return pFrame;
}

MBFrameLook* MIDLResource::FindFrameLook(string szItem)
{
	map<string, MBFrameLook*>::iterator itor = m_FrameLookMap.find(szItem);
	if (itor != m_FrameLookMap.end())
	{
		return (*itor).second;
	}

	return NULL;
}

void MIDLResource::InsertWidget(const char* pItemName, MWidget* pWidget )
{
	m_WidgetMap.insert(MWidgetMMap::value_type(string(pItemName), pWidget) );
}

void MIDLResource::TransText(const char* szSrc, char* szOut, int maxlen)
{
	strcpy_safe(szOut, maxlen, szSrc);
}

void MIDLResource::GetRebounds(MXmlElement& element)
{
	char szItem[256] = {0, };

	MXmlElement childElement;

	element.GetAttribute(szItem, "item");
	MRECT rt = GetRect(element);

	MWidgetList widget_list;
	FindWidgets(widget_list, string(szItem));

	for (MWidgetList::iterator itor = widget_list.begin(); itor != widget_list.end(); ++itor)
	{
		MWidget* pWidget = (*itor);
		pWidget->SetBounds( rt);
		pWidget->m_IDLRect = rt;
	}
}