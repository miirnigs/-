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

bool Sqlite::OpenDB(std::string path)
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

bool Sqlite::GetTables(std::vector<std::string>& lstTable)
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
			lstTable.push_back(std::string(name));
		}
		sqlite3_finalize(pStmt);
		pStmt = nullptr;
		return TRUE;
	}
	return FALSE;
}

bool Sqlite::GetColumn(std::string strTable, std::string strColum)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	bool result = FALSE;
	if (!Sqlite::GetInstance().Exec(Converter::combine_string("SELECT *FROM sqlite_master WHERE name = '", strTable, "' AND sql LIKE '%", strColum, "%'"), [](void* pThis, int argc, char** argv, char** col)->int {
		// �з��ؽ���ͻ�����ص��������ѯ����Ӧ������
		// ò��һ��Ҫ����0��0Ӧ���Ǽ������ң���ֹͣ
		(*(bool*)pThis) = TRUE;		// ��Ϊ����������void��ǿת��bool��ȡַ�Ϳ����޸�ֵ
		return 0;
		}, &result))
	{
		return FALSE;
	}
		return result;
}

bool Sqlite::GetData(std::string tableName, std::vector<std::vector<std::string> >& lstData)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	// ����exec��װ
	if (!Sqlite::GetInstance().Exec(Converter::combine_string("SELECT *FROM ", tableName).c_str(), [](void* pThis, int argc, char** argv, char** col)->int {
		// �з��ؽ���ͻ�����ص��������ѯ����Ӧ������
		// ò��һ��Ҫ����0��0Ӧ���Ǽ������ң���ֹͣ
		for (int i = 0; i < argc; i++)
		{
			if (!i)
			{
				// i = 0��������ʹ���һ����vector��֮������ݾ�ֱ��push_back�����vector�����
				// vector֮��ǵø��ո�
				(*(std::vector<std::vector<std::string> >*)pThis).push_back(std::vector<std::string>(1, argv[i]));		// ��ʼ��һ��ֵ��ֵΪargv[0]
			}
			else if (i && argv[i] != NULL)		// ע�⣡������һ����������˼���Сʱ�ŷ��֣����ݿⷵ��ֵ�п��ܷ���NULL������Ҫ��NULL�ж�
			{
				// ���������ݲ��뵽vector����
				(*(std::vector<std::vector<std::string> >*)pThis).back().push_back(argv[i]);
			}
		}
		return 0;
		}, &lstData))
	{
		return FALSE;
	}
		return TRUE;
}

std::string Sqlite::GetLastError()const
{
	// ��ȡ������ϢVector���һ��Ԫ��
	if (lstError.size())
	{
		return Converter::combine_string(lstError.back().first, "|", lstError.back().second);
	}
	return "No error have been finded";
}

bool Sqlite::Exec(std::string sql, int(*CallBack)(void*, int, char**, char**), void* pThis)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	char* pError = nullptr;
	if (sqlite3_exec(hSqlite, sql.c_str(), (*CallBack), pThis, &pError) != SQLITE_OK)
	{
		// TODO: ����Function�����ʱ�亯��
		tm time = Time::get_date_time();

		lstError.push_back(make_pair(Converter::combine_string(
			time.tm_year + 1900, "��",
			time.tm_mon + 1, "��",
			time.tm_mday, "�� ",
			time.tm_hour, ":",
			time.tm_min, ":",
			time.tm_sec), std::string(pError)));
		return FALSE;
	}

	return TRUE;		// �ɹ�����true
}

bool Sqlite::Exec(std::string sql)
{
	if (!IsConnected())
	{
		return FALSE;
	}

	char* pError = nullptr;
	if (sqlite3_exec(hSqlite, sql.c_str(), NULL, NULL, &pError) != SQLITE_OK)
	{
		tm time = Time::get_date_time();


		lstError.push_back(make_pair(Converter::combine_string(
			time.tm_year + 1900, "��",
			time.tm_mon + 1, "��",
			time.tm_mday, "�� ",
			time.tm_hour, ":",
			time.tm_min, ":",
			time.tm_sec), std::string(pError)));
		return FALSE;
	}

	return TRUE;		// �ɹ�����true
}



