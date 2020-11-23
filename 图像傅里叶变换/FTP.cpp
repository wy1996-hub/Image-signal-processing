#include "FTP.h"

FTP::FTP(Mat src):img(src)
{
	if (img.empty())
	{
		cout << "load image failed!" << endl;
		return;
	}
}

void FTP::FTPprocess()
{
	int height = getOptimalDFTSize(img.rows);
	int width = getOptimalDFTSize(img.cols);//����ʺ�DFT��ͼ��ߴ�
	Mat padded;
	copyMakeBorder(img, padded, 0, height - img.rows, 0, width - img.cols, BORDER_CONSTANT, Scalar::all(0));
	vector<Mat> planes = { Mat_<float>(padded),Mat::zeros(padded.size(),CV_32F) };
	Mat complexImg;
	merge(planes, complexImg);//�ϲ���һ��������Ϊdft������

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
	normalize(org_mag, org_mag, 0, 255, NORM_MINMAX);//Ƶ��ͼ����normalize
	imshow("org_mag", org_mag);

	Mat center_mag = FTPCentralize(mag);//���Ļ�
	cv::normalize(center_mag, center_mag, 0, 255, NORM_MINMAX);//float��mat���һ������0,1����0,255��
	imshow("center_mag", center_mag);

	Mat idft_mag;
	cv::idft(complexImg, idft_mag, DFT_REAL_OUTPUT /*| DFT_SCALE*/);//����Ҷ���任
	//��ʱ���ܼ�DFT_SCALE�������޷���ʾ��ȫ�Ǻ�ͼ
	idft_mag = idft_mag(Rect(0, 0, img.cols, img.rows));
	/*normalize(idft_mag, idft_mag, 0, 255, NORM_MINMAX);*/
	idft_mag.convertTo(idft_mag, CV_8UC1);//��ͨ���Ŀ���ͼƬ��ҪconvertTo������normalize
	imshow("idft_mag", idft_mag);
	imwrite("idft_mag.jpg", idft_mag);
}

Mat& FTP::FTPCentralize(Mat& src)//Ƶ�����Ļ�
{
	src = src(Rect(0, 0, src.cols&-2, src.rows&-2));//����ͼ���Сλż��ֵ
	int cx = src.cols / 2;
	int cy = src.rows / 2;
	Mat tmp;
	Mat q1(src, Rect(0, 0, cx, cy));
	Mat q4(src, Rect(cx, 0, cx, cy));
	Mat q2(src, Rect(0, cy, cx, cy));
	Mat q3(src, Rect(cx, cy, cx, cy));

	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);

	q4.copyTo(tmp);
	q2.copyTo(q4);
	tmp.copyTo(q2);	
	return src;
}

/*
	���ܣ�ͨ������Ҷ��任������Ƶ��˻���ʵ�ֿ�����
	����A�������ͣ���0,1��
	����B�������ͣ�ģ�壬��0,1������Ȼ���Ϊȫ��ͼ��
	���C�����������ͼ
*/
Mat FTP::DFTconvolve(Mat A, Mat B)
{
	Mat C;
	C.create(abs(A.rows-B.rows)+1, abs(A.cols - B.cols) + 1, A.type());

	Size dftSize;
	dftSize.width = getOptimalDFTSize(A.cols + B.cols - 1);
	dftSize.height = getOptimalDFTSize(A.rows + B.rows - 1);

	Mat tmpA(dftSize, A.type(), Scalar::all(0)); 
	Mat tmpB(dftSize, B.type(), Scalar::all(0));
	Mat roiA(tmpA, Rect(0, 0, A.cols, A.rows));
	Mat roiB(tmpB, Rect(0, 0, B.cols, B.rows));
	A.copyTo(roiA);
	B.copyTo(roiB);

	dft(tmpA, tmpA, 0, A.rows);
	dft(tmpB, tmpB, 0, B.rows);
	mulSpectrums(tmpA, tmpB, tmpA, 0, false);//Ƶ�����

	idft(tmpA, tmpA, DFT_SCALE, C.rows);

	tmpA(Rect(0, 0, C.cols, C.rows)).copyTo(C);
	return C;
}

FTP::~FTP()
{
	cv::destroyAllWindows();
}
