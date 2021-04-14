#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include "Water_meter.h"

using namespace std;
using namespace cv;


int main(int argc, char**argv)
{
	Mat test1,img = imread("123.jpg");
	Point2f center;
	float radius;
	Find_Roi(img,  center,  radius);
	Mat img_vred = Find_MeterRange(img, center, radius);
	center_axis(img_vred, test1);
	test1.convertTo(test1, CV_8UC1);
	vector<SLine> G_SLine = Extract_Message(test1, test1);
	center = Point(int(test1.rows / 2), int(test1.cols / 2));//更新圆心坐标
	float length_Max1 = 0, length_Max2 = 0;
	float length_Line_Max1 = 0, length_Line_Max2 = 0;
	Point pointer1, pointer2;
	Find_Pointer(G_SLine, center, length_Max1, length_Max2, length_Line_Max1, 
										length_Line_Max2, pointer1, pointer2);
	Find_Scale(test1, G_SLine, length_Max1, center, radius);
	vector<Point> inter_point =  Intersection(G_SLine, center,  radius);
	Sort_Point(inter_point, center);
	for (size_t i = 0; i < inter_point.size(); i++)
	{
		//circle(test1, inter_point[i], 3, Scalar(0, 0, 255), 4);
		line(test1, inter_point[i],center, Scalar(0, 0, 255),1);
	}
	cout<<"最终度数为："<< Calculate(inter_point, center, pointer1, pointer2, radius);
 	return 0;
}