#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;
int FillBlock(Mat src, Mat &mask, Point center);
int main(int argc,char**argv)
{
	Mat inputImage = imread("123.jpg", 0);
	threshold(inputImage, inputImage, 1, 255, THRESH_BINARY_INV | THRESH_OTSU);
	//创建一个mask为：边界1，中间255
	Mat mask = Mat::zeros(inputImage.size(), CV_8UC1);
	mask.setTo(255);
	Mat rows = Mat::ones(Size(inputImage.cols, 1), CV_8UC1), cols = Mat::zeros(Size(1, inputImage.rows), CV_8UC1);
	Mat src_rows_beg = mask.row(0);
	Mat src_rows_end = mask.row(inputImage.rows - 1);
	Mat src_cols_beg = mask.col(0);
	Mat src_cols_end = mask.col(inputImage.cols - 1);
	rows.copyTo(src_rows_beg); rows.copyTo(src_rows_end);
	cols.copyTo(src_cols_beg); cols.copyTo(src_cols_end);
	int temp = FillBlock(inputImage, mask, Point(1, 1));
	return 0;
}

//---Fill parts areas
//---@src		：input distance image/float/one signal
//---@mask		：mask image/ucahr/one signal
//---@center	：nowdays operator centers/ucahr
int FillBlock(Mat src, Mat &mask, Point center)
{
	uchar back = src.at<uchar>(center.y, center.x);//当前值像素值
	vector<Point> fill_point;//存储被填充的点
	int count = 0, count_mount = 1;//count；当前操作像素点，count_mount：总计像素数量
	fill_point.push_back(center);
	while (count < count_mount)
	{
		vector<uchar*> img;
		vector<uchar*> msk;
		for (int i = -1; i < 2; i++)
		{
			img.push_back(src.ptr<uchar>(fill_point[count].y + i));
			msk.push_back(mask.ptr<uchar>(fill_point[count].y + i));
		}
		for (size_t i = 0; i < 3; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				//---和当前像素大小一样、且不为自身、且为未操作过的像素
				if (img[i][fill_point[count].x + j] == back && !(j == 0 && i == 1) && msk[i][fill_point[count].x + j] == 255)
				{
					fill_point.push_back(Point(fill_point[count].x + j, fill_point[count].y + i - 1));
					msk[i][fill_point[count].x + j] = 1;//可填充区域先赋值为1，为了便于操作
				}
			}
		}
		msk[1][fill_point[count].x] = 1;//当前像素填充为1
		count_mount = fill_point.size() - 1;//循环的核心，
		fill_point.erase(fill_point.begin());//执行一个删除一个，不占用内存
	}
	return 0;
}