#include "SongZangCalib.h"
#include <iostream>
#include <fstream>

// ��ȡ�궨���������������꣬������������������
bool findFeaturePoints(const vector<Mat>& calibImages,
	vector<vector<Point3f>>& objPointsSeq,
	vector<vector<Point2f>>& camPointsSeq,
	Size& imageSize,
	const Size& boardSize,
	const Size& squareSize,
	int blobFilter,
	int calibPlate)
{
	assert(calibImages.size() > 0 && calibImages[0].channels() == 1);

	// ��ȡ����������
	SimpleBlobDetector::Params params;
	switch (blobFilter)
	{
	case 0:
	{
		params.filterByArea = true;  params.minArea = 1200; params.maxArea = 22500; break;
	}
	case 1:
	{
		params.filterByCircularity = true; params.minCircularity = static_cast<float>(0.8); break;
	}
	case 2:
	{
		params.filterByColor = true; params.minThreshold = 50; params.maxThreshold = 120; break;
	}
	case 3:
	{
		params.filterByInertia = true; params.minInertiaRatio = static_cast<float>(0.1); break;
	}
	case 4:
	{
		params.filterByConvexity = true; params.minConvexity = static_cast<float>(0.8); break;
	}
	default:
		break;
	}
	Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);

	vector<Point2f> camPoints;
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

	vector<Rect> rect;
	rect.emplace_back(Rect(145, 130, 2822, 2295));//1
	rect.emplace_back(Rect(26, 104, 3038, 2583));//2
	rect.emplace_back(Rect(507, 233, 2891, 2413));//3
	rect.emplace_back(Rect(29, 438, 2809, 2245));//4
	rect.emplace_back(Rect(1020, 466, 2803, 2252));//5
	rect.emplace_back(Rect(923, 57, 2850, 2349));//6
	rect.emplace_back(Rect(573, 290, 2856, 2267));//7
	rect.emplace_back(Rect(240, 114, 2919, 2255));//8
	rect.emplace_back(Rect(702, 45, 2872, 2352));//9
	rect.emplace_back(Rect(155, 265, 2846, 2359));//10
	rect.emplace_back(Rect(743, 278, 2945, 2336));//11
	rect.emplace_back(Rect(186, 183, 2910, 2390));//12
	rect.emplace_back(Rect(879, 202, 2849, 2365));//13
	rect.emplace_back(Rect(621, 340, 2909, 2369));//14

	ofstream fout("./calibtest/camPointsSeq.txt");

	for (size_t i = 0; i < calibImages.size(); i++)
	{
		fout << "��" << i + 1 << "�ű궨ͼ------" << endl;
		Mat image = calibImages[i];
		Mat imageInput = image(rect[i]);
		threshold(imageInput, imageInput, 0, 255, THRESH_OTSU);
		erode(imageInput, imageInput, kernel);
		bitwise_not(imageInput, imageInput);

		if (i == 0)
		{
			imageSize.width = image.cols;
			imageSize.height = image.rows;
		}

		bool ok = findCirclesGrid(imageInput, boardSize, camPoints, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, blobDetector);

		if (false == ok)
		{
			cout << "��" << i + 1 << "����Ƭ��ȡԲ��ʧ�ܣ���ɾ�������±궨��" << endl;
			return false;
		}
		cornerSubPix(imageInput, camPoints, Size(1, 1), Size(-1, -1), TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, DBL_EPSILON));

		int xval = rect[i].tl().x;
		int yval = rect[i].tl().y;
		for (size_t n = 0; n < camPoints.size(); n++)
		{
			camPoints[n].x += xval;
			camPoints[n].y += yval;
		}
		if (camPoints[0].x > camPoints[0].y)
		{
			reverse(camPoints.begin(), camPoints.end());
		}

		camPointsSeq.emplace_back(camPoints);

		///////////////////////////////////////////////////////////////////////
		for (size_t n = 0; n < camPoints.size(); n++)
		{
			fout << n << "," << camPoints[n].x << "," << camPoints[n].y << endl;
		}
		fout << endl;
		////////////////////////////////////////
	}
	cout << "�ǵ���ȡ��ɣ�����" << endl;

	// ��ȡ��������������
	for (size_t n = 0; n < calibImages.size(); n++)
	{
		cout << "��" << n + 1 << "�ű궨�����������------" << endl;
		vector<Point3f> objPoints;
		for (size_t i = 0; i < boardSize.height; i++)// һ�ж��ٵ�
		{
			for (size_t j = 0; j < boardSize.width; j++)// һ�ж��ٵ�
			{
				Point3f realPoint;
				realPoint.y = static_cast<float>(j*squareSize.width);
				realPoint.x = static_cast<float>(i*squareSize.height);
				realPoint.z = static_cast<float>(0);
				objPoints.push_back(realPoint);
			}
		}
		objPointsSeq.emplace_back(objPoints);
	}

	return true;
}

// ����궨
bool camCalibration(const vector<vector<Point3f>>& objPointsSeq,
	const vector<vector<Point2f>>& camPointsSeq,
	const Size& imageSize,
	Mat& camMatrix,
	Mat& distCoeffs,
	vector<Mat>& vecsR,
	vector<Mat>& vecsT,
	int calibType)
{
	assert(objPointsSeq.size() != 0 && camPointsSeq.size() != 0);

	double err = calibrateCamera(objPointsSeq, camPointsSeq, imageSize, camMatrix, distCoeffs, vecsR, vecsT,
		CALIB_FIX_K3, TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, DBL_EPSILON));
	// CALIB_USE_INTRINSIC_GUESS �������������г����궨�����Ļ����ϲ���ʹ�ã����α궨ʹ�ô˲�������fx��fy��������ֵΪ��

	string fileName = calibType == 0 ? "./calibtest/calibration_cam.txt" : "./calibtest/calibration_proj.txt";
	if (!writeZhangCalibFiles(fileName, err, camMatrix, distCoeffs, vecsR, vecsT))
		return false;

	cout << "�궨��ɣ�����" << endl;
	return true;
}

//˫���Բ�ֵ
float biInterpolate(const Mat& srcImg, float i, float j)
{
	int x1 = static_cast<int>(floor(j));
	int y1 = static_cast<int>(floor(i));
	int x2 = static_cast<int>(ceil(j));
	int y2 = static_cast<int>(ceil(i));

	float valij = *srcImg.ptr<float>(y1, x1);
	float valij1 = *srcImg.ptr<float>(y1, x2);
	float vali1j = *srcImg.ptr<float>(y2, x1);
	float vali1j1 = *srcImg.ptr<float>(y2, x2);

	float val = (y2 - i)*(x2 - j)*valij + (y2 - i)*(j - x1)*valij1 + (i - y1)*(x2 - j)*vali1j + (i - y1)*(j - x1)*vali1j1;
	return val;
}

// ����������ӳ�䣺���cam-->ͶӰ��proj
bool featCoordinateMap(const vector<Mat>& horizontalPhases,
	const vector<Mat>& verticalPhases,
	const Size& projResolution,
	const vector<vector<Point2f>>& camPointsSeq,
	vector<vector<Point2f>>& projPointsSeq,
	int frq)
{
	assert(horizontalPhases.size() != 0 && verticalPhases.size() != 0 && camPointsSeq.size() != 0);

	ofstream fout("./calibtest/projPointsSeq.txt");

	for (size_t n = 0; n < camPointsSeq.size(); n++)
	{
		Mat phaseH = horizontalPhases[n];
		Mat phaseV = verticalPhases[n];

		fout << "��" << n + 1 << "��ͶӰͼ------" << endl;

		vector<Point2f> projPoints(camPointsSeq[n].size());
		for (size_t t = 0; t < camPointsSeq[n].size(); t++)
		{
			// ����ֱ��ȡ������λ
			//int x = static_cast<int>(round(camPointsSeq[n][t].x));
			//int y = static_cast<int>(round(camPointsSeq[n][t].y));
			//projPoints[t].x = (*phaseV.ptr<float>(y, x) * (static_cast<float>(projResolution.width) / frq) / (2 * CV_PI));
			//projPoints[t].y = (*phaseH.ptr<float>(y, x) * (static_cast<float>(projResolution.height) / frq) / (2 * CV_PI));

			// ����˫���Բ�ֵ����λ
			float row = camPointsSeq[n][t].y;
			float col = camPointsSeq[n][t].x;
			float valH = biInterpolate(phaseH, row, col);
			float valV = biInterpolate(phaseV, row, col);
			projPoints[t].x = (valV * (static_cast<float>(projResolution.width) / frq) / (2 * CV_PI));
			projPoints[t].y = (valH * (static_cast<float>(projResolution.height) / frq) / (2 * CV_PI));

			// ���ʴ�ѧ����
			//int ws = 15;// ������ȵ�һ��
			//int width = phaseH.cols;
			//int height = phaseH.rows;
			//Mat camWinPts(Size(2, 4 * ws * ws), CV_32F);
			//Mat projWinPts(Size(2, 4 * ws * ws), CV_32F);
			//int x = static_cast<int>(round(camPointsSeq[n][t].x));//������col
			//int y = static_cast<int>(round(camPointsSeq[n][t].y));//������row
			//if (x>ws && y>ws && x+ws<width && y+ws<height)
			//{
			//	int index = 0;
			//	for (size_t i = y - ws; i < y + ws; i++)
			//	{
			//		for (size_t j = x - ws; j < x + ws; j++)
			//		{
			//			*camWinPts.ptr<float>(index, 0) = j;
			//			*camWinPts.ptr<float>(index, 1) = i;
			//			*projWinPts.ptr<float>(index, 0) = *phaseV.ptr<float>(i, j) * (static_cast<float>(projResolution.width) / frq) / (2 * CV_PI);
			//			*projWinPts.ptr<float>(index, 1) = *phaseH.ptr<float>(i, j) * (static_cast<float>(projResolution.width) / frq) / (2 * CV_PI);
			//			index++;
			//		}
			//	}
			//}
			//Mat H = findHomography(camWinPts, projWinPts);
			//H.convertTo(H, CV_32F);
			//Point3f pt(camPointsSeq[n][t].x, camPointsSeq[n][t].y, 1.0);
			//Point3f ppt = Point3f(Mat(H*Mat(pt)));
			//projPoints[t].x = ppt.x / ppt.z;
			//projPoints[t].y = ppt.y / ppt.z;

			fout << t << "," << projPoints[t].x << "," << projPoints[t].y << endl;
		}
		projPointsSeq.emplace_back(projPoints);
		fout << endl;
	}

	return true;
}

// ͶӰ��������궨
bool projCalibration(const vector<vector<Point3f>>& objPointsSeq,
	const vector<vector<Point2f>>& projPointsSeq,
	const Size& imageSize,
	Mat& projMatrix,
	Mat& distCoeffs,
	vector<Mat>& vecsR,
	vector<Mat>& vecsT)
{
	if (!camCalibration(objPointsSeq, projPointsSeq, imageSize, projMatrix, distCoeffs, vecsR, vecsT, 1))
		return false;

	return true;
}

// �����ͶӰ������˫Ŀ�궨
bool steroCalibration(const vector<vector<Point3f>>& objPointsSeq,
	const vector<vector<Point2f>>& camPointsSeq,
	const vector<vector<Point2f>>& projPointsSeq,
	Mat& camMatrix, Mat& camCoeffs,
	Mat& projMatrix, Mat& projCoeffs,
	const Size& imageSize,
	Mat& vecR,
	Mat& vecT,
	Mat& vecE,
	Mat& vecF)
{
	assert(objPointsSeq.size() != 0 && camPointsSeq.size() != 0 && projPointsSeq.size() != 0);

	double err = stereoCalibrate(objPointsSeq, camPointsSeq, projPointsSeq, camMatrix, camCoeffs, projMatrix, projCoeffs,
					imageSize, vecR, vecT, vecE, vecF, 
					/*CALIB_USE_INTRINSIC_GUESS + CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5,*/
					CALIB_FIX_INTRINSIC,//�����ͶӰ�ǹ�ѧ�����ͺŲ�ͬ����˲��ɸ��ĸ��Ե��ڲ������ϵ��
					TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, DBL_EPSILON));
	// ˫Ŀ����궨ʱ��imageSize����ϴ�ķֱ���

	string fileName = "./calib_result/calibration_stereo.txt";
	if (!writeStereoCalibFiles(fileName, err, camMatrix, camCoeffs, projMatrix, projCoeffs, vecR, vecT))
		return false;

	cout << "˫Ŀ����궨���!!!" << endl;
	return true;
}

// ˫Ŀ�������У�����������߽��������Ƕ�άͼ��У��
bool undistortImg(const Mat &srcImage,
	Mat &dstImage,
	const Mat &camMatrix,
	const Mat &camCoeffs,
	const Mat &projMatrix,
	const Mat &projCoeffs,
	const Mat &vecR,
	const Mat &vecT)
{
	assert(!srcImage.empty() && !camMatrix.empty() && !camCoeffs.empty() && !vecR.empty()
		&& !projMatrix.empty() && !projCoeffs.empty() && !vecT.empty());

	Mat map1, map2, R1, R2, P1, P2, Q;

	stereoRectify(camMatrix, camCoeffs, projMatrix, projCoeffs, Size(srcImage.cols, srcImage.rows), vecR, vecT, R1, R2, P1, P2, Q);
	initUndistortRectifyMap(camMatrix, camCoeffs, R1, P1, srcImage.size(), CV_32FC1, map1, map2);//��У�������
	remap(srcImage, dstImage, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

	return true;
}

// ��ά�ؽ������ؾ�����λ-->�����������ϵ
bool reconstruction(const Mat& absPhase,
	const Mat& camMatrix,
	const Mat& projMatrix,
	const Mat& vecR,
	const Mat& vecT,
	const Size& projResolution,
	Mat& dstImage,
	int frq,
	int projMode)
{
	assert(!absPhase.empty() && !camMatrix.empty() && !projMatrix.empty() && !vecR.empty() && !vecT.empty());

	// �������ͶӰ�ǵ�Ӧ����
	Mat_<float> camM = (Mat_<float>(3, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);
	Mat projM;
	hconcat(vecR, vecT, projM);

	Mat Hcam = camMatrix * camM;
	Mat Hproj = projMatrix * projM;

	float P = .0; // projMode<<0����ֱ����<<1��ˮƽ����
	projMode == 0 ? P = static_cast<float>(projResolution.width) / frq : P = static_cast<float>(projResolution.height) / frq;
	Mat Vp(absPhase.size(), absPhase.type());
	for (size_t i = 0; i < absPhase.rows; i++)
	{
		for (size_t j = 0; j < absPhase.cols; j++)
		{
			*Vp.ptr<float>(i, j) = (*absPhase.ptr<float>(i, j)) * P / (2 * CV_PI);
		}
	}

	// ��λ��ά����ӳ��
	float Hcam11 = *Hcam.ptr<float>(0, 0);
	float Hcam12 = *Hcam.ptr<float>(0, 1);
	float Hcam13 = *Hcam.ptr<float>(0, 2);
	float Hcam21 = *Hcam.ptr<float>(1, 0);
	float Hcam22 = *Hcam.ptr<float>(1, 1);
	float Hcam23 = *Hcam.ptr<float>(1, 2);
	float Hcam31 = *Hcam.ptr<float>(2, 0);
	float Hcam32 = *Hcam.ptr<float>(2, 1);
	float Hcam33 = *Hcam.ptr<float>(2, 2);
	float Hcam14 = *Hcam.ptr<float>(0, 3);
	float Hcam24 = *Hcam.ptr<float>(1, 3);
	float Hcam34 = *Hcam.ptr<float>(2, 3);

	float Hproj21 = *Hproj.ptr<float>(1, 0);
	float Hproj22 = *Hproj.ptr<float>(1, 1);
	float Hproj23 = *Hproj.ptr<float>(1, 2);
	float Hproj24 = *Hproj.ptr<float>(1, 3);
	float Hproj31 = *Hproj.ptr<float>(2, 0);
	float Hproj32 = *Hproj.ptr<float>(2, 1);
	float Hproj33 = *Hproj.ptr<float>(2, 2);
	float Hproj34 = *Hproj.ptr<float>(2, 3);

	Mat xyz(Size(1, 3), CV_32F);
	int rows = absPhase.rows;
	int cols = absPhase.cols;

	//ofstream fp("./ply/test0525.txt");

	for (size_t i = 0; i < rows; i++)
	{
		for (size_t j = 0; j < cols; j++)
		{
			float projPos = *Vp.ptr<float>(i, j);// ͶӰ��������
			Mat_<float> mat1 = (Mat_<float>(3, 3) << Hcam11 - (j + 1)*Hcam31, Hcam12 - (j + 1)*Hcam32, Hcam13 - (j + 1)*Hcam33,
				Hcam21 - (i + 1)*Hcam31, Hcam22 - (i + 1)*Hcam32, Hcam23 - (i + 1)*Hcam33,
				Hproj21 - projPos*Hproj31, Hproj22 - projPos*Hproj32, Hproj23 - projPos*Hproj33);
			Mat_<float> mat2 = (Mat_<float>(3, 1) << (j + 1)*Hcam34 - Hcam14,
				(i + 1)*Hcam34 - Hcam24,
				projPos*Hproj34 - Hproj24);
			xyz = (mat1.inv())*mat2;

			// ����������ݽ��
			for (size_t k = 0; k < 3; k++)
			{
				*dstImage.ptr<float>(i*cols + j, k) = *xyz.ptr<float>(k, 0);
			}
			//fp << *xyz.ptr<float>(0, 0) << "," << *xyz.ptr<float>(1, 0) << "," << *xyz.ptr<float>(2, 0) << endl;
		}
	}
	cout << "��ά���ݴ�����ɣ�����" << endl;
	//fp.close();

	return true;
}

// �������ϱ궨����
bool writeZhangCalibFiles(const string& fileName,
	const double& reprojErr,
	const Mat& intrinsicMatrix,
	const Mat& distCoeffs,
	const vector<Mat>& vecsR,
	const vector<Mat>& vecsT)
{
	assert(vecsR.size() != 0 && vecsT.size() != 0 && !intrinsicMatrix.empty() && !distCoeffs.empty() && !fileName.empty());

	// �궨�����ļ���ʱ������
	//time_t now = time(nullptr);
	//string t = ctime(&now);
	//string fileName = "./calib_result/calibration.txt";

	ofstream fp(fileName, ios::out);
	if (!fp) return false;

	fp << "��ͶӰ���" << endl << reprojErr << endl;	
	Mat rotationMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	fp << "����ڲ�������" << endl;
	fp << intrinsicMatrix << endl;
	fp << "����ϵ��" << endl;
	fp << distCoeffs << endl;
	for (int i = 0; i < vecsR.size(); i++)
	{
		fp << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fp << vecsR[i] << endl;

		// ����ת����ת��Ϊ���Ӧ����ת����
		Rodrigues(vecsR[i], rotationMatrix);
		fp << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fp << rotationMatrix << endl;
		fp << "��" << i + 1 << "��ͼ���ƽ������" << endl;
		fp << vecsT[i] << endl;
	}

	fp.close();
	return true;
}

// ����˫Ŀ����궨����
bool writeStereoCalibFiles(const string& fileName,
	const double& reprojErr,
	const Mat& camMatrix,
	const Mat& camCoeffs,
	const Mat& projMatrix,
	const Mat& projCoeffs,
	const Mat& vecR,
	const Mat& vecT)
{
	assert(!vecR.empty() && !vecT.empty() && !camMatrix.empty() && !camCoeffs.empty() 
		&& !projMatrix.empty() && !projCoeffs.empty() && !fileName.empty());

	ofstream fp(fileName, ios::out);
	if (!fp) return false;

	fp << "��ͶӰ���" << endl << reprojErr << endl;

	fp << "����ڲ�������" << endl;
	fp << camMatrix << endl;
	fp << "�������ϵ��" << endl;
	fp << camCoeffs << endl;

	fp << "ͶӰ���ڲ�������" << endl;
	fp << projMatrix << endl;
	fp << "ͶӰ�ǻ���ϵ��" << endl;
	fp << projCoeffs << endl;

	fp << "��ת����" << endl;
	fp << vecR << endl;
	fp << "ƽ������" << endl;
	fp << vecT << endl;

	fp.close();
	return true;
}

// ����ϵͳ�궨���ò�������ת��
bool vecsMat2vecsFloat(const vector<Mat>& sysParameters,
	vector<float>& sysFloatParams)
{
	assert(!sysParameters.empty());

	for (size_t n = 0; n < sysParameters.size(); n++)
	{
		Mat sys = sysParameters[n];
		for (size_t i = 0; i < sys.rows; i++)
		{
			for (size_t j = 0; j < sys.cols; j++)
			{
				float val = *sys.ptr<double>(i, j);
				sysFloatParams.emplace_back(val);
			}
		}
	}

	return true;
}

//ϵͳ�궨�ܺ���
bool SongZhangCalibration(const vector<Mat>& camCalibImg, const vector<Rect>& imgRectVec,
	const vector<vector<Mat>>& projCalibImg, vector<float>& sysParams)
{
	assert(camCalibImg.size() > 3 && camCalibImg[0].channels() == 1);//�궨ͼ�����ٴ���3��, ��Ϊ��ͨ��
	int calibImgNum = camCalibImg.size();
	/***************����궨******************/
	vector<vector<Point3f>> objPointsSeq;
	vector<vector<Point2f>> camPointsSeq;
	Size imageSize = camCalibImg[0].size();
	Size boardSize = Size(9, 11);
	Size squareSize(10, 10);
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
	vector<Mat> vecsR;
	vector<Mat> vecsT;

	////�궨ͼ��ü�
	//vector<Mat> croppedImg(calibImgNum);
	//for (size_t i = 0; i < calibImgNum; i++)
	//{
	//	croppedImg[i] = camCalibImg[i](camCalibImg[i]);
	//}

	findFeaturePoints(camCalibImg, objPointsSeq, camPointsSeq, imageSize, boardSize, squareSize);
	camCalibration(objPointsSeq, camPointsSeq, imageSize, cameraMatrix, distCoeffs, vecsR, vecsT);

	/*****************ͶӰ�Ǳ궨*********************/
	vector<vector<Point2f>> projPointsSeq;
	Mat projMatrix;
	Mat projdistCoeffs;
	Size projImageSize(1280, 720);
	vector<Mat> vecsR2;
	vector<Mat> vecsT2;
	vector<vector<Mat>> phaseVector(calibImgNum, vector<Mat>(2));
	//phaseVector = getAllHVPhase(projCalibImg);//����λ
	featCoordinateMap(phaseVector[0], phaseVector[1], projImageSize, camPointsSeq, projPointsSeq);
	projCalibration(objPointsSeq, projPointsSeq, projImageSize, projMatrix, projdistCoeffs, vecsR2, vecsT2);

	Mat vecR;
	Mat vecT;
	Mat vecE;
	Mat vecF;
	steroCalibration(objPointsSeq, camPointsSeq, projPointsSeq, cameraMatrix, distCoeffs, projMatrix, projdistCoeffs,
		imageSize, vecR, vecT, vecE, vecF);

	vector<Mat> sysParameters;
	sysParameters.push_back(cameraMatrix);
	sysParameters.push_back(distCoeffs);
	sysParameters.push_back(projMatrix);
	sysParameters.push_back(projdistCoeffs);
	sysParameters.push_back(vecR);
	sysParameters.push_back(vecT);
	vecsMat2vecsFloat(sysParameters, sysParams);

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	int count = 7;
	vector<Mat> srcImages(count);
	for (size_t i = 0; i < count; i++)
	{
		string path = "./camcalib/0 (" + std::to_string(i + 1) + ").bmp";
		srcImages[i] = imread(path, IMREAD_GRAYSCALE);
	}
	//////////////////////
	vector<vector<Point3f>> objPointsSeq;
	vector<vector<Point2f>> camPointsSeq;
	Size imageSize;
	Size boardSize = Size(9, 11);
	Size squareSize(10, 10);

	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
	vector<Mat> vecsR;
	vector<Mat> vecsT;

	findFeaturePoints(srcImages, objPointsSeq, camPointsSeq, imageSize, boardSize, squareSize);
	camCalibration(objPointsSeq, camPointsSeq, imageSize, cameraMatrix, distCoeffs, vecsR, vecsT);
	//////////////////////

	//featCoordinateMap();

	//////////////////////
	vector<vector<Point2f>> projPointsSeq = camPointsSeq;
	Mat cameraMatrix2;
	Mat distCoeffs2;
	Size imageSize2(1080, 720);
	vector<Mat> vecsR2;
	vector<Mat> vecsT2;
	projCalibration(objPointsSeq, projPointsSeq, imageSize, cameraMatrix2, distCoeffs2, vecsR2, vecsT2);

	//////////////////////
	Mat vecR;
	Mat vecT;
	Mat vecE;
	Mat vecF;
	steroCalibration(objPointsSeq, camPointsSeq, projPointsSeq, cameraMatrix, distCoeffs, cameraMatrix2, distCoeffs2, imageSize,
		vecR, vecT, vecE, vecF);
	//////////////////////

	Mat src = imread("./camcalib/0 (1).bmp", IMREAD_GRAYSCALE);
	Mat dst(src.size(), src.type());
	//undistortImg(src, dst, cameraMatrix, distCoeffs, vecR);
	//���ɱ궨���е�ͼ���У��ֱ��ʹ��opencv��undistortImg����
	//imwrite("dst.jpg", dst);

	//////////////////////
	//reconstruction()

	return 0;
}