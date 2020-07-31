#pragma 
#include <winsock2.h>

class CClient_Wins
{
public:
	static CClient_Wins& Instance(){
		static CClient_Wins instance;
		return instance;
	}
	CClient_Wins();
	bool ConnectSerever(char ServerIP[]);
	bool Sendbefore(char ServerIP[]);
	void connected_do_before(char ServerIP[]);
	void SendMsg(char ServerIP[], const char SendBuff[], unsigned int nSendLen);
	void RecMsg(char RecvBuff[]);
	void stopRecv();
	~CClient_Wins();

private:
	SOCKET m_cSocket;
	bool m_terminate;
};
