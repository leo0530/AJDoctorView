#if 0
#include<opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
using namespace cv;
#include<iostream>
#include<string>
using namespace std;
//输入图像
Mat img;
//灰度值归一化
Mat bgr;
//HSV图像
Mat hsv;
//色相
int hmin = 0;
int hmin_Max = 360;
int hmax = 360;
int hmax_Max = 360;
//饱和度
int smin = 0;
int smin_Max = 255;
int smax = 255;
int smax_Max = 255;
//亮度
int vmin = 106;
int vmin_Max = 255;
int vmax = 250;
int vmax_Max = 255;
//显示原图的窗口
string windowName = "src";
//输出图像的显示窗口
string dstName = "dst";
//输出图像
Mat dst;
//回调函数
void callBack(int, void*)
{
	//输出图像分配内存
	dst = Mat::zeros(img.size(), CV_32FC3);
	//掩码
	Mat mask;
	inRange(hsv, Scalar(hmin, smin / float(smin_Max), vmin / float(vmin_Max)), Scalar(hmax, smax / float(smax_Max), vmax / float(vmax_Max)), mask);
	//只保留
	for (int r = 0; r < bgr.rows; r++)
	{
		for (int c = 0; c < bgr.cols; c++)
		{
			if (mask.at<uchar>(r, c) == 255)
			{
				dst.at<Vec3f>(r, c) = bgr.at<Vec3f>(r, c);
			}
		}
	}
	//输出图像
	imshow(dstName, dst);
	//保存图像
	dst.convertTo(dst, CV_8UC3, 255.0, 0);
	imwrite("HSV_inRange.jpg", dst);
}
int main(int argc, char*argv[])
{
	//输入图像
	img = imread("sample.jpg", IMREAD_COLOR);
	if (!img.data || img.channels() != 3)
		return -1;
	imshow(windowName, img);
	//彩色图像的灰度值归一化
	img.convertTo(bgr, CV_32FC3, 1.0 / 255, 0);
	//颜色空间转换
	cvtColor(bgr, hsv, COLOR_BGR2HSV);
	//定义输出图像的显示窗口
	namedWindow(dstName, WINDOW_GUI_EXPANDED);
	//调节色相 H
	createTrackbar("hmin", dstName, &hmin, hmin_Max, callBack);
	createTrackbar("hmax", dstName, &hmax, hmax_Max, callBack);
	//调节饱和度 S
	createTrackbar("smin", dstName, &smin, smin_Max, callBack);
	createTrackbar("smax", dstName, &smax, smax_Max, callBack);
	//调节亮度 V
	createTrackbar("vmin", dstName, &vmin, vmin_Max, callBack);
	createTrackbar("vmax", dstName, &vmax, vmax_Max, callBack);
	callBack(0, 0);
	waitKey(0);
	return 0;
}
#endif

#include <iostream>
#include<opencv2\opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define PI 3.1415926

using namespace cv;
using namespace std;

void RGB2HSV(double red, double green, double blue, double& hue, double& saturation, double& intensity)
{

	double r, g, b;
	double h, s, i;

	double sum;
	double minRGB, maxRGB;
	double theta;

	r = red / 255.0;
	g = green / 255.0;
	b = blue / 255.0;

	minRGB = ((r < g) ? (r) : (g));
	minRGB = (minRGB<b) ? (minRGB) : (b);

	maxRGB = ((r>g) ? (r) : (g));
	maxRGB = (maxRGB > b) ? (maxRGB) : (b);

	sum = r + g + b;
	i = sum / 3.0;

	if (i < 0.001 || maxRGB - minRGB < 0.001)
	{
		h = 0.0;
		s = 0.0;
	}
	else
	{
		s = 1.0 - 3.0*minRGB / sum;
		theta = sqrt((r - g)*(r - g) + (r - b)*(g - b));
		theta = acos((r - g + r - b)*0.5 / theta);
		if (b <= g)
			h = theta;
		else
			h = 2 * PI - theta;
		if (s <= 0.01)
			h = 0;
	}

	hue = (int)(h * 180 / PI);
	saturation = (int)(s * 100);
	intensity = (int)(i * 100);
}

Mat picture_red(Mat input)
{

	Mat frame;
	Mat srcImg = input;
	frame = srcImg;
	waitKey(1);
	int width = srcImg.cols;
	int height = srcImg.rows;

	int x, y;
	double B = 0.0, G = 0.0, R = 0.0, H = 0.0, S = 0.0, V = 0.0;
	Mat vec_rgb = Mat::zeros(srcImg.size(), CV_8UC1);
	for (x = 0; x < height; x++)
	{
		for (y = 0; y < width; y++)
		{
			B = srcImg.at<Vec3b>(x, y)[0];
			G = srcImg.at<Vec3b>(x, y)[1];
			R = srcImg.at<Vec3b>(x, y)[2];
			RGB2HSV(R, G, B, H, S, V);
			//红色范围，范围参考的网上。可以自己调
			if ((H >= 312 && H <= 360) && (S >= 17 && S <= 100) && (V>18 && V < 100))
				vec_rgb.at<uchar>(x, y) = 255;
			/*cout << H << "," << S << "," << V << endl;*/
		}
	}
	imshow("hsv", vec_rgb);
	return vec_rgb;
}

void O_x1y1(Mat in, double *x1, double *y1, double *x2, double *y2)

{
	Mat matSrc = in;
	/*Mat matSrc = imread("qwer9.png", 0);*/

	GaussianBlur(matSrc, matSrc, Size(5, 5), 0);//高斯滤波，除噪点

	vector<vector<Point> > contours;//contours的类型，双重的vector

	vector<Vec4i> hierarchy;//Vec4i是指每一个vector元素中有四个int型数据。

	//阈值
	threshold(matSrc, matSrc, 100, 255, THRESH_BINARY);//图像二值化

	//寻找轮廓，这里注意，findContours的输入参数要求是二值图像，二值图像的来源大致有两种，第一种用threshold，第二种用canny
	findContours(matSrc.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// 计算矩

	vector<Moments> mu(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  计算矩中心:

	vector<Point2f> mc(contours.size());

	for (int i = 0; i < contours.size(); i++)

	{

		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		cout << mc[i] << endl;

	}

	/// 绘制轮廓
	Mat drawing = Mat::zeros(matSrc.size(), CV_8UC1);

	for (int i = 0; i < contours.size(); i++)

	{

		Scalar color = Scalar(255);

		//drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());//绘制轮廓函数

		circle(drawing, mc[i], 4, color, -1, 8, 0);


	}
	*x1 = mc[0].x;
	*y1 = mc[0].y;
	*x2 = mc[contours.size() - 1].x;
	*y2 = mc[contours.size() - 1].y;

	imshow("outImage", drawing);

}

int main()
{

	double xx1, yy1, xx2, yy2;
	double x1, y1, x2, y2;

	Mat matSrc = imread("1.JPG");

	Mat middle = picture_red(matSrc);
	O_x1y1(middle, &xx1, &yy1, &xx2, &yy2);
	x1 = xx1;
	y1 = yy1;
	x2 = xx2;
	y2 = yy2;

	imshow("原图", matSrc);
	imshow("red", picture_red(matSrc));



	cout << "红点1：" << x1 << "，" << y1 << "; " << "红点2：" << x2 << "， " << y2 << endl;
	waitKey();

	return 0;
}