#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CServer_Wins.h"
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#define sPORT	6000
using namespace std;
CServer_Wins::CServer_Wins()/* :m_terminal(false)*/
{
	m_bLinkFlag = false;

	//必须进行如下初始化， 否则socket()会返回10093错误
	//初始化WSA  
	//std::cout << "CServer_Wins---->enter!" << std::endl;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) //通过一个进程初始化ws2_32.dll
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}

	//创建套接字
	m_slisten = socket(AF_INET, SOCK_STREAM, 0);
	if (m_slisten == INVALID_SOCKET)
	{
		std::cout << WSAGetLastError() << std::endl;
		std::cout << "Socket Error!" << std::endl;

		return;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET; //设置地址家族
	sin.sin_port = htons(sPORT); //设置端口号，inet_addr("192.168.1.0");
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//INADDR_ANY; //设置地址

													  //绑定端口
	if ((bind(m_slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR))
		//套接字， 指向sockaddr_in结构体的指针，参数长度 
	{
		std::cout << "Bind Error!" << std::endl;
	}


	if (listen(m_slisten, 1) == SOCKET_ERROR)  //套接字, 为该套接字排队的最大连接数
												//此时， slisten 变为监听套接字
	{
		std::cout << "Listen Error!" << std::endl;
		closesocket(m_slisten); //关闭监听套接字
		WSACleanup(); //终止Ws2_32.dll的使用
		return;
	}

	//设置Socket为非阻塞模式
	int retVal;//调用Socket函数的返回值
	int iMode = 1;
	retVal = ioctlsocket(m_slisten, FIONBIO, (u_long FAR*) &iMode);
	if (retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed!\n");
		closesocket(m_slisten); //关闭监听套接字
		WSACleanup();
		return;
	}

}

bool CServer_Wins::Accept()
{
	struct sockaddr_in remoteAddr;     //存储 通过accept 得到的 客户端IP地址
	int nAddrlen = sizeof(remoteAddr); //IP地址长度

	 //监听客户端连接，一旦连接不再监听
		while (!m_bLinkFlag)
		{
			sClient = accept(m_slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
			std::cout << "TCP Server is begin to work...." << std::endl;
			if (sClient == INVALID_SOCKET)
			{
				printf("accept error !\n");
				//continue;
			}
			else
			{
				m_bLinkFlag = true;
				printf("CServer_Wins accept successful\n");
			}

			Sleep( 10);
		}
	return true;
}

void CServer_Wins::RecMsg(char revData[])
{
	//while(true) //(m_bLinkFlag)
	{
		//接收数据
		int ret_num = 0; //接收回来的字节数 
		ret_num = recv(sClient, revData, 2048, 0);
		if (ret_num > 0)
		{
			//revData[ret_num] = '\0';
			//std::cout << "Server Receive Successful!" << std::endl;
		}
		else
		{
			std::cout << "ret_num:" << ret_num << " socket recv failed..." << std::endl;
		}
	}
}
void CServer_Wins::stopRec()
{
	std::cout << "Server stopRec..." << std::endl;
	m_terminal = true;
	m_bLinkFlag = false;
}
CServer_Wins::~CServer_Wins()
{
	std::cout << "~CServer_Wins" << std::endl;
	closesocket(m_slisten); //关闭监听套接字
	WSACleanup(); //终止Ws2_32.dll的使用
}