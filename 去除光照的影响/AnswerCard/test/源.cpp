
#include <opencv2/opencv.hpp>
#include <iostream>
#include "math.h"
using namespace cv;
using namespace std;
//double maxW, minW, maxH = 0.0, minH = 0.0, pos_min = 0.0, pos_max = 0.0;
int main(int argc, char*argv[])
{
	Mat inputImage = imread("123.jpg");

	//-----HSV空间分离
	cvtColor(inputImage, inputImage, CV_BGR2GRAY);
	Mat img;
	img = inputImage.clone();
	Mat gray1, gray2, gray;
	GaussianBlur(img, gray1, Size(51, 51),18);
	subtract(img, gray1, gray);
	//---3,jpg:(53,5) 2.jpg(55,40),1.jpg(55,10)
	adaptiveThreshold(gray, gray1, 255, ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 21, 3);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(gray1, gray1, MORPH_CLOSE, kernel);
	//Laplacian(splitImage[2], gray1, CV_8UC1, 5);
	//subtract(splitImage[2],gray1 , gray1);
	/*Mat wjy;
	Mat kernel = (Mat_<double>(3, 3) << 0, -1, 0, -1, 4, -1, 0, -1, 0);
	filter2D(splitImage[2], wjy, splitImage[2].depth(), kernel);
	Mat gausImage;
	add(wjy, splitImage[2], gausImage);*/

	/*
	//计算局部直方图
	double k[255];
	for (size_t i = 0; i < splitImage[2].rows; i++)
	{
	for (size_t j = 0; j < splitImage[2].cols; j++) k[splitImage[2].at<uchar>(i, j)]++;
	}
	double maxW, minW, maxH = 0.0, minH = 0.0, pos_min = 0.0, pos_max = 0.0;
	for (size_t i = 0; i < 255; i++)
	{
	if (i >= 150) maxH = k[i] >= maxH ? k[i] : maxH;
	else minH = k[i] > minH ? k[i] : minH;
	pos_max = maxH == k[i] ? i : pos_max;
	pos_min = minH == k[i] ? i : pos_min;
	}
	Mat test;
	GaussianBlur(splitImage[2], test, Size(11, 11), 0);
	add(splitImage[2], test, splitImage[2]);
	//minMaxLoc(splitImage[2], &minW, &maxW);// V空间最值
	//按比例变化
	Mat output_image;
	output_image.create(splitImage[2].size(), splitImage[2].type());
	for (int i = 0; i < splitImage[2].rows; i++)
	{
	uchar *in = splitImage[2].ptr<uchar>(i);
	uchar *out = output_image.ptr<uchar>(i);
	for (int j = 0; j < splitImage[2].cols; j++)
	{
	//out[j] = saturate_cast<uchar>(in[j] * (log(in[j]) / log(maxW)));
	out[j] = in[j] >= pos_max ? 255 : in[j];//saturate_cast<uchar>(in[j] * (1.00*pos_max));
	out[j] = in[j] <= pos_min ? 0 : out[j];//saturate_cast<uchar>(in[j] * (1.00*pos_max));
	out[j] = in[j] > pos_min && in[j] <= 150 ? saturate_cast<uchar>(in[j] * (in[j] / (1.00*pos_max))) : out[j];// 255 * ((in[j] - pos_min) / (pos_max - pos_min));
	out[j] = in[j] < pos_max && in[j] > 150 ? saturate_cast<uchar>(in[j] * (in[j] / (1.00 * 150))) : out[j];// 255 * ((in[j] - pos_min) / (pos_max - pos_min));		}
	}
	*/

	//Mat test;
	//normalize(splitImage[2], splitImage[2], 0, 1);
	//splitImage[2].convertTo(test,CV_8UC1);
	//cvtColor(inputImage, inputImage, CV_BGR2GRAY);
	//equalizeHist(inputImage, inputImage);
	//Mat kernel = (Mat_<double>(3, 3) << 0, -1, 0, -1, 0, 5, -1, 0, -1, 0);
	//filter2D(splitImage[2], splitImage[2], splitImage[2].depth(), kernel);
	//Mat gausImage1, gausImage2,gausImage;
	//imageAdjust(splitImage[2], splitImage[2], 0, 0.5, 0.5, 1, 1);
	//GaussianBlur(splitImage[2], gausImage1, Size(3, 3), 0);
	//GaussianBlur(splitImage[2], gausImage2, Size(7, 7), 0);
	//subtract(gausImage1, splitImage[2], gausImage);
	//cvtColor(inputImage,inputImage,CV_BGR2GRAY);
	//threshold(inputImage,inputImage,230,255, THRESH_BINARY);
	//adaptiveThreshold(inputImage, inputImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 5, 0);
	waitKey(0);
	return 0;

}