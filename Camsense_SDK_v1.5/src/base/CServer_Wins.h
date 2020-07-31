#pragma once

#include <winsock2.h>

class CServer_Wins
{
public:
	CServer_Wins();
	void SendMsd();
	void RecMsg(char revData[]);
	void stopRec();
	bool Accept();
	~CServer_Wins();

	bool m_bLinkFlag;

private:
	bool m_terminal;
	SOCKET sClient;
	SOCKET m_slisten; //创建监听套接字
};