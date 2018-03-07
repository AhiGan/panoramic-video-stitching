#include "CylindricalMapping2.h"
#include <omp.h>

CylindricalMapping2::CylindricalMapping2(double fvalue, cv::Mat* imageIn)
{
	width = imageIn[0].cols;
	height = imageIn[0].rows;
	f = fvalue;
	in = imageIn;
	mapWidth = 2 * f*atan(double(width) / 2 / f);
	mapHeight = height;
	for (int i = 0; i < 6;i++)
	{
		out[i] = cv::Mat::zeros(mapHeight, mapWidth, imageIn[i].type());
	}
	angle2 = atan((double)width / 2 / f);
}


Position CylindricalMapping2::mappingPointCaculate(int newx1, int newy1)
{
	angle = newx1 / f;
	double x, y;//�ֱ��ʾӳ�䵽ԭͼ��x��y���� ʹ�ö��߳�ʱ���� ʹ�õ��߳�ʱ����
	if (angle <= angle2)
		x = (double)width / 2 - tan(angle2 - angle)*f;
	else
		x = (double)width / 2 + tan(angle - angle2)*f;
	if (newy1 <= (double)height / 2)
		y = (double)height / 2 - ((double)height / 2 - newy1) / cos(abs(angle2 - angle));
	else
		y = (double)height / 2 + (newy1 - (double)height / 2) / cos(abs(angle2 - angle));
	return Position(x, y);
}

bool CylindricalMapping2::cylindricalMapThread(int hBegin, int hEnd, int wBegin, int wEnd, int picIndex)
{
	//����ͶӰ���ͼ��
	uchar s0[3];
	uchar s1[3];
	uchar s2[3];
	uchar s3[3];
	uchar s4[3];
	Position p(0, 0);
	double x, y;
	double u, v;//���ڼ�¼��ȡ��x��y��С������
	//��������
	for (int i = hBegin; i < hEnd; i++)
	{
		for (int j = wBegin; j < wEnd; j++)
		{
			p = mappingPointCaculate(j, i);
			x = p.x;
			y = p.y;
			if (x >= 0 && x < width - 1 && y >= 0 && y < height - 1)//�жϵõ��ĵ��Ƿ��䵽ԭʼͼ���ڲ�
			{
				u = x - int(x); v = y - int(y);	
				for (int i = 0; i < 3;i++)
				{
					s0[i] = in[picIndex].at<cv::Vec3b>(int(y), int(x))[i];
					s1[i] = in[picIndex].at<cv::Vec3b>(int(y), int(x)+1)[i];
					s2[i] = in[picIndex].at<cv::Vec3b>(int(y)+1, int(x))[i];
					s3[i] = in[picIndex].at<cv::Vec3b>(int(y)+1, int(x)+1)[i];
					s4[i] = int((1 - u)*(1 - v)*s0[i] + (1 - v)*u*s1[i] + v*(1 - u)*s2[i] + u*v*s3[i]);//˫���Բ�ֵ����
				}
			}
			else if (x == width - 1 || y == height - 1)//������䵽ԭͼ���Ե
			{
				for (int i = 0; i < 3;i++)
				{
					s4[i] = in[picIndex].at<cv::Vec3b>(int(y), int(x))[i];
				}
			}
			else
			{
				s4[0] = s4[1] = s4[2] = 0;
			}
			for (int k = 0; k < 3;k++)
			{
				out[picIndex].at<cv::Vec3b>(i, j)[k] = s4[k];
			}
		}
	}
	return true;
}

bool CylindricalMapping2::cylindricalMapping()
{
	//ͼ��ӳ�䱾��ʹ�ö��߳� ����ͼ��ӳ��֮��ʹ�õ��߳�
	//ʹ�ö��̹߳�������
	static const int numThreads = 8;
	std::vector<std::thread> cylindricalMapThreads(numThreads);
	for (int index = 0; index < 6;index++)
	{
		for (int i = 0; i < numThreads; i++)
		{
			switch (i)
			{
			case 0:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, 0, mapHeight / 2, 0, mapWidth / 4, index);
				break;
			case 1:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, mapHeight / 2, mapHeight, 0, mapWidth / 4, index);
				break;
			case 2:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, 0, mapHeight / 2, mapWidth / 4, mapWidth / 2, index);
				break;
			case 3:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, mapHeight / 2, mapHeight, mapWidth / 4, mapWidth / 2, index);
				break;
			case 4:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, 0, mapHeight / 2, mapWidth / 2, mapWidth * 3 / 4, index);
				break;
			case 5:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, mapHeight / 2, mapHeight, mapWidth / 2, mapWidth * 3 / 4, index);
				break;
			case 6:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, 0, mapHeight / 2, mapWidth * 3 / 4, mapWidth, index);
				break;
			case 7:
				cylindricalMapThreads[i] = std::thread(&CylindricalMapping2::cylindricalMapThread, this, mapHeight / 2, mapHeight, mapWidth * 3 / 4, mapWidth, index);
				break;
			default:
				std::cout << "ͼ��ƴ���߳����������" << std::endl;
				break;
			}
		}
		for (int i = 0; i < numThreads; i++)
		{
			cylindricalMapThreads[i].join();
		}
	}
	return true;
}
cv::Mat* CylindricalMapping2::getMapresult()
{
	return out;
}
CylindricalMapping2:: ~CylindricalMapping2(){}