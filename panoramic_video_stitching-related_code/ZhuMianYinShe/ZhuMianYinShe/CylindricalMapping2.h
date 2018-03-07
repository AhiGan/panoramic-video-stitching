#ifndef CYLINMAP2_H
#define CYLINMAP2_H
#include <cmath>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <thread>
#include <mutex>

struct Position
{
	double x;
	double y;
	Position(double x1, double y1){ x = x1; y = y1; }
};

//ע�� ����Ĭ�ϴ������6������ͷ��6��ͼ��
class CylindricalMapping2
{
public:
	CylindricalMapping2(double fvalue, cv::Mat* imageIn);
	Position mappingPointCaculate(int newx1, int newy1);//��ȡ������һ����ڵ�ͼ���ϵ�λ��
	bool cylindricalMapThread(int hBegin, int hEnd, int wBegin, int wEnd, int picIndex);//Ϊ�������ӳ���ٶ� ���ٶ��߳�,�����ֱ��ʾ���̴߳���õ�������ӳ�䲿�ֵĿ�ʼλ�úͽ���λ�ã����һ��������ʾ��ǰ�����ͼ����ţ�0��5��
	bool cylindricalMapping();//��ԭʼͼ����ӳ����Ӧ������ͼ�����浽out�У������Ƿ�ɹ�
	cv::Mat* getMapresult();//���صĶ���ӳ��ͼ��out
	~CylindricalMapping2();
private:
	int width, height;//ʵ��ͼ��Ŀ�͸�
	int mapWidth, mapHeight;
	double f;//���ͼ���Ӧ�Ľ���
	cv::Mat out[6];//���ڴ洢ӳ�䵽�����ϵ�6��ͼ��
	cv::Mat*  in;//���ڱ���ԭʼ6��ͼ��ĵ�ַ
	double angle;//���ڱ���ÿ�μ���Բ��ӳ���ʱ��Ӧ�ĽǶ�
	double angle2;//���ڱ���ԭʼͼ��������ĵĽǶȵ�һ��
	std::mutex mut;//��
};
#endif