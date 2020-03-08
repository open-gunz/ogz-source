#pragma once

#include "MButton.h"
#include "MListBox.h"
#include "StringView.h"

class MComboListBox : public MListBox{
protected:
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
public:
	MComboListBox(MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MComboListBox(){}

	DECLARE_LOOK(MListBoxLook)
	DECLARE_LOOK_CLIENT()

#define MINT_COMBOLISTBOX	"ComboListBox"
	virtual const char* GetClassName(){ return MINT_COMBOLISTBOX; }
};

class MIDLResource;

class MComboBox : public MButton{
	friend MIDLResource;

	MComboListBox*	m_pListBox;
	int			m_nDropHeight;
	MListener*	m_pComboBoxListener;

	int			m_nComboType;
	int			m_nNextComboBoxTypeSize;

	bool		m_bAutoDrop;
	bool		m_bDropUnder;

protected:
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);

public:
	MComboBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MComboBox();

	void SetDropSize(int nHeight);

	void SetComboType(int nType) {
		m_nComboType = nType;
	}

	int GetComboType() {
		return m_nComboType;
	}

	void SetNextComboBoxTypeSize(int nSize) {
		m_nNextComboBoxTypeSize = nSize;
	}

	int GetNextComboBoxTypeSize() {
		return m_nNextComboBoxTypeSize;
	}

	void SetNextSel();
	void SetPrevSel();

	void Add(const StringView& szItem);
	void Add(MListItem* pItem);
	const char* GetString(int i);
	MListItem* Get(int i);
	void Remove(int i);
	void RemoveAll();
	int GetCount();
	int GetSelIndex();
	bool SetSelIndex(int i);
	const char* GetSelItemString();
	MListItem* GetSelItem();

	// Field Support
	void AddField(const char* szFieldName, int nTabSize) { m_pListBox->AddField(szFieldName, nTabSize); }
	void RemoveField(const char* szFieldName) { m_pListBox->RemoveField(szFieldName); }
	MLISTFIELD* GetField(int i) { return m_pListBox->GetField(i); }
	int GetFieldCount() { return m_pListBox->GetFieldCount(); }

	bool IsVisibleHeader() { return m_pListBox->IsVisibleHeader(); }
	void SetVisibleHeader(bool bVisible) { m_pListBox->SetVisibleHeader(bVisible); }

	virtual void SetListener(MListener* pListener);
	virtual MListener* GetListener();

	void Sort();

	void SetListboxAlignment( MAlignmentMode am)		{ m_pListBox->m_FontAlign = am; }

#define MINT_COMBOBOX	"ComboBox"
	virtual const char* GetClassName(){ return MINT_COMBOBOX; }

	virtual void OnReleaseFocus();

	void CloseComboBoxList();
};

#define MCMBBOX_CHANGED		"changed"