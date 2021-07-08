#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/thread/thread.hpp>

using namespace std;
//------------------------------------------------------------------------------------------------//

#include <pcl\keypoints\uniform_sampling.h>
//���Ȳ���
void sampleUniform(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudSampled)
{
	//����ָ���뾶�����壬��ÿһ�����ھ���������������ĵ���Ϊ�²�����������
	//�����˲����²���������һ�������壻���Ȳ�������һ����
	//�²����ķ��Ⱥܴ�

	pcl::UniformSampling<pcl::PointXYZ> us;
	us.setInputCloud(cloud);
	us.setRadiusSearch(0.005f);
	us.filter(*cloudSampled);
}

#include <pcl\filters\random_sample.h>
//�������
void sampleRandom(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudSampled)
{
	//��򵥣�ֱ�����ù̶����������

	pcl::RandomSample<pcl::PointXYZ> rs;
	rs.setInputCloud(cloud);
	rs.setSample(3000);//���������������ĸ���
	rs.filter(*cloudSampled);
}

#include <pcl\filters\normal_space.h>//���߿ռ����
#include <pcl\features\normal_3d_omp.h>//���̼߳���
//���㷨����
void estimateNormals(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::Normal>::Ptr& cloudNormals)
{
	pcl::NormalEstimationOMP<pcl::PointXYZ, pcl::Normal> n;
	n.setInputCloud(cloud);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>());
	n.setSearchMethod(tree);
	n.setNumberOfThreads(6);
	n.setKSearch(30);
	n.compute(*cloudNormals);
}
//���߿ռ����
void sampleNormalSpace(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudSampled)
{
	//�ڷ������ռ��ھ������������ʹ��ѡ��֮��ķ��߷ֲ������ܴ�
	//�������Ϊ�����������仯��ĵط�ʣ���϶࣬�仯С�ĵط�ʣ���ϡ�٣�����Ч��֤��������
	//ĳЩ�������и�ƽ�����ݼ�����ģ�͵�С��������ȷ����ȷ�Ķ���������Ҫ
	//������������������ͨ��ֻ���������㼯��ѡ�񼸸��������ᵼ��ĳЩ�����޷�ȷ����ȷ����任
	//���ݽǶȿռ��еķ���λ�����洢�㣬Ȼ�󾡿��ܾ��ȵ�����Щ�洢���ϲ���
	//��ʹ�õر��������ж���ģ��봫ͳ���������ķ�����ȣ����нϵ͵ļ���ɱ�����³���Խϲ�

	//���㷨����(��װ����һ������������Ҳ���Բ���װ)
	pcl::PointCloud<pcl::Normal>::Ptr cloudNormals(new pcl::PointCloud<pcl::Normal>);
	estimateNormals(cloud, cloudNormals);
	//����
	pcl::NormalSpaceSampling<pcl::PointXYZ, pcl::Normal> nss;
	nss.setInputCloud(cloud);// �����������
	nss.setNormals(cloudNormals);// ��������������ϼ���ķ���
	nss.setBins(2, 2, 2);// ����x,y,z����bins�ĸ���
	nss.setSeed(0); // �������ӵ�
	nss.setSample(2000); // ����Ҫ��������������
	nss.filter(*cloudSampled);
}

#include <pcl\filters\sampling_surface_normal.h>
//�����ռ����
void sampleSurfaceNormal(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointNormal>::Ptr& cloudSampled)
{
	//������ռ仮��Ϊ����ֱ��ÿ�������а������N���㣬����ÿ���������������
	//ʹ��ÿ�������е�N����������Normal,�����ڵ����в����㶼����������ͬ�ķ���
	//pcl::pointnormal�����������ͣ���normalestimation���޷�����ֱ�ӵõ�����xyz��normal����Ч���
	//�����Ҫ�ȼ���pcl::normal��Ȼ���xyz��normal�ϲ���������һ�ֹ���pcl::pointnormal�ķ�����

	// 1 ���߹���
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	tree->setInputCloud(cloud);
	n.setInputCloud(cloud);
	n.setSearchMethod(tree);
	n.setKSearch(20);
	n.compute(*normals);

	// 2 ����XYZ�ͷ������ֶ�--���ϲ�xyz��normal
	pcl::PointCloud<pcl::PointNormal>::Ptr cloudNormals(new pcl::PointCloud<pcl::PointNormal>);
	pcl::concatenateFields(*cloud, *normals, *cloudNormals);

	// 3 ����
	pcl::SamplingSurfaceNormal <pcl::PointNormal> ssn;
	ssn.setInputCloud(cloudNormals);
	ssn.setSample(10);     // ����ÿ���������������� n
	ssn.setRatio(0.1f);    // ���ò�����&
	ssn.filter(*cloudSampled);// ����ĵ�������ÿ������������������n x &
}

#include <pcl\surface\mls.h>
#include <pcl\search\kdtree.h>//����ʹ��kdtree��ͷ�ļ�������֮һ��
//MLS�����ϲ���
void sampleMLS(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudSampled)
{
	//�ϲ�����һ�ֱ����ؽ���������������������Ԥ�ڣ�ͨ���ڲ�ӵ�еĵ������ݣ��ָ�ԭ���ı���
	//���ӵĲ�����裬�������׼ȷ������ڵ����²���ʱ��һ��Ҫ����һ��ԭʼ����

	// 1 �����ϲ�������
	pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointXYZ> up;//����ʵ����һ�������ƶ���С���˵ĵ����ϲ���
	up.setInputCloud(cloud);

	// 2 ������������
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree;

	// 3 ���ò���
	up.setSearchMethod(tree);	
	up.setSearchRadius(0.1);//������������İ뾶
	up.setUpsamplingMethod(pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointXYZ>::SAMPLE_LOCAL_PLANE);
	up.setUpsamplingRadius(0.04);//���ò����İ뾶	
	up.setUpsamplingStepSize(0.02);//���������Ĵ�С
	up.process(*cloudSampled);
}
//ͬʱ�����ϲ����ͼ��㷨����
void sampleMLSEx(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointNormal>::Ptr& cloudSampled)
{
	// 1 �����ϲ�������
	pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal> up;//����ʵ����һ�������ƶ���С���˵ĵ����ϲ���
	up.setInputCloud(cloud);

	// 2 ������������
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree;

	// 3 ���ò���
	up.setSearchMethod(tree);
	up.setSearchRadius(0.1);//������������İ뾶
	up.setComputeNormals(true);//���㷨����
	up.setPolynomialFit(true);
	up.setUpsamplingMethod(pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal>::SAMPLE_LOCAL_PLANE);
	up.setUpsamplingRadius(0.04);
	up.setUpsamplingStepSize(0.02);
	up.setPolynomialFit(2);  //MLS��ϵĽ�����Ĭ����2
	//up.setProjectionMethod(pcl::MLSResult::ProjectionMethod::SIMPLE);
	//up.setNumberOfThreads(6);
	up.setPointDensity(100);
	up.process(*cloudSampled);
}


//------------------------------------------------------------------------------------------------//

//��ʾ����--˫����
void visualizeSampledCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudSampled)
{
	//PCL���ӻ�����
	boost::shared_ptr<pcl::visualization::PCLVisualizer> view(new pcl::visualization::PCLVisualizer("ShowClouds"));
	int v1 = 0, v2 = 0;
	view->createViewPort(.0, .0, .5, 1., v1);
	view->setBackgroundColor(0., 0., 0., v1);
	view->addText("Raw point clouds", 10, 10, "text1", v1);
	view->createViewPort(.5, .0, 1., 1., v2);
	view->setBackgroundColor(0., 0., 0., v2);
	view->addText("sampled point clouds", 10, 10, "text2", v2);

	//����z�ֶν�����Ⱦ
	pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZ> fildColor(cloud, "z");
	pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZ> fildSampledColor(cloudSampled, "z");
	view->addPointCloud<pcl::PointXYZ>(cloud, fildColor, "Raw point clouds", v1);
	view->addPointCloud<pcl::PointXYZ>(cloudSampled, fildSampledColor, "sampled point clouds", v2);
	//���õ��Ƶ���Ⱦ����,string &id = "cloud"�൱�ڴ���ID
	//����z�ֶν�����Ⱦ������Ҫ������Ⱦÿ��������ɫ
	//view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 1, 0, "Raw point clouds", v1);//���õ�����ɫ
	//view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 0, 1, "sampled point clouds", v2);
	while (!view->wasStopped())
	{
		view->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}

//------------------------------------------------------------------------------------------------//

int maininstance2()
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloudSampled(new pcl::PointCloud<pcl::PointXYZ>);

	string cloudPath = "../bunny.pcd";
	if (pcl::io::loadPCDFile(cloudPath, *cloud) != 0)
		return -1;
	cout << "���ص���������" << cloud->points.size() << "��" << endl;

	//------------------------------��������---------------------------------
	//���Ȳ���
	//sampleUniform(cloud, cloudSampled);
	//�������
	//sampleRandom(cloud, cloudSampled);
	//���߿ռ����
	//sampleNormalSpace(cloud, cloudSampled);
	//�����ռ����
	//sampleSurfaceNormal(cloud, cloudSampled);
	//MLS�����ϲ���
	//sampleMLS(cloud, cloudSampled);
	//sampleMLSEx(cloud, cloudSampled);
	//----------------------------------------------------------------------

	cout << "�˲�����������" << cloudSampled->points.size() << "��" << endl;
	visualizeSampledCloud(cloud, cloudSampled);

	return 0;
}