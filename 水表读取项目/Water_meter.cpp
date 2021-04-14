
#include "Water_meter.h"

//********using white color locate center********//
//---初步寻找中心，为了大概确定仪表盘的位置，之后会进行校核
void Find_Roi(InputArray _src, Point2f& center, float& radius)
{
	Mat src = _src.getMat().clone();
	cvtColor(src, src, CV_BGR2HSV);
	Mat img_h, img_s, img_v;
	vector<Mat> hsv_vec;
	split(src, hsv_vec);
	img_h = hsv_vec[0];
	img_s = hsv_vec[1];
	img_v = hsv_vec[2];
	//Mat img_vred1 = ((img_h > 0& img_h < 10)|(img_h > 156 & img_h < 180)
	//	&(img_s > 43 & img_s < 255)&(img_v > 46 & img_v < 255));//Red Extract
	Mat img_vred1 = ((img_h > 160 & img_h < 180));//Red Extract
												  //Mat img_vred2 = ((img_h > 150 & img_h < 180)
												  //	&(img_s > 10 & img_s < 18)&(img_v > 46 & img_v < 255));//White Extract
	Mat img_vred = img_vred1.clone();
	//add(img_vred1, img_vred2, img_vred);
	Mat kernel1 = getStructuringElement(MORPH_CROSS, Size(15, 15), Point(-1, -1));
	morphologyEx(img_vred, img_vred, MORPH_DILATE, kernel1);
	//---拟合圆心
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(img_vred, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	vector<Point> max_contours = contours[0];
	for (size_t i = 0; i < contours.size(); i++)
		max_contours = minAreaRect(max_contours).size.area() > minAreaRect(contours[i]).size.area() ? max_contours : contours[i];
	//Mat showImage = Mat::zeros(img_vred.size(), CV_8UC1);
	contours[0] = max_contours;
	/*drawContours(showImage, contours, 0, Scalar(255, 255, 255), 1);
	Point2f center;
	float radius;*/
	minEnclosingCircle(max_contours, center, radius);//得到圆心和半径
}

//*******利用圆心和半径得到的初步范围，再对表盘进行区域提取，再进行范围内红色提取
Mat Find_MeterRange(Mat src, Point2f& center, float& radius)
{
	//利用水漫算法制作掩膜（ROI）区域
	Mat mask = Mat::zeros(src.size(), CV_8UC3);
	circle(mask, center, radius, Scalar(255, 255, 255), 1);
	//circle(mask, center, radius, Scalar(255, 255, 255),radius*2);
	floodFill(mask, center, Scalar(255, 255, 255));//黑色区域变成白色，遇到白色区域停止
	bitwise_not(mask, mask);
	//---利用掩膜对图像进行操作
	subtract(src, mask, mask);
	//---预处理到二值图
	Mat img = mask(Range(center.y - radius, center.y + radius), Range(center.x - radius, center.x + radius));
	//---利用红色空间进行精确定位
	cvtColor(img, img, CV_BGR2HSV);
	Mat img_h1, img_s1, img_v1;
	vector<Mat> hsv_vec1;
	split(img, hsv_vec1);
	img_h1 = hsv_vec1[0];
	img_s1 = hsv_vec1[1];
	img_v1 = hsv_vec1[2];
	Mat img_vred10 = ((img_s1 > 150));//Red Extract
	return img_vred10;
}

//---查找红色指针的范围，目的为了之后进行尺度的变换
//--@G_SLine								端点值
//--@center									中心值
//--@length_Max1、length_Max2				圆心到端点最大值
//--@length_Line_Max1、length_Line_Max2		线段最大值
//--@pointer1、pointer2						指针端点坐标
void Find_Pointer(vector<SLine>& G_SLine, Point2f center,float& length_Max1,
	float& length_Max2, float& length_Line_Max1, float& length_Line_Max2, 
	Point& pointer1, Point& pointer2)
{
	for (int i = 0; i < G_SLine.size(); i++)//找指针坐标1
	{
		float line_length = sqrt(pow(G_SLine[i].begin.x - G_SLine[i].end.x, 2) + pow(G_SLine[i].begin.y - G_SLine[i].end.y, 2));
		if (line_length < length_Line_Max1) continue;
		length_Line_Max1 = line_length;
		float a = sqrt(pow(G_SLine[i].begin.x - center.x, 2) + pow(G_SLine[i].begin.y - center.y, 2));
		float b = sqrt(pow(G_SLine[i].end.x - center.x, 2) + pow(G_SLine[i].end.y - center.y, 2));
		length_Max1 = (a > b ? a : b);
		pointer1 = a > b ? G_SLine[i].begin : G_SLine[i].end;
		cout << length_Max1 << " " << G_SLine[i].begin << " " << G_SLine[i].end << endl;
	}
	for (int i = 0; i < G_SLine.size(); i++)//找指针坐标2
	{
		float line_length = sqrt(pow(G_SLine[i].begin.x - G_SLine[i].end.x, 2) + pow(G_SLine[i].begin.y - G_SLine[i].end.y, 2));
		float a = sqrt(pow(G_SLine[i].begin.x - center.x, 2) + pow(G_SLine[i].begin.y - center.y, 2));
		float b = sqrt(pow(G_SLine[i].end.x - center.x, 2) + pow(G_SLine[i].end.y - center.y, 2));
		length_Max2 = (a > b ? a : b);
		//线段长度可以小点，但是最远距离应该和length_Max1差别不大才对
		if (line_length + length_Max1*0.06 < length_Line_Max2
			|| line_length == length_Line_Max1
			|| length_Max2 < length_Max1*0.8) continue;
		length_Line_Max2 = line_length;
		pointer2 = a > b ? G_SLine[i].begin : G_SLine[i].end;
		cout << length_Max2 << " " << G_SLine[i].begin << " " << G_SLine[i].end << endl;
	}
}

//---去除多余干扰，找到刻度位置
void Find_Scale(InputArray& _src, vector<SLine>& G_SLine, float length_Max1,Point2f& center,float& radius)
{
	Mat src = _src.getMat();
	Mat showImg = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i < G_SLine.size(); i++)
	{//---这里限制刻度长度不小于等于两个像素、刻度所在范围不得大于length_Max且不小于一个尺度值
		if (sqrt(pow(G_SLine[i].begin.x - G_SLine[i].end.x, 2) + pow(G_SLine[i].begin.y - G_SLine[i].end.y, 2)) <= 4)
		{//刻度长度不小于等于两个像素
			G_SLine.erase(G_SLine.begin() + i);
			i--;
			continue;
		}
		float a = sqrt(pow(G_SLine[i].begin.x - center.x, 2) + pow(G_SLine[i].begin.y - center.y, 2));
		float b = sqrt(pow(G_SLine[i].end.x - center.x, 2) + pow(G_SLine[i].end.y - center.y, 2));
		change(a, b);
		if (a > 0.78*length_Max1 || b < 0.48*length_Max1)
		{//刻度所在范围不得大于length_Max且不小于一个尺度值
			G_SLine.erase(G_SLine.begin() + i);
			i--;
			continue;
		}
		//line(showImg, G_SLine[i].begin, G_SLine[i].end, Scalar(0, 0, 255), 1);
	}
	//---**********************************----//
	//---利用线段的方向性质去精确寻找刻度（精确估计）
	vector<Point> circle_point;
	for (int i = 0; i < G_SLine.size(); i++)
	{
		float bais = TrasForm(G_SLine[i].begin, G_SLine[i].end, center);
		if (bais < 1.5*length_Max1)
		{
			//line(showImg, G_SLine[i].begin, G_SLine[i].end, Scalar(255, 255, 0), 1);
			//---中心代替端点值
			Point mid = Point((G_SLine[i].begin.x + G_SLine[i].end.x) / 2, (G_SLine[i].begin.y + G_SLine[i].end.y) / 2);
			circle_point.push_back(mid);
			continue;
		}
		G_SLine.erase(G_SLine.begin() + i);
		i--;
	}
	//---再利用找到的线段对圆心进行纠正
	minEnclosingCircle(circle_point, center, radius);
	circle(showImg, center, 1, Scalar(255, 255, 255), 1);
	circle(showImg, center, radius, Scalar(255, 255, 255), 1);
	//---去除最后一些干扰，利用半径和点的距离
	for (size_t i = 0; i < G_SLine.size(); i++)
	{
		float distance = sqrt(pow(circle_point[i].x - center.x, 2) + pow(circle_point[i].y - center.y, 2));
		if (abs(distance - radius) < 5)
		{
			line(showImg, G_SLine[i].begin, G_SLine[i].end, Scalar(0, 255, 0), 1);
			continue;
		}
		G_SLine.erase(G_SLine.begin() + i);
		circle_point.erase(circle_point.begin() + i);
		i--;
	}
	showImg.copyTo(src);
}

//------@_src:输入为二值化图像，背景为0
//------@dst :输出为CV_8UC1
void center_axis(InputArray _src, Mat&  dst)
{
	//---存储位置和数据
	typedef struct MyStruct
	{
		Point position;
		float num;
	}MyStruct;
	//----定义查表数据/这个表格有问题，暂时不想去改进
	int wjy_array[] = { 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
		1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
		1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
		1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
		1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0,
		1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0 };
	//---本程序用的是这个表格
	uchar lut[] = { 200, 206, 220, 204, 0, 207, 0, 204, 0, 207, 221, 51, 1, 207, 221, 51,
		0, 0, 221, 204, 0, 0, 0, 204, 1, 207, 221, 51, 1, 207, 221, 51 };
	Mat src = _src.getMat();
	//Mat dst = _dst.getMat();
	distanceTransform(src, src, DIST_L2, DIST_MASK_3, 5);
	normalize(src, src, 0, 255, NORM_MINMAX);//只要背景为0即可，没必要0-255
	Mat img_row = src.reshape(0, 1);//转化为一行数据，方便处理
	vector<MyStruct> my_vector;//目标数据、排序数据
	for (size_t j = 0; j < img_row.cols; j++)//Mat放到vector便于操作
	{
		if (img_row.at<float>(0, j) == 0) continue;
		MyStruct my_struct;
		my_struct.num = saturate_cast<float>(img_row.at<float>(0, j));
		my_struct.position = Point(saturate_cast<int>(j % src.cols), saturate_cast<int>(j / src.cols));
		my_vector.push_back(my_struct);
	}
	//----排序目标数据，返回位置和数值
	for (size_t i = 0; i < my_vector.size(); i++)
	{
		if (my_vector[i].num == 0) continue;//背景不处理
		for (size_t j = i; j < my_vector.size(); j++)
		{
			MyStruct temp;
			if (my_vector[i].num >= my_vector[j].num)
			{
				if (my_vector[j].num == 0) continue;//背景不处理
				temp = my_vector[j];
				my_vector[j] = my_vector[i];
				my_vector[i] = temp;
			}
		}
	}
	//根据查表处理排序数据，这里都为目标数据，不需要进行背景判断了
	for (size_t i = 0; i < my_vector.size(); i++)
	{
		//----边界数据不处理，至于改进也很简单，懂点程序就可以了
		if (my_vector[i].position.y == 1
			|| my_vector[i].position.x == 1
			|| my_vector[i].position.y == src.rows - 1
			|| my_vector[i].position.x == src.cols - 1
			|| src.at<float>(my_vector[i].position.y, my_vector[i].position.x) == 0) continue;
		else
		{
			char num[] = { 1,1,1,1,1,1,1,1 };
			//---这里比较low，不想去用左移变换了，用的人自己改进吧
			num[0] = src.at<float>(my_vector[i].position.y - 1, my_vector[i].position.x - 1)
	> 0 ? 1 : 0;
			num[1] = src.at<float>(my_vector[i].position.y - 1, my_vector[i].position.x)
		> 0 ? 1 : 0;
			num[2] = src.at<float>(my_vector[i].position.y - 1, my_vector[i].position.x + 1)
	> 0 ? 1 : 0;
			num[3] = src.at<float>(my_vector[i].position.y, my_vector[i].position.x - 1)
	> 0 ? 1 : 0;
			num[4] = src.at<float>(my_vector[i].position.y, my_vector[i].position.x + 1)
						> 0 ? 1 : 0;
			num[5] = src.at<float>(my_vector[i].position.y + 1, my_vector[i].position.x - 1)
						> 0 ? 1 : 0;
			num[6] = src.at<float>(my_vector[i].position.y + 1, my_vector[i].position.x)
						> 0 ? 1 : 0;
			num[7] = src.at<float>(my_vector[i].position.y + 1, my_vector[i].position.x + 1)
						> 0 ? 1 : 0;
			int sum = num[0] + num[1] * 2 + num[2] * 4 + num[3] * 8
				+ num[4] * 16 + num[5] * 32 + num[6] * 64 + num[7] * 128;
			src.at<float>(my_vector[i].position.y, my_vector[i].position.x) = ((lut[uchar(sum / 8)] >> sum % 8) & 1) != 1 ? 255 : 0;
		}
	}
	dst = src.clone();
	dst.convertTo(dst, CV_8UC1);
}

//------拓扑信息提取
vector<SLine> Extract_Message(InputArray& _src,Mat& dst)
{
	Mat src = _src.getMat();
	Mat showImg = Mat::zeros(src.size(), CV_8UC3);
	//标记：
	//---	255：待处理骨架
	//---   1  ：已处理骨架
	//---	126  ：端点XXXX端点保存但是不做处理
	//---	处理完之后保存在一个容器之后封装起来
	/*
	struct SLine
	{
		Point begin;
		Point end;
	};
	*/
	vector<SLine> G_SLine;
	for (size_t i = 1; i < src.rows-1; i++)
	{
		for (size_t j = 1; j < src.cols-1; j++)
		{
			
			if (src.at<uchar>(i, j) == 0) continue;
			if(Charge(Point(j, i), src,0)) continue;
			SLine sline;//临时存放端点
			sline.begin = Point(j, i);//直接添加头端点
			vector<Point> point;
			point.push_back(Point(j, i));
			while (point.size() != 0)
			{
				src.at<uchar>(point[0].y, point[0].x) = 0;//直接把头端点消失，后面如果需要再进行对比距离取出来
				if(src.at<uchar>(point[0].y, point[0].x + 1)!=0)			point.push_back(Point(point[0].x+1, point[0].y));
				else if (src.at<uchar>(point[0].y, point[0].x - 1) != 0)	point.push_back(Point(point[0].x - 1, point[0].y));
				else if (src.at<uchar>(point[0].y+1, point[0].x + 1) != 0)	point.push_back(Point(point[0].x + 1, point[0].y + 1));
				else if (src.at<uchar>(point[0].y+1, point[0].x - 1) != 0)	point.push_back(Point(point[0].x - 1, point[0].y + 1));
				else if (src.at<uchar>(point[0].y+1, point[0].x ) != 0)		point.push_back(Point(point[0].x, point[0].y + 1));
				else if (src.at<uchar>(point[0].y-1, point[0].x + 1) != 0)	point.push_back(Point(point[0].x + 1, point[0].y - 1));
				else if (src.at<uchar>(point[0].y-1, point[0].x - 1) != 0)	point.push_back(Point(point[0].x - 1, point[0].y - 1));
				else if (src.at<uchar>(point[0].y-1, point[0].x ) != 0)		point.push_back(Point(point[0].x, point[0].y - 1));
				else point.push_back(Point(j, i));
				src.at<uchar>(point[1].y, point[1].x) = 0;//添加的点直接赋值为0，后期处理得出位置
				if (point[0] == point[1])//孤立点的情况
				{
					sline.end = point[0];
					point.erase(point.begin());
					point.erase(point.begin());
					continue;
				}
				point.erase(point.begin());
				if (Charge(Point(point[0].x, point[0].y), src,1)) continue;//不是端点
				//---端点处理
				sline.end = point[0];
				point.erase(point.begin());
				line(showImg, sline.begin, sline.end, Scalar(0, 0, 255), 1);
			}	
			G_SLine.push_back(sline);
		}
	}
	dst = showImg.clone();
	return G_SLine;
}

//判断是否为端点
bool Charge(Point center,Mat src,uchar num)
{
	int k = num;
	if (src.at<uchar>(center.y - 1, center.x - 1) != 0) k++;
	if (src.at<uchar>(center.y - 1, center.x + 1) != 0) k++;
	if (src.at<uchar>(center.y - 1, center.x) != 0) k++;
	if (src.at<uchar>(center.y + 1, center.x - 1) != 0) k++;
	if (src.at<uchar>(center.y + 1, center.x + 1) != 0) k++;
	if (src.at<uchar>(center.y + 1, center.x) != 0) k++;
	if (src.at<uchar>(center.y, center.x - 1) != 0) k++;
	if (src.at<uchar>(center.y, center.x + 1) != 0) k++;
	return k == 2;
	//return  bool((src.at<uchar>(center.y - 1, center.x - 1) == 0 ? 0 : 1 +
	//	(src.at<uchar>(center.y - 1, center.x + 1)) == 0 ? 0 : 1 +
	//	(src.at<uchar>(center.y - 1, center.x)) == 0 ? 0 : 1 +
	//	(src.at<uchar>(center.y + 1, center.x - 1)) == 0 ? 0 : 1 +
	//		(src.at<uchar>(center.y + 1, center.x + 1)) == 0 ? 0 : 1 +
	//		(src.at<uchar>(center.y + 1, center.x)) == 0 ? 0 : 1 +
	//			(src.at<uchar>(center.y, center.x - 1)) == 0 ? 0 : 1 +
	//			(src.at<uchar>(center.y, center.x + 1)) == 0 ? 0 : 1) == 2);
}

//---最大最小判断
void change(float& max, float& min)
{
	float mid;
	mid = max > min ? max : min;
	min = mid == max ? min : max;
	max = mid;
}

//---坐标转换并且求交点距离
//@-- ab两点表示一条直线，center代表与X轴平行的直线
//@-- 求取ab与目标直线交点，计算交点距离center的距离
float TrasForm(Point a,Point b,Point center)
{
	int x1 = int(a.x) - int(center.x);
	int y1 = -int(a.y) + int(center.y);
	int x2 = int(b.x) - int(center.x);
	int y2 = -int(b.y) + int(center.y);

	/*a = Point(a.x - center.x, -a.y + center.y);
	b = Point(b.x - center.x, -b.y + center.y);*/
	center = Point(center.x - center.x, -center.y + center.y);
	//&&&&         (y-y1)/(x-x1)=(y2-y1)/(x2-x1),令 y=0 求解x
	return abs(x1 + y1*((x2 - x1) / (y2 - y1)));
}

//---求直线与圆最近交点坐标
vector<Point> Intersection(vector<SLine> G_SLine, Point2f& center, float& radius)
{
	//---      y = kx + b && (x+c)(x+c)+(y+d)(y+d) = r*r
	float c = -center.x, d = -center.y;
	float r = radius;
	vector<Point> inter_point;
	for (size_t i = 0; i < G_SLine.size(); i++)
	{
		float x1 = G_SLine[i].begin.x, y1 = G_SLine[i].begin.y, x2 = G_SLine[i].end.x, y2 = G_SLine[i].end.y;
		float k = (y2 - y1) / (x2 - x1);
		float b = y1 - k*x1;
		float x3 = -(sqrt((k*k + 1)*r*r - c*c*k*k + (2 * c*d + 2 * b*c)*k - d*d - 2 * b*d - b*b) + (b + d)*k + c) / (k*k + 1);
		float y3 = -(k*(sqrt(k*k*r*r + r*r - c*c*k*k + 2 * c*d*k + 2 * b*c*k - d*d - 2 * b*d - b*b) + c) + d*k*k - b) / (k*k + 1);
		float x4 = (sqrt((k*k + 1)*r*r - c*c*k*k + (2 * c*d + 2 * b*c)*k - d*d - 2 * b*d - b*b) + (-b - d)*k - c) / (k*k + 1);
		float y4 = -(k*(c - sqrt(k*k*r*r + r*r - c*c*k*k + 2 * c*d*k + 2 * b*c*k - d*d - 2 * b*d - b*b)) + d*k*k - b) / (k*k + 1);
		float d1 = sqrt(pow((x3 - G_SLine[i].begin.x), 2) + pow(y3 - G_SLine[i].begin.y, 2));
		float d2 = sqrt(pow((x4 - G_SLine[i].begin.x), 2) + pow(y4 - G_SLine[i].begin.y, 2));
		if (d1 > d2)
		{
			inter_point.push_back(Point(x4, y4));
			continue;
		}
		inter_point.push_back(Point(x3, y3));
	}
	return inter_point;
}

//---从小到大排序已知点
//——！！！注意这里的排序是默认最小的点在center上方且在最左侧
//——！！！注意这里的排序是默认最大的点在center下方且在最右侧
//——！！！如果不满足条件将计算错误（纯软件定位有一点缺陷）
void Sort_Point(vector<Point>& inter_point,Point2f center)
{
	//分割上下方点
	vector<Point> Up_Point;
	vector<Point> Dw_Point;
	for (int i = 0; i < inter_point.size(); i++)
	{
		if (inter_point[i].y < center.y) Up_Point.push_back(inter_point[i]);
		else Dw_Point.push_back(inter_point[i]);
		inter_point.erase(inter_point.begin() + i);
		i--;
	}
	for (size_t i = 0; i < Up_Point.size(); i++)
	{
		for (size_t j = i+1; j < Up_Point.size(); j++)
		{
			if (Up_Point[i].x < Up_Point[j].x)continue;
			Point mid = Up_Point[j];
			Up_Point[j] = Up_Point[i];
			Up_Point[i] = mid;
		}
		inter_point.push_back(Up_Point[i]);
	}
	for (size_t i = 0; i < Dw_Point.size(); i++)
	{
		for (size_t j = i + 1; j < Dw_Point.size(); j++)
		{
			if (Dw_Point[i].x > Dw_Point[j].x)continue;
			Point mid = Dw_Point[j];
			Dw_Point[j] = Dw_Point[i];
			Dw_Point[i] = mid;
		}
		inter_point.push_back(Dw_Point[i]);
	}
}

//---对刻度进行插值
//---得到平均角度--->>>用于计算总角度
//---起始点的位置--->>>用做基准计算指针和起点的夹角
//---图像的每个角度代表实际的值--->>>通过上面两个得到的值就可计算
//---@aim_Point 指针在圆上的刻度位置
//---@A_angle	平均角度
//---@return	true:从起点开始，false：从尾点开始
bool Interpolation(vector<Point>& inter_point, Point2f& center,Point& aim_Point,float& A_angle)
{
	float begin = sqrt(pow(inter_point[0].x - aim_Point.x, 2) + pow(inter_point[0].y - aim_Point.y, 2));
	float end = sqrt(pow(inter_point.back().x - aim_Point.x, 2) + pow(inter_point.back().y - aim_Point.y, 2));
	float angle[4];//四个刻度的平均值表示总体平均刻度
	//float G_angle;
	bool charge;
	//----防止指针把起始点的刻度覆盖，所以靠近指针的坐标去除不用
	if (begin <= end)
	{//从起点开始插值
		for (size_t i = 0; i < 4; i++)
		{//涉及坐标转换和向量点积
			float x1 = float(inter_point[i].x) - float(center.x);
			float y1 = -float(inter_point[i].y) + float(center.y);
			float x2 = float(inter_point[i+1].x) - float(center.x);
			float y2 = -float(inter_point[i+1].y) + float(center.y);
			angle[i] = (acos((x1*x2 + y1*y2) / (sqrt(pow(x1, 2) + pow(y1, 2))* sqrt(pow(x2, 2) + pow(y2, 2))))) * 180 / CV_PI;
		}
		charge = true;
	}
	else
	{//从末尾开始插值
		vector<Point>::iterator it = inter_point.end();
		for (size_t i = 0; i < 4; i++)
		{//涉及坐标转换和向量点积
			float x1 = float(it->x) - float(center.x);
			float y1 = -float(it->y) + float(center.y);
			float x2 = float((it-1)->x) - float(center.x);
			float y2 = -float((it-1)->y) + float(center.y);
			angle[i] = (acos((x1*x2 + y1*y2) / (sqrt(pow(x1, 2) + pow(y1, 2))* sqrt(pow(x2, 2) + pow(y2, 2))))) * 180 / CV_PI;
			it--;
		}
		charge = false;
		//G_angle = A_angle * 11;//刻度总范围
	}
	for (size_t i = 0; i < 4; i++)
	{//排序角度
		for (size_t j = i + 1; j < 4; j++)
		{
			change(angle[i], angle[j]);
		}
	}
	A_angle = (angle[1] + angle[2]) / 2;//去除最大和最小值之后取平均
	//G_angle = A_angle * 11;//刻度总范围
	return charge;
}

//---查找起始点的位置和包含的角度范围
//---本算法通过角度比例来近似求解水表盘读书
float Calculate(vector<Point>& inter_point, Point2f& center, Point pointer1, Point pointer2, float radius)
{
	Point meter_center_point = Point((pointer1.x + pointer2.x) / 2, (pointer1.y + pointer2.y) / 2);//中心指针最远坐标
	vector<SLine> input_point;
	SLine line_point;
	line_point.begin = center;
	line_point.end	 = meter_center_point;
	input_point.push_back(line_point);
	vector<Point> point = Intersection(input_point, center, radius);//找指针在圆上的刻度
	float A_angle,present_Point;
	bool flag = Interpolation(inter_point, center, meter_center_point, A_angle);
	//---利用 y = kx + b,点和直线的位置关系判断夹角是否大于180度
	if (flag)
	{	
		int x1 = int(inter_point[0].x) - int(center.x);
		int y1 = -int(inter_point[0].y) + int(center.y);
		int x2 = int(meter_center_point.x) - int(center.x);
		int y2 = -int(meter_center_point.y) + int(center.y);
		present_Point = (acos((x1*x2 + y1*y2) / (sqrt(pow(x1, 2) + pow(y1, 2))* sqrt(pow(x2, 2) + pow(y2, 2))))) * 180 / CV_PI;
		float k = y1 / x1;
		float b = y1 - k * x1;
		//---起点位于第二象限(大于180用补角度表示)
		present_Point = k*x2 + b > y2 ? 360 - present_Point : present_Point;
		//---起点位于第一象限(暂时不去考虑)
	}
	else
	{
		float x1 = float(inter_point.back().x) - float(center.x);
		float y1 = -float(inter_point.back().y) + float(center.y);
		float x2 = float(meter_center_point.x) - float(center.x);
		float y2 = -float(meter_center_point.y) + float(center.y);
		present_Point = (acos((x1*x2 + y1*y2) / (sqrt(pow(x1, 2) + pow(y1, 2))* sqrt(pow(x2, 2) + pow(y2, 2))) ))*180/CV_PI;
		float k = y1 / x1;
		float b = y1 - k * x1;
		//---终点位于第三象限(大于180度用补角度表示)
		present_Point = k*x2 + b < y2 ? 360 - present_Point : present_Point;
		present_Point = 11 * A_angle - present_Point;
		//---起点位于第一象限(暂时不去考虑)
	}
	return present_Point / (11 * A_angle) * 11 * 5;
}





