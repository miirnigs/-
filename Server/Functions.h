#pragma once
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <sstream>
#include <string>
using namespace std;

// һ����Ҫ�ĺ���������
namespace functions
{
	/*ʱ����*/
	/*PS: ���������ʼ��*/
	class Time
	{
	public:
		Time() = delete;		// ��ʼ�����캯��
		~Time() = default;

		// ��ȡ��ǰʱ�䣬����tm�ṹ
		static tm GetDateTime();
	};
	
	/*�ַ���ת����*/
	/*PS: ���������ʼ��*/
	class Converter
	{
	public:
		Converter() = delete;;		// ��ʼ�����캯��
		~Converter() = default;

		// �����д�������ݺϲ�ת��string
		template <typename T, typename... Arg>
		static string CombineString(const T& t, Arg&&... arg)		// һ�����ø�һ����ֵ����
		{
			stringstream ss;
			ss << t;
			return ss.str() + CombineString(forward<Arg>(arg)...);		// ����ת��
		}

		// ��ֹ�ݹ麯��
		template <typename T>
		static string CombineString(const T& t)
		{
			stringstream ss;
			ss << t;
			return ss.str();
		}
	};

	/*·����*/
	/*PS: ���������ʼ��*/
	class Path
	{
	public:
		Path() = delete;;		// ��ʼ�����캯��
		~Path() = default;
		
		// ��ȡ��ǰEXE·����˫б�ܽ�β(Ĭ�ϲ����ص�ǰ�ļ���)
		static string GetExePath(bool fileName = FALSE);
	};
}