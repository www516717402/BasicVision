
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

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


/*
图像边界为4
目标边界为1(测试使用255)		--->> 意思是作为检测的边界，这个边界是不断变化的
背景为2
目标为0
确定目标边界3					--->> 意思是边界1经过涨水、查表确定的最终边界
*/
void FindRidge(InputArray& _src, Mat& mask, vector<Point> Edge_Point, uchar thred = 5);
vector<Point> FindEdge(Mat& src,Mat& mask);//直接用掩膜去制作
//vector<Point> FindEdge(InputArray& _src, Mat& mask);//利用原图去制作，此方法不能修补图像
int FillBlock(InputArray& _src, vector<Point>& Edge_Point, Mat& mask, int level, Point center);
void FloorEdge(InputArray& _src, vector<Point>& Edge_Point, Mat& mask, int level);
bool CheckData(Mat& mask, Point center);
int main(int argc,char**argv)
{
	Mat inputImage = imread("222.png", 0);
	//---为写入参数做准备
	threshold(inputImage, inputImage, 0, 255, THRESH_BINARY | THRESH_OTSU);

	//---调用函数
	vector<Point> Edge_Point;
	Mat mask;
	FindRidge(inputImage, mask, Edge_Point);
	return 0;
}
//----寻找山脊线函数
//----@_src	：	二值图
//----@_mask:	无要求，改进之后直接在内部进行操作
//----@thred:	距离变换之后的阈值，找山脊线-->>(0-10),修补图像(大于10)
void FindRidge(InputArray& _src, Mat& mask, vector<Point> Edge_Point, uchar thred)
{
	Mat src = _src.getMat();// , mask = _mask.getMat();

	//---制作掩膜，边界为4，背景为2，目标为0
	mask = src.clone();
	bitwise_not(mask, mask);
	distanceTransform(mask, mask, DIST_L2, DIST_MASK_3, 5);
	normalize(mask, mask, 0, 255, NORM_MINMAX);
	mask.convertTo(mask, CV_8UC1);
	threshold(mask, mask, thred, 255, THRESH_BINARY_INV);
	Mat temp = mask.clone();

	//---接着上面还没制作完成的mask继续制作
	bitwise_not(mask, mask);
	mask = mask - 253;//背景初始化为2
	Mat rows = Mat::ones(Size(src.cols, 1), CV_8UC1), cols = Mat::zeros(Size(1, src.rows), CV_8UC1);
	rows.setTo(4); cols.setTo(4);//边界初始化为4
	Mat src_rows_beg = mask.row(0),				src_cols_beg = mask.col(0);
	Mat src_rows_end = mask.row(src.rows - 1),	src_cols_end = mask.col(src.cols - 1);
	rows.copyTo(src_rows_beg); rows.copyTo(src_rows_end);
	cols.copyTo(src_cols_beg); cols.copyTo(src_cols_end);

	//---查找目标边界，并且掩膜边界初始化为1
	Edge_Point = FindEdge(temp, mask);
	//---findcontours查找会有漏的,这里修补mask
	bool TF = true;
	while (TF)
	{
		TF = false;
		for (size_t i = 0; i < mask.rows - 1; i++)
		{
			uchar* msk_up = mask.ptr(i - 1);
			uchar* msk = mask.ptr(i);
			uchar* msk_dw = mask.ptr(i + 1);
			for (size_t j = 0; j < mask.cols - 1; j++)
			{
				uchar _temp_data = msk[j];
				msk[j] = msk[j] == 2 && (msk_up[j] == 255 || msk_up[j] == 0)
					&& (msk_dw[j] == 255 || msk_dw[j] == 0)
					&& (msk[j - 1] == 255 || msk[j - 1] == 0)
					&& (msk[j + 1] == 255 || msk[j + 1] == 0) ? 0 : msk[j];
				msk[j] = msk[j] == 0 && (msk_up[j] == 2 || msk_dw[j] == 2 || msk[j - 1] == 2 || msk[j + 1] == 2) ? 2 : msk[j];
				TF = _temp_data != msk[j] ? true : TF;//判断是否还有错误的边界
			}
		}
	}

	//---距离变换之后归一化到0-255的uchar格式
	distanceTransform(src, src, DIST_L2, DIST_MASK_3, 5);
	normalize(src, src, 0, 255, NORM_MINMAX);
	src.convertTo(src, CV_8UC1);

	//直方图计算，加速后面涨水的速度
	const int histSize = 255;
	float range[] = { 0, 255 };
	const float* histRange = { range };
	Mat hist;
	calcHist(&src, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

	//涨水0-255
	hist = hist.reshape(1,1);//列转换成行
	normalize(hist, hist, 0, 1000, NORM_MINMAX);
	hist.convertTo(hist, CV_32FC1);
	//float* ptr = hist.ptr(0);
	for (size_t level = 0; level <= 255; level++)
	{
		if (!hist.at<float>(0,level)) continue;//直方图加速，如果为0说明没有这个像素就不用循环了// cout << "i = " << i << " " << int(ptr[i]) << endl;
		FloorEdge(src, Edge_Point, mask, level);
	}
}

/*
//----寻找图像的边界
//----@_src	:二值图，背景为0
//----@		:返回边界的像素
vector<Point> FindEdge(InputArray& _src, Mat& mask)
{
	Mat src1 = _src.getMat(),src = src1.clone();
	//---先腐蚀，然后再去找边界，边界是前景边界，如果不腐蚀那就是背景边界
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(src, src, MORPH_ERODE, kernel);
	vector<Point> wjy_Point;//存储边界所有的像素点
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		for (size_t j = 0; j < contours[i].size(); j++)
		{
			wjy_Point.push_back(contours[i][j]);//存储
			mask.at<uchar>(contours[i][j]) = 255;//掩膜边界赋值
		}
	}
	return wjy_Point;
}
*/

//----寻找图像的边界
//----这个函数写的很差，原图是扩展的原图，mask是边界等处理的眼膜图，之后进行改进吧
//----@src	:
//----@		:返回边界的像素
vector<Point> FindEdge(Mat& src,Mat& mask)
{
	//Mat src = mask.clone();
	//---先腐蚀，然后再去找边界，边界是前景边界，如果不腐蚀那就是背景边界
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	//morphologyEx(src, src, MORPH_ERODE, kernel);
	vector<Point> wjy_Point;//存储边界所有的像素点
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		for (size_t j = 0; j < contours[i].size(); j++)
		{
			wjy_Point.push_back(contours[i][j]);//存储
			mask.at<uchar>(contours[i][j]) = 255;//掩膜边界赋值
		}
	}
	return wjy_Point;
}

//----涨水判断边界函数
//----@_src			：距离图
//----@Edge_Point	: 边界点
//----@_mask		: 掩膜图
//----@level		: 水位值
void FloorEdge(InputArray& _src, vector<Point>& Edge_Point, Mat& mask,int level)
{
	Mat src = _src.getMat();
	for (int i = 0; i < Edge_Point.size(); i++)
	{
		//---上下左右四个位置
		vector<Point> temp_vector;
		temp_vector.push_back(Point(Edge_Point[i].x, Edge_Point[i].y - 1));
		temp_vector.push_back(Point(Edge_Point[i].x, Edge_Point[i].y + 1));
		temp_vector.push_back(Point(Edge_Point[i].x - 1, Edge_Point[i].y));
		temp_vector.push_back(Point(Edge_Point[i].x + 1, Edge_Point[i].y));
		uchar* msk = mask.ptr(Edge_Point[i].y);
		uchar* img = src.ptr(Edge_Point[i].y);
		if (img[Edge_Point[i].x] > level)	continue;//安全水位不做判断
		bool Flag = true;//判断是否有凹池
		uchar count_num = 0;//这个用来解决靠的很近的线段，但是效果不好，可以不要这个条件
		for (size_t j = 0; j < temp_vector.size(); j++)//这里是当前像素低于或等于水位
		{
			uchar* pre_data = mask.ptr(temp_vector[j].y);
			if (pre_data[temp_vector[j].x] == 2 || pre_data[temp_vector[j].x] == 4)//背景或者边界
			{
				pre_data[temp_vector[j].x] = 2;
				continue;
			}
			else if (pre_data[temp_vector[j].x] == 128 || pre_data[temp_vector[j].x] == 255)
			{
				continue;//确定边界				
				count_num++;
			}
			else
			{
				if (src.at<uchar>(temp_vector[j]) >= level)//旁边像素大于水位
				{			
					count_num++;
					pre_data[temp_vector[j].x] = 255;//mask边界跟进
					Edge_Point.push_back(temp_vector[j]);	
					//Edge_Point.insert(Edge_Point.begin()+i+1,temp_vector[j]);//删除当前数据
				}
				else//小于水位就是有凹池
				{
					int temp = FillBlock(src, Edge_Point, mask, level, Edge_Point[i]);//凹池一次性填充
					Flag = false;
				}
			}
		}
		if (false)//没有出现凹池、因为有凹池的话，当前位置赋值为背景2，不存在查表直接去除了
		{
			msk[Edge_Point[i].x] = 128;//开口防水之后的点肯定为边界点，在这里把封口堵住并且确定为128
			Edge_Point.erase(Edge_Point.begin() + i);//删除当前数据
			i--;//光标不移动，直接处理下一个
			continue;
		}
		else if (count_num == 4)
		{
			Edge_Point.push_back(Edge_Point[i]);
			Edge_Point.erase(Edge_Point.begin() + i);//删除当前数据
			i--;//光标不移动，直接处理下一个
			continue;
		}
		else if (msk[Edge_Point[i].x] == 255)//没有凹池且当前值为未确定边界、因为确定边界3不需要判断
		{
			msk[Edge_Point[i].x] = CheckData(mask, Edge_Point[i]) == true ? 128 : 2;//查表
			Edge_Point.erase(Edge_Point.begin() + i);//删除当前数据
			i--;//光标不移动，直接处理下一个
		}
	}
}

//---填充凹池
//---@src		：input distance image/float/one signal
//---@mask		：mask image/ucahr/one signal
//---@center	：nowdays operator centers/ucahr
int FillBlock(InputArray& _src, vector<Point>& Edge_Point, Mat& mask, int level, Point center)
{
	Mat src = _src.getMat();
	mask.at<uchar>(center) = 2;//有凹槽，打开一个口子让水流进去
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
				//---比水位低、且不为自身、且为未操作过的像素
				if (img[i][fill_point[count].x + j] < level && !(j == 0 && i == 1) && msk[i][fill_point[count].x + j] == 0)
				{
					fill_point.push_back(Point(fill_point[count].x + j, fill_point[count].y + i - 1));
					msk[i][fill_point[count].x + j] = 2;//填充为背景
				}
				else if (img[i][fill_point[count].x + j] >= level && msk[i][fill_point[count].x + j] == 0)
				{
					Edge_Point.push_back(Point(fill_point[count].x + j, fill_point[count].y + i - 1));
					msk[i][fill_point[count].x + j] = 255;//填充为边界
				}
			}
		}
		//msk[1][fill_point[count].x] = 2;//当前像素填充为1
		count_mount = fill_point.size() - 1;//循环的核心，
		fill_point.erase(fill_point.begin());//执行一个删除一个，不占用内存
	}
	return 0;
}

//---查表函数
//---返回true为保留值，false为删除值
bool CheckData(Mat& mask, Point center)
{
	uchar* msk_up = mask.ptr(center.y - 1);
	uchar* msk = mask.ptr(center.y);
	uchar* msk_dw = mask.ptr(center.y + 1);
	int num[8];
	//---把边界255、实际边界3、目标0都当做参考，这里有点绕人，仔细想一下就好了
	int sum = (num[0] = msk_up[center.x - 1] == 255 || msk_up[center.x - 1] == 128 || msk_up[center.x - 1] == 0 ? 1 : 0)
			+ (num[1] = msk_up[center.x] == 255 || msk_up[center.x] == 128 || msk_up[center.x] == 0 ? 1 : 0) * 2
			+ (num[2] = msk_up[center.x + 1] == 255 || msk_up[center.x + 1] == 128 || msk_up[center.x + 1] == 0 ? 1 : 0) * 4
			+ (num[3] = msk[center.x - 1] == 255 || msk[center.x - 1] == 128 || msk[center.x - 1] == 0 ? 1 : 0) * 8
			+ (num[4] = msk[center.x + 1] == 255 || msk[center.x + 1] == 128 || msk[center.x + 1] == 0 ? 1 : 0) * 16
			+ (num[5] = msk_dw[center.x - 1] == 255 || msk_dw[center.x - 1] == 128 || msk_dw[center.x - 1] == 0 ? 1 : 0) * 32
			+ (num[6] = msk_dw[center.x] == 255 || msk_dw[center.x] == 128 || msk_dw[center.x] == 0 ? 1 : 0) * 64
			+ (num[7] = msk_dw[center.x + 1] == 255 || msk_dw[center.x + 1] == 128 || msk_dw[center.x + 1] == 0 ? 1 : 0) * 128;
	return ((lut[uchar(sum / 8)] >> sum % 8) & 1) != 1 ? true : false;
}



