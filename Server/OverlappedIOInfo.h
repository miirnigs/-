#pragma once
#include <Windows.h>
#define MAX_BUFFER 1024 * 8		// �շ���������С

// �̳�OVERLAPPED�ṹ
// ��ΪAcceptEx���첽�ģ�Ͷ����AcceptEx���ټ��������ڹ����̵߳�GetQueuedCompletionStatus��
// ��ʵ�ϻ��кܶ಻ͬ��Ͷ�ݣ������߳���ʵ������û��֪���������Ǹ��ͻ����׽��ֻ��߾������Ϣ֮���
// �̳���OVERLAPPED�ṹ�Ϳ�����ĩβ���Ͽͻ����׽��֣��ڹ����߳���Ϳ��Ե�֪���ĸ��ͻ����׽���
class COverlappedIOInfo : public OVERLAPPED
{
public:
	COverlappedIOInfo()
	{
		m_clientSocket = INVALID_SOCKET;		// ��ʼ��Ϊ��Ч���׽���

	}

	~COverlappedIOInfo()
	{
		if (m_clientSocket != INVALID_SOCKET)
		{
			closesocket(m_clientSocket);
			m_clientSocket = INVALID_SOCKET;
		}
	}

	void ResetOverlapped()
	{
		Internal = InternalHigh = 0;
		Offset = OffsetHigh = 0;
		hEvent = NULL;
	}

	void ResetRecvBuffer()
	{
		ZeroMemory(m_crecvBuf, MAX_BUFFER);
		m_recvBuf.buf = m_crecvBuf;
		m_recvBuf.len = MAX_BUFFER;
	}

	void ResetSendBuffer()
	{
		ZeroMemory(m_csendBuf, MAX_BUFFER);
		m_sendBuf.buf = m_csendBuf;
		m_sendBuf.len = MAX_BUFFER;
	}
public:
	SOCKET m_clientSocket;		// �ͻ��˵��׽���

	// ���ջ�����������AcceptEx, WSARecv����
	WSABUF m_recvBuf;
	char m_crecvBuf[MAX_BUFFER];

	// ���ͻ������� ����WSASend����
	WSABUF m_sendBuf;
	char m_csendBuf[MAX_BUFFER];

	sockaddr_in m_addr;		// �Զ˵�ַ
};
