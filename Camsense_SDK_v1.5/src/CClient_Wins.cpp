
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CClient_Wins.h"
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

struct sockaddr_in sin_client;
#define nPORT 6000
CClient_Wins::CClient_Wins() :m_terminate(false)
{
	//必须进行如下初始化， 否则socket()会返回10093错误
	//初始化WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) //通过一个进程初始化ws2_32.dll
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}	

}
bool CClient_Wins::ConnectSerever(char ServerIP[])
{
	bool ret = false;
	sin_client.sin_addr.S_un.S_addr = inet_addr(ServerIP); //设置地址
	if (connect(m_cSocket, (sockaddr*)&sin_client, sizeof(sin_client)) == 0)
	{
		ret = true;
		std::cout << "TCP client connect to server successful..."<< std::endl;
	}
	else {
		std::cout << "TCP client connect to server failed..." << std::endl;
	}

	return ret;
}
bool CClient_Wins::Sendbefore(char ServerIP [])
{
	//创建套接字
	bool ret = false;

	m_cSocket = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
	if (m_cSocket == INVALID_SOCKET)
	{
		printf("%d\n", WSAGetLastError());
		printf("%s\n", "Socket Error!");

		return false;
	}

	sin_client.sin_family = AF_INET; //设置地址家族
	sin_client.sin_port = htons(nPORT); //设置端口号，inet_addr("192.168.1.0");
	ret = ConnectSerever(ServerIP);	
	
	return ret;
}
void CClient_Wins::connected_do_before(char ServerIP[])
{
	bool ret  = false;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) //通过一个进程初始化ws2_32.dll
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}
	m_cSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_cSocket == INVALID_SOCKET)
	{
		printf("%d\n", WSAGetLastError());
		printf("%s\n", "Socket Error!");

		return ;
	}

	sin_client.sin_family = AF_INET; //设置地址家族
	sin_client.sin_port = htons(nPORT); //设置端口号，inet_addr("192.168.1.0");
	ret = ConnectSerever(ServerIP);		
}

void CClient_Wins::SendMsg(char ServerIP[], const char SendBuff[], unsigned int nSendLen )
{
	int rtn = send(m_cSocket, SendBuff, nSendLen, 0);
	//std::cout <<"send_length:"<<nSendLen << std::endl;
}
void CClient_Wins::RecMsg(char RecvBuff[])
{
	char buffer[2049];
	int num = 0;
	while (!m_terminate)
	{
		num = recv(m_cSocket, buffer, strlen(RecvBuff), 0);//阻塞

		if (num > 0)
		{
			buffer[num] = '\0';
			std::cout << "Server say:" << buffer << std::endl;
			//closesocket(m_cSocket);
		}
		else {
			std::cout << "Client Receive failed!" << " num:" << num << std::endl;
		}
	}

	//closesocket(m_cSocket); //关闭监听套接字
}
void CClient_Wins::stopRecv()
{
	std::cout << "stopRecv..." << std::endl;	
	m_terminate = true;
	closesocket(m_cSocket);
}
CClient_Wins::~CClient_Wins()
{
	closesocket(m_cSocket); //关闭监听套接字
	WSACleanup(); //终止Ws2_32.dll的使用
}