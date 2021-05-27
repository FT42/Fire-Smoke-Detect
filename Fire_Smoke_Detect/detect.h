#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stack>

using namespace cv;
using namespace std;

class Detect
{
public:
	Detect();
	~Detect();
	struct Feather
	{
		int area;
		Rect boundingbox;
		string num;
		int label;
		Point centroid;
		Point rectcenter;
	};
	int CheckColor(Mat& src, Mat& bin);
	int CheckColor2(Mat& src, Mat& bin);
	int Regiongrow(Mat& src, Mat& dst, Point& Regionstart);
	double Contour(Mat& bin);
	int BwLabel(Mat& src, vector<Feather>& featherList);
	int Cenprocess(Mat& src, vector<Feather>& featherList, Point& centroid);
	Mat Diffframe(Mat& src, Mat& dst);
	void Select(Mat& src, Mat& src2);
	void Select(Mat& src);
	void Select2(Mat& src, Mat& src2);
	void Select2(Mat& src);
	bool Dynamic(Mat& src, Mat& src2);
	void F_Detect(Mat& src, Mat& src2);
	void F_Detect(Mat& src);
	Mat orginal;
	Mat binary;
	Mat fire_bin;
	Mat smoke_bin;
	Mat fin_bin;
	Mat finalfr;
	Mat fireframe;
	Mat smokeframe;
	Mat finframe;
	vector<Feather> featherList;
	Point center;
private:

};
