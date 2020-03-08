#pragma once

#include <stdio.h>
#include <string>
#include "rapidxml.hpp"
#include "MUtil.h"
#include "SafeString.h"
#include "StringView.h"
#include "optional.h"

using MXmlDomDoc = rapidxml::xml_document<>;
using MXmlDomDocPtr = MXmlDomDoc*;
using MXmlDomNodePtr = rapidxml::xml_node<>*;
using MXmlDomElementPtr = rapidxml::xml_node<>*;
using MXmlDomPIPtr = rapidxml::xml_node<>*;
using MXmlDomNamedNodeMapPtr = rapidxml::xml_node<>*;
using MXmlDomTextPtr = rapidxml::xml_node<>*;
using MXmlDomParseError = rapidxml::parse_error;
using MXmlDomNodeType = rapidxml::node_type;

class MXmlDocument;

template <typename T>
struct MXmlIterator : IteratorBase<MXmlIterator<T>, T, std::forward_iterator_tag>
{
	MXmlIterator(T Node) : Node(Node) {}

	bool operator==(const MXmlIterator& rhs) const
	{
		return Node.GetXmlDomNodePtr() == rhs.Node.GetXmlDomNodePtr();
	}

	T& operator*() { return Node; }
	const T& operator*() const { return Node; }

	MXmlIterator& operator++() {
		Node.NextSibling();
		return *this;
	}

	T Node;
};

struct MXmlAttribute
{
	StringView Name;
	StringView Value;
};

struct MXmlAttributeIterator : IteratorBase<MXmlAttributeIterator, MXmlAttribute, std::input_iterator_tag>
{
	MXmlAttributeIterator(rapidxml::xml_attribute<>* a) : a(a) {}

	bool operator==(const MXmlAttributeIterator& rhs) const
	{
		return a == rhs.a;
	}

	MXmlAttribute operator*() const
	{
		return {StringView(a->name(), a->name_size()), StringView(a->value(), a->value_size())};
	}

	MXmlAttributeIterator& operator++() {
		a = a->next_attribute();
		return *this;
	}

	rapidxml::xml_attribute<>* a;
};

class MXmlNode
{
protected:
	MXmlDomNodePtr		m_pDomNode{};

public:
	MXmlNode() = default;
	MXmlNode(MXmlDomNodePtr m_pDomNode) : m_pDomNode{ m_pDomNode } {}

	MXmlDomNodePtr	GetXmlDomNodePtr() const { return m_pDomNode; }
	void			SetXmlDomNodePtr(MXmlDomNodePtr pNode) { m_pDomNode = pNode; }

	bool IsEmpty() const { return m_pDomNode == nullptr; }

	void GetNodeName(ArrayView<char> Out) { GetNodeName(Out.data(), int(Out.size())); }
	void GetNodeName(char* sOutStr, int maxlen);
	StringView GetNodeName() const;
	void GetText(char* sOutStr, int nMaxCharNum);
	void GetText(ArrayView<char> Out) { GetText(Out.data(), int(Out.size())); }
	void GetTextUnsafe(char* Out) { GetText(Out, INT_MAX); }
	void SetText(const char* sText);
	
	int	GetChildNodeCount();
	MXmlDomNodeType GetNodeType();
	bool HasChildNodes();

	void NextSibling();
	void PreviousSibling();

	bool AppendChild(MXmlNode node);

	bool FindChildNode(const char* sNodeName, MXmlNode* pOutNode);

	MXmlNode GetParent() { if (m_pDomNode) return MXmlNode(m_pDomNode->parent()); else return MXmlNode(); }
	MXmlNode GetChildNode(int iIndex);

	Range<MXmlIterator<MXmlNode>> Children() const
	{
		return {MXmlNode{m_pDomNode->first_node()}, MXmlNode{nullptr}};
	}
};

class MXmlElement : public MXmlNode
{
public:
	MXmlElement() = default;
	MXmlElement(MXmlNode Node) : MXmlNode{ Node } {}
	using MXmlNode::MXmlNode;

	void GetTagName(ArrayView<char> Out) { GetTagName(Out.data(), int(Out.size())); }
	void GetTagName(char* sOutStr, int maxlen) { MXmlNode::GetNodeName(sOutStr, maxlen); }
	StringView GetTagName() const { return MXmlNode::GetNodeName(); }
	
	void GetContents(ArrayView<char> sOutStr) { MXmlNode::GetText(sOutStr); }
	void GetContents(int* ipOutValue);
	void GetContents(bool* bpOutValue);
	void GetContents(float* fpOutValue);
	void GetContents(std::string* pstrValue);

	void SetContents(const char* sStr) { MXmlNode::SetText(sStr); }
	void SetContents(int iValue);
	void SetContents(bool bValue);
	void SetContents(float fValue);

	bool GetChildContents(char* sOutStr, const char* sChildTagName, int nMaxCharNum = -1);
	bool GetChildContents(int* iOutValue, const char* sChildTagName);
	bool GetChildContents(float* fOutValue, const char* sChildTagName);
	bool GetChildContents(bool* bOutValue, const char* sChildTagName);

	optional<StringView> GetAttribute(StringView AttrName, bool CaseSensitive = false) const;
	bool GetAttribute(ArrayView<char> OutText, const char* AttrName, const char* DefaultText = "") {
		return GetAttribute(OutText.data(), int(OutText.size()), AttrName, DefaultText);
	}
	bool GetAttribute(char* sOutText, int maxlen, const char* sAttrName, const char* sDefaultText = "");
	bool GetAttribute(int* ipOutValue, const char* sAttrName, int nDefaultValue = 0);
	bool GetAttribute(bool* bOutValue, const char* sAttrName, bool bDefaultValue = false);
	bool GetAttribute(float* fpOutValue, const char* sAttrName, float fDefaultValue = 0.0f);
	bool GetAttribute(std::string* pstrOutValue, const char* sAttrName, const char* sDefaultValue = "");
	bool AddAttribute(const char* sAttrName, const char* sAttrText);
	bool AddAttribute(const char* sAttrName, int iAttrValue);
	bool AddAttribute(const char* sAttrName, bool bAttrValue);
	bool SetAttribute(const char* sAttrName, char* sAttrText);
	bool SetAttribute(const char* sAttrName, int iAttrValue);
	bool RemoveAttribute(const char* sAttrName);

	int GetAttributeCount();
	void GetAttribute(int index, ArrayView<char> OutName, ArrayView<char> OutValue) {
		GetAttribute(index, OutName.data(), int(OutName.size()), OutValue.data(), int(OutValue.size()));
	}
	void GetAttribute(int index, char* szoutAttrName, int maxlen1, char* szoutAttrValue, int maxlen2);
	Range<MXmlAttributeIterator> Attributes() const { return {m_pDomNode->first_attribute(), nullptr}; }

	bool AppendChild(const char* sTagName, const char* sTagText = NULL);
	bool AppendChild(MXmlElement aChildElement);

	MXmlElement	CreateChildElement(const char* sTagName);

	bool AppendText(const char* sText);

	Range<MXmlIterator<MXmlElement>> Children() const
	{
		return {MXmlElement{m_pDomNode->first_node()}, MXmlElement{nullptr}};
	}
};

class MXmlDocument final
{
public:
	// Dummy methods for backwards compatibility.
	bool Create() { return true; }
	bool Destroy() { return true; }

	bool				LoadFromFile(const char* m_sFileName, class MZFileSystem* FileSystem = nullptr);
	// If Size is -1, szBuffer must be null-terminated.
	bool				LoadFromMemory(char* szBuffer, size_t Size = -1);

	bool				SaveToFile(const char* m_sFileName);

	bool				CreateProcessingInstruction( const char* szHeader = "version=\"1.0\"");
	bool				Delete(MXmlNode* pNode);

	MXmlElement			CreateElement(const char* sName);

	bool				AppendChild(MXmlNode node);

	MXmlElement			GetDocumentElement() { return MXmlElement(Root); }

protected:
	// Parses the data in FileBuffer.
	bool Parse(const char* Filename = nullptr);

	// The RapidXML document.
	MXmlDomDoc Doc;

	// For some reason, all MAIET XMLs have an "<XML>" tag wrapped around their data, even though
	// MXml users expect not to see it.
	// So, this data member holds that element in order to give it out in GetDocumentElement.
	MXmlDomNodePtr Root{};

	// The memory in the file.
	// RapidXML doesn't save it, it only gives you back pointers into the memory you gave it, so we
	// need to save it ourselves.
	std::string FileBuffer;
};
