#include<iostream>
#include <opencv2/opencv.hpp>
#include <math.h>
using namespace cv;
using namespace std;

int Threshold_Value = 176;
const int Threshold_Max_value = 255;
const int Threshold_type_value = 3;

Mat input_image, threshold_image, output_image, Middle_image;

void Threshold_Image_Bar(int, void *);

int main(int argc, char**argv)
{
	input_image = imread("b.jpg");
	if (input_image.data == NULL) {
		return -1; cout << "can't open image.../";
	}
	//----------简单预处理
	imshow("Sourse Image", input_image);
	blur(input_image, Middle_image, Size(3, 3), Point(-1, -1), 4);
	imshow("Blur Image", Middle_image);
	cvtColor(Middle_image, Middle_image, COLOR_RGB2GRAY);
	imshow("Gray Image", Middle_image);
	//-----------利用比例对图像进行ROI操作
	const float init_pointx =  saturate_cast<float>(Middle_image.cols / 7);
	const float init_pointy =  saturate_cast<float>(Middle_image.rows / 7);
	Rect roi_rect = Rect(Point2f(2 * init_pointx, 2 * init_pointy), Point2f(6 * init_pointx, 6 * init_pointy));
	Mat  roi_Image = Middle_image(roi_rect);
	Middle_image = roi_Image;
	//----------这里使用大法定律自适应操作图像，程序的稳定性很高
	threshold(Middle_image, threshold_image, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);//这个65可以更改看效果
	imshow("Threshold Image", threshold_image);
	//----------这里通过形态学操作对图像稍作调整
	Mat kernel_rect   = getStructuringElement(MORPH_ELLIPSE, Size(30, 30), Point(-1, -1));
	Mat kernel_circle = getStructuringElement(MORPH_ELLIPSE, Size(10, 10), Point(-1, -1));
	morphologyEx(threshold_image, threshold_image, MORPH_CLOSE, kernel_circle);
	Mat RedImage = threshold_image.clone();
	/*--------这是当时求外面圆的代码，这里不需要了，求外面的圆不准确
	Mat otherImage;
	Canny(threshold_image, otherImage, 50, 200);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat showImage = Mat::zeros(RedImage.size(), CV_8UC1);
	findContours(RedImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
	if (boundingRect(contours[i]).area() > 10000)//这个参数大概就可以
	{
	drawContours(showImage, contours, static_cast<int>(i), Scalar(255, 255, 255), 1);
	Point2f center;
	float radius;
	minEnclosingCircle(contours[i], center, radius);
	Mat result = Mat::zeros(RedImage.size(), CV_8UC3);
	circle(input_image, center, radius, Scalar(0, 0, 255), 2);
	}
	}
	*/
	morphologyEx(RedImage, threshold_image, MORPH_OPEN, kernel_rect);
	//---------图像删除旁边干扰区域
	for (size_t i = 0; i < threshold_image.rows; i++)
	{
		for (size_t j = 0; j < threshold_image.cols; j++)
		{
			RedImage.at<uchar>(i, j) = saturate_cast<uchar>(RedImage.at<uchar>(i, j) - threshold_image.at<uchar>(i, j));
		}
	}
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat showImage = Mat::zeros(RedImage.size(), CV_8UC1);
	findContours(RedImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		//------如果圆找的位置太离谱，这里可进行优化
		//------如果找的圆位置不精确这里更改数字
		if (minAreaRect(contours[i]).size.area() > 10000 && minAreaRect(contours[i]).size.height > 80 && minAreaRect(contours[i]).size.width > 80)//这个参数大概就可以
		{
			drawContours(showImage, contours, static_cast<int>(i), Scalar(255, 255, 255), 1);
		}
	}
	//-----------将点存入容器进行点集操作
	vector<Point> points;
	for (int i = 0; i < showImage.rows; i++)
	{
		for (int j = 0; j < showImage.cols; j++)
		{
			if (showImage.at<uchar>(i, j) == 255)
			{
				points.push_back(Point(j, i));
			}
		}
	}
	//----------点拟合圆，这里代码只是找一个圆，也可优化成找多个圆
	Point2f center;
	float radius;
	if (points.data() == 0)
	{
		printf("Don't detecte point");
		return -1;
	}
	minEnclosingCircle(points, center, radius);
	center.x += 2 * init_pointx;
	center.y += 2 * init_pointy;
	Mat result = Mat::zeros(RedImage.size(), CV_8UC3);
	circle(input_image, center, radius, Scalar(0, 0, 255), 2);

	/*namedWindow("Threshold Image", 1);
	createTrackbar("阈值调整", "Threshold Image", &Threshold_Value, 255, Threshold_Image_Bar);
	Threshold_Image_Bar(0, 0);*/
	imshow("result", input_image);
	waitKey(0);
	return 0;
}
//------这是调试用的滑块，已经调试好了，代码搬上去了，如果你需要调试那就再使用
void Threshold_Image_Bar(int, void *)
{
	threshold(Middle_image, threshold_image, 65, 255,THRESH_BINARY_INV);//110,65
	imshow("Threshold Image", threshold_image);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(50, 50), Point(-1, -1));//这个参数无所谓的
	Mat RedImage = threshold_image.clone();
	/*
	Mat otherImage;
	Canny(threshold_image, otherImage, 50, 200);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat showImage = Mat::zeros(RedImage.size(), CV_8UC1);
	findContours(RedImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (boundingRect(contours[i]).area() > 10000)//这个参数大概就可以
		{
			drawContours(showImage, contours, static_cast<int>(i), Scalar(255, 255, 255), 1);
			Point2f center;
			float radius;
			minEnclosingCircle(contours[i], center, radius);
			Mat result = Mat::zeros(RedImage.size(), CV_8UC3);
			circle(input_image, center, radius, Scalar(0, 0, 255), 2);
		}
	}
	*/
	morphologyEx(RedImage, threshold_image, MORPH_OPEN, kernel);
	for (size_t i = 0; i < threshold_image.rows; i++)
	{
		for (size_t j = 0; j < threshold_image.cols; j++)
		{
			RedImage.at<uchar>(i, j) = saturate_cast<uchar>(RedImage.at<uchar>(i, j) - threshold_image.at<uchar>(i, j));
		}
	}
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat showImage = Mat::zeros(RedImage.size(), CV_8UC1);
	findContours(RedImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (boundingRect(contours[i]).area() > 20000)//这个参数大概就可以
		{
			drawContours(showImage, contours, static_cast<int>(i), Scalar(255, 255, 255), 1);
		}
	}
	vector<Point> points;
	for (int i = 0; i < showImage.rows; i++)
	{
		for (int j = 0; j < showImage.cols; j++)
		{
			if (showImage.at<uchar>(i, j) == 255)
			{
				points.push_back(Point(j, i));
			}
		}
	}
	Point2f center;
	float radius;
	minEnclosingCircle(points, center, radius);
	Mat result = Mat::zeros(RedImage.size(), CV_8UC3);
	circle(input_image, center, radius, Scalar(0, 0, 255), 2);
}