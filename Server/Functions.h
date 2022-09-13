#pragma once

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
		static tm get_date_time();
	};
	
	/*�ַ���ת����*/
	/*PS: ���������ʼ��*/
	class Converter
	{
	public:
		Converter() = delete;;		// ��ʼ�����캯��
		~Converter() = default;

		// �����д�������ݺϲ�ת��string
		template <class ...Args>
		static auto combine_string(Args ...args)->std::string
		{
			std::ostringstream os;
			int arr[] = { (([](std::ostringstream& os, auto t) {		// C++ 14��������������ģ�廯
				os << t;
				})(os,args),0)... };		// ���������ϲ��ַ�������ʼ���б�չ��������
			return os.str();
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
		static std::string get_exe_path(bool fileName = FALSE);
	};
}