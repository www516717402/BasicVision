/*
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void waterSegment(InputArray& _src, OutputArray& _dst, int& noOfSegment, Mat testimage);
Mat thinImage(Mat & src, int maxIterations);
void segMerge(Mat& image, Mat& segments, int& numSeg);
int main(int argc, char** argv) {
	
	Mat inputImage = imread("111.jpg");
	assert(!inputImage.data);
	Mat graImage, outputImage,src;
	int offSegment;
	//图像细化  
	src = thinImage(inputImage,-1);
	//显示图像  
	src = src * 255;
	waterSegment(inputImage, outputImage, offSegment, src);

	waitKey(0);
	return 0;
}

void waterSegment(InputArray& _src,OutputArray& _dst,int& noOfSegment,Mat testimage)
{
	Mat src = _src.getMat();//dst = _dst.getMat();
	Mat grayImage;
	cvtColor(src, grayImage,CV_BGR2GRAY);
	threshold(grayImage, grayImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//Mat kernel = getStructuringElement(MORPH_RECT, Size(9, 9), Point(-1, -1));
	//morphologyEx(grayImage, grayImage, MORPH_CLOSE, kernel);
	distanceTransform(grayImage, grayImage, DIST_L2, DIST_MASK_3, 5);
	normalize(grayImage, grayImage,0,1, NORM_MINMAX);
	grayImage.convertTo(grayImage, CV_8UC1);
	threshold(grayImage, grayImage,0,255, THRESH_BINARY | THRESH_OTSU);
	////morphologyEx(grayImage, grayImage, MORPH_CLOSE, kernel);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat showImage = Mat::zeros(grayImage.size(), CV_32SC1);
	findContours(testimage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	for (size_t i = 0; i < contours.size(); i++)
	{
		//这里static_cast<int>(i+1)是为了分水岭的标记不同，区域1、2、3。。。。这样才能分割
		drawContours(showImage, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i+1)), 1);
	}
	////drawContours(showImage, contours, static_cast<int>(0), Scalar::all(static_cast<int>(1)), 1);
	////drawContours(showImage, contours, static_cast<int>(1), Scalar::all(static_cast<int>(2)), 1);
	////drawContours(showImage, contours, static_cast<int>(2), Scalar::all(static_cast<int>(3)), 1);
	//drawContours(showImage, contours, static_cast<int>(3), Scalar::all(static_cast<int>(4)), 1);
	//drawContours(showImage, contours, static_cast<int>(4), Scalar::all(static_cast<int>(4)), 1);
	//drawContours(showImage, contours, static_cast<int>(5), Scalar::all(static_cast<int>(4)), 1);
	//drawContours(showImage, contours, static_cast<int>(6), Scalar::all(static_cast<int>(4)), 1);
	////drawContours(showImage, contours, static_cast<int>(7), Scalar::all(static_cast<int>(6)), 1);
	//drawContours(showImage, contours, static_cast<int>(8), Scalar::all(static_cast<int>(4)), 1);
	//drawContours(showImage, contours, static_cast<int>(9), Scalar::all(static_cast<int>(4)), 1);
	//drawContours(showImage, contours, static_cast<int>(10), Scalar::all(static_cast<int>(4)), 1);
	////drawContours(showImage, contours, static_cast<int>(11), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(12), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(13), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(14), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(15), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(16), Scalar::all(static_cast<int>(5)), 1);
	//drawContours(showImage, contours, static_cast<int>(17), Scalar::all(static_cast<int>(4)), 1);
	////drawContours(showImage, contours, static_cast<int>(18), Scalar::all(static_cast<int>(5)), 1);
	////drawContours(showImage, contours, static_cast<int>(19), Scalar::all(static_cast<int>(5)), 1);

	//Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	//morphologyEx(src, src, MORPH_ERODE, k);
	watershed(src, showImage);
	int num = contours.size();
	segMerge(src, showImage, num);
	//随机分配颜色
	vector<Vec3b> colors;
	for (size_t i = 0; i < contours.size(); i++) {
		int r = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int b = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	// 显示
	Mat dst = Mat::zeros(showImage.size(), CV_8UC3);
	int index = 0;
	for (int row = 0; row < showImage.rows; row++) {
		for (int col = 0; col < showImage.cols; col++) {
			index = showImage.at<int>(row, col);
			if (index > 0 && index <= contours.size()) {
				dst.at<Vec3b>(row, col) = colors[index - 1];
			}
			else if (index == -1)
			{
				dst.at<Vec3b>(row, col) = Vec3b(255, 255, 255);
			}
			else {
				dst.at<Vec3b>(row, col) = Vec3b(0, 0, 0);
			}
		}
	}
}

Mat thinImage (Mat & src,int maxIterations)
{
	maxIterations = -1;
	assert(src.type() == CV_8UC1);
	cv::Mat dst;
	int width = src.cols;
	int height = src.rows;
	src.copyTo(dst);
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	//将原图像转换为二值图像  
	threshold(dst, dst, 0, 1, THRESH_BINARY | THRESH_OTSU);
	int count = 0;  //记录迭代次数  
	while (true)
	{
		count++;
		if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达  
			break;
		std::vector<uchar *> mFlag; //用于标记需要删除的点  
									//对点标记  
		for (int i = 0; i < height; ++i)
		{
			uchar * p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//如果满足四个条件，进行标记  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
					{
						//标记  
						mFlag.push_back(p + j);
					}
				}
			}
		}

		//将标记的点删除  
		for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}

		//直到没有点满足，算法结束  
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//将mFlag清空  
		}

		//对点标记  
		for (int i = 0; i < height; ++i)
		{
			uchar * p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//如果满足四个条件，进行标记  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
					{
						//标记  
						mFlag.push_back(p + j);
					}
				}
			}
		}

		//将标记的点删除  
		for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}

		//直到没有点满足，算法结束  
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//将mFlag清空  
		}
	}
	return dst;
}
// 功能：代码 7-2 分水岭分割合并
// 作者：朱伟 zhu1988wei@163.com
// 来源：《OpenCV图像处理编程实例》
// 博客：http://blog.csdn.net/zhuwei1988
// 更新：2016-8-1
// 说明：版权所有，引用或摘录请联系作者，并按照上面格式注明出处，谢谢。// 
// 分割合并
void segMerge(Mat & image, Mat & segments, int & numSeg)
{
	// 对一个分割部分进行像素统计
	vector<Mat> samples;
	// 统计数据更新
	int newNumSeg = numSeg;
	// 初始化分割部分
	for (int i = 0; i <= numSeg; i++)
	{
		Mat sampleImage;
		samples.push_back(sampleImage);
	}
	// 统计每一个部分
	for (int i = 0; i < segments.rows; i++)
	{
		for (int j = 0; j < segments.cols; j++)
		{
			// 检查每个像素的归属
			int index = segments.at<int>(i, j);
			if (index >= 0 && index<numSeg)
				samples[index].push_back(image(Rect(j, i, 1, 1)));
		}
	}
	// 创建直方图
	vector<MatND> hist_bases;
	Mat hsv_base;
	// 直方图参数设置
	int h_bins = 35;
	int s_bins = 30;
	int histSize[] = { h_bins, s_bins };
	// hue 变换范围 0 to 256, saturation 变换范围0 to 180
	float h_ranges[] = { 0, 256 };
	float s_ranges[] = { 0, 180 };
	const float* ranges[] = { h_ranges, s_ranges };
	// 使用第0与1通道
	int channels[] = { 0, 1 };
	// 直方图生成
	MatND hist_base;
	for (int c = 1; c < numSeg; c++)
	{
		if (samples[c].dims>0) {
			// 将区域部分转换成hsv
			cvtColor(samples[c], hsv_base, CV_BGR2HSV);
			// 直方图统计
			calcHist(&hsv_base, 1, channels, Mat(),
				hist_base, 2, histSize, ranges, true, false);
			// 直方图归一化
			normalize(hist_base, hist_base, 0, 1,
				NORM_MINMAX, -1, Mat());
			// 添加到统计集
			hist_bases.push_back(hist_base);
		}
		else
		{
			hist_bases.push_back(MatND());
		}
		hist_base.release();
	}
	double similarity = 0;
	vector<bool> mearged;
	for (int k = 0; k < hist_bases.size(); k++)
	{
		mearged.push_back(false);
	}
	// 统计每一个部分的直方图相似
	for (int c = 0; c<hist_bases.size(); c++)
	{
		for (int q = c + 1; q<hist_bases.size(); q++)
		{
			if (!mearged[q])
			{
				// 判断直方图的维度
				if (hist_bases[c].dims>0 && hist_bases[q].dims>0)
				{
					// 直方图对比
					similarity = compareHist(hist_bases[c],
						hist_bases[q], CV_COMP_BHATTACHARYYA);
					if (similarity > 0.8)
					{
						mearged[q] = true;
						if (q != c)
						{
							// 区域部分减少
							newNumSeg--;
							for (int i = 0; i < segments.rows; i++)
							{
								for (int j = 0; j < segments.cols; j++)
								{
									int index = segments.at<int>(i, j);
									// 合并
									if (index == q)
									{
										segments.at<int>(i, j) = c;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	numSeg = newNumSeg;
}
*/

#include<opencv2/opencv.hpp>
#include<iostream>
using namespace std;
using namespace cv;

#define WINDOW_NAME "【程序窗口1】"
Mat g_maskImage, g_srcImage;
Point prevPt(-1, -1);
static void on_Mouse(int event, int x, int y, int flags, void*);

int main()
{
	//载入原图，初始化掩膜和灰度图
	g_srcImage = imread("123.png");
	imshow(WINDOW_NAME, g_srcImage);
	Mat srcImage, grayImage;
	g_srcImage.copyTo(srcImage);
	cvtColor(g_srcImage, g_maskImage, COLOR_BGR2GRAY);
	cvtColor(g_maskImage, grayImage, COLOR_GRAY2BGR);
	g_maskImage = Scalar::all(0);
	//设置鼠标回调函数
	setMouseCallback(WINDOW_NAME, on_Mouse);
	//轮询按键
	while (1)
	{
		int c = waitKey(0);
		if ((char)c == 27) break;
		if ((char)c == '2') {   //按键‘2’， 恢复源图
			g_maskImage = Scalar::all(0);
			srcImage.copyTo(g_srcImage);
			imshow("image", g_srcImage);
		}
		if ((char)c == '1' || (char)c == ' ') {
			//定义一些参数            
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			//寻找轮廓
			findContours(g_maskImage, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			//轮廓为空时的处理
			if (contours.empty()) continue;
			//复制掩膜
			Mat maskImage(g_maskImage.size(), CV_32S);
			maskImage = Scalar::all(0);
			//循环绘制出轮廓
			int compCount = 0;
			for (int index = 0; index >= 0; index = hierarchy[index][0], compCount++)
				drawContours(maskImage, contours, index, Scalar::all(compCount + 1), -1, LINE_8, hierarchy);
			//compCount为零时的处理
			if (compCount == 0)
				continue;
			//生成随机颜色
			vector<Vec3b> colorTab;
			for (unsigned int i = 0; i < compCount; i++) {
				int b = theRNG().uniform(0, 255);
				int g = theRNG().uniform(0, 255);
				int r = theRNG().uniform(0, 255);
				colorTab.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
			}
			//计算处理时间并输出到窗口中
			double dTime = (double)getTickCount();
			watershed(srcImage, maskImage);
			dTime = (double)getTickCount() - dTime;
			printf("处理时间=%gms\n", dTime*1000. / getTickFrequency());
			//双层循环，将分水岭图像遍历存入watershedImage中
			Mat watershedImage(maskImage.size(), CV_8UC3);
			for (unsigned int i = 0; i < maskImage.rows; i++)
				for (unsigned int j = 0; j < maskImage.cols; j++)
				{
					int index = maskImage.at<int>(i, j);
					if (index == -1)
						watershedImage.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
					else if (index <= 0 || index > compCount)
						watershedImage.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
					else
						watershedImage.at<Vec3b>(i, j) = colorTab[index - 1];
				}
			//混合灰度图和分水岭效果图并显示最终的窗口
			//watershedImage = watershedImage*0.5 + grayImage*0.5;
			imshow("watershed transform", watershedImage);
		}
	}
	return 0;
}

static void on_Mouse(int event, int x, int y, int flags, void*)
{
	//处理鼠标不在窗口中的情况
	if (x < 0 || x >= g_srcImage.cols || y < 0 || y >= g_srcImage.rows) return;
	//处理鼠标左键相关消息
	if (event == EVENT_LBUTTONUP || !(flags & EVENT_FLAG_LBUTTON))   //左键抬起动作或处于没有按下状态
		prevPt = Point(-1, -1);
	else if (event == EVENT_LBUTTONDOWN)  //左键按下动作
		prevPt = Point(x, y);
	//鼠标左键按下并移动，绘制出白色线条
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		Point pt(x, y);
		if (prevPt.x < 0) prevPt = pt;
		line(g_maskImage, prevPt, pt, Scalar::all(255), 5);
		line(g_srcImage, prevPt, pt, Scalar::all(255), 5);
		prevPt = pt;
		imshow(WINDOW_NAME, g_srcImage);
	}

}