#pragma once
#include "IOCP.h"
#include "OverlappedIOInfo.h"

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
	void SetDataRecvAdapter(std::function<void(COverlappedIOInfo* pInfo)> Func)
	{
		DoRecv = Func;
	}
	// �������ݴ�������������ڷ��������ݺ����ô���ĺ���ָ��
	// PS: ���û���ã���Ĭ�ϲ��ᴦ���������Ϣ
	void SetDataSendAdapter(std::function<void(COverlappedIOInfo* pInfo)> Func)
	{
		DoSend = Func;
	}
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
	// Ͷ��һ����������WSASend�����ʱ���Զ�֪ͨ��ɶ˿�
	bool PostSend(COverlappedIOInfo* pInfo, const std::string& strData);

	// ��ȡ��ǰ����IP�б�
	std::vector<std::string> GetOnlineIPs();
private:
	WSAData m_wsaData;		// winsock�汾����
	SOCKET m_sListen;		// �������׽���
	CIOCP m_iocp;		// ��ɶ˿ڵķ�װ

	LPFN_ACCEPTEX m_lpfnAcceptEx;		// AcceptEx����ָ��
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptSockAddrs;		// GetAcceptExSockaddrs����ָ��

	std::vector<std::thread> m_workThreadPool;		// ��Ź����̵߳�����
	std::vector<SOCKET> m_acceptSockets;		// �����ǰ�����õ��׽��֣�������AccrptExʹ��
	std::vector<COverlappedIOInfo*> m_clientSockets;		// ����Ѿ����ӵĽṹ�������л��������Զ�IP���ͻ����׽��ֵ�

	std::mutex m_mu;		// �Ѿ������׽��������Ļ�����

	std::function<void(COverlappedIOInfo* pInfo)> DoRecv;		// �����ѵ���-��Ϣ������ָ��
	std::function<void(COverlappedIOInfo* pInfo)> DoSend;		// �����ѷ���-��Ϣ������ָ��
	
};