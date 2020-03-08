#include "stdafx.h"
#include "ZIDLResource.h"
#include "ZGame.h"
#include "Mint4R2.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "MBScrollBarLook.h"
#include "ZFrame.h"
#include "ZMapListBox.h"
#include "ZScoreListBox.h"
#include "ZMeshView.h"
#include "ZMeshViewList.h"
#include "ZCharacterView.h"
#include "ZCharacterViewList.h"
#include "ZEquipmentListBox.h"
#include "ZStageInfoBox.h"
#include "ZItemSlotView.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZCanvas.h"
#include "ZPlayerSelectListBox.h"
#include "ZBmNumLabel.h"
#include "ZClanListBox.h"
#include "ZServerView.h"
#include "ZStringResManager.h"
#include "ZActionKey.h"
#include "RGMain.h"

ZIDLResource::ZIDLResource() = default;
ZIDLResource::~ZIDLResource() = default;

ZCanvas* ZIDLResource::GetCanvas(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	ZCanvas* pCanvas = new ZCanvas("", pParentWidget, pListener);
	InsertWidget(element, pCanvas);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pCanvas, childElement, szBuf);
	}

	return pCanvas;
}

ZMapListBox* ZIDLResource::GetMapListBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	ZMapListBox* pListBox = new ZMapListBox("", pParentWidget, pListener);
	InsertWidget(element, pListBox);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pListBox, childElement, szBuf);
	}

	return pListBox;
}

ZScoreBoardFrame* ZIDLResource::GetScoreBoardFrame(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;
	MBFrameLook* pFrameLook = NULL;

	pListener = pParentWidget = GetParentWidget(element);
	ZScoreBoardFrame* pFrame = new ZScoreBoardFrame("", pParentWidget, pListener);
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
				pFrame->ChangeCustomLook((MFrameLook*)pFrameLook);
			}
		}
	}


	return pFrame;
}


ZScoreListBox* ZIDLResource::GetScoreListBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	ZScoreListBox* pListBox = new ZScoreListBox("", pParentWidget, pListener);
	InsertWidget(element, pListBox);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pListBox, childElement, szBuf);
	}

	return pListBox;
}

ZItemSlotView* ZIDLResource::GetItemSlot(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZItemSlotView* pWidget = new ZItemSlotView("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	pWidget->SetParts(MMCIP_END);
	char szParts[64];
	element.GetAttribute(szParts, "parts", "");
	if (!_stricmp(szParts, "head")) pWidget->SetParts(MMCIP_HEAD);
	else if (!_stricmp(szParts, "chest")) pWidget->SetParts(MMCIP_CHEST);
	else if (!_stricmp(szParts, "hands")) pWidget->SetParts(MMCIP_HANDS);
	else if (!_stricmp(szParts, "legs")) pWidget->SetParts(MMCIP_LEGS);
	else if (!_stricmp(szParts, "feet")) pWidget->SetParts(MMCIP_FEET);
	else if (!_stricmp(szParts, "fingerl")) pWidget->SetParts(MMCIP_FINGERL);
	else if (!_stricmp(szParts, "fingerr")) pWidget->SetParts(MMCIP_FINGERR);
	else if (!_stricmp(szParts, "melee")) pWidget->SetParts(MMCIP_MELEE);
	else if (!_stricmp(szParts, "primary")) pWidget->SetParts(MMCIP_PRIMARY);
	else if (!_stricmp(szParts, "secondary")) pWidget->SetParts(MMCIP_SECONDARY);
	else if (!_stricmp(szParts, "custom1")) pWidget->SetParts(MMCIP_CUSTOM1);
	else if (!_stricmp(szParts, "custom2")) pWidget->SetParts(MMCIP_CUSTOM2);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pWidget, childElement, szBuf);

		if (!strcmp(szBuf, "BITMAP"))
		{
			MBitmap* pBitmap = GetBitmap(childElement);
			if (pBitmap != NULL)
			{
				pWidget->SetBackBitmap(pBitmap);
			}
		}
		if( !strcmp(szBuf, "STRETCH") )
		{
			pWidget->SetStretch( true );
		}
	}

	return pWidget;

}
ZMeshView* ZIDLResource::GetMeshView(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZMeshView* pWidget = new ZMeshView("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pWidget, childElement, szBuf);

		if (!strcmp(szBuf, "BUTTONLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBButtonLook*>::iterator itor = m_ButtonLookMap.find(szItem);
			if (itor != m_ButtonLookMap.end())
			{
				pWidget->ChangeCustomLook((MButtonLook*)(*itor).second);
			}
			pWidget->SetLook(true);
		}
	}

	return pWidget;
}

ZMeshViewList* ZIDLResource::GetMeshViewList(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZMeshViewList* pWidget = new ZMeshViewList("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if(GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;

		if(!strcmp(szBuf, "ITEMWIDTH")){
			int nWidth = 80;
			childElement.GetContents(&nWidth);
			pWidget->SetItemWidth(nWidth);
		}

	}

	return pWidget;
}

ZCharacterView* ZIDLResource::GetCharacterView(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZCharacterView* pWidget = new ZCharacterView("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		GetCommonWidgetProperty(pWidget, childElement, szBuf);

		if(!strcmp(szBuf, "VISIBLEEQUIPMENT")){
			bool bVisibleEquipment = false;
			childElement.GetContents(&bVisibleEquipment);
			pWidget->SetVisibleEquipment(bVisibleEquipment);
		}
	}

	return pWidget;
}

ZCharacterViewList* ZIDLResource::GetCharacterViewList(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZCharacterViewList* pWidget = new ZCharacterViewList("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if(GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;

		if(!strcmp(szBuf, "ITEMWIDTH")){
			int nWidth = 80;
			childElement.GetContents(&nWidth);
			pWidget->SetItemWidth(nWidth);
		}
	}

	return pWidget;
}

ZEquipmentListBox* ZIDLResource::GetEquipmentListBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZEquipmentListBox* pWidget = new ZEquipmentListBox("", pParentWidget, pParentWidget);
	pWidget->m_FontAlign = MAM_VCENTER;

	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;
		else if (!strcmp(szBuf, "LISTBOXLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBListBoxLook*>::iterator itor = m_ListBoxLookMap.find(szItem);
			if (itor != m_ListBoxLookMap.end())
			{
				MBListBoxLook* pListBoxLook = NULL;
				pListBoxLook = (*itor).second;
				pWidget->ChangeCustomLook(pListBoxLook);
			}
		}else if (!strcmp(szBuf, "SCROLLBARLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBScrollBarLook*>::iterator itor = m_ScrollBarLookMap.find(szItem);
			if (itor != m_ScrollBarLookMap.end())
				pWidget->GetScrollBar()->ChangeCustomLook((*itor).second);
		}else if (!strcmp(szBuf, "ARROWLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBArrowLook*>::iterator itor = m_ArrowLookMap.find(szItem);
			if (itor != m_ArrowLookMap.end())
				pWidget->GetScrollBar()->ChangeCustomArrowLook((*itor).second);
		}else if (!strcmp(szBuf, "THUMBLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBThumbLook*>::iterator itor = m_ThumbLookMap.find(szItem);
			if (itor != m_ThumbLookMap.end())
				pWidget->GetScrollBar()->ChangeCustomThumbLook((*itor).second);
		}
	}

	return pWidget;
}

ZStageInfoBox* ZIDLResource::GetStageInfoBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZStageInfoBox* pWidget = new ZStageInfoBox("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;
		else if (!strcmp(szBuf, "STAGEINFOITEMLOOK"))
		{
			char szItem[256];
			memset(szItem, 0, sizeof(szItem));
			childElement.GetContents(szItem);

			map<string, MBListBoxLook*>::iterator itor = m_ListBoxLookMap.find(szItem);
			if (itor != m_ListBoxLookMap.end())
			{
				MBListBoxLook* pListBoxLook = NULL;
				pListBoxLook = (*itor).second;
				pWidget->SetLook(pListBoxLook);
			}
		}
	}

	return pWidget;
}

ZRoomListBox* ZIDLResource::GetRoomListBox( MXmlElement& element )
{
	MXmlElement childElement;
	char szBuf[4096];
	char szAttr[4096];
	
	MWidget* pParentWidget = GetParentWidget(element);
	ZRoomListBox* pWidget = new ZRoomListBox("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if(GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;
		else	if( strcmp(szBuf, "BITMAP") == 0)
		{
			childElement.GetAttribute(szAttr, "type" );
			if( strcmp(szAttr,"frame") == 0)
			{
				pWidget->SetFrameImage( GetBitmap(childElement));				
			}
			else if( strcmp(szAttr, "back") == 0 )
			{
				childElement.GetContents(szAttr);

				MBitmap* pBitmap =GetBitmap(childElement);
				if (pBitmap)
				{
					pWidget->SetBannerImage(szAttr, pBitmap);
					GetRGMain().AddMapBanner(szAttr, pBitmap);
				}
			}
 			else if( _stricmp( szAttr,"icon" )==0 )
			{
				int mode;
				childElement.GetAttribute(&mode, "mode");
				childElement.GetContents(szAttr);
				MBitmap* pBitmap = GetBitmap(childElement);
				if(pBitmap)
					pWidget->SetIconImage( (MMATCH_GAMETYPE)mode, pBitmap );
			}
		}
		else if ( strcmp(szBuf, "SIZE") == 0 )
		{
			childElement.GetAttribute(szAttr, "type" );
			if( strcmp(szAttr,"width") == 0)
			{
				float w;
				childElement.GetContents( &w );
				pWidget->SetWidth( w );
			}
			if( strcmp(szAttr,"height") == 0)
			{
				float h;
				childElement.GetContents( &h );
				pWidget->SetHeight( h );
			}
		}
	}
	return pWidget;
}

ZClanListBox* ZIDLResource::GetClanListBox( MXmlElement& element )
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZClanListBox* pWidget = new ZClanListBox("", pParentWidget, pParentWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if(GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;
	}
	return pWidget;
}

ZServerView* ZIDLResource::GetServerView(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[ 4096];

	MWidget* pParentWidget = GetParentWidget( element);
	ZServerView* pWidget = new ZServerView( "", pParentWidget, pParentWidget);
	InsertWidget( element, pWidget);

	int iCount = element.GetChildNodeCount();
	for (int i = 0;  i < iCount;  i++)
	{
		memset(szBuf, 0, sizeof( szBuf));
		childElement = element.GetChildNode( i);
		childElement.GetTagName( szBuf);

		GetCommonWidgetProperty( pWidget, childElement, szBuf);
	}

	return pWidget;
}

ZPlayerListBox* ZIDLResource::GetPlayerListBox( MXmlElement& element )
{
	MXmlElement childElement;
	char szBuf[4096];
	char szAttr[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZPlayerListBox* pWidget = new ZPlayerListBox("", pParentWidget, pParentWidget);
	pWidget->SetListener(pWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	bool bMode1 = false;

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pWidget, childElement, szBuf)) {
			continue;
		}
		else if( strcmp(szBuf, "SIZE" ) == 0 )
		{
			childElement.GetAttribute(szAttr, "type");
			if( strcmp(szAttr,"width") == 0 )
			{
				float temp;
				childElement.GetContents(&temp);
				pWidget->SetWidth( temp );
			}
			else if( strcmp(szAttr,"height") == 0 )
			{
				float temp;
				childElement.GetContents(&temp);
				pWidget->SetHeight( temp );
			}
		}
		else if(strcmp(szBuf, "MODE1" ) == 0 ) {
			bMode1 = true;
		}
	}

	if(bMode1==false)
		pWidget->InitUI(ZPlayerListBox::PlayerListMode::Channel);
	else
		pWidget->InitUI(ZPlayerListBox::PlayerListMode::Stage);

	return pWidget;
}

ZPlayerSelectListBox* ZIDLResource::GetPlayerSelectListBox(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];

	MWidget* pParentWidget = GetParentWidget(element);
	ZPlayerSelectListBox* pWidget = new ZPlayerSelectListBox("SelectPlayer", pParentWidget, pParentWidget);
	pWidget->SetListener(pWidget);
	InsertWidget(element, pWidget);

	int iCount = element.GetChildNodeCount();

	bool bMode1 = false;

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if(GetCommonWidgetProperty(pWidget, childElement, szBuf)) continue;
		else if(!strcmp(szBuf, "MULTISELECT"))
		{
			pWidget->m_bMultiSelect = true;
		}
	}
	return pWidget;
}

ZBmNumLabel* ZIDLResource::GetBmNumLabel(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[1024];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	ZBmNumLabel* pBmLabel = new ZBmNumLabel(MINT_ZBMNUMLABEL, pParentWidget, pListener);
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

ZActionKey* ZIDLResource::GetActionKey(MXmlElement& element)
{
	MXmlElement childElement;
	char szBuf[4096];
	MWidget* pParentWidget;	MListener* pListener;

	pListener = pParentWidget = GetParentWidget(element);
	ZActionKey* pActionKey = (ZActionKey*)Mint::GetInstance()->NewWidget(MINT_ACTIONKEY, "", pParentWidget, pListener);
	InsertWidget(element, pActionKey);

	int iCount = element.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		childElement = element.GetChildNode(i);
		childElement.GetTagName(szBuf);

		if (GetCommonWidgetProperty(pActionKey, childElement, szBuf)) continue;	}

	return pActionKey;
}

void ZIDLResource::Parse(MXmlElement& element)
{
	MIDLResource::Parse(element);

	char szTagName[256];
	element.GetTagName(szTagName);

	// Gunz Customized Widgets...
	if (!strcmp(szTagName, "MAPLISTBOX"))
	{
		GetMapListBox(element);
	}
	else if (!strcmp(szTagName, "CANVAS"))
	{
		GetCanvas(element);
	}
	else if (!strcmp(szTagName, "SCORELISTBOX"))
	{
		GetScoreListBox(element);
	}
	else if (!strcmp(szTagName, "SCOREBOARD"))
	{
		GetScoreBoardFrame(element);
	}
	else if (!strcmp(szTagName, "MESHVIEW"))
	{
		GetMeshView(element);
	}
	else if (!strcmp(szTagName, "MESHVIEWLIST"))
	{
		GetMeshViewList(element);
	}
	else if (!strcmp(szTagName, "CHARACTERVIEW"))
	{
		GetCharacterView(element);
	}
	else if (!strcmp(szTagName, "CHARACTERVIEWLIST"))
	{
		GetCharacterViewList(element);
	}
	else if (!strcmp(szTagName, "EQUIPMENTLISTBOX"))
	{
		GetEquipmentListBox(element);
	}
	else if(!strcmp(szTagName, "STAGEINFOBOX"))
	{
		GetStageInfoBox(element);
	}
	else if (!strcmp(szTagName, "ITEMSLOT"))
	{
		GetItemSlot(element);
	}
	else if (!strcmp(szTagName, "ROOMLIST"))
	{
		GetRoomListBox(element);
	}
	else if(!strcmp(szTagName,"PLAYERLISTBOX"))
	{
		GetPlayerListBox(element);
	}
	else if(!strcmp(szTagName,"PLAYERSELECTLISTBOX"))
	{
		GetPlayerSelectListBox(element);
	}
	else if(!strcmp(szTagName,"ZBMNUMLABEL"))
	{
		GetBmNumLabel(element);
	}
	else if (!strcmp(szTagName, "CLANLIST"))
	{
		GetClanListBox(element);
	}
	else if (!strcmp(szTagName, "SERVERVIEW"))
	{
		GetServerView(element);
	}
	else if (!strcmp(szTagName,"ACTIONKEY"))
	{
		GetActionKey(element);
	}
}

MFrame*	ZIDLResource::CreateFrame(const char* szName, MWidget* pParent, MListener* pListener)
{
	MFrame* pFrame = new ZFrame(szName, pParent, pListener);
	return pFrame;
}

MFont* ZIDLResource::CreateFont(char* szAliasName, char* szFontName, int nHeight, 
								bool bBold, bool bItalic, int nOutlineStyle, bool bAntialiasing, DWORD nColorArg1, DWORD nColorArg2)
{
	MFontR2* pNew = new MFontR2;
	pNew->Create(szAliasName, szFontName, nHeight, 1.0f, bBold, bItalic, nOutlineStyle, -1, bAntialiasing, nColorArg1, nColorArg2);
	return pNew;
}

void ZIDLResource::TransText(const char* szSrc, char* szOut, int maxlen)
{
	strcpy_safe(szOut, maxlen, ZGetStringResManager()->GetStringFromXml(szSrc));

	if (!_strnicmp(szOut, "STR:", 4))
	{
		mlog("%s , %s\n", szSrc, szOut);
	}
}

void ZGetInterfaceSkinPath(char* pOutPath, int maxlen, const char* szSkinName)
{
	sprintf_safe(pOutPath, maxlen, "%s%s/", PATH_INTERFACE, szSkinName);
}
