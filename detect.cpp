#include "detect.h"

Detect::Detect(){}
Detect::~Detect(){}

constexpr auto PI = 3.14159;

int Detect::CheckColor(Mat& src, Mat& bin) 
{
	//参数 1，输入图像；2，输出图像
	Mat dst;
	dst.release();
	dst = src.clone();
	//mat to compute YCbCr
	Mat s0, s1, s2, s3, s4, s5;
	double avgofR = 0;
	//(3x3)kernal to dilate or erode
	Mat kernal = Mat::ones(Size(3, 3), CV_8UC1);

	//Mat array to store R-G-B three channel
	Mat array[10];

	//split 3 channel
	split(dst, array);

	//compute mean of R channel
	avgofR = mean(array[2])[0];

	//compute S and I in HSI and compute mask of both
	array[0].convertTo(s0, CV_32FC1);
	array[1].convertTo(s1, CV_32FC1);
	array[2].convertTo(s2, CV_32FC1);
	s3 = 0.257 * s2 + 0.564 * s1 + 0.098 * s0 + 16; //Y
	s4 = -0.148 * s2 - 0.291 * s1 + 0.439 * s0 + 128;//Cb
	s5 = 0.439 * s2 - 0.368 * s1 - 0.071 * s0 + 128;//Cr
	array[3] = (s3 > s4); //Y>Cb
	array[4] = (s5 > s4); //Cr>Cb
	array[5] = (abs(s2 - s3) > 35);//|R-Y|>35
	array[6] = (s3 > mean(s3)[0]);//Y>Ym
	array[7] = (s5 > mean(s5)[0]);//Cr>Crm
	array[8] = (s4 < mean(s4)[0]);//Cb<Cbm
	array[9] = (abs(s4 - s5) > 35);//|CB-Cr|>35

	//compute mask of RGB
	array[0] = array[1] >= array[0]; //mask of G>B
	array[1] = array[2] >= array[1]; //mask of R>G
	array[2] = (array[2] >= avgofR) & array[1] & array[0] & array[3] & array[4] & array[5] & array[6] & array[7] & array[8] & array[9];  //mask of all
	//dilate and erode to make smooth conters
	erode(array[2], array[2], kernal, Point(-1, -1), 1); //first using erode clear error
	dilate(array[2], array[2], kernal, Point(-1, -1), 2);//second using twice dilate to make full of conters
	erode(array[2], array[2], kernal, Point(-1, -1), 1); //third using erode make conters smooth

	bin = array[2];
	binary = array[2];
	return 0;
}
int Detect::CheckColor2(Mat& src, Mat& bin)
{
	//参数 1，输入图像；2，输出图像
	Mat dst,dst_hsv;
	dst.release();
	dst = src.clone();
	//mat to compute HSI
	Mat s0, s1, s2, s3, s4, s5;
	double avgofR = 0;
	//(3x3)kernal to dilate or erode
	Mat kernal = Mat::ones(Size(3, 3), CV_8UC1);

	//Mat array to store R-G-B three channel
	Mat array[10];

	//split 3 channel
	split(dst, array);

	//compute mean of R channel
	avgofR = mean(array[2])[0];

	//compute S and I in HSI and compute mask of both
	array[3] = min(array[0], min(array[1], array[2]));
	array[0].convertTo(s0, CV_32FC1);
	array[1].convertTo(s1, CV_32FC1);
	array[2].convertTo(s2, CV_32FC1);
	array[3].convertTo(s3, CV_32FC1);
	s4 = (s0 + s1 + s2 ) / 3;//I
	s3 = 1 - 3 * s3 / (s0 + s1 + s2); //S


	array[4] = (s4 >= 80) & (s4 <= 220);

	cvtColor(dst, dst_hsv, COLOR_BGR2HSV);

	Mat array2[5];
	split(dst_hsv, array2);

	array2[0] = (array2[0] <= 150) & (array2[0] >= 100); //H
	array2[1] = (array2[1] <= 150) & (array2[0] >= 80); //S
	array2[2] = (array2[2] <= 220) & (array2[2] >= 150); //V

	array[6] = abs(array[0] - array[1]);
	array[6] = (array[6] < 20);
	array[6] = (array[6] > 15);
	array[7] = abs(array[1] - array[2]);
	array[7] = (array[7] < 20);
	array[7] = (array[7] > 15);
	array[8] = abs(array[0] - array[2]);
	array[8] = (array[8] < 20);
	array[8] = (array[8] > 15);

	array[9] = array[6] & array[7] & array[8] & array2[0] & array2[2] & array2[1] & array[4];
	//dilate and erode to make smooth conters
	erode(array[9], array[9], kernal, Point(-1, -1), 1); //first using erode clear error
	dilate(array[9], array[9], kernal, Point(-1, -1), 3);//second using twice dilate to make full of conters
	erode(array[9], array[9], kernal, Point(-1, -1), 1); //third using erode make conters smooth

	bin = array[9];
	binary = array[9];
	return 0;
}
int Detect::Regiongrow(Mat& src, Mat& dst, Point& Regionstart)
{
	//参数  1、待生长输入图像；2、生长完成输出图像；3、初始生长点;
	Point Regiongrowing, Regionup;//待生长位置
	int curgray = 0;//当前灰度值
	int srcgray = 0;//初始灰度值
	int growlabel = 0;//标记是否生长过
	int neighbor[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 } };
	dst = Mat::zeros(src.size(), CV_8UC1);
	stack<Point> Pointstack;//生长点堆栈
	dst.at<uchar>(Regionstart.y, Regionstart.x) = 255;//标记生长位置
	Pointstack.push(Regionstart);

	srcgray = src.at<uchar>(Regionstart.y, Regionstart.x);

	while (!Pointstack.empty())
	{
		Regionup = Pointstack.top();
		Pointstack.pop();

		for (int i = 0; i < 8; i++)
		{
			Regiongrowing.x = Regionup.x + neighbor[i][0];
			Regiongrowing.y = Regionup.y + neighbor[i][1];

			if (Regiongrowing.x<0 || Regiongrowing.x>(src.cols - 1) || Regiongrowing.y<0 || Regiongrowing.y>(src.rows - 1))
				continue;

			growlabel = dst.at<uchar>(Regiongrowing.y, Regiongrowing.x);

			if (growlabel == 0)
			{
				curgray = src.at<uchar>(Regiongrowing.y, Regiongrowing.x);
				if (abs(curgray - srcgray) < 15)
				{
					dst.at<uchar>(Regiongrowing.y, Regiongrowing.x) = 255;
					Pointstack.push(Regiongrowing);
				}
			}
		}
	}
	return 0;
}
double Detect::Contour(Mat& bin)
{
	Point startpoint;
	bool bfindstart = false;
	Mat contour = Mat::zeros(bin.size(), CV_8UC1);
	int neighbor[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 } };
	int direction = 0;
	for (int i = 0; i < bin.rows; i++)
	{
		for (int j = 0; j < bin.cols; j++)
		{
			if (bin.at<uchar>(i, j) == 255)
			{
				startpoint = Point(j, i);
				bfindstart = true;
			}
		}
	}

	Point currentpoint = startpoint;
	bool bfindbord;
	bfindstart = false;
	vector<Point> stack;
	stack.push_back(startpoint);
	while (!bfindstart)
	{
		bfindbord = false;
		while (!bfindbord)
		{
			bool flag=((currentpoint.y + neighbor[direction][1]) != bin.rows)&&((currentpoint.y + neighbor[direction][1]) >= 0)&&
			((currentpoint.x + neighbor[direction][0]) != bin.cols)&&((currentpoint.x + neighbor[direction][0]) >= 0);
			if (flag&&(bin.at<uchar>(currentpoint.y+neighbor[direction][1], currentpoint.x + neighbor[direction][0]) == 255))
			{
				bfindbord = true;
				currentpoint.y += neighbor[direction][1];
				currentpoint.x += neighbor[direction][0];
				stack.push_back(currentpoint);
				if (currentpoint == startpoint) 
				{ 
					stack.pop_back(); 
					bfindstart = true; 
				}
				direction -= 2;
				if (direction < 0)
				{
					direction += 8;
				}
			}
			else
			{
				direction++;
				direction %= 8;
			}
		}
	}
	Point dispoint = stack[0];
	double arc,dis;
	dis = 0;
	for (vector<Point>::iterator it = stack.begin(); it < stack.end(); it++)
	{
		contour.at<uchar>(it->y, it->x) = 255;
		arc = pow((dispoint.y - it->y), 2) + pow((dispoint.x - it->x), 2);
		arc = sqrt(arc);
		dis += arc;
		dispoint.y = it->y;
		dispoint.x = it->x;
	}
	return dis;
}
int Detect::BwLabel(Mat& src, vector<Feather>& featherList)
{
	//参数 输入图像 输出图像 检测区域容器
	int labelValue = 0;
	Point seed, neighbor;
	stack<Point> pointStack;    // 堆栈

	int area = 0;               // 用于计算连通域的面积
	Point centroid,rectcenter;			//用于计算连通域的质心和外接矩形的中心点
	int leftBoundary = 0;       // 连通域的左边界，即外接最小矩形的左边框，横坐标值，依此类推
	int rightBoundary = 0;
	int topBoundary = 0;
	int bottomBoundary = 0;
	Rect box;                   // 外接矩形框
	Feather feather;
	Mat dst;

	featherList.clear();    // 清除数组

	dst.release();
	dst = src.clone();

	int rows = dst.rows;
	int cols = dst.cols;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (dst.at<uchar>(i, j) == 255)
			{
				area = 0;				//连通域面积初始化
				labelValue++;           // labelValue最大为254，最小为1.
				seed = Point(j, i);     // Point（横坐标，纵坐标）
				centroid = seed;		//种子点设为初始质心
				dst.at<uchar>(seed) = labelValue;	//标记种子点
				pointStack.push(seed);	//种子点入栈

				area++;
				leftBoundary = seed.x;
				rightBoundary = seed.x;
				topBoundary = seed.y;
				bottomBoundary = seed.y;

				while (!pointStack.empty())
				{
					neighbor = Point(seed.x + 1, seed.y);
					if ((seed.x != (cols - 1)) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (rightBoundary < neighbor.x)
							rightBoundary = neighbor.x;
					}

					neighbor = Point(seed.x, seed.y + 1);
					if ((seed.y != (rows - 1)) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (bottomBoundary < neighbor.y)
							bottomBoundary = neighbor.y;

					}

					neighbor = Point(seed.x - 1, seed.y);
					if ((seed.x != 0) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (leftBoundary > neighbor.x)
							leftBoundary = neighbor.x;
					}

					neighbor = Point(seed.x, seed.y - 1);
					if ((seed.y != 0) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (topBoundary > neighbor.y)
							topBoundary = neighbor.y;
					}
					neighbor = Point(seed.x + 1, seed.y + 1);
					if ((seed.x != (cols - 1)) && (seed.y != (rows - 1)) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (rightBoundary < neighbor.x)
							rightBoundary = neighbor.x;
						if (bottomBoundary < neighbor.y)
							bottomBoundary = neighbor.y;
					}
					neighbor = Point(seed.x + 1, seed.y - 1);
					if ((seed.x != (cols - 1)) && (seed.y != 0) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (rightBoundary < neighbor.x)
							rightBoundary = neighbor.x;
						if (topBoundary > neighbor.y)
							topBoundary = neighbor.y;
					}
					neighbor = Point(seed.x - 1, seed.y - 1);
					if ((seed.x != 0) && (seed.y != 0) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (leftBoundary > neighbor.x)
							leftBoundary = neighbor.x;
						if (topBoundary > neighbor.y)
							topBoundary = neighbor.y;
					}
					neighbor = Point(seed.x - 1, seed.y + 1);
					if ((seed.x != 0) && (seed.y != (rows - 1)) && (dst.at<uchar>(neighbor) == 255))
					{
						dst.at<uchar>(neighbor) = labelValue;
						pointStack.push(neighbor);

						centroid.x += neighbor.x;
						centroid.y += neighbor.y;
						area++;
						if (leftBoundary > neighbor.x)
							leftBoundary = neighbor.x;
						if (bottomBoundary < neighbor.y)
							bottomBoundary = neighbor.y;
					}

					seed = pointStack.top();
					pointStack.pop();
				}

				//int 转换为String
				std::string s = std::to_string(labelValue);

				centroid.x = centroid.x / area;
				centroid.y = centroid.y / area;

				box = Rect(leftBoundary, topBoundary, rightBoundary - leftBoundary, bottomBoundary - topBoundary);

				rectcenter.x = (leftBoundary + rightBoundary) / 2;
				rectcenter.y = (topBoundary + bottomBoundary) / 2;
				feather.area = area;
				feather.boundingbox = box;
				feather.label = labelValue;
				feather.num = s;
				feather.centroid = centroid;
				feather.rectcenter = rectcenter;
				featherList.push_back(feather);
			}
		}
	}
	return 0;
}
int Detect::Cenprocess(Mat& src,vector<Feather>& featherList, Point& centroid)
{
	//参数 检测区域容器 输出点
	float area = 0, weight = 0, tempweight = 0;
	Point maxcentroid, maxrectcenter;
	centroid = Point(0,0);
	if (featherList.size() != 0)
	{
		for (vector<Feather>::iterator it = featherList.begin(); it < featherList.end(); it++)
		{
			if (it->area > area)
			{
				area = it->area;
			}
		}
		for (vector<Feather>::iterator it = featherList.begin(); it < featherList.end(); it++)
		{
			tempweight = it->area / area;
			if (tempweight == 1) 
			{ 
				tempweight = 0.9; 
				maxcentroid = it->centroid; 
				maxrectcenter = it->rectcenter;
			}
			else 
			{ 
				tempweight = 0.1; 
			}
			Point cenpro = tempweight * it->centroid;
			weight += tempweight;
			centroid.x += cenpro.x;
			centroid.y += cenpro.y;
		}
		centroid.x = centroid.x / weight;
		centroid.y = centroid.y / weight;
	}
	if (src.at<uchar>(centroid.y, centroid.x) == 255)
	{
		centroid = centroid;
	}
	else
	{
		centroid = maxcentroid;
	}
	return 0;
}
Mat Detect::Diffframe(Mat& src, Mat& dst)
{
	Mat src_gray;
	cvtColor(src, src_gray,COLOR_BGR2GRAY);
	GaussianBlur(src_gray, src_gray, Size(3, 3), 0);
	Mat dst_gray;
	cvtColor(dst, dst_gray, COLOR_BGR2GRAY);
	GaussianBlur(dst_gray, dst_gray, Size(3, 3), 0);
	Mat outframe;
	absdiff(src_gray, dst_gray, outframe);
	threshold(outframe, outframe, 10, 255, THRESH_BINARY);
	return outframe;
}
void Detect::Select(Mat& src,Mat& src2)
{
	Mat bin, bin2, frame, frame2, corp, corp_pre, corp3, mask;
	frame = src.clone();
	frame2 = src2.clone();
	int h, w, flag, rectarea;
	double arclen, area, epsilon, ration, roundness, rectangularity;
	vector<vector<Point>> contours;
	CheckColor(frame, bin);
	CheckColor(frame2, bin2);
	findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	vector<vector<Point>> contours_ploy(contours.size());
	vector<RotatedRect> rect(contours.size());
	vector<Rect> rect2(contours.size());
	vector<Rect> rect3;
	vector<Rect> rect4;
	Rect finrect;
	for (int i = 0; i < contours.size(); i++)
	{
		arclen = arcLength(contours[i], true);
		area = contourArea(contours[i]);
		roundness = (4 * CV_PI * area) / (arclen * arclen);
		epsilon = max(3, int(arclen * 0.015));
		approxPolyDP(contours[i], contours_ploy[i], epsilon, false);
		rect[i] = minAreaRect(contours[i]);
		rectarea = rect[i].size.height * rect[i].size.width;
		if (rectarea == 0) { rectangularity = 0; }
		else { rectangularity = area / rectarea; }
		rect2[i] = boundingRect(contours[i]);
		h = int(rect[i].size.height);
		w = int(rect[i].size.width);
		if (min(h, w) == 0)
		{
			ration = 0;
		}
		else
		{
			ration = max(h, w) / min(h, w);
		}
		if ((ration < 5) && (area > 20) && (contours_ploy[i].size() > 5) && rectangularity < (0.8) && roundness < (0.8))
		{
			rect3.push_back(rect2[i]);
		}

	}
	if (rect3.size() != 0)
	{
		int x_tempmax, x_tempmin;
		x_tempmax = rect3[0].width + rect3[0].x;
		x_tempmin = rect3[0].x;
		for (int i = 0; i < rect3.size(); i++)
		{
			x_tempmax = max((rect3[i].width + rect3[i].x), x_tempmax);
			x_tempmin = min(rect3[i].x, x_tempmin);
		}
		int y_tempmax, y_tempmin;
		y_tempmax = rect3[0].height + rect3[0].y;
		y_tempmin = rect3[0].y;
		for (int i = 0; i < rect3.size(); i++)
		{
			y_tempmax = max((rect3[i].height + rect3[i].y), y_tempmax);
			y_tempmin = min(rect3[i].y, y_tempmin);
		}
		int x_min, x_width, y_min, y_height;
		x_min = max(x_tempmin, 0);
		x_width = min(x_tempmax, src.cols);
		y_min = max(y_tempmin, 0);
		y_height = min(y_tempmax, src.rows);
		corp = bin(Rect(x_min, y_min, x_width - x_min, y_height - y_min));
		corp_pre = bin2(Rect(x_min, y_min, x_width - x_min, y_height - y_min));
		finrect = Rect(x_min, y_min, x_width - x_min, y_height - y_min);
		bool flag = Dynamic(corp, corp_pre);
		if (flag)
		{
			mask = Mat::zeros(bin.size(), CV_8UC1);
			mask(Rect(x_min, y_min, x_width - x_min, y_height - y_min)).setTo(255);
			bin.copyTo(corp3, mask);
			cv::rectangle(frame, Rect(x_min, y_min, x_width - x_min, y_height - y_min), Scalar(0, 255, 255), 2);
			cv::putText(frame, "fire", Point(x_min, y_min), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 1);

		}
		else
		{
			corp3 = Mat::zeros(src.size(), CV_8UC1);
		}
	}
	else
	{
		corp3 = Mat::zeros(src.size(), CV_8UC1);
	}
	fireframe = frame;
	fire_bin = corp3;
}
void Detect::Select(Mat& src)
{
	Mat bin, frame, corp, mask;
	frame = src.clone();
	int h, w, flag, rectarea;
	double arclen, area, epsilon, ration, roundness, rectangularity;
	vector<vector<Point>> contours;
	CheckColor(frame, bin);
	findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	vector<vector<Point>> contours_ploy(contours.size());
	vector<RotatedRect> rect(contours.size());
	vector<Rect> rect2(contours.size());
	vector<Rect> rect3;
	for (int i = 0; i < contours.size(); i++)
	{
		arclen = arcLength(contours[i], true);
		area = contourArea(contours[i]);
		roundness = (4 * CV_PI * area) / (arclen * arclen);
		epsilon = max(3, int(arclen * 0.015));
		approxPolyDP(contours[i], contours_ploy[i], epsilon, false);
		rect[i] = minAreaRect(contours[i]);
		rectarea = rect[i].size.height * rect[i].size.width;
		if (rectarea == 0) { rectangularity = 0; }
		else { rectangularity = area / rectarea; }
		rect2[i] = boundingRect(contours[i]);
		h = int(rect[i].size.height);
		w = int(rect[i].size.width);
		if (min(h, w) == 0)
		{
			ration = 0;
		}
		else
		{
			ration = max(h, w) / min(h, w);
		}
		if ((ration < 5) && (area > 20) && (contours_ploy[i].size() > 5) && rectangularity < (0.8) && roundness < (0.8))
		{
			rect3.push_back(rect2[i]);
		}

	}
	if (rect3.size() != 0)
	{
		int x_tempmax, x_tempmin;
		x_tempmax = rect3[0].width + rect3[0].x;
		x_tempmin = rect3[0].x;
		for (int i = 0; i < rect3.size(); i++)
		{
			x_tempmax = max((rect3[i].width + rect3[i].x), x_tempmax);
			x_tempmin = min(rect3[i].x, x_tempmin);
		}
		int y_tempmax, y_tempmin;
		y_tempmax = rect3[0].height + rect3[0].y;
		y_tempmin = rect3[0].y;
		for (int i = 0; i < rect3.size(); i++)
		{
			y_tempmax = max((rect3[i].height + rect3[i].y), y_tempmax);
			y_tempmin = min(rect3[i].y, y_tempmin);
		}
		int x_min, x_width, y_min, y_height;
		x_min = max(x_tempmin, 0);
		x_width = min(x_tempmax, src.cols);
		y_min = max(y_tempmin, 0);
		y_height = min(y_tempmax, src.rows);
		mask = Mat::zeros(bin.size(), CV_8UC1);
		mask(Rect(x_min, y_min, x_width - x_min, y_height - y_min)).setTo(255);
		bin.copyTo(corp, mask);
		cv::rectangle(frame, Rect(x_min, y_min, x_width - x_min, y_height - y_min), Scalar(0, 255, 255), 2);
		cv::putText(frame, "fire", Point(x_min, y_min), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 1);
		
	}
	else
	{
		corp = Mat::zeros(src.size(), CV_8UC1);
	}
	fireframe = frame;
	fire_bin = corp;
}
void Detect::Select2(Mat& src, Mat& src2)
{
	Mat bin, bin2, frame, frame2, corp, corp_pre, corp3, mask;
	frame = src.clone();
	frame2 = src2.clone();
	int h, w, flag, rectarea;
	double arclen, area, epsilon, ration, roundness, rectangularity;
	vector<vector<Point>> contours;
	CheckColor2(frame, bin);
	CheckColor2(frame2, bin2);
	findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	vector<vector<Point>> contours_ploy(contours.size());
	vector<RotatedRect> rect(contours.size());
	vector<Rect> rect2(contours.size());
	vector<Rect> rect3;
	vector<Rect> rect4;
	Rect finrect;
	for (int i = 0; i < contours.size(); i++)
	{
		arclen = arcLength(contours[i], true);
		area = contourArea(contours[i]);
		roundness = (4 * CV_PI * area) / (arclen * arclen);
		epsilon = max(3, int(arclen * 0.015));
		approxPolyDP(contours[i], contours_ploy[i], epsilon, false);
		rect[i] = minAreaRect(contours[i]);
		rectarea = rect[i].size.height * rect[i].size.width;
		if (rectarea == 0) { rectangularity = 0; }
		else { rectangularity = area / rectarea; }
		rect2[i] = boundingRect(contours[i]);
		h = int(rect[i].size.height);
		w = int(rect[i].size.width);
		if (min(h, w) == 0)
		{
			ration = 0;
		}
		else
		{
			ration = max(h, w) / min(h, w);
		}
		if ((ration < 5) && (area > 20) && (contours_ploy[i].size() > 5) && rectangularity < (0.8) && roundness < (0.8))
		{
			rect3.push_back(rect2[i]);
		}

	}
	if (rect3.size() != 0)
	{
		int x_tempmax, x_tempmin;
		x_tempmax = rect3[0].width + rect3[0].x;
		x_tempmin = rect3[0].x;
		for (int i = 0; i < rect3.size(); i++)
		{
			x_tempmax = max((rect3[i].width + rect3[i].x), x_tempmax);
			x_tempmin = min(rect3[i].x, x_tempmin);
		}
		int y_tempmax, y_tempmin;
		y_tempmax = rect3[0].height + rect3[0].y;
		y_tempmin = rect3[0].y;
		for (int i = 0; i < rect3.size(); i++)
		{
			y_tempmax = max((rect3[i].height + rect3[i].y), y_tempmax);
			y_tempmin = min(rect3[i].y, y_tempmin);
		}
		int x_min, x_width, y_min, y_height;
		x_min = max(x_tempmin, 0);
		x_width = min(x_tempmax, src.cols);
		y_min = max(y_tempmin, 0);
		y_height = min(y_tempmax, src.rows);
		corp = bin(Rect(x_min, y_min, x_width - x_min, y_height - y_min));
		corp_pre = bin2(Rect(x_min, y_min, x_width - x_min, y_height - y_min));
		finrect = Rect(x_min, y_min, x_width - x_min, y_height - y_min);
		bool flag = Dynamic(corp, corp_pre);
		if (flag)
		{
			mask = Mat::zeros(bin.size(), CV_8UC1);
			mask(Rect(x_min, y_min, x_width - x_min, y_height - y_min)).setTo(255);
			bin.copyTo(corp3, mask);
			cv::rectangle(frame, Rect(x_min, y_min, x_width - x_min, y_height - y_min), Scalar(255, 255, 255), 2);
			cv::putText(frame, "smoke", Point(x_min, y_min), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);
		}
		else
		{
			corp3 = Mat::zeros(src.size(), CV_8UC1);
		}
	}
	else
	{
		corp3 = Mat::zeros(src.size(), CV_8UC1);
	}
	smokeframe = frame;
	smoke_bin = corp3;
}
void Detect::Select2(Mat& src)
{
	Mat bin, frame, corp, mask;
	frame = src.clone();
	int h, w, flag, rectarea;
	double arclen, area, epsilon, ration, roundness, rectangularity;
	vector<vector<Point>> contours;
	CheckColor2(frame, bin);
	findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	vector<vector<Point>> contours_ploy(contours.size());
	vector<RotatedRect> rect(contours.size());
	vector<Rect> rect2(contours.size());
	vector<Rect> rect3;
	vector<Rect> rect4;
	Rect finrect;
	for (int i = 0; i < contours.size(); i++)
	{
		arclen = arcLength(contours[i], true);
		area = contourArea(contours[i]);
		roundness = (4 * CV_PI * area) / (arclen * arclen);
		epsilon = max(3, int(arclen * 0.015));
		approxPolyDP(contours[i], contours_ploy[i], epsilon, false);
		rect[i] = minAreaRect(contours[i]);
		rectarea = rect[i].size.height * rect[i].size.width;
		if (rectarea == 0) { rectangularity = 0; }
		else { rectangularity = area / rectarea; }
		rect2[i] = boundingRect(contours[i]);
		h = int(rect[i].size.height);
		w = int(rect[i].size.width);
		if (min(h, w) == 0)
		{
			ration = 0;
		}
		else
		{
			ration = max(h, w) / min(h, w);
		}
		if ((ration < 5) && (area > 20) && (contours_ploy[i].size() > 5) && rectangularity < (0.8) && roundness < (0.8))
		{
			rect3.push_back(rect2[i]);
		}

	}
	if (rect3.size() != 0)
	{
		int x_tempmax, x_tempmin;
		x_tempmax = rect3[0].width + rect3[0].x;
		x_tempmin = rect3[0].x;
		for (int i = 0; i < rect3.size(); i++)
		{
			x_tempmax = max((rect3[i].width + rect3[i].x), x_tempmax);
			x_tempmin = min(rect3[i].x, x_tempmin);
		}
		int y_tempmax, y_tempmin;
		y_tempmax = rect3[0].height + rect3[0].y;
		y_tempmin = rect3[0].y;
		for (int i = 0; i < rect3.size(); i++)
		{
			y_tempmax = max((rect3[i].height + rect3[i].y), y_tempmax);
			y_tempmin = min(rect3[i].y, y_tempmin);
		}
		int x_min, x_width, y_min, y_height;
		x_min = max(x_tempmin, 0);
		x_width = min(x_tempmax, src.cols);
		y_min = max(y_tempmin, 0);
		y_height = min(y_tempmax, src.rows);
		mask = Mat::zeros(bin.size(), CV_8UC1);
		mask(Rect(x_min, y_min, x_width - x_min, y_height - y_min)).setTo(255);
		bin.copyTo(corp, mask);
		cv::rectangle(frame, Rect(x_min, y_min, x_width - x_min, y_height - y_min), Scalar(255, 255, 255), 2);
		cv::putText(frame, "smoke", Point(x_min, y_min), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);
		
	}
	else
	{
		corp = Mat::zeros(src.size(), CV_8UC1);
	}
	smokeframe = frame;
	smoke_bin = corp;
}
bool Detect::Dynamic(Mat& src,Mat& src2)
{
	bool flag = false;
	Point centroid, centroid2;
	centroid = centroid2 = Point(0, 0);
	int srcarea = 0;
	int src2area = 0;
	double arc1 = 0, area1 = 0, arc2 = 0, area2 = 0, andarea = 0, orarea = 0;
	Mat andtemp, ortemp;
	vector<vector<Point>> contours1;
	vector<vector<Point>> contours2;
	findContours(src, contours1, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	findContours(src2, contours2, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	for (int i = 0; i < contours1.size(); i++)
	{
		arc1 += arcLength(contours1[i],true);
		area1 += contourArea(contours1[i]);
	}
	for (int i = 0; i < contours2.size(); i++)
	{
		arc2 += arcLength(contours2[i], true);
		area2 += contourArea(contours2[i]);
	}
	contours1.clear();
	contours1.clear();
	//前景面积
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 255)
			{
				srcarea++;
				centroid.x += i;
				centroid.y += j;
			}
		}
	}
	//后一帧面积
	for (int i = 0; i < src2.rows; i++)
	{
		for (int j = 0; j < src2.cols; j++)
		{
			if (src2.at<uchar>(i, j) == 255)
			{
				src2area++;
				centroid2.x += i;
				centroid2.y += j;
			}
		}
	}
	if(srcarea!=0&&src2area!=0)
	{
		centroid.x /= srcarea;
		centroid2.x /= src2area;
		centroid.y /= srcarea;
		centroid2.y /= src2area;
	}
	double cendist = sqrt(pow((centroid.x - centroid2.x), 2) + pow((centroid.y - centroid2.y), 2));
	bitwise_and(src,src2,andtemp);
	bitwise_or(src,src2,ortemp);
	//交集面积
	for (int i = 0; i < andtemp.rows; i++)
	{
		for (int j = 0; j < andtemp.cols; j++)
		{
			if (andtemp.at<uchar>(i, j) == 255) { andarea++; }
		}
	}
	//并集面积
	for (int i = 0; i < ortemp.rows; i++)
	{
		{for (int j = 0; j < ortemp.cols; j++)
			if (ortemp.at<uchar>(i, j) == 255) { orarea++; }
		}
	}
	
	if (((((abs(area1 - area2) / area1)) > 0) && ((abs(arc1 - arc2) / arc1) > 0)) && ((andarea / orarea) > 0.5) && (cendist > 2))
	{ 
		flag = true; 
	}
	return flag;
}
void Detect::F_Detect(Mat& src, Mat& src2)
{
	GaussianBlur(src2, src2, Size(5, 5), 0, 0);
	GaussianBlur(src2, src2, Size(5, 5), 0, 0);
	orginal = src;
	Select(src, src2);
	Select2(src, src2);
	bitwise_or(fireframe, smokeframe, finframe);
	bitwise_or(fire_bin, smoke_bin, fin_bin);
	cvtColor(finframe, finframe, COLOR_BGR2RGB);
	cvtColor(orginal, orginal, COLOR_BGR2RGB);
}
void Detect::F_Detect(Mat& src)
{
	GaussianBlur(src, src, Size(5, 5), 0, 0);
	orginal = src;
	Select(src);
	Select2(src);
	bitwise_or(fireframe, smokeframe, finframe);
	bitwise_or(fire_bin, smoke_bin, fin_bin);
	cvtColor(finframe, finframe, COLOR_BGR2RGB);
	cvtColor(orginal, orginal, COLOR_BGR2RGB);
}