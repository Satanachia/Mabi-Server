#pragma once

/*----------------------------------
					NetAddress
----------------------------------*/

class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(std::wstring ip, uint16 port);
	~NetAddress();

	//NetAddress(const NetAddress& _Other) = delete;
	//NetAddress(NetAddress&& _Other) noexcept = delete;
	//NetAddress& operator=(const NetAddress& _Other) = delete;
	//NetAddress& operator=(const NetAddress&& _Other) noexcept = delete;
	
	SOCKADDR_IN&	GetSocketAddr()		{ return _sockAddr; }
	std::wstring		GetIpAddress();
	uint16					GetPort()					{ return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _sockAddr = {};
};

