#include <opencv2/opencv.hpp>
using namespace std;

void main()
{
	static const int numCams = 6;//����ͷ����
	vector<CvCapture*> cap(numCams);
	for (int i = numCams - 1; i >= 0; i--)
	{
		cap[i] = cvCaptureFromCAM(i);
		cvSetCaptureProperty(cap[i], CV_CAP_PROP_FPS, 30);//֡��
		cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_WIDTH, 1280);//���ؿ�
		cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_HEIGHT, 720);//���ظ�
		if (!cap[i])
		{
			cout << "create camera" << i + 1 << " capture error" << endl;
			system("pause");
			exit(-1);
		}
	}
	cvNamedWindow("img1", 1); cvNamedWindow("img2", 1); //����ʱע�͵�����һ��
	cvNamedWindow("img3", 1); cvNamedWindow("img4", 1); cvNamedWindow("img5", 1); cvNamedWindow("img6", 1);
	vector<IplImage*> img(numCams);
	while (1)
	{
		for (int i = 0; i < numCams; i++)
		{
			img[i] = cvQueryFrame(cap[i]);
		}
		cvShowImage("img1", img[0]);
		cvShowImage("img2", img[1]);
		cvShowImage("img3", img[2]);
		cvShowImage("img4", img[3]);
		cvShowImage("img5", img[4]);
		cvShowImage("img6", img[5]);
		cvWaitKey(3);
	}
	cvDestroyAllWindows();
	for (int i = 0; i < numCams; i++)
	{
		cvReleaseCapture(&cap[i]);
		cvReleaseImage(&img[i]);
	}
	system("pause");
}