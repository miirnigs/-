#include "pch.h"
#include "Sqlite.h"

Sqlite::Sqlite() :hSqlite(nullptr)
{
}

Sqlite::~Sqlite()
{
	CloseDB();		// �ر����ݿ�����
}

Sqlite& Sqlite::GetInstance()
{
	// ��̬�ֲ������
	static Sqlite instance;
	return instance;		// ���ص���Զ�������static��ַ
}

bool Sqlite::OpenDB(string path)
{
	// �������ݿ⣬��������ھͻᴴ��
	return (hSqlite || sqlite3_open(path.c_str(), &hSqlite) == SQLITE_OK);
}

void Sqlite::CloseDB()
{
	if (hSqlite && sqlite3_close(hSqlite) == SQLITE_OK)
	{
		hSqlite = nullptr;		// ���ݿ�������
	}
}

bool Sqlite::IsConnected()const
{
	return hSqlite;		// �о���ͻ᷵��true
}

bool Sqlite::GetTables(vector<string>& lstTable)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	/*��ΪҪ�ѱ����ŵ�vector�������Exec�ص���װȥ��ѯ��ܸ��ӣ�ֱ�ӵ���stmt�����*/
	sqlite3_stmt* pStmt;		// ���������������Ϊ���ܶ������ͬʱ�������ָ��
	sqlite3_prepare(hSqlite, "select name from sqlite_master where type='table' order by name", -1, &pStmt, 0);
	if (pStmt)
	{
		while (sqlite3_step(pStmt) == SQLITE_ROW)
		{
			char* name = (char*)sqlite3_column_text(pStmt, 0);
			lstTable.push_back(string(name));
		}
		sqlite3_finalize(pStmt);
		pStmt = nullptr;
		return TRUE;
	}
	return FALSE;
}

bool Sqlite::GetColumn(string strTable, string strColum)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	bool result = FALSE;
	if (!Sqlite::GetInstance().Exec(Converter::CombineString("SELECT *FROM sqlite_master WHERE name = '", strTable, "' AND sql LIKE '%", strColum, "%'"), [](void* pThis, int argc, char** argv, char** col)->int {
		// �з��ؽ���ͻ�����ص���������ѯ����Ӧ������
		// ò��һ��Ҫ����0��0Ӧ���Ǽ������ң���ֹͣ
		(*(bool*)pThis) = TRUE;		// ��Ϊ����������void��ǿת��bool��ȡַ�Ϳ����޸�ֵ
		return 0;
		}, &result))
	{
		return FALSE;
	}
		return result;
}

bool Sqlite::GetData(string tableName, vector<vector<string> >& lstData)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	// ����exec��װ
	if (!Sqlite::GetInstance().Exec(Converter::CombineString("SELECT *FROM ", tableName).c_str(), [](void* pThis, int argc, char** argv, char** col)->int {
		// �з��ؽ���ͻ�����ص���������ѯ����Ӧ������
		// ò��һ��Ҫ����0��0Ӧ���Ǽ������ң���ֹͣ
		for (int i = 0; i < argc; i++)
		{
			if (!i)
			{
				// i = 0��������ʹ���һ����vector��֮������ݾ�ֱ��push_back�����vector�����
				// vector֮��ǵø��ո�
				(*(vector<vector<string> >*)pThis).push_back(vector<string>(1, argv[i]));		// ��ʼ��һ��ֵ��ֵΪargv[0]
			}
			else if (i && argv[i] != NULL)		// ע�⣡������һ����������˼���Сʱ�ŷ��֣����ݿⷵ��ֵ�п��ܷ���NULL������Ҫ��NULL�ж�
			{
				// ���������ݲ��뵽vector����
				(*(vector<vector<string> >*)pThis).back().push_back(argv[i]);
			}
		}
		return 0;
		}, &lstData))
	{
		return FALSE;
	}
		return TRUE;
}

string Sqlite::GetLastError()const
{
	// ��ȡ������ϢVector���һ��Ԫ��
	if (lstError.size())
	{
		return Converter::CombineString(lstError.back().first, "|", lstError.back().second);
	}
	return "No error have been finded";
}

bool Sqlite::Exec(string sql, int(*CallBack)(void*, int, char**, char**), void* pThis)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	char* pError = nullptr;
	if (sqlite3_exec(hSqlite, sql.c_str(), (*CallBack), pThis, &pError) != SQLITE_OK)
	{
		tm time = Time::GetDateTime();

		lstError.push_back(make_pair(Converter::CombineString(
			time.tm_year + 1900, "��",
			time.tm_mon + 1, "��",
			time.tm_mday, "�� ",
			time.tm_hour, ":",
			time.tm_min, ":",
			time.tm_sec), string(pError)));
		return FALSE;
	}

	return TRUE;		// �ɹ�����true
}

bool Sqlite::Exec(string sql)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	char* pError = nullptr;
	if (sqlite3_exec(hSqlite, sql.c_str(), NULL, NULL, &pError) != SQLITE_OK)
	{
		tm time = Time::GetDateTime();


		lstError.push_back(make_pair(Converter::CombineString(
			time.tm_year + 1900, "��",
			time.tm_mon + 1, "��",
			time.tm_mday, "�� ",
			time.tm_hour, ":",
			time.tm_min, ":",
			time.tm_sec), string(pError)));
		return FALSE;
	}

	return TRUE;		// �ɹ�����true
}


