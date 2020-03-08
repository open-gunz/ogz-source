#include "stdafx.h"
#include "upnp.h"
#include "NATUPnP.h"
#include "MSocket.h"

UPnP::UPnP() = default;

UPnP::~UPnP()
{
	Destroy();
}

bool UPnP::Create(u16 Port)
{
	IUPnPNAT * Nat = NULL;
	IStaticPortMappingCollection * PortMappingCollection = NULL;
	IStaticPortMapping * PortMap = NULL;
	HRESULT Result;
	wchar_t Protocol[256];
	wchar_t InternalClient[256];
	wchar_t Description[256];

	Destroy();

#ifdef MFC
	TRACE("UPnP: Adding port\n");
#endif

	if(!GetIp())
	{
		return false;
	}

	size_t num_chars_converted{};
	mbstowcs_s(&num_chars_converted, InternalClient, m_Address, 256);

	wcscpy_s(Protocol, L"UDP");
	wcscpy_s(Description, L"Gunz");

	// Create IUPnPNat
	Result = CoCreateInstance(CLSID_UPnPNAT, NULL, CLSCTX_INPROC_SERVER, IID_IUPnPNAT, (void **)&Nat);
	if(FAILED(Result))
	{
#ifdef MFC
		TRACE("UPnP: Unable to create UPnPNAT interface\n");
#endif
		return false;
	}

	Result = Nat->get_StaticPortMappingCollection(&PortMappingCollection);

	if(!PortMappingCollection || FAILED(Result))
	{
		if(PortMappingCollection) PortMappingCollection->Release();
		Nat->Release();

#ifdef MFC
		TRACE("UPnP: Unable to acquire a static portmapping collection\n");
#endif
		return false;
	}

	Result = PortMappingCollection->Add(Port, Protocol, Port, InternalClient,
		VARIANT_TRUE, Description, &PortMap);

	if(!PortMap || FAILED(Result))
	{
		if(PortMap) PortMap->Release();
		PortMappingCollection->Release();
		Nat->Release();

#ifdef MFC
		TRACE("UPnP: Unable add port\n");
#endif
		return false;
	}

#ifdef MFC
	TRACE("UPnP: Port %d forwarded to %s\n", Port, m_Address);
#endif

	PortMap->Release();
	PortMappingCollection->Release();
	Nat->Release();

	m_Port = Port;

	return true;
}

void UPnP::Destroy()
{
	IUPnPNAT * Nat = NULL;
	IStaticPortMappingCollection * PortMappingCollection = NULL;
	HRESULT Result;
	wchar_t Protocol[256];
	WORD Port;

	if(m_Port == 0) return;
	Port = m_Port;
	m_Port = 0;

#ifdef MFC
	TRACE("UPnP: Removing Port\n");
#endif

	wcscpy_s(Protocol, L"TCP");

	// Create IUPnPNat
	Result = CoCreateInstance(CLSID_UPnPNAT, NULL, CLSCTX_INPROC_SERVER, IID_IUPnPNAT, (void **)&Nat);
	if(FAILED(Result))
	{
#ifdef MFC
		TRACE("UPnP: Unable to create UPnPNAT interface\n");
#endif
		return;
	}

	Result = Nat->get_StaticPortMappingCollection(&PortMappingCollection);

	if(!PortMappingCollection || FAILED(Result))
	{
		if(PortMappingCollection) PortMappingCollection->Release();
		Nat->Release();

#ifdef MFC
		TRACE("UPnP: Unable to acquire a static portmapping collection\n");
#endif
		return;
	}

	Result = PortMappingCollection->Remove(Port, Protocol);

	if(FAILED(Result))
	{
		PortMappingCollection->Release();
		Nat->Release();

#ifdef MFC
		TRACE("UPnP: Unable to remove port\n");
#endif
		return;
	}
}

bool UPnP::GetIp()
{
	char HostName[256];
	hostent * Host;
	MSocket::sockaddr_in Addr;

	auto ret = gethostname(HostName, 256);

	if(ret != 0)
	{
#ifdef MFC
		TRACE("UPnP: gethostname failed\n");
#endif
		return false;
	}else{
#ifdef MFC
		TRACE("UPnP: HostName: %s\n", HostName);
#endif
	}
#pragma warning(push)
#pragma warning(disable: 4996)
	Host = gethostbyname(HostName);
#pragma warning(pop)
	if(Host == NULL)
	{
#ifdef MFC
		TRACE("UPnP: gethostbyname failed\n");
#endif
		return false;
	}
	Addr.sin_addr.s_addr = ((MSocket::in_addr *)Host->h_addr)->s_addr;
	auto Address = GetIPv4String(Addr.sin_addr);

	if(Address.empty())
	{
#ifdef MFC
		TRACE("UPnP: inet_ntoa failed\n");
#endif
		return false;
	}

	strcpy_safe(m_Address, Address.c_str());

#ifdef MFC
	TRACE("UPnP: Local Ip: %s\n", m_Address);
#endif

	return true;
}