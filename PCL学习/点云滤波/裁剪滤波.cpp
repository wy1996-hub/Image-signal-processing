#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/thread/thread.hpp>

using namespace std;
//------------------------------------------------------------------------------------------------//

#include <pcl\filters\crop_hull.h>
#include <pcl\surface\concave_hull.h>
//CropHull�����ά������ڲ�������ȡ
void cutCropHull(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudCut)
{
	//һ���˲���������һ����άͼ�Σ�Ȼ��ü�λ��ͼ��֮�ڵĵ���

	// 1 Ϊ�˹���2D��ն���Σ���������2Dƽ����ƣ���Щƽ�����2D��ն���εĶ���
	pcl::PointCloud<pcl::PointXYZ>::Ptr boundingbox(new pcl::PointCloud<pcl::PointXYZ>);
	boundingbox->push_back(pcl::PointXYZ(0.1, -0.1, 0));
	boundingbox->push_back(pcl::PointXYZ(-0.1, 0.1, 0));
	boundingbox->push_back(pcl::PointXYZ(-0.1, -0.1, 0));
	boundingbox->push_back(pcl::PointXYZ(0.1, 0.1, 0));

	// 2 ������2Dƽ��㹹��͹��--����͹��(��ն����)������͹����״
	pcl::ConvexHull<pcl::PointXYZ> hull;//����͹������
	hull.setInputCloud(boundingbox);//�����������
	hull.setDimension(2);//����͹��ά��
	vector<pcl::Vertices> polygons;//����pcl:Vertices���͵����������ڱ���͹������
	pcl::PointCloud<pcl::PointXYZ>::Ptr surfaceHull(new pcl::PointCloud<pcl::PointXYZ>);//�õ�����������͹����״
	hull.reconstruct(*surfaceHull, polygons);//����2D͹�����

	// 3 ����CropHull�����˲��õ�2D���͹����Χ�ڵĵ��ƣ��˴���ά����Ҫ������͹��ά��һ��
	pcl::CropHull<pcl::PointXYZ> bb_filter;//����CropHull����
	bb_filter.setDim(2);                 
	bb_filter.setInputCloud(cloud);      
	bb_filter.setHullIndices(polygons);//�����ն���εĶ���
	bb_filter.setHullCloud(surfaceHull);//�����ն���ε���״
	bb_filter.filter(*cloudCut);
}

#include <pcl\filters\crop_box.h>
//CropBox������ڲ�����������ȡ
void cutCropBox(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudCut)
{
	//һ���˲���������һ����ά����ͼ�Σ������û����˸���������������

	pcl::CropBox<pcl::PointXYZ> cb;
	cb.setInputCloud(cloud);	
	cb.setMin(Eigen::Vector4f(-0.1f, -0.1, -0.1, -0.1));//�趨����ռ�����
	cb.setMax(Eigen::Vector4f(0.1f, 0.1, 0.1, 0.1));
	cb.setKeepOrganized(false);//���ϣ���ܹ���ȡ��ɾ�����������������Ϊtrue
	cb.setUserFilterValue(0.1f);//�ṩһ�������˵ĵ�Ӧ������Ϊ��ֵ��������ɾ������,��setKeepOrganized����
	cb.filter(*cloudCut);
}

#include <pcl\filters\local_maximum.h>
//LocalMaximum�����ֲ����ĵ�
void cutLocalMax(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudCut)
{
	//����ÿһ���㣬����z����ɾ����Щ��������ǵ��ھӣ�ͨ���뾶�����ҵ����ֲ����ĵ�

	//һ�ֹ���������ݵķ�ʽ
	//pcl::PointCloud<pcl::PointXYZ> cloud_in, cloud_out;
	//cloud_in.height = 1;
	//cloud_in.width = 3;
	//cloud_in.is_dense = true;
	//cloud_in.resize(4);
	//cloud_in[0].x = 0;    cloud_in[0].y = 0;    cloud_in[0].z = 0.25;
	//cloud_in[1].x = 0.25; cloud_in[1].y = 0.25; cloud_in[1].z = 0.5;
	//cloud_in[2].x = 0.5;  cloud_in[2].y = 0.5;  cloud_in[2].z = 1;
	//cloud_in[3].x = 5;    cloud_in[3].y = 5;    cloud_in[3].z = 2;
	//pcl::LocalMaximum<pcl::PointXYZ> lm;
	//lm.setInputCloud(cloud_in.makeShared());

	pcl::LocalMaximum<pcl::PointXYZ> lm;
	lm.setInputCloud(cloud);
	lm.setRadius(1.0f);//��������ȷ��һ�����Ƿ�Ϊ�ֲ����ֵ�İ뾶
	lm.filter(*cloudCut);//�����˲������������˲���ĵ���
}

#include <pcl\filters\grid_minimum.h>
//GridMinimum��ȡդ����͵�
void cutGridMin(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudCut)
{
	//�������ػ���һ�ֱȽ��ձ�ĵ��ƴ���ʽ
	//voxelgrid��������դ����ȡդ���ڵ�������
	//approximatevoxelgrid��������դ����ȡդ���ڵ�������
	//GridMinimum����ά������xyƽ�滮������Ѱ��ÿ����������С��z���ƴ��������
	//������Ч���˸߳���Ϣ��ϵ���������糵ͣ������

	float resolution = .001f;//�ֱ���ԽС����Խ�ܼ����ֱ���Խ������Խϡ��
	pcl::GridMinimum<pcl::PointXYZ> gm(resolution);//GridMinimum�಻����Ĭ�Ϲ��캯��
	gm.setInputCloud(cloud);
	gm.filter(*cloudCut);
}

//------------------------------------------------------------------------------------------------//

//��ʾ����--˫����
void visualizeCutCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudCut)
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
	pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZ> fildCutColor(cloudCut, "z");
	view->addPointCloud<pcl::PointXYZ>(cloud, fildColor, "Raw point clouds", v1);
	view->addPointCloud<pcl::PointXYZ>(cloudCut, fildCutColor, "sampled point clouds", v2);
	//���õ��Ƶ���Ⱦ����,string &id = "cloud"�൱�ڴ���ID
	//����z�ֶν�����Ⱦ������Ҫ������Ⱦÿ��������ɫ
	//view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 1, 0, "Raw point clouds", v1);//���õ�����ɫ
	//view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 0, 1, "sampled point clouds", v2);
	view->addCoordinateSystem(.1);
	while (!view->wasStopped())
	{
		view->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}

//------------------------------------------------------------------------------------------------//

int main()
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloudCut(new pcl::PointCloud<pcl::PointXYZ>);

	string cloudPath = "../bunny.pcd";
	if (pcl::io::loadPCDFile(cloudPath, *cloud) != 0)
		return -1;
	cout << "���ص���������" << cloud->points.size() << "��" << endl;

	//------------------------------��������---------------------------------

	//CropHull���������ڲ�������ȡ
	//cutCropHull(cloud, cloudCut);
	//CropBox������ڲ�����������ȡ
	//cutCropBox(cloud, cloudCut);
	//LocalMaximum�����ֲ����ĵ�
	//cutLocalMax(cloud, cloudCut);
	//GridMinimum��ȡդ����͵�
	cutGridMin(cloud, cloudCut);

	//----------------------------------------------------------------------

	cout << "�˲�����������" << cloudCut->points.size() << "��" << endl;
	visualizeCutCloud(cloud, cloudCut);

	return 0;
}