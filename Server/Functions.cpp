#include "Functions.h"

tm functions::Time::GetDateTime()
{
	time_t now = time(0);
	tm types;		// ָ�򱾵�ʱ���tm�ṹ��

	localtime_s(&types, &now);
	return types;
}

string functions::Path::GetExePath(bool fileName)
{
	char szFilePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

	if (!fileName)
	{
		// ����Ҫ�ļ���
		(strrchr(szFilePath, '\\'))[1] = 0;
	}
	return string(szFilePath);
}
