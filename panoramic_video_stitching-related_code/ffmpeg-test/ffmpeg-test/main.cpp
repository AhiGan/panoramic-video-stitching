#include "mp42yuv.h"
#include "yuv2jpgƫ��.h"
#include <iostream>
#include <sstream>
#include <direct.h>
using namespace std;

void main()
{
	stringstream ss;
	char foldername[100], yuvname[3];
	string pathname;
	cout << "��ע�⣺mp4�ļ�Ӧ��˳ʱ��˳���ţ�\n���������0.mp4~5.mp4���ļ��е����ƣ���ffmpeg-test��Ŀ¼�£���\n";
	cin >> pathname;
	for (int i = 0; i < 6; i++)
	{
		ss << pathname << "//" << i;
		ss >> foldername;
		if (_mkdir(foldername) == -1)//�����ļ���
		{
			cout << "�ļ���" << i << "�Ѵ��ڣ���ɾ����\n";
			return;
		}
		int result = FileWriteYUV(foldername);//mp4תyuv
		if (result == -1)
		{
			cout << "some error in FileWriteYUV" << endl;
			return;
		}
		FileWriteFrames(foldername);//yuvתjpg
		ss.clear();
	}
	for (int i = 0; i < 6; i++)//ɾ��yuv�ļ�
	{
		ss << pathname << "//" << i << ".yuv";
		ss >> yuvname;
		remove(yuvname);
		ss.clear();
	}
}