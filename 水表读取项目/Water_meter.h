#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>


using namespace std;
using namespace cv;

//---ÍØÆË¶Ëµã¼ÇÂ¼
struct SLine
{
	Point begin;
	Point end;
};
void Find_Roi(InputArray _src,Point2f& center,float& radius);
Mat Find_MeterRange(Mat src, Point2f& center, float& radius);
void Find_Pointer(vector<SLine>& G_SLine, Point2f center, float& length_Max1,
	float& length_Max2, float& length_Line_Max1, float& length_Line_Max2,
	Point& pointer1, Point& pointer2);
void Find_Scale(InputArray& _src, vector<SLine>& G_SLine, float length_Max1, Point2f& center, float& radius);
void center_axis(InputArray _src, Mat&  dst);
bool Charge(Point center, Mat src, uchar num);
vector<SLine> Extract_Message(InputArray& _src, Mat& dst);
void change(float& max, float& min);
float TrasForm(Point a, Point b, Point center);
vector<Point> Intersection(vector<SLine> G_SLine, Point2f& center, float& radius);
void Sort_Point(vector<Point>& inter_point, Point2f center);
bool Interpolation(vector<Point>& inter_point, Point2f& center, Point& aim_Point, float& A_angle);
float Calculate(vector<Point>& inter_point, Point2f& center, Point pointer1, Point pointer2, float radius);