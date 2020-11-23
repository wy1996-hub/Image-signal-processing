#include <iostream>
#include "FTP.h"

void FTPprocess()
{
	Mat img = imread("test.jpg", IMREAD_GRAYSCALE);
	if (img.empty())
	{
		cout << "load image failed!" << endl;
		return ;
	}
	imshow("img", img);
	int height = getOptimalDFTSize(img.rows);
	int width = getOptimalDFTSize(img.cols);//����ʺ�DFT��ͼ��ߴ� ,ѡȡ���ʺ���fft�Ŀ�͸�
	Mat padded;
	copyMakeBorder(img, padded, 0, height - img.rows, 0, width - img.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(),CV_32F) };
	Mat complexImg;
	merge(planes, 2, complexImg);//planes[0], planes[1]��ʵ�����鲿
	cv::dft(complexImg, complexImg, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);//��ɢ����Ҷ�任
	split(complexImg, planes);//��ú���ʵ�����鲿ͨ����vector<Mat>	

	cv::Mat ph, mag;//��������׺���λ��
	cv::phase(planes[0], planes[1], ph);
	cv::magnitude(planes[0], planes[1], mag);
	//��ʵ��planes[0]���鲿planes[1]�õ�������mag����λ��ph
	//�����Ҫ��ʵ��planes[0]���鲿planes[1]�����߷�����mag����λ��ph���в�������������и���
	mag += Scalar::all(1);
	cv::log(mag, mag);
	Mat org_mag = mag.clone();//��ʱ����mat��Ϊfloat
	normalize(org_mag, org_mag, 0, 255, NORM_MINMAX);
	imshow("org_mag", org_mag);


	mag = mag(Rect(0, 0, mag.cols&-2, mag.rows&-2));//����ͼ���СΪż��ֵ
	int cx = mag.cols / 2;
	int cy = mag.rows / 2;
	Mat tmp;
	Mat q1(mag, Rect(0, 0, cx, cy));
	Mat q4(mag, Rect(cx, 0, cx, cy));
	Mat q2(mag, Rect(0, cy, cx, cy));
	Mat q3(mag, Rect(cx, cy, cx, cy));
	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);
	q4.copyTo(tmp);
	q2.copyTo(q4);
	tmp.copyTo(q2);
	normalize(mag, mag, 0, 255, NORM_MINMAX);//Ƶ��ͼ����normalize
	imshow("center_mag", mag);

	Mat idft_mag1, idft, idft_mag2;
	cv::idft(complexImg, idft_mag1, DFT_REAL_OUTPUT /*| DFT_SCALE*/);//����Ҷ���任
	 //�����Ļ�֮��ķ�����mag����λ��ph�ָ�ʵ��planes[0]���鲿planes[1]
	cv::polarToCart(mag, ph, planes[0], planes[1]);
	cv::merge(planes, 2, idft);
	cv::dft(idft, idft_mag2, DFT_REAL_OUTPUT |/* DFT_SCALE|*/ DFT_INVERSE);

	idft_mag1.convertTo(idft_mag1, CV_8UC1);
	idft_mag2.convertTo(idft_mag2, CV_8UC1);//��ͨ���Ŀ���ͼƬ��ҪconvertTo������normalize
	imshow("idft_mag1", idft_mag1);
	imshow("idft_mag2", idft_mag2);
}

int main(int argc, char* argv[])
{	
	Mat img = imread("test.jpg", IMREAD_GRAYSCALE);
	if (img.empty())
	{
		cout << "load image failed!" << endl;
		return -1;
	}

	FTP t(img);
	//t.FTPprocess();

	Mat p = Mat::ones(9, 9, CV_8UC1);
	Mat p2, img2, result;
	p.convertTo(p2, CV_32FC1, 1.0 / 81);
	img.convertTo(img2, CV_32FC1, 1.0 / 255);
	result = t.DFTconvolve(img2, p2);
	imshow("result", result);

	cv::waitKey();
	system("pause");
	return 0;
}
