#pragma once

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define CHESSBOARD 0
#define CIRCLEGRID 1

// ��ȡ�궨���������������꣬������������������
bool findFeaturePoints( const vector<Mat>& calibImages,
					    vector<vector<Point3f>>& objPointsSeq,
					    vector<vector<Point2f>>& camPointsSeq,
						Size& imageSize,
						const Size& boardSize,
						const Size& squareSize,
						int blobFilter = 0,
					    int calibPlate = CIRCLEGRID);

// ����궨
bool camCalibration( const vector<vector<Point3f>>& objPointsSeq,
					 const vector<vector<Point2f>>& camPointsSeq,
					 const Size& imageSize,
					 Mat& camMatrix,
					 Mat& distCoeffs,
					 vector<Mat>& vecsR,
					 vector<Mat>& vecsT,
					 int calibType = 0);

// ����������ӳ�䣺���cam-->ͶӰ��proj
bool featCoordinateMap( const vector<Mat>& horizontalPhases,
					    const vector<Mat>& verticalPhases,
						const Size& projResolution,
						const vector<vector<Point2f>>& camPointsSeq,
						vector<vector<Point2f>>& projPointsSeq,
						int frq = 70);

// ͶӰ��������궨
bool projCalibration( const vector<vector<Point3f>>& objPointsSeq,
					  const vector<vector<Point2f>>& projPointsSeq,
					  const Size& imageSize,
					  Mat& projMatrix,
					  Mat& distCoeffs,
					  vector<Mat>& vecsR,
					  vector<Mat>& vecsT);

// �����ͶӰ������˫Ŀ�궨
bool steroCalibration(  const vector<vector<Point3f>>& objPointsSeq,
						const vector<vector<Point2f>>& camPointsSeq,
						const vector<vector<Point2f>>& projPointsSeq,
						Mat& camMatrix, Mat& camCoeffs, 
						Mat& projMatrix, Mat& projCoeffs, 
						const Size& imageSize,
						Mat& vecR,
						Mat& vecT,
						Mat& vecE,
						Mat& vecF);

// ˫Ŀ�������У�����������߽��������Ƕ�άͼ��У��
bool undistortImg(const Mat &srcImage,
	Mat &dstImage,
	const Mat &camMatrix,
	const Mat &camCoeffs,
	const Mat &projMatrix,
	const Mat &projCoeffs,
	const Mat &vecR,
	const Mat &vecT);

// ��ά�ؽ������ؾ�����λ-->�����������ϵ
bool reconstruction( const Mat& absPhase,
					 const Mat& camMatrix,
					 const Mat& projMatrix, 
					 const Mat& vecR,
					 const Mat& vecT, 
					 const Size& projResolution,
					 Mat& dstImage,
				     int frq = 70,
					 int projMode = 0);

// �������ϱ궨����
bool writeZhangCalibFiles(	const string& fileName, 
							const double& reprojErr,
							const Mat& intrinsicMatrix,
							const Mat& distCoeffs,
							const vector<Mat>& vecsR,
							const vector<Mat>& vecsT);

// ����˫Ŀ����궨����
bool writeStereoCalibFiles( const string& fileName, 
							const double& reprojErr, 
							const Mat& camMatrix, 
							const Mat& camCoeffs, 
							const Mat& projMatrix,  
							const Mat& projCoeffs, 
							const Mat& vecR, 
							const Mat& vecT);

//˫���Բ�ֵ
float biInterpolate(const Mat& srcImg, float i, float j);

// ����ϵͳ�궨���ò�������ת��
bool vecsMat2vecsFloat(const vector<Mat>& sysParameters,
					   vector<float>& sysFloatParams);

//ϵͳ�궨�ܺ���
bool SongZhangCalibration(  const vector<Mat>& camCalibImg, 
							const vector<Rect>& imgRectVec,
							const vector<vector<Mat>>& projCalibImg, 
							vector<float>& sysParams);