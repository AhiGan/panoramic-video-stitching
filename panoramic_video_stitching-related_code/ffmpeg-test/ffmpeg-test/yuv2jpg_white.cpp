#include "yuv2jpgƫ��.h"
#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <fstream>
#include <io.h>

using namespace std;
using namespace cv;

#define FCount 1000//��С����
#define ISizeX 1280
#define ISizeY 720

unsigned char Y[FCount][ISizeX][ISizeY];
unsigned char buf[FCount][ISizeY / 2][ISizeX / 2];
unsigned char buf2[FCount][ISizeY / 2][ISizeX / 2];


// ��ͼƬ�ļ�д��
void FileWriteFrames(char* foldername)
{
	char yuvpath[100] = "";
	sprintf(yuvpath, "%s%s", foldername, ".yuv");
	FILE* file = fopen(yuvpath, "rb");
	int size;
	if (file)
	{
		size = _filelength(_fileno(file));
		size /= (ISizeX * ISizeY * 1.5);
		fclose(file);
	}
	ifstream readMe(yuvpath, ios::in | ios::binary);//�򿪲���yuv����
	IplImage *image, *rgbimg, *yimg, *uimg, *vimg, *uuimg, *vvimg;
	rgbimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 3);
	image = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 3);

	yimg = cvCreateImageHeader(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);//���ȷ���
	uimg = cvCreateImageHeader(cvSize(ISizeX / 2, ISizeY / 2), IPL_DEPTH_8U, 1);//����������ɫ�ȷ���
	vimg = cvCreateImageHeader(cvSize(ISizeX / 2, ISizeY / 2), IPL_DEPTH_8U, 1);

	uuimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);
	vvimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);
	int nframes;
	for (nframes = 0; nframes < size; nframes++)
	{
		char nframesstr[50];
		readMe.read((char*)Y[nframes], ISizeX*ISizeY);
		readMe.read((char*)buf[nframes], ISizeX / 2 * ISizeY / 2);
		readMe.read((char*)buf2[nframes], ISizeX / 2 * ISizeY / 2);
		cvSetData(yimg, Y[nframes], ISizeX);
		cvSetData(uimg, buf[nframes], ISizeX / 2);
		cvSetData(vimg, buf2[nframes], ISizeX / 2);

		cvResize(uimg, uuimg, CV_INTER_LINEAR);
		cvResize(vimg, vvimg, CV_INTER_LINEAR);
		cvMerge(yimg, vvimg, uuimg, NULL, image);//�ϲ���ͨ��Ϊ��ͨ��
		cvCvtColor(image, rgbimg, CV_YUV2BGR);

		stringstream ss;//����ת��ͳһת��Ϊchar* ����
		ss << foldername;
		ss << "//";
		ss << nframes;//�ļ�����0��ʼ����
		ss << ".jpg";
		cout << "д��ȥ1֡" << endl;
		ss >> nframesstr;
		cvSaveImage(nframesstr, rgbimg);
	}
	readMe.close();
	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);
	cvReleaseImage(&image);
}


