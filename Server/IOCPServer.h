#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>		// ���µ�ͷ�ļ�
#include <MSWSock.h>		// һ��Ҫ����WinSock2.h������
#pragma comment(lib, "ws2_32.lib")
#include "IOCP.h"
#include "OverlappedIOInfo.h"
using namespace std;

enum IOOperType
{ 
	TYPE_ACP,		// accept�¼�������µ���������
	TYPE_RECV,		// ���ݽ����¼�
	TYPE_SEND,		// ���ݷ����¼�
	TYPE_CLOSE		// �ر��¼�
};

class CIOCPServer
{
public:
	CIOCPServer();		// Ĭ�Ϲ��캯��
	~CIOCPServer();		// ��������

public:
	// ��ʼ��WSA�汾
	bool InitWSA();
	// �����˿�
	bool StartListen(unsigned short port);
	// �������ݴ�������������ڽ��յ����ݺ����ô���ĺ���ָ��
	// PS: ���û���ã���Ĭ�ϲ��ᴦ����յ�����Ϣ
	void SetDataAdapter(void(*p)(COverlappedIOInfo*));
	// �رշ�����
	void CloseServer();
private:
	// �����������߳�
	int StartWorkThreadPool();
	// ��ȡAcceptEx��GetAcceptExSockaddrs�ĺ���ָ��
	bool GetLPFNAcceptExAndGetAcceptSockaddrs();
	// ��������ɾ���Ѿ����ӵĿͻ��˵Ľṹ
	void DelectLink(SOCKET socket);
	// �пͻ����룬����ͻ��˽����¼�
	bool DoAccept(COverlappedIOInfo* pInfo, DWORD NumberOfBytes);

	// Ͷ��AcceptEx��������AcceptEx����accept����
	bool PostAccept(COverlappedIOInfo* pInfo);
	// Ͷ��һ����������WSARecv�����ʱ���Զ�֪ͨ��ɶ˿�
	bool PostRecv(COverlappedIOInfo* pInfo);

	// ����˽��յ����ݵĴ���
	//void DoRecv(COverlappedIOInfo* pInfo);
private:
	WSAData m_wsaData;		// winsock�汾����
	SOCKET m_sListen;		// �������׽���
	CIOCP m_iocp;		// ��ɶ˿ڵķ�װ

	LPFN_ACCEPTEX m_lpfnAcceptEx;		// AcceptEx����ָ��
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptSockAddrs;		// GetAcceptExSockaddrs����ָ��

	vector<thread> m_workThreadPool;		// ��Ź����̵߳�����
	vector<SOCKET> m_acceptSockets;		// �����ǰ�����õ��׽��֣�������AccrptExʹ��
	vector<COverlappedIOInfo*> m_clientSockets;		// ����Ѿ����ӵĽṹ�������л��������Զ�IP���ͻ����׽��ֵ�

	mutex m_mu;		// �Ѿ������׽��������Ļ�����

	/*������std::function��functional.h���ȻTMD�и���bind�ĺ�����Socket��bind�غ���*/
	void(*DoRecv)(COverlappedIOInfo*);		// ����ָ�룬����˽��յ����ݵĴ���		
};