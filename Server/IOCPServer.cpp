#include "pch.h"
#include "IOCPServer.h"

CIOCPServer::CIOCPServer() :DoRecv(NULL), m_lpfnAcceptEx(NULL), m_lpfnGetAcceptSockAddrs(NULL), m_sListen(NULL), m_wsaData()
{
}

CIOCPServer::~CIOCPServer()
{
	// �رշ�����������
	CloseServer();
	WSACleanup();
}

bool CIOCPServer::InitWSA()
{
	// WinSock��ʼ�������°汾2.2
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
	{
		// ��ʼ��ʧ��
		return FALSE;
	}
	return TRUE;
}

bool CIOCPServer::StartListen(unsigned short port)
{
	if (!port)
	{
		return FALSE;
	}

	// ����TCP�׽��֣�Socket��Ҫ��accept����Ͷ�ݵ���ɶ˿ڣ����socket���Ա������ص�IO
	// ��ͨSocket��֧��IO�ص�������Ҫ��WSASocket
	// af: IP ��ַ���ͣ�������AF_INET[IPv4]��Ҳ������AF_INET6[IPv6]
	// type; �׽������ͣ�������SOCK_STREAM[�������ӵ��׽���]��Ҳ������SOCK_DGRAM[�����ӵ��׽���]
	// protocol: ����Э�飬������IPPROTO_TCP[TCP����Э��]��Ҳ������IPPTOTO_UDP[UDP����Э��]
	// lpProtocolInfo: ��NULL�ͺã���NULL��ǰ����������Ч
	// g: �Ѵ��ڵ��׽���Ⱥ���ID����0����
	// dwFlags: �׽���������������Ϊ��Ҫ�ص�IO������ʹ��WSA_FLAG_OVERLAPPED
	m_sListen = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sListen == INVALID_SOCKET)
	{
		// �����׽���ʧ��
		return FALSE;
	}

	// ������ɶ˿�
	if (!m_iocp.Create())
	{
		// ��ɶ˿ڴ���ʧ��
		return FALSE;
	}

	// ���׽��ֵ���ɶ˿��ϣ�����Ͷ�ݵ�AcceptEx�пͻ��������ˣ��ͻ�����˿ڻ�õ�֪ͨ
	// �����߳��ڵ���GetQueuedCompletionStatus��ȡ����completionKey�ͻ���TYPE_ACP
	if (!m_iocp.AssociateSocket(m_sListen, TYPE_RUN))
	{
		return FALSE;
	}
	
	// ��ʼ��Socket��һϵ�в���
	sockaddr_in server;		// һ����sockaddr_in������sockaddr
	server.sin_family = AF_INET;
	server.sin_port = htons(port);		// �Ѷ˿ں������ֽ���ת��Ϊ�����ֽ���
	server.sin_addr.s_addr = INADDR_ANY;		// 0.0.0.0���κε�ַ
	
	if (::bind(m_sListen, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		return FALSE;
	}

	if (listen(m_sListen, SOMAXCONN) == SOCKET_ERROR)
	{
		return FALSE;
	}

	//�����������߳�
	if (StartWorkThreadPool() == 0)
	{
		return FALSE;
	}

	// ��ȡAcceptEx��GetAcceptExSockaddrs�ĺ���ָ��
	if (!GetLPFNAcceptExAndGetAcceptSockaddrs())
	{
		return FALSE;
	}

	// Ͷ��AcceptEx����
	for (int i = 0; i < 15; i++)
	{
		COverlappedIOInfo* pInfo = new COverlappedIOInfo();		// �����ڴ棬��Ҫdelete
		if (!PostAccept(pInfo))
		{
			delete pInfo;
			return FALSE;
		}
	}

	return TRUE;
}

void CIOCPServer::CloseServer()
{
	// ���IOCP�̶߳��У� �˳������̣߳� �����е��̷߳���TYPE_CLOSE��Ϣ
	for (unsigned int i = 0; i < m_workThreadPool.size(); i++)
	{
		m_iocp.PostStatus(TYPE_CLOSE);		// ����ɶ˿�Ͷ��TYPE_CLOSE״̬����Ϊ�Ƕ��У�����һ����ÿһ���̶߳��յ�
	}

	// �����ǰ�����õĵ��׽���
	for (auto iter = m_acceptSockets.begin(); iter != m_acceptSockets.end(); iter++)
	{
		closesocket(*iter);
	}
	m_acceptSockets.clear();

	// ��������ӵ��׽��ֲ���ջ���
	for (auto iter = m_clientSockets.begin(); iter != m_clientSockets.end(); iter++)
	{
		COverlappedIOInfo* ol = *iter;
		closesocket(ol->m_clientSocket);
		delete ol;
	}
	m_clientSockets.clear();
}

int CIOCPServer::StartWorkThreadPool()
{
	// ��ϵͳ�Զ��������������߳�
	int nCount = std::thread::hardware_concurrency();

	for (int i = 0; i < nCount; i++)
	{
		// ���������̻߳��Զ�ִ��
		m_workThreadPool.emplace_back(
			[this]()
			{
				// ��ѭ��ȥ�����ɶ˿ڵ�״̬
				while (1)
				{
					DWORD NumberOfBytes = 0;
					ULONG_PTR completionKey = 0;		// PULONG_PTR��ULONG_PTR��ָ��
					OVERLAPPED* ol = NULL;

					// ��������GetQueuedCompletionStatus��ȡ��ɶ˿ڵ�״̬
					if (GetQueuedCompletionStatus(this->m_iocp.GetIOCP(), &NumberOfBytes, &completionKey, &ol, WSA_INFINITE))
					{
						COverlappedIOInfo* olinfo = (COverlappedIOInfo*)ol;
						if (completionKey == TYPE_CLOSE)
						{
							// �˳������߳� 
							break;
						}
						if (NumberOfBytes == 0 && (olinfo->m_type == IOEventType::TYPE_RECV || olinfo->m_type == IOEventType::TYPE_SEND))
						{
							// ������յ����ݻ��߷��������ݵ��ֽ���Ϊ0����ζ�ſͻ��˶Ͽ�����
							this->DelectLink(olinfo->m_clientSocket);
							continue;
						}

						// �����GetQueuedCompletionStatus�����¼��Ѿ����
						switch (olinfo->m_type)
						{
						case IOEventType::TYPE_ACP:
						{
							// �ͻ��˽������
							this->DoAccept(olinfo, NumberOfBytes);
							this->PostAccept(olinfo);		// Ͷ��һ���µ�AcceptEx
							break;
						}
						case IOEventType::TYPE_RECV:
						{
							// ���ݽ������
							if (this->DoRecv != NULL)
							{
								// �����ݴ���ĺ���
								this->DoRecv(olinfo);
							}
							this->PostRecv(olinfo);
							break;
						}
						case IOEventType::TYPE_SEND:
						{
							//���ݷ������
							if (this->DoSend != NULL)
							{
								// ����Ϣ����ĺ���
								this->DoSend(olinfo);
							}
							break;
						}
						}
					}
				}
			}
		);
	}
	return nCount;
}

bool CIOCPServer::GetLPFNAcceptExAndGetAcceptSockaddrs()
{
	DWORD ByteReturned = 0;

	// ��ȡAcceptEx����ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (SOCKET_ERROR == WSAIoctl(
		m_sListen,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&ByteReturned,
		NULL, NULL
	))
	{
		return FALSE;
	}

	// ��ȡGetAcceptExSockAddrs����ָ��
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (SOCKET_ERROR == WSAIoctl(
		m_sListen,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptSockAddrs,
		sizeof(m_lpfnGetAcceptSockAddrs),
		&ByteReturned,
		NULL, NULL
	))
	{
		return FALSE;
	}
	return TRUE;
}

bool CIOCPServer::PostAccept(COverlappedIOInfo* pInfo)
{
	// �����Ϻ���ָ�벻����NULL
	if (m_lpfnAcceptEx == NULL)
	{
		return FALSE;
	}

	// �Ȱѵ�ǰ��socket����һ�£����socket�п�����Invalid_sockets��Ҳ�п����ǹ����߳��Ǳߴ��������Ѿ��ù���socket
	SOCKET socket = pInfo->m_clientSocket;

	// ���ָ����Ļ�������ԭ���ṹ��ͬʱ�����µĿͻ����׽���
	pInfo->ResetSendBuffer();
	pInfo->ResetRecvBuffer();
	pInfo->ResetOverlapped();
	pInfo->m_clientSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	pInfo->m_type = IOEventType::TYPE_ACP;

	if (pInfo->m_clientSocket == INVALID_SOCKET)
	{
		// �����µĿͻ����׽���ʧ��
		return FALSE;
	}

	// ����AcceptEx����accept���󣬴����´����Ľṹ
	DWORD byteReceived = 0;
	if (FALSE == m_lpfnAcceptEx
	(
		m_sListen,		// �������׽���
		pInfo->m_clientSocket,		// �����Ŀͻ����׽���
		pInfo->m_recvBuf.buf,		// ������
		pInfo->m_recvBuf.len - (sizeof(SOCKADDR_IN) + 16) * 2,		// ��������С
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&byteReceived,
		pInfo		// ����Ľṹ�ڹ����߳������ܹ�����
	))
	{
		// AcceptEx�������FALSE��һ������ʧ�ܣ������������997��ʵҲ�ǳɹ�
		if (ERROR_IO_PENDING != WSAGetLastError())
		{
			// ��������벻��997�Ǿ���ʧ��
			return FALSE;
		}
	}

	// ��������������û��ʹ�ù����׽��֣��滻��δʹ�õ��׽���
	std::vector<SOCKET>::iterator iter = m_acceptSockets.begin();
	for (; iter != m_acceptSockets.end(); ++iter)
	{
		if (*iter == socket)
		{
			*iter = pInfo->m_clientSocket;
		}
	}

	// �б���û���ҵ�socket
	if (iter == m_acceptSockets.end())
	{
		m_acceptSockets.push_back(pInfo->m_clientSocket);
	}

	return TRUE;
}

bool CIOCPServer::PostRecv(COverlappedIOInfo* pInfo)
{
	DWORD BytesRecevd = 0, dwFlags = 0;

	// �����WSA�ṹ����
	pInfo->ResetOverlapped();
	pInfo->ResetRecvBuffer();

	// ��ʶ��
	pInfo->m_type = IOEventType::TYPE_RECV;

	// WSARecvӦ��һ���׽���һ��ֻ��Ͷ��һ��������WSASend����һ��Ͷ�ݶ��
	if (WSARecv(pInfo->m_clientSocket, &pInfo->m_recvBuf, 1, &BytesRecevd, &dwFlags, (OVERLAPPED*)pInfo, NULL) != 0)
	{
		// WSARecv�������0��һ������ʧ�ܣ������������997��ʵҲ�ǳɹ�
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			// ��������벻��997�Ǿ���ʧ��
			return FALSE;
		}
	}
	return TRUE;
}

bool CIOCPServer::PostSend(COverlappedIOInfo* pInfo, const std::string& strData)
{
	DWORD ByteSend = 0, dwFlags = 0;

	// �����ݷŽ�WSABUF�ṹ��
	pInfo->ResetOverlapped();
	pInfo->ResetSendBuffer();

	// ��ʶ��
	pInfo->m_type = IOEventType::TYPE_SEND;
	strcpy_s(pInfo->m_csendBuf, strData.c_str());

	// ����WSASend��������
	if (WSASend(pInfo->m_clientSocket, &pInfo->m_sendBuf, 1, &ByteSend, dwFlags, (OVERLAPPED*)pInfo, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			// ��������벻��997�Ǿ���ʧ��
			return FALSE;
		}
	}
	return TRUE;
}

std::vector<std::string> CIOCPServer::GetOnlineIPs()
{
	// �Զ��������������������������ʱ�Զ�����
	std::lock_guard<std::mutex> lock(m_mu);
	std::vector<std::string> lstIps;

	//// ѭ������vector
	//for (auto item : m_clientSockets)
	//{
	//	char ip[INET_ADDRSTRLEN] = { '\0' };
	//	inet_ntop(AF_INET, &item->m_addr.sin_addr.S_un.S_addr, ip, 16);

	//	lstIps.push_back(inet_ntoa(item->m_addr.sin_addr));
	//}
	return lstIps;
}

void CIOCPServer::DelectLink(SOCKET socket)
{
	// �Զ��������������������������ʱ�Զ�����
	std::lock_guard<std::mutex> lock(m_mu);

	for (auto iter = m_clientSockets.begin(); iter != m_clientSockets.end(); iter++)
	{
		if ((*iter)->m_clientSocket == socket)
		{
			// ����ҵ���ͬ��socket�Ǿ�ɾ�������ṹ
			// ���ܹ�ֱ��ɾ����Ҫ�ѵ�����ȡַ����ָ�룬��ɾ�������ָ�룬�����ָ��ָ��DoAccept�ﴴ������Ƭ�ڴ棬��ȡֱַ��delete��ɾ�����������
			COverlappedIOInfo* ol = *iter;
			closesocket(socket);
			m_clientSockets.erase(iter);
			delete ol;
			break;
		}
	}
}

bool CIOCPServer::DoAccept(COverlappedIOInfo* pInfo, DWORD NumberOfBytes)
{
	// �Զ��������������������������ʱ�Զ�����
	std::lock_guard<std::mutex> lock(m_mu);

	// AcceptEx������Ҫ���յ�һ���ֽڲŻᴥ��������NumberOfBytesһ������0
	SOCKADDR_IN* ClientAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	if (NumberOfBytes > 0)
	{
		// �յ���һ֡����
		if (m_lpfnGetAcceptSockAddrs)
		{
			// GetAcceptExSockaddrs����ָ�����
			/*���յ����ݷ�Ϊ3���֣�1. �ͻ��˷���������  2. ���ص�ַ  3. Զ�˵�ַ*/
			SOCKADDR_IN* LocalAddr = NULL;
			int localLen = sizeof(SOCKADDR_IN);

			m_lpfnGetAcceptSockAddrs(
				pInfo->m_recvBuf.buf,
				pInfo->m_recvBuf.len - (sizeof(SOCKADDR_IN) + 16) * 2,
				sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16,
				(LPSOCKADDR*)&LocalAddr,
				&localLen,
				(LPSOCKADDR*)&ClientAddr,
				&remoteLen
			);

			// �����ȡ���Զ˵�ַ���ͽ�������ͻ��ˣ������������ӣ������ȡ�������Ͳ�����ֱ�ӷ���
			if (ClientAddr != NULL)
			{
				// �����ȡ���Զ˵�ַ
				COverlappedIOInfo* olinfo = new COverlappedIOInfo();		// �µĽṹ�ᱻ�ŵ�m_clientSockets��
				olinfo->m_clientSocket = pInfo->m_clientSocket;
				olinfo->m_addr = *ClientAddr;

				// Ϊ����ͻ��˵��׽��ְ�Recv����ɶ˿�
				if (m_iocp.AssociateSocket(olinfo->m_clientSocket, TYPE_RUN))
				{
					// �󶨳ɹ����ѽṹ�ŵ�����Ѿ����ӵĽṹ�������ﲢͶ��Recv
					PostRecv(olinfo);
					m_clientSockets.push_back(olinfo);
					return FALSE;
				}
				else
				{
					// ��ʧ�ܾ�ɾ�������Ľṹ����������
					delete olinfo;
					return FALSE;
				}
			}
		}
	}
	return FALSE;
}