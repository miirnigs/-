#pragma once
// Ҫ��Ԥ����ͷ�Ļ���SQLiteһ��Ҫ�þ�̬���ģʽ����
// �����Դ��sqlite3.c�Ļ���ǿ����Ԥ����ͷ�ᴥ������:
// error C2857: ��Դ�ļ���û���ҵ��� /Ycpch.h ������ѡ��ָ���ġ�#include�����

// ͨ�õ�ͷ�ļ�
#include "framework.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <sstream>
#include <io.h>

#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>		// ���µ�ͷ�ļ�
#include <MSWSock.h>		// һ��Ҫ����WinSock2.h������
#pragma comment(lib, "ws2_32.lib")
#include <functional>
#include <utility>

#include <WS2tcpip.h>

// �Զ������Ϣ
#define WM_LIST_CHANGE		(WM_USER + 150)		// �Զ�����Ϣ�����������濨�ܶԻ����ܶԻ���������
#define WM_LIST_ADD			(WM_USER + 151)		// �Զ�����Ϣ������������¼��б��������
#define WM_START_IOCP		(WM_USER + 152)		// �Զ�����Ϣ��֪ͨ�����濪��IOCP������
#define WM_STOP_IOCP		(WM_USER + 153)		// �Զ�����Ϣ��֪ͨ������ر�IOCP������

// �Զ����ʶ��
#define TYPE_RUN	NULL	// IOCP��CompKey��ָʾ�̼߳�������
#define TYPE_CLOSE	1		// IOCP��CompKey��ָʾ�߳�ֹͣ����