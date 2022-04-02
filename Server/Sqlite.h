#pragma once
#include "Functions.h"
#include <iostream>
#include <Windows.h>
#include <sqlite3.h>
#include <vector>
#include <utility>
using namespace functions;

 /*����SQLite���ݿ�*/
class Sqlite
{
	/*ԭ��������������ɻ���ģ��뵽ֻ��һ�����ݿ⣬�������ɵ�����һ��*/
private:
	Sqlite();		// ˽��Ĭ�Ϲ��캯������ֹ���⹹��
	Sqlite(const Sqlite&) = delete;		// ɾ����������
	Sqlite& operator=(const Sqlite&) = delete;
public:
	~Sqlite();
public:
	// ��ȡ��ʵ�������صľֲ��������ã����Բ��ü���
	static Sqlite& GetInstance();
public:
	// �����ݿ�
	bool OpenDB(string path);
	// �ر����ݿ�
	void CloseDB();
	// ���ݿ��Ƿ�������
	bool IsConnected()const;
	// ��ȡ���Ĵ�����Ϣ
	string GetLastError()const;

	// ��ȡ���б�(vector�����ã����ȡ�õı���)
	// PS: �޷���ȡ������Ϣ
	bool GetTables(vector<string>& lstTable);
	// �ж϶�Ӧ�ı����Ƿ���ڶ�Ӧ������
	bool GetColumn(string strTable, string strColum);

	// ��ָ���ı��ȡ��������(Ҫ��ѯ�ı�����һ����άvector���������ȡ�õ�����)
	// PS: vector��vector֮��Ҫ��һ���ո񣬲�Ȼ�������ᵱ��>>λ�������vector�ﲻ�ܴ�char*����Ϊchar*ָ��ĵ�ַ�ں�����������ͷ��ˣ����ⲿ�Ƕ�ȡ�����ģ����Թ���
	// ���ص�������|�ָ�����split���Էָ�
	bool GetData(string tableName, vector<vector<string> >& lstData);

	// ִ��SQL���(sql��� | �ص����� | voidָ��[�ᱻ���ɵ�һ����������ص�����])
	// PS: ���Ƕ�sqlite.exec�����ķ�װ�������ϻ���prepare->step->column->finalize
	bool Exec(string sql, int(*CallBack)(void*, int, char**, char**), void* pThis);
	// ִ��SQL���(sql���) û�лص�������ֻ����ʾ�ɹ�����ʧ��
	// PS:���Ƕ�sqlite.exec�����ķ�װ�������ϻ���prepare->step->column->finalize
	bool Exec(string sql);
private:
	sqlite3* hSqlite;		// ���ݿ���

	/*PS: �ܿӣ��������pair�������vectorҪ����һ���ո񣡣���Ȼ��������ʶ��� >> λ���������±������*/
	vector<pair<string, string> > lstError;		// ������Ϣ(ʱ�� | ������Ϣ)

};