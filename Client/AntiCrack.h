#pragma once
#include <string>
#include <objbase.h>
#include <random>
using namespace std;

class AntiCrack
{
public:
	/// <summary>
	/// ����һ������ַ���
	/// (Ϊ�˷�ֹ��random�����¶ϵ㣬����GUID����C++11������random����������ַ���)
	/// </summary>
	/// <returns>���ɵ�����ַ���</returns>
	string GetRandomString()
	{
		GUID guid;
		HRESULT hResult = CoCreateGuid(&guid);
		if (hResult == S_OK)
		{
			return ([](const GUID& guid)->string 
			{
				char buf[64] = { 0 };
				sprintf_s(buf, sizeof(buf),
					"%08X%04x%04x%02X%02x%02x%02x%02X%02x%02x%02x",
					guid.Data1, guid.Data2, guid.Data3,
					guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
					guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
				return string(buf);
				})(guid);
		}
		else
		{
			string buffer;		// buffer: ���淵��ֵ
			std::default_random_engine e((unsigned int)time(nullptr));
			std::uniform_int_distribution<int> u(0, 35);		// ����ұ�����

			for (int i = 0; i < 32; i++) {
				char tmp = u(e);		// ���һ��С�� 36 ��������0-9��a-z �� 36 ���ַ�
				if (tmp < 10) 
				{			
					// ��������С�� 10���任��һ�����������ֵ� ASCII
					tmp += '0';
				}
				else
				{	
					// ���򣬱任��һ��Сд��ĸ�� ASCII
					tmp -= 10;
					tmp += 'a';
				}
				buffer += tmp;
			}
			return buffer;
		}
	}
};

