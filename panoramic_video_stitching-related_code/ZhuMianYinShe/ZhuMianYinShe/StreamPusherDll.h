// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� STREAMPUSHERDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// STREAMPUSHERDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#include <stdio.h>
#define __STDC_CONSTANT_MACROS

#ifdef STREAMPUSHERDLL_EXPORTS
#define STREAMPUSHERDLL_API __declspec(dllexport)
#else
#define STREAMPUSHERDLL_API __declspec(dllimport)
#endif

// �����Ǵ� StreamPusherDll.dll ������
class STREAMPUSHERDLL_API CStreamPusherDll {
public:
	CStreamPusherDll(void);
	// TODO:  �ڴ�������ķ�����
};

extern STREAMPUSHERDLL_API int nStreamPusherDll;

STREAMPUSHERDLL_API int fnStreamPusherDll(void);

extern "C"
{
	STREAMPUSHERDLL_API void  encodeAndPushInit(int Fwidth, int Fheight, const char* pushAddress);
	STREAMPUSHERDLL_API void  encodeAndPush(int Fwidth, int Fheight, unsigned char* tpData);
	STREAMPUSHERDLL_API void  pushEnd();
}