#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;
using namespace cv;

class wavelet
{
public:
	wavelet();
	~wavelet();

/*********************************************000000000*****************************************************/
	int thundershock();


/************************************************1111111*****************************************************/
	void laplace_decompose(Mat& src, int s, Mat &wave);//С���ֽ�
	void wave_recover(Mat full_scale, Mat &original, int level);//С����ԭ
	void ware_operate(Mat &full_scale, int level);//С������


/***********************************************2222222222******************************************************/
	Mat DWT(const Mat &_src, const string _wname, const int _level);// С���任	
	Mat IDWT(const Mat &_src, const string _wname, const int _level);// С����任
	void wavelet_D(const string _wname, Mat &_lowFilter, Mat &_highFilter);// �ֽ��
	void wavelet_R(const string _wname, Mat &_lowFilter, Mat &_highFilter);// �ع���
	Mat waveletDecompose(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter);// С���ֽ�
	Mat waveletReconstruct(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter);// С���ؽ�
};

