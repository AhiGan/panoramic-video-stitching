#include <opencv2/nonfree/nonfree.hpp>  
#include "CylindricalMapping2.h"
#include "ChongheStitch.h"
#include "StreamPusherDll.h"
#include <stdio.h>

using namespace cv;
using namespace std;

const char *out_filename = "result.avi";//ȫ����Ƶ·��

//��img1��img2����surf������⡢��������������ƥ��õ���Ӧ���󣨱任���󣩵����������ǵõ��ı任����,ע���������и�inlier_minRx���ڱ�ʾ��ͼ����ߵĺ���ͼ�غϵ�λ�ã�����Ĭ��Ϊ0�����������û���õ�������Ժ�Ҫ�ĵĻ��������ƴ�Ӻ���ҲҪ��Ӧ�ĸ�
void getHimprove(Mat& img1, Mat& img2, Mat& H);
//������õ�H�任�õ���׼λ�õı任����H1
void changeHtoH1Improve(Mat& img1, int& imgleftWidth, int& imgRightBeginHeight, Mat&H, Mat& H1);
//ͨ���任���� ������ͼ����ƴ����һ�����һ������������ͼ�غϲ��ֵĿ��
//������ĸĽ�����ͼ�任����ͼ�ӽǣ��к���ͼ�غ�����Ҳ�в��غ����򣬲��Ҳ��غ�����λ���غ��������࣬imgLeftWidth����������κ����ͼû�к���ͼ�غ�����Ŀ�ȣ�imgRightBeginHeight����������ͼ��ƴ��ͼ�еĿ�ʼλ�õĸ�
void framePinJieInit(Mat img[6], Mat img_result[6], Mat H[6], int imgLeftWidth[6], int width[6], int height[6], int guoduPexelNums);//��ɶ��߳�ͼ��ƴ�ӳ�ʼ����������Ϊ���̺߳�join()���ͷŵ�imgPinJieResult�����ݣ��ʲ����ȳ�ʼ��imgPinJieResult����ֹ����Ϊ���߳�join()�����ͷŵ���ʹ�ú�����ͼ���ںϿ��Լ���
void framePinJieNew3(Mat& img1, Mat& img2, Mat& img_result, Mat& H, int imgLeftWidth, int imgRightBeginHeight, int width, int height, int guoduPexelNums);//ǰ�������������framePinJieNew2�汾һ���� width��height������������img1ת��֮��Ŀ�͸�,���һ����������ƴ���ں�ʱ�������صĿ��
bool cylindricalStitch2(Mat img[6], Mat H[6], int imgLeftWidth[6], int imgRightBeginHeight[6], Mat& imgResult);//������ͼ��ͬʱƴ�ӣ���ȡȫ��ͼ�񣬲��ö��߳�

int main(int argc, char** argv)
{
	int frame_num;//��Ҫ���ɵ�֡��
	int mode;
	cout << "��ѡ��ģʽ��\n1.��Ƶ֡ƴ�ӣ�����1��\n2.USB����ͷʵʱƴ��(����2)\n";
	cin >> mode;
	Mat imgResult;//����֡
	int final_frame_width;//����֡������
	int final_frame_height;//����֡������
	Mat H[6];//��Ӧ����_����
	Mat H1[6];//��Ӧ����_final
	//��񻯱任����Ͳ��õ���Ӧƴ���������
	int imgleftwidth[6];
	int imgrightbeginheight[6];
	Mat MapPic[6];//��ȡ��������ӳ����ͼ��
	CvCapture* cap[6];//����ͷ�����֡
	stringstream ss;
	string folderpath;//ͼ��ƴ��ʱ���ļ���·��
	int cam[6] = { 0 };//ͼ��ƴ��ʱ����ʼ֡���
	string filename_temp;//��ʱ���ļ���
	VideoWriter writer;//��Ƶд��
	double final_video_fps;//ȫ����Ƶ֡��
	if (mode == 1)
	{
		cout << "�����������Ƶ֡�ļ���0~5���ļ���·����\n";
		cin >> folderpath;
		cout << "�밴˳������ÿ����ʼ֡�ı�ţ�\n";
		for (int i = 0; i < 6; i++)
		{
			cin >> cam[i];
		}
		cout << "������ȫ����Ƶ֡�ʣ�\n";
		cin >> final_video_fps;
	}
	else if (mode == 2)
	{
		int fps = 30, frame_width = 1280, frame_height = 720;
		cout << "����������ͷ֡�� ���ؿ� ���ظߣ�\n";
		cin >> fps >> frame_width >> frame_height;
		for (int i = 5; i >= 0; i--)//Ĭ�ϼ�������Դ���������ͷ
		{
			cap[i] = cvCaptureFromCAM(i);
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FPS, fps);//֡��
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_WIDTH, frame_width);//���ؿ�
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_HEIGHT, frame_height);//���ظ�
			if (!cap[i])
			{
				cout << "create camera" << i << " capture error" << endl;
				return -1;
			}
		}
	}
	cout << "��������Ҫ���ɵ�֡����\n";
	cin >> frame_num;
	cout << "\n��ȴ�...\n" << endl;
	for (int i = 0; i < frame_num; i++)
	{
		if (mode == 1)
		{
			for (int j = 0; j < 6; j++)
			{
				ss << folderpath << "//" << j << "//" << i + cam[j] << ".jpg";
				ss >> filename_temp;
				MapPic[j] = imread(filename_temp);
				ss.clear();
			}
		}
		else if (mode == 2)
		{
			for (int j = 0; j < 6; j++)
			{
				MapPic[j] = cvQueryFrame(cap[j]);
			}
		}

		//����ӳ�䣨���Խ�ԭʼͼ������ӳ��ĵ�ӳ��ͼ�� ʹ�ö��߳�ʵ�ָ���
		CylindricalMapping2 CylinMaper(615, MapPic);//С����� ��1 �뾶+����==615
		CylinMaper.cylindricalMapping();
		Mat *MapPic;
		MapPic = CylinMaper.getMapresult();

		if (i == 0)//��Ӧ����ͨ����һ��6��ͼ��ȡ
		{
			getHimprove(MapPic[0], MapPic[1], H[0]);
			getHimprove(MapPic[1], MapPic[2], H[1]);
			getHimprove(MapPic[2], MapPic[3], H[2]);
			getHimprove(MapPic[3], MapPic[4], H[3]);
			getHimprove(MapPic[4], MapPic[5], H[4]);
			getHimprove(MapPic[5], MapPic[0], H[5]);

			changeHtoH1Improve(MapPic[0], imgleftwidth[0], imgrightbeginheight[0], H[0], H1[0]);
			changeHtoH1Improve(MapPic[1], imgleftwidth[1], imgrightbeginheight[1], H[1], H1[1]);
			changeHtoH1Improve(MapPic[2], imgleftwidth[2], imgrightbeginheight[2], H[2], H1[2]);
			changeHtoH1Improve(MapPic[3], imgleftwidth[3], imgrightbeginheight[3], H[3], H1[3]);
			changeHtoH1Improve(MapPic[4], imgleftwidth[4], imgrightbeginheight[4], H[4], H1[4]);
			changeHtoH1Improve(MapPic[5], imgleftwidth[5], imgrightbeginheight[5], H[5], H1[5]);
		}

		cylindricalStitch2(MapPic, H1, imgleftwidth, imgrightbeginheight, imgResult);

		if (i == 0)//ͨ����һ��ȫ��ͼ�������ؿ��
		{
			if (imgResult.cols % 2 == 0)//ż������,����-1
				final_frame_width = imgResult.cols;
			else
				final_frame_width = imgResult.cols - 1;
			if (imgResult.rows % 2 == 0)//ż������,����-1
				final_frame_height = imgResult.rows;
			else
				final_frame_height = imgResult.rows - 1;
			//encodeAndPushInit(final_frame_width, final_frame_height, out_filename);
			writer.open(out_filename, CV_FOURCC('M', 'J', 'P', 'G'), final_video_fps, Size(final_frame_width, final_frame_height));
		}
		resize(imgResult, imgResult, Size(final_frame_width, final_frame_height), 0, 0, CV_INTER_LINEAR);
		//encodeAndPush(final_frame_width, final_frame_height, imgResult.data);//����&����
		writer << imgResult;
		cout << "��" << i + 1 << "֡������ϣ�\n";
	}
	//pushEnd();
	if (mode == 2)
	{
		for (int i = 0; i < 6; i++)//�ͷ�����ͷ
			cvReleaseCapture(&cap[i]);
	}
	remove("ds.h264");//ɾ���м��ļ�
	remove("output.yuv");
	return 0;
}

void getHimprove(Mat& img11, Mat& img22, Mat& H)
{
	int width = img11.cols / 2;//��Ϊimg1��img2�Ŀ�͸���ͬ 
	int height = img11.rows;
	Mat img1 = Mat::zeros(height, width, img11.type());
	Mat img2 = Mat::zeros(height, width, img22.type());
	uchar*p1_1 = img1.data;
	uchar*p1_2 = img11.data;
	uchar*p2_1 = img2.data;
	uchar*p2_2 = img22.data;

	for (int row = 0; row<height; row++)
	{
		p1_2 = img11.data + row*img11.step + width*img11.elemSize();
		p2_2 = img22.data + row*img22.step;
		for (int col = 0; col<width; col++)
		{
			*p1_1 = *p1_2; *(p1_1 + 1) = *(p1_2 + 1); *(p1_1 + 2) = *(p1_2 + 2);
			*p2_1 = *p2_2; *(p2_1 + 1) = *(p2_2 + 1); *(p2_1 + 2) = *(p2_2 + 2);
			p1_1 += img1.elemSize();
			p1_2 += img11.elemSize();
			p2_1 += img2.elemSize();
			p2_2 += img22.elemSize();
		}
	}

	initModule_nonfree();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ� 
	Ptr<FeatureDetector> detector = FeatureDetector::create("SURF");//����SIFT������������ɸĳ�SIFT/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("SURF");//���������������������ɸĳ�SIFT/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//��������ƥ����  
	if (detector.empty() || descriptor_extractor.empty())
		cout << "fail to create detector!";

	//��������  
	vector<KeyPoint> m_LeftKey, m_RightKey;
	detector->detect(img1, m_LeftKey);//���img1�е�SIFT�����㣬�洢��m_LeftKey��  
	detector->detect(img2, m_RightKey);

	//����������������������Ӿ��󣬼�������������  
	Mat descriptors1, descriptors2;
	descriptor_extractor->compute(img1, m_LeftKey, descriptors1);
	descriptor_extractor->compute(img2, m_RightKey, descriptors2);

	//����ƥ��  
	vector<DMatch> matches;//ƥ����  
	descriptor_matcher->match(descriptors1, descriptors2, matches);//ƥ������ͼ�����������  

	//����ƥ�����о����������Сֵ  
	//������ָ���������������ŷʽ���룬�������������Ĳ��죬ֵԽС��������������Խ�ӽ�  
	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i<matches.size(); i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//ɸѡ���Ϻõ�ƥ���  
	vector<DMatch> goodMatches;
	for (int i = 0; i<matches.size(); i++)
	{
		if (matches[i].distance < 0.2 * max_dist)
		{
			goodMatches.push_back(matches[i]);
		}
	}

	//RANSACƥ�����
	vector<DMatch> m_Matches = goodMatches;
	//����ռ�
	int ptCount = (int)m_Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	//��Keypointת��ΪMat
	Point2f pt;
	for (int i = 0; i<ptCount; i++)
	{
		pt = m_LeftKey[m_Matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = m_RightKey[m_Matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}

	// ��RANSAC��������F
	vector<uchar> m_RANSACStatus;//����������ڴ洢RANSAC��ÿ�����״̬
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);

	// ����Ұ�����

	int OutlinerCount = 0;
	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0)//״̬Ϊ0��ʾҰ��
		{
			OutlinerCount++;
		}
	}
	int InlinerCount = ptCount - OutlinerCount;//�����ڵ�

	// �������������ڱ����ڵ��ƥ���ϵ
	vector<Point2f> m_LeftInlier;
	vector<Point2f> m_RightInlier;
	vector<DMatch> m_InlierMatches;

	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount = 0;
	float inlier_minRx = img1.cols;//���ڴ洢�ڵ�����ͼ��С�����꣬�Ա�����ں�

	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0) + width;//�������width ��Ϊ��Ҫ������ԭͼ��Ŀռ�λ��
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;

			if (m_RightInlier[InlinerCount].x<inlier_minRx) inlier_minRx = m_RightInlier[InlinerCount].x;   //�洢�ڵ�����ͼ��С������ ���루�������ͼ����ͼ�غϲ��ֵ�����Ҳ�غϣ�

			InlinerCount++;
		}
	}
	inlier_minRx = 0;

	//����H���Դ洢RANSAC�õ��ĵ�Ӧ����
	H = findHomography(m_LeftInlier, m_RightInlier, RANSAC);
}
void changeHtoH1Improve(Mat& img1, int& imgLeftWidth, int& imgRightBeginHeight, Mat&H, Mat& H1)
{
	//�洢��ͼ�Ľǣ�����任����ͼλ��
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(img1.cols, 0);
	obj_corners[2] = Point(img1.cols, img1.rows); obj_corners[3] = Point(0, img1.rows);
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);

	int drift = scene_corners[1].x;//����ƫ����
	//�Լ����
	imgLeftWidth = abs(min(scene_corners[0].x, scene_corners[3].x));
	imgRightBeginHeight = abs(min(scene_corners[0].y, scene_corners[1].y));

	//�½�һ������洢��׼���Ľǵ�λ��
	int width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	float origin_x = 0, origin_y = 0;
	//�Լ����
	origin_x = min(scene_corners[0].x, scene_corners[3].x);
	origin_y = min(scene_corners[0].y, scene_corners[1].y);
	width -= int(origin_x);
	height -= int(origin_y);

	//��ȡ�µı任����ʹͼ��������ʾ
	for (int i = 0; i<4; i++) {
		scene_corners[i].x -= origin_x;
		//��ѡ������Ҫ��������
		scene_corners[i].y -= origin_y; //��Ϊ����ͼ��ƴ�Ӻ�������ı任����ͬ�����ڵ�һ��ͼ��ƴ�Ӻ������������ԣ��Ľ�������ƴ�Ӻ����������Ҫ����
	}
	H1 = getPerspectiveTransform(obj_corners, scene_corners);
}
void framePinJieInit(Mat img[6], Mat img_result[6], Mat H[6], int imgLeftWidth[6], int width[6], int height[6], int guoduPexelNums)
{
	vector<Point2f> obj_corners(4);
	vector<Point2f> scene_corners(4);
	int imgResultHeight, imgResultWidth;
	int nextIndex;

	//�Ľ�1 ��Ϊ���������ȡ����ԭʼͼ���С��ͬ�����Կ���ͳһ����
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(img[0].cols, 0);
	obj_corners[2] = Point(img[0].cols, img[0].rows); obj_corners[3] = Point(0, img[0].rows);
	for (int i = 0; i < 6; i++)
	{
		perspectiveTransform(obj_corners, scene_corners, H[i]);
		//�½�һ������洢��׼���Ľǵ�λ��
		width[i] = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
		height[i] = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
		nextIndex = (i + 1) % 6;
		imgResultHeight = max(height[i], img[nextIndex].rows);
		imgResultWidth = img[nextIndex].cols / 2 + imgLeftWidth[i] - width[i] / 2 + 2 * guoduPexelNums;//ע������width��img2.colsΪż���������Ժ���forѭ���в�ͬ�Ĵ���
		img_result[i] = Mat::zeros(imgResultHeight, imgResultWidth, img[i].type());
		img_result[i].rows = imgResultHeight;
		img_result[i].cols = imgResultWidth;
	}
}
void framePinJieNew3(Mat& img1, Mat& img2, Mat& img_result, Mat& H, int imgLeftWidth, int imgRightBeginHeight, int width, int height, int guoduPexelNums)
{
	//�½�һ������洢��׼���Ľǵ�λ��
	Mat imageturn = Mat::zeros(height, width, img1.type());

	//����ͼ��任����ʾЧ��
	warpPerspective(img1, imageturn, H, Size(width, height));
	int imgResultHeight = max(height, img2.rows);

	//��imgLeftWidth < width/2ʱ��˵��ͼ��img1��imgturn��ͼ����������⣬�任����H��׼ȷ����ʱ�޷�����ƴ����ȥ
	if (imgLeftWidth < width / 2)
	{
		cout << "�任������㲻׼ȷ��imgLeftWidth < width/2���޷�����ƴ����ȥ" << endl;
		return;
	}

	uchar* ptr1 = imageturn.data;
	uchar* ptr2 = img2.data;
	uchar* ptr3 = img_result.data;
	double alpha;

	for (int row = 0; row<imgResultHeight; row++)
	{
		ptr1 += (width / 2 - guoduPexelNums)*imageturn.elemSize();//widthΪ������ż��ʱ��ϵ���1�����أ����������������Ĺ�ϵ���ʽ��ͬ
		for (int col = width / 2 - guoduPexelNums; col<imgLeftWidth; col++)
		{
			*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
			ptr3 += img_result.elemSize();
			ptr1 += imageturn.elemSize();
		}
		for (int col = 0; col<(img2.cols) / 2 + guoduPexelNums; col++)
		{
			if (row<imgRightBeginHeight)
			{
				if (col < width - imgLeftWidth)
				{
					*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
					ptr3 += img_result.elemSize();
					ptr1 += imageturn.elemSize();
				}
				else
				{
					*ptr3 = 0; *(ptr3 + 1) = 0; *(ptr3 + 2) = 0;
					ptr3 += img_result.elemSize();
				}
			}
			else if (row < imgRightBeginHeight + img2.rows)
			{
				if (col<width - imgLeftWidth)
				{
					alpha = (double)col / (width - imgLeftWidth);
					if (*ptr1 == 0)
					{
						*ptr3 = *ptr2; *(ptr3 + 1) = *(ptr2 + 1); *(ptr3 + 2) = *(ptr2 + 2);
					}
					else
					{
						*ptr3 = *ptr1*(1 - alpha) + *ptr2*alpha; *(ptr3 + 1) = *(ptr1 + 1)*(1 - alpha) + *(ptr2 + 1)*alpha; *(ptr3 + 2) = *(ptr1 + 2)*(1 - alpha) + *(ptr2 + 2)*alpha;
					}
					ptr1 += imageturn.elemSize();
					ptr2 += img2.elemSize();
					ptr3 += img_result.elemSize();
				}
				else
				{
					*ptr3 = *ptr2; *(ptr3 + 1) = *(ptr2 + 1); *(ptr3 + 2) = *(ptr2 + 2);
					ptr2 += img2.elemSize();
					ptr3 += img_result.elemSize();
				}
				//����img2��벿�֣�ȥ�����ɵ㣩
			}
			else
			{
				if (col<width - imgLeftWidth)
				{
					*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
					ptr1 += imageturn.elemSize();
					ptr3 += img_result.elemSize();
				}
				else
				{
					*ptr3 = 0; *(ptr3 + 1) = 0; *(ptr3 + 2) = 0;
					ptr3 += img_result.elemSize();
				}
			}
		}
		//����img2��벿�֣�ȥ�����ɵ㣩
		if (row >= imgRightBeginHeight && row < img2.rows + imgRightBeginHeight)
		{
			if (img2.cols % 2 == 0)
				ptr2 += (img2.cols / 2 - guoduPexelNums)*img2.elemSize();
			else
				ptr2 += ((img2.cols) / 2 + 1 - guoduPexelNums)*img2.elemSize();
		}
	}
}

bool cylindricalStitch2(Mat img[6], Mat H[6], int imgLeftWidth[6], int imgRightBeginHeight[6], Mat& imgResult)
{
	//���¶��߳����6������ͼ���ƴ�ӣ��õ������м��ں������ͼ��
	static const int numThreads = 6;
	vector<thread> cylindricalThreads(numThreads);
	Mat imgPinJieResult[6];

	//��ɶ��߳�ͼ��ƴ�ӳ�ʼ����������Ϊ���̺߳�join()���ͷŵ�imgPinJieResult�����ݣ��ʲ����ȳ�ʼ��imgPinJieResult����ֹ����Ϊ���߳�join()�����ͷŵ�
	int width[6], height[6];
	int guoduLength = 10;
	framePinJieInit(img, imgPinJieResult, H, imgLeftWidth, width, height, guoduLength);
	
	//���̴߳���ͼ��ƴ��
	for (int i = 0; i < numThreads; i++)
	{
		switch (i)
		{
			case 0:
				cylindricalThreads[0] = thread(&framePinJieNew3, img[0], img[1], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 1:
				cylindricalThreads[1] = thread(&framePinJieNew3, img[1], img[2], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 2:
				cylindricalThreads[2] = thread(&framePinJieNew3, img[2], img[3], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 3:
				cylindricalThreads[3] = thread(&framePinJieNew3, img[3], img[4], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 4:
				cylindricalThreads[4] = thread(&framePinJieNew3, img[4], img[5], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 5:
				cylindricalThreads[5] = thread(&framePinJieNew3, img[5], img[0], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			default:
				cout << "some error in cylindricalStitch2 cylindricalThreads" << endl;
				return false;
				break;
		}
	}

	//���߳�
	for (int i = 0; i < numThreads; i++)
	{
		cylindricalThreads[i].join();
	}

	//���¶��߳��������6��ͼ��������ںϣ��õ������ںϵ�ͼ��
	static const int numThreads2 = 3;
	vector<thread> cylindricalThreads2(numThreads2);
	ChongheStitch Stitcher;
	Mat imgChongHeStitchResult[3];
	int resultWidth[3];
	int resultHeight[3];//�ֱ����ڴ洢�ںϺ������ͼ��Ŀ�͸�
	int central[6];

	//����Ҳ����ʹ�ö��̴߳���
	for (int i = 0; i < numThreads2;i++)
	{
		Stitcher.chongHeStitch3Init(imgPinJieResult[i * 2], imgPinJieResult[i * 2 + 1], guoduLength, resultWidth[i], resultHeight[i], central[i * 2], central[i * 2 + 1]);
	}
	for (int i = 0; i < numThreads2;i++)
	{
		imgChongHeStitchResult[i] = cv::Mat::zeros(resultHeight[i], resultWidth[i], imgPinJieResult[i*2].type());
	}
	for (int i = 0; i < numThreads2; i++)
	{
		switch (i)
		{
		case 0:
			cylindricalThreads2[0] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[0], imgPinJieResult[1], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		case 1:
			cylindricalThreads2[1] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[2], imgPinJieResult[3], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		case 2:
			cylindricalThreads2[2] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[4], imgPinJieResult[5], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		default:
			cout << "some error in cylindricalStitch2 cylindricalThreads2" << endl;
			return false;
			break;
		}
	}
	for (int i = 0; i < numThreads2; i++)
	{
		cylindricalThreads2[i].join();
	}

	//���¶������õ�3��ͼ������ںϣ��õ����յ�ȫ��ͼ
	Mat imgRongheResult;
	imgRongheResult = Stitcher.chongHeStitch(imgChongHeStitchResult[0], imgChongHeStitchResult[1], guoduLength);
	imgResult = Stitcher.chongHeStitch(imgRongheResult, imgChongHeStitchResult[2], guoduLength);
	return true;
}