#ifndef CHONGHESTITCH_H
#define CHONGHESTITCH_H
#include <iostream>
#include <cv.h>
#include <cmath>
class ChongheStitch{
public:
	cv::Mat chongHeStitch(cv::Mat& img1, cv::Mat& img2, int chongheWidth);//���img1��img2���ںϣ������������ڵ��Ѿ��ںϹ���ͼ�����м䲿�ֵõ������򣩣����һ��������ʾ�غ�������
	//Ϊʹ�ö��̣߳��ںϹ�����Ϊʹ���ںϵ�ͼ���ܹ������߳���ʼ�մ��ڣ������ĵİ汾��Ҳ��Ϊ�˺����ں϶����ĸ���
	void chongHeStitch3Init(cv::Mat& img1, cv::Mat& img2, int chongheWidth, int& resultWidth, int& resultHeight,int& central1, int& central2);
	void chongHeStitch3(cv::Mat& img1, cv::Mat& img2, cv::Mat& imgResult, int chongheWidth, int& central1, int& central2);//����һ������������ͬ��central1��central2�ֱ��ʾimg1��img2ƴ�ӵı�Ե�������������м�λ�ã�������Կ���һ������
};
#endif

