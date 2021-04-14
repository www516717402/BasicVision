#include <opencv2/opencv.hpp>
#include <iostream>
#include "math.h"
using namespace cv;
using namespace std;

//---存储目标区域的参数
typedef struct MyStruct
{
	Rect my_rec;//目标矩阵
	Mat my_img;//目标ROI区域
}MyStruct;

int main(int argc, char*argv[])
{
	Mat inputImage = imread("1.png");
	Mat showImage = inputImage.clone();
	cvtColor(inputImage, inputImage, CV_BGR2GRAY);
	//---由于图像直接是灰度图/二值图，所以不需要进行特别的处理
	Mat morph, gray = inputImage.clone(), showGray;
	showGray.create(inputImage.size(), CV_8UC1);
	showGray.setTo(0);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(inputImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	Rect rec_adapt;
	//----这里是程序的第一个核心，排除干扰找到数字区域
	//----其中这里没必要条件那么死，后面的还会进一步去判断，而且条件太苛刻容易把主要信息弄丢
	for (size_t i = 0; i < contours.size(); i++)
	{
		//----矩形区域非零像素占总的比例，防止有较大的空白区域干扰检测结果
		//----矩形的长宽限制，也可以再增加额外条件：长宽比例等
		int x = minAreaRect(contours[i]).boundingRect().x;
		int y = minAreaRect(contours[i]).boundingRect().y;
		int width = minAreaRect(contours[i]).boundingRect().width;
		int height = minAreaRect(contours[i]).boundingRect().height;
		Mat true_image;
		int true_pix_count;
		double true_pix_rate;
		//---如果部分目标被去除，可在此进行优化
		if (x < 0 || y < 0) true_pix_rate = 1;
		else
		{
			true_image = gray(Rect(x, y, width, height));
			true_pix_count = countNonZero(true_image);
			true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(minAreaRect(contours[i]).boundingRect().area());
		}			 
		double angle = minAreaRect(contours[i]).angle;
		bool flag_angle = (angle == 9 ||  angle == 180 || angle == 0 ) ? true : false;//|| angle == 270
		if (minAreaRect(contours[i]).size.height >= 10 && minAreaRect(contours[i]).size.height <= 20 && minAreaRect(contours[i]).size.width >= 4 && minAreaRect(contours[i]).size.width <= 30  && flag_angle && true_pix_rate <= 0.8)//
		{
			drawContours(showGray, contours, static_cast<int>(i), Scalar(255, 255, 255), 1);
		}
	}
	//---形态学处理，对区域进行连接与拆分（具体的细节看操作图片）
	//---如果细节部分操作的不好可以在此进行优化---//
	Mat img1;
	Mat kernel_x = getStructuringElement(MORPH_RECT, Size(20,1));
	Mat kernel_y = getStructuringElement(MORPH_RECT, Size(1, 28));
	Mat kernel_x_l = getStructuringElement(MORPH_RECT, Size(20, 1));
	morphologyEx(showGray, showGray, MORPH_DILATE, kernel_x);
	morphologyEx(showGray, showGray, MORPH_DILATE, kernel_x);
	//morphologyEx(showGray, showGray, MORPH_DILATE, kernel_x);
	morphologyEx(showGray, img1, MORPH_OPEN, kernel_y);
	showGray = showGray - img1;
	morphologyEx(showGray, showGray, MORPH_CLOSE, kernel_x_l);
	//---对目标区域进行矩形包括
	findContours(showGray, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	vector<MyStruct> my_class;
	for (size_t i = 0; i < contours.size(); i++)
	{	
		if (boundingRect(contours[i]).width > 60)
		{
			int x = minAreaRect(contours[i]).boundingRect().x;
			int y = minAreaRect(contours[i]).boundingRect().y;
			int width = minAreaRect(contours[i]).boundingRect().width;
			int height = minAreaRect(contours[i]).boundingRect().height;
			MyStruct Struct_temp;
			Struct_temp.my_rec = boundingRect(contours[i]);
			Struct_temp.my_img = showImage(Rect(x, y, width, height)).clone();
			my_class.push_back(Struct_temp);
			rectangle(showImage, boundingRect(contours[i]), Scalar(0, 0, 255));
		}		
	}
	

	waitKey(0);
	return 0;

}