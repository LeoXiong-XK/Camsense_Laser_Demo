
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CClient_Wins.h"
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

struct sockaddr_in sin_client;
#define nPORT 6000
CClient_Wins::CClient_Wins() :m_terminate(false)
{
	//����������³�ʼ���� ����socket()�᷵��10093����
	//��ʼ��WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) //ͨ��һ�����̳�ʼ��ws2_32.dll
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}	

}
bool CClient_Wins::ConnectSerever(char ServerIP[])
{
	bool ret = false;
	sin_client.sin_addr.S_un.S_addr = inet_addr(ServerIP); //���õ�ַ
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
	//�����׽���
	bool ret = false;

	m_cSocket = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
	if (m_cSocket == INVALID_SOCKET)
	{
		printf("%d\n", WSAGetLastError());
		printf("%s\n", "Socket Error!");

		return false;
	}

	sin_client.sin_family = AF_INET; //���õ�ַ����
	sin_client.sin_port = htons(nPORT); //���ö˿ںţ�inet_addr("192.168.1.0");
	ret = ConnectSerever(ServerIP);	
	
	return ret;
}
void CClient_Wins::connected_do_before(char ServerIP[])
{
	bool ret  = false;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) //ͨ��һ�����̳�ʼ��ws2_32.dll
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

	sin_client.sin_family = AF_INET; //���õ�ַ����
	sin_client.sin_port = htons(nPORT); //���ö˿ںţ�inet_addr("192.168.1.0");
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
		num = recv(m_cSocket, buffer, strlen(RecvBuff), 0);//����

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

	//closesocket(m_cSocket); //�رռ����׽���
}
void CClient_Wins::stopRecv()
{
	std::cout << "stopRecv..." << std::endl;	
	m_terminate = true;
	closesocket(m_cSocket);
}
CClient_Wins::~CClient_Wins()
{
	closesocket(m_cSocket); //�رռ����׽���
	WSACleanup(); //��ֹWs2_32.dll��ʹ��
}