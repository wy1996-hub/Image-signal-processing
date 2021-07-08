#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/thread/thread.hpp>

using namespace std;

// ��Ҫ�����˲���4�����
// 1 ���������ܶȲ�����
// 2 ��Ⱥ��(���������ڵ���ԭ��������)
// 3 �²���
// 4 ����
//------------------------------------------------------------------------------------------------//

#include <pcl/filters/passthrough.h>
//ֱͨ�˲���
void filterPassThrough(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//���˵���ָ��ά�ȷ�����ȡֵ���ڸ���ֵ���ڵĵ�
	//��Ҫ�����ڳ�ʼ������ȥ��ƫ��ϴ����Ⱥ�����
	pcl::PassThrough<pcl::PointXYZ> pass;
	pass.setInputCloud(cloud);
	pass.setFilterFieldName("z"); //�˲��ֶ���������ΪZ�᷽��
	pass.setFilterLimits(0, 1); //�����ڹ��˷����ϵĹ��˷�Χ
	pass.setKeepOrganized(true); //����������ƽṹ
	pass.setNegative(true); //���ñ�����Χ�ڵĻ��ǹ��˵���Χ�ڵģ���־Ϊfalseʱ������Χ�ڵĵ�
	pass.filter(*cloudFiltered);
}

#include <pcl/filters/voxel_grid.h>
//�����˲���
void filterVoxelGrid(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//�����²�����ͬʱ���ƻ����Ƽ��νṹ��������״����
	//����²���Ч�ʸߣ������ƻ�����΢�۽ṹ
	//���������ıƽ�����ʾ��Ϊ׼ȷ
	//��Сɺ������ĵ����������Сդ�����������е��Ƶ�����
	pcl::VoxelGrid<pcl::PointXYZ> vg;
	vg.setInputCloud(cloud);
	vg.setLeafSize(.01f, .01f, .01f);
	vg.filter(*cloudFiltered);
}
#include <pcl\filters\approximate_voxel_grid.h>
void filterApproximateVoxelGrid(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//���������ıƽ������ڹ�ϣ����������޴�ĳ����ٶȻ��Ķ�
	//��Сɺ������ĵ����������Сդ�����������е��Ƶ�����
	pcl::ApproximateVoxelGrid<pcl::PointXYZ> avg;
	avg.setInputCloud(cloud);
	avg.setLeafSize(.005f, .005f, .005f);//��������դ��Ҷ��С,��ֵԽС����Խ��
	avg.filter(*cloudFiltered);
}

#include <pcl\kdtree\kdtree_flann.h>
/*error C2079: ��pcl::KdTreeFLANN::param_radius_��ʹ��δ����� struct��flann::SearchParams��*/
/*opencv��pclͬʱ����flann����ֳ�ͻ���⣬PCLflann�й����÷�����opencv�й����ݵ�ǰ��*/
//�Ľ��������˲���
void filterVoxelGridEx(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//ʹ�����������е������λ�ô����������أ������ĵ㲻һ����ԭʼ�����еĵ�
	//���ܻ���ʧԭʼ���Ƶ�ϸС����
	//��ԭʼ�����о���������������ĵ�����������ĵ㣬��ά��ͬʱ����ߵ������ݵı��׼ȷ��
	pcl::VoxelGrid<pcl::PointXYZ> vg;
	vg.setInputCloud(cloud);
	vg.setLeafSize(.003f, .003f, .003f);
	pcl::PointCloud<pcl::PointXYZ>::Ptr voxelCloud(new pcl::PointCloud<pcl::PointXYZ>);
	vg.filter(*voxelCloud);
	//k��������
	//�����²����Ľ����ѡ�����������ĵ���Ϊ���յ��²�����
	pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
	kdTree.setInputCloud(cloud);
	pcl::PointIndicesPtr inds = boost::shared_ptr<pcl::PointIndices>(new pcl::PointIndices());//������������ڽ�����ȡ���������±�����
	for (size_t i = 0; i < voxelCloud->points.size(); i++)
	{
		pcl::PointXYZ pt = voxelCloud->points[i];
		int k = 1;//���������
		vector<int> pointIdxKNNSearch(k);
		vector<float> pointKNNSquaredDistance(k);
		//��kdTree��setInputCloud(cloud)��������������˲���ĵ���
		if (kdTree.nearestKSearch(pt, k, pointIdxKNNSearch, pointKNNSquaredDistance) > 0)
			inds->indices.emplace_back(pointIdxKNNSearch[0]);
	}
	pcl::copyPointCloud(*cloud, inds->indices, *cloudFiltered);
}

#include <pcl\filters\statistical_outlier_removal.h>
//ͳ���˲���
void filterStddevMulThresh(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//����ɨ�������ܶȲ����ȵĵ������ݣ������е����Ҳ�����ϡ�����Ⱥ��
	//�ܶȲ�����ϡ����Ⱥ��ᵼ�¹��ƾֲ���������ʱ���㸴�ӣ����ִ���Ӱ����׼
	//ͳ���˲�����ȥ��������Ⱥ�㣨��Ⱥ��һ���ɲ����������룬�ڿռ��зֲ�ϡ�裩
	//ÿ���㶼���һ������Ϣ����ĳ�������Խ�ܼ��������Ϣ��Խ����������������Ϣ����Ϣ��ԽС��������Ⱥ�������Ϣ���Ժ��Բ���
	//��������ܶȣ���ÿ���㵽�������K�����ƽ�����룻���������е�ľ���Ӧ�ù��ɸ�˹�ֲ���ͨ�������ľ�ֵ�뷽���޳���Ⱥ��
	pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
	sor.setInputCloud(cloud);
	sor.setMeanK(50);//���ÿ������Χ��50���ڽ���
	sor.setStddevMulThresh(2);//2���ı�׼��
	sor.filter(*cloudFiltered);
}

#include <pcl\filters\radius_outlier_removal.h>
//�뾶�˲���
void filterRadius(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//�趨ÿ����������һ���뾶��Χd���������㹻��Ľ���n���������ɾ���õ�
	//��Ҫ���ȶ����е������ݹ���KD��
	//�뾶�˲�����ͳ���˲������򵥴ֱ������ٶȿ죬����������µĵ�һ�������ܼ���
	pcl::RadiusOutlierRemoval<pcl::PointXYZ> ror;
	ror.setInputCloud(cloud);
	ror.setRadiusSearch(0.003);//�����뾶0.003m�ڵ��ڽ���
	ror.setMinNeighborsInRadius(20);//С��20����ɾ��
	ror.filter(*cloudFiltered);
}

#include <pcl\filters\conditional_removal.h>
//�����˲���
void filterCondition(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//�����û��Լ��趨�������ض�����������

	// 1 ���������˲�����Ҫʹ�õ�����
	// 1.1 �ֶ�����
	pcl::ConditionAnd<pcl::PointXYZ>::Ptr rangeCond(new pcl::ConditionAnd<pcl::PointXYZ>);//ʵ��������ָ��
	rangeCond->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(new
		pcl::FieldComparison<pcl::PointXYZ>("z", pcl::ComparisonOps::GT, 0)));//�����z�ֶ��ϴ���(pcl::ComparisonOps::GT greater than)��ֵ������
	rangeCond->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(new
		pcl::FieldComparison<pcl::PointXYZ>("z", pcl::ComparisonOps::LT, 15.0)));//С����ֵ
	// 1.2 ��������
	pcl::ConditionOr<pcl::PointNormal>::Ptr curvatureCond(new pcl::ConditionOr<pcl::PointNormal>);
	curvatureCond->addComparison(pcl::FieldComparison<pcl::PointNormal>::ConstPtr(new
		pcl::FieldComparison<pcl::PointNormal>("curvature", pcl::ComparisonOps::GT, 1.)));

	// 2 ʹ���˲���
	pcl::ConditionalRemoval<pcl::PointXYZ> cr;
	cr.setInputCloud(cloud);
	cr.setCondition(rangeCond);
	cr.setKeepOrganized(true);//���ֵ��ƽṹ���˲��������Ŀû�м��٣�ʹ��nan����
	cr.filter(*cloudFiltered);

	// 3 ȥ��nan��
	vector<int> mapping;
	pcl::removeNaNFromPointCloud(*cloudFiltered, *cloudFiltered, mapping);
}

#include <pcl\filters\model_outlier_removal.h>
#include <pcl/ModelCoefficients.h>
//ģ���˲���
void filterModel(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//����ģ�ͺ͵�֮��ľ�����˵����е�����
	//�������������һ�Σ��Զ�--���˷����޵�--APIָ��ģ��֮��ĵ�--ָ������ֵ

	//����ģ��
	pcl::ModelCoefficients coeff;
	coeff.values.resize(4);
	coeff.values[0] = 0;
	coeff.values[1] = 0;
	coeff.values[2] = 0;
	coeff.values[3] = 1;

	pcl::ModelOutlierRemoval<pcl::PointXYZ> mor;
	mor.setInputCloud(cloud);
	mor.setModelType(pcl::SACMODEL_SPHERE);
	mor.setModelCoefficients(coeff);
	mor.setThreshold(0.5);
	mor.filter(*cloudFiltered);
}

#include <pcl\filters\project_inliers.h>
//ͶӰ�˲���
void filterProject(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//��ǰ��ά���������̫�󣬷�չ���Ǻܳ���
	//����ͶӰ����άƽ���ܹ�����ͼ���㷨���д���
	//����ά�Ƚ���ͶӰ����ʧһЩ������Ϣ
	//���ά�Ƚ���ͶӰ���ܽ��͵�����Ϣ����ʧ
	//����ͶӰӦ�����ķ�ʽ�����ɸ���ͼ

	//����ax+by+cz+d=0��ƽ��ģ�ͣ�����ϵ��Ϊa=b=d=0,c=1��ƽ�棬��z����ص�ȫ��ͶӰ��xyƽ��
	pcl::ModelCoefficients::Ptr coeff(new pcl::ModelCoefficients);
	coeff->values.resize(4);
	coeff->values[0] = coeff->values[1] = coeff->values[3] = 0;
	coeff->values[2] = 1;

	//Assertion `px != 0' failed. �ض�������ָ��û�г�ʼ����������
	pcl::ProjectInliers<pcl::PointXYZ>::Ptr proj(new pcl::ProjectInliers<pcl::PointXYZ>);
	proj->setInputCloud(cloud);
	proj->setModelType(pcl::SACMODEL_PLANE);
	proj->setModelCoefficients(coeff);
	proj->filter(*cloudFiltered);
}
//������ͶӰ�����棬���ǽ�����һ��ƽ��
struct sphere
{
	float centerX;
	float centerY;
	float centerZ;
	float radius;
};
typedef pcl::PointXYZ PointT;
void filterProjectEx(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	PointT pt;
	sphere sp;
	sp.centerX = sp.centerY = sp.centerZ = 0;
	sp.radius = 1;
	for (size_t i = 0; i < cloud->points.size(); i++)
	{
		float d = sqrt(pow(cloud->points[i].x - sp.centerX, 2) + pow(cloud->points[i].y - sp.centerY, 2) + pow(cloud->points[i].z - sp.centerZ, 2));
		pt.x = cloud->points[i].x * sp.radius / d;
		pt.y = cloud->points[i].y * sp.radius / d;
		pt.z = cloud->points[i].z * sp.radius / d;
		cloudFiltered->points.emplace_back(pt);
	}
}

#include <pcl\filters\extract_indices.h>
//������ȡ��
void extractPointIdxs(vector<int> indexes, pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//ָ��һ�����������Ҫ��������ԭʼ������������ȡ��������Ҫʹ��������ȡ��
	//indices�Ľṹ���Ǽ򵥵�--vector<int>--pcl::IndicesPtr(new vector<int>())

	// 1 ȡ����Ҫ������
	pcl::PointIndices indices;
	for (size_t i = 0; i < indexes.size(); i++)
	{
		indices.indices.emplace_back(indexes[i]);
	}

	// 2 ������ȡ��
	pcl::ExtractIndices<pcl::PointXYZ> extr;
	extr.setInputCloud(cloud);
	extr.setIndices(boost::make_shared<pcl::PointIndices>(indices));//��������

																	// 3 ��ȡ�������Լ�ʣ�µĵ�
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloudExtract(new pcl::PointCloud<pcl::PointXYZ>);
	extr.filter(*cloudExtract);//��ȡ��Ӧ�����ĵ���
	extr.setNegative(true);//��ȡ����֮��ʣ�µĵ�
	extr.filter(*cloudFiltered);
}
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
//��һ���㼯����ȡ�Ӽ�
void extractFromPointCloud()
{
	pcl::PCLPointCloud2::Ptr cloud_blob(new pcl::PCLPointCloud2), cloud_filtered_blob(new pcl::PCLPointCloud2);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>), cloud_p(new pcl::PointCloud<pcl::PointXYZ>), cloud_f(new pcl::PointCloud<pcl::PointXYZ>);
	// �����������
	pcl::PCDReader reader;
	reader.read("table_scene_lms400.pcd", *cloud_blob);
	std::cerr << "PointCloud before filtering: " << cloud_blob->width * cloud_blob->height << " data points." << std::endl;
	// �����˲�������:ʹ��Ҷ��СΪ1cm���²���
	pcl::VoxelGrid<pcl::PCLPointCloud2> sor;
	sor.setInputCloud(cloud_blob);
	sor.setLeafSize(0.01f, 0.01f, 0.01f);
	sor.filter(*cloud_filtered_blob);//�����˲�(�²���)��ĵ��Ʒ��õ�cloud_filtered_blob

	cout << cloud_filtered_blob->width * cloud_filtered_blob->height << endl;

	// ת��Ϊģ�����
	pcl::fromPCLPointCloud2(*cloud_filtered_blob, *cloud_filtered);//���²�����ĵ���PCLPointCloud2����ת��ΪPoinCloud����
	cout << "PointCloud after filtering: " << cloud_filtered->points.size() << " data points." << endl;
	// ���²���������ݴ������
	pcl::PCDWriter writer;
	writer.write<pcl::PointXYZ>("table_scene_lms400_downsampled.pcd", *cloud_filtered, false);
	pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients());
	pcl::PointIndices::Ptr inliers(new pcl::PointIndices());  //����һ��PointIndices�ṹ��ָ��
	// �����ָ����
	pcl::SACSegmentation<pcl::PointXYZ> seg;
	// ��ѡ
	seg.setOptimizeCoefficients(true); //���öԹ��Ƶ�ģ�����Ż�����
	// ��ѡ
	seg.setModelType(pcl::SACMODEL_PLANE);//���÷ָ�ģ�����
	seg.setMethodType(pcl::SAC_RANSAC);//����ʹ���ĸ�����������Ʒ���
	seg.setMaxIterations(1000);//��������
	seg.setDistanceThreshold(0.01);//�����Ƿ�Ϊģ���ڵ�ľ�����ֵ
	// �����˲�������
	pcl::ExtractIndices<pcl::PointXYZ> extract;
	int i = 0, nr_points = (int)cloud_filtered->points.size();
	// ��������30%ԭʼ��������ʱ
	while (cloud_filtered->points.size() > 0.3 * nr_points)
	{
		// �����µĵ����зָ����ƽ����ɲ���
		seg.setInputCloud(cloud_filtered);
		seg.segment(*inliers, *coefficients);
		if (inliers->indices.size() == 0)
		{
			cout << "Could not estimate a planar model for the given dataset." << endl;
			break;
		}
		// �����ڲ�
		extract.setInputCloud(cloud_filtered);
		extract.setIndices(inliers);
		extract.setNegative(false);
		extract.filter(*cloud_p);
		cout << "cloud_filtered: " << cloud_filtered->size() << endl;//�����ȡ֮��ʣ���
		cout << "----------------------------------" << endl;
		//����
		cout << "PointCloud representing the planar component: " << cloud_p->points.size() << " data points." << endl;
		std::stringstream ss;
		ss << "table_scene_lms400_plane_" << i << ".pcd"; //��ÿһ�ε���ȡ���������ļ�����
		writer.write<pcl::PointXYZ>(ss.str(), *cloud_p, false);
		// �����˲�������
		extract.setNegative(true);//��ȡ���
		extract.filter(*cloud_f);//��������ȡ������浽cloud_f
		cloud_filtered.swap(cloud_f);//��cloud_filtered��cloud_f����

		i++;
	}

	cout << "cloud_filtered: " << cloud_filtered->size() << endl;

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_seg1(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_seg2(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_voxel(new pcl::PointCloud<pcl::PointXYZ>);

	pcl::io::loadPCDFile("table_scene_lms400_plane_0.pcd", *cloud_seg1);
	pcl::io::loadPCDFile("table_scene_lms400_plane_1.pcd", *cloud_seg2);
	pcl::io::loadPCDFile("table_scene_lms400_downsampled.pcd", *cloud_voxel);
	/*//����ȡ�������ͳ��ѧ�˲�
	pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor1;
	sor1.setInputCloud(cloud_seg2);
	sor1.setMeanK(50);
	sor1.setStddevMulThresh(1);
	sor1.filter(*cloud_f);
	cout<<cloud_f->size()<<endl;*/

	pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer);
	viewer->initCameraParameters();

	int v1(0);
	viewer->createViewPort(0, 0, 0.25, 1, v1);
	viewer->setBackgroundColor(0, 0, 255, v1);
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> color1(cloud_voxel, 244, 89, 233);
	viewer->addPointCloud(cloud_voxel, color1, "cloud_voxel", v1);

	int v2(0);
	viewer->createViewPort(0.25, 0, 0.5, 1, v2);
	viewer->setBackgroundColor(0, 255, 255, v2);
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> color2(cloud_seg1, 244, 89, 233);
	viewer->addPointCloud(cloud_seg1, color2, "cloud_seg1", v2);

	int v3(0);
	viewer->createViewPort(0.5, 0, 0.75, 1, v3);
	viewer->setBackgroundColor(34, 128, 0, v3);
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> color3(cloud_seg2, 244, 89, 233);
	viewer->addPointCloud(cloud_seg2, color3, "cloud_seg2", v3);

	int v4(0);
	viewer->createViewPort(0.75, 0, 1, 1, v4);
	viewer->setBackgroundColor(0, 0, 255, v4);
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> color4(cloud_filtered, 244, 89, 233);
	viewer->addPointCloud(cloud_filtered, color4, "cloud_statical", v4);

	viewer->spin();
}

#include <pcl\filters\convolution_3d.h>
//��˹�˲�
void filterConvolution3D(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//��˹�˲������ø�˹������������Ҷ�任���Ծ��и�˹��������
	//��˹�˲�ƽ��Ч����С���˲���ͬʱ�ܽϺõر�������ԭò������ʹ��
	//��ĳһ������ǰ��n�����ݼ�Ȩƽ����Զ���ڲ�������ĵ㱻����Ϊ�̶��Ķ˵㣬������ʶ���϶�Ͷ˵�

	//���ڸ�˹�˺����ľ���˲�ʵ��
	pcl::filters::GaussianKernel<pcl::PointXYZ, pcl::PointXYZ> kernel;
	kernel.setSigma(4);//��׼��������������
	kernel.setThresholdRelativeToSigma(4);
	kernel.setThreshold(0.05);//������ֵ

	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	tree->setInputCloud(cloud);

	//����convolution��ز���
	pcl::filters::Convolution3D<pcl::PointXYZ, pcl::PointXYZ, pcl::filters::GaussianKernel<pcl::PointXYZ, pcl::PointXYZ>> convolution;
	convolution.setInputCloud(cloud);
	convolution.setKernel(kernel);
	convolution.setNumberOfThreads(6);
	convolution.setSearchMethod(tree);
	convolution.setRadiusSearch(0.01);
	convolution.convolve(*cloudFiltered);
}
#include <boost/random.hpp> //�����
//������Ӹ�˹����
void GenerateGaussNoise(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud, 
	pcl::PointCloud<pcl::PointXYZ>::Ptr& noiseCloud, double miu = 0, double sigma = 0.002)
{
	noiseCloud->points.resize(cloud->points.size());//������cloud��size��ֵ������ 
	noiseCloud->header = cloud->header;
	noiseCloud->width = cloud->width;
	noiseCloud->height = cloud->height;
	//ģ���������ɣ��˴������������µ㣬ֻ�ǽ�ԭ��ƫ����һ������
	boost::mt19937 seed;   // ���������
	seed.seed(static_cast<unsigned int>(time(0)));
	boost::normal_distribution<> nd(miu, sigma);  // ����һ�����ض�����ֵ�ͱ�׼�����̬�ֲ���
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<>> ok(seed, nd);
	for (size_t i = 0; i < cloud->size(); ++i)
	{
		noiseCloud->points[i].x = cloud->points[i].x + static_cast<float>(ok());
		noiseCloud->points[i].y = cloud->points[i].y + static_cast<float>(ok());
		noiseCloud->points[i].z = cloud->points[i].z + static_cast<float>(ok());
	}
}

#include <pcl\filters\bilateral.h>
//��ȡtxt��ת��Ϊ����---�ɵ���TXT��ȡ���ƺ�����ģ��
bool readTXT2PointCloud(const string& filePath, const char tag,
	pcl::PointCloud<pcl::PointXYZI>::Ptr& cloud)
{
	cout << "read in txt file start ..." << endl;
	ifstream fp(filePath);
	string lineStr;
	while (getline(fp, lineStr))
	{
		vector<string> strVec;
		string s;
		stringstream ss(lineStr);
		//tag == '';һ��ʹ�ÿո���ָ�
		while (getline(ss, s, tag))
		{
			strVec.emplace_back(s);
		}
		pcl::PointXYZI pt;
		pt.x = stod(strVec[0]);
		pt.y = stod(strVec[1]);
		pt.z = stod(strVec[2]);
		pt.intensity = stoi(strVec[3]);
		cloud->points.emplace_back(pt);
	}
	return true;
}
//˫���˲�
/*
	˫���˲�������ĵ��Ƹ�ʽ��Ҫ��
	--pcl::PointXYZI
*/
void filterBilateral(pcl::PointCloud<pcl::PointXYZI>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZI>::Ptr& cloudFiltered)
{
	//һ�ַ������˲������Դﵽ���ֱ�Ե��ͬʱ��ͼ�����ƽ����Ч����
	//����ʵ�飬ʱ�临�ӶȽϸߣ���ĵ��Ʋ��ʺϣ�
	//ȥ��Ч��Ч����Ҫ����ʵ�ʵ������������������Ҫƽ���ͱ�����Ե�����ʹ��
	//�޸ĵ��λ�ã�����ɾ����
	//pcl::BilateralFilter<PointT>ʵ�ֵ������ݵ�˫���˲�����Ҫʹ�õ��Ƶ�ǿ����Ϣ������pcd��ʽ����û��ǿ����Ϣ���ʴ�ֱ�Ӷ�ȡ�˰���intensity��TXT�ļ�
	//����ԭ�������Ϊ�α����������������±���
	//error LNK2001: �޷��������ⲿ���� "public: virtual void __cdecl pcl::BilateralFilter<struct pcl::PointXYZ>::applyFilter(class pcl::PointCloud<struct pcl::PointXYZ> &)"

	//����kdtree
	pcl::search::KdTree<pcl::PointXYZI>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZI>);
	//�����˲�
	pcl::BilateralFilter<pcl::PointXYZI>::Ptr bf(new pcl::BilateralFilter<pcl::PointXYZI>);
	bf->setInputCloud(cloud);
	bf->setSearchMethod(tree);//�����ķ�ʽ���в���
	bf->setHalfSize(0.1);//���ø�˹˫���˲�����һ��Ĵ�С
	bf->setStdDev(0.03);//���ñ�׼�����
	bf->filter(*cloudFiltered);
}
#include <pcl\filters\fast_bilateral.h>
//����˫���˲�
/*	
	��Ҫע����ǣ����˲���������Ʊ���������֯�ģ�Ҳ��������ͼƬ�������տ������ŵġ�
	��˸��˲�һ���ǶԴ�rgbd���ɵĵ��ƽ��д����
	����ĵ��Ƹ�ʽ--pcl::PointXYZRGB--pcl::PointXYZRGBA
*/
void filterBilateralEx(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloudFiltered)
{
	//�Կռ��ϵ��ǿ�Ƚ����²����Ӷ�����
	//�뾭���˫���˲�����ȣ�������ͬʱ��÷�����׼ȷ
	//��ά�ռ��£�˫���˲������Ա�ʾΪ�򵥵ķ����Ծ��
	//��ά�ռ��¡��²����Ծ�����Ӱ�첻�����ֽ��Ƽ��������ڿ�������ͬʱ��ʹ�ٶ�������������

	pcl::FastBilateralFilter<pcl::PointXYZRGB> fbf;
	fbf.setInputCloud(cloud);
	fbf.setSigmaS(5);
	fbf.setSigmaR(.03f);// ����˫���˲������ڿռ�����/���ڵĸ�˹�ı�׼��
	fbf.filter(*cloudFiltered);// ���ø�˹�ı�׼��Կ�������ǿ�Ȳ�(�����ǵ�����������)���������ر���Ȩ�ĳ̶�
}
#include <pcl\filters\fast_bilateral_omp.h>
//���߳̿���˫���˲�
void filterBilateralExOMP(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloudFiltered)
{
	//ʵ��������֯�ĵ�����ƽ�������Ϣ���߳̿���˫���˲�
	//--
	pcl::FastBilateralFilterOMP<pcl::PointXYZRGB> fbf;
	fbf.setInputCloud(cloud);
	fbf.setSigmaS(5);
	fbf.setSigmaR(.03f);
	fbf.setNumberOfThreads(6);//�����߳�����
	fbf.filter(*cloudFiltered);
}

#include <pcl\filters\median_filter.h>
//��ֵ�˲�
void filterMedian(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloudFiltered)
{
	//��򵥺͹㷺Ӧ�õ�ͼ�����˲���
	//ʹ���������ڵ������ݵ�ͳ����ֵ��������������ë�̣�Ч���Ϻ�
	//�Ա˴˿����Ļ��ӵ������˳�Ч������
	//ע�⣺���㷨ֻ����--����֯��PointXYZRGB--δת��(�Լ��趨��ȸ߶ȼ��Ƿ�dense)�ģ���������꣩����ȣ�z������
	//�ṩδ��֯�ĵ��Ƹ���ʵ�������

	pcl::MedianFilter<pcl::PointXYZRGB> m;
	m.setInputCloud(cloud);
	m.setWindowSize(10);
	m.setMaxAllowedMovement(.1f);//һ������������z���ƶ���������
	m.filter(*cloudFiltered);
}

#include <pcl\features\normal_3d_omp.h>//ʹ��OMP����3D����
#include <pcl\filters\sampling_surface_normal.h>//�������ķ�������Ϣ
#include <pcl\filters\shadowpoints.h>
//�Ƴ���Ե��������
void filterShadowPoints(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//shadowpoints�Ƴ������ڱ�Ե�Ĳ��������ϵ������
	//��Ҫ����ĵ����������Ϣ

	//OMP����
	pcl::NormalEstimationOMP<pcl::PointXYZ, pcl::PointNormal> omp;
	omp.setInputCloud(cloud);
	omp.setNumberOfThreads(6);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);//����kd�������н��ڵ�����
	omp.setSearchMethod(tree);
	omp.setKSearch(10);//���Ʒ���������ʱ����Ҫ�����Ľ�������
	pcl::PointCloud<pcl::PointNormal>::Ptr normals(new pcl::PointCloud<pcl::PointNormal>);//�нӷ����������������
	omp.compute(*normals);
	//��ʼ�˲�
	pcl::ShadowPoints<pcl::PointXYZ, pcl::PointNormal> sp(true);//��������extract removed indices
	sp.setInputCloud(cloud);
	sp.setThreshold(.01f);
	sp.setNormals(normals);
	sp.filter(*cloudFiltered);
}


//��ʾ����--������
void visualizeFilteredCloudSingle(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//������ʾ
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	viewer->setBackgroundColor(0, 0, 0);
	pcl::visualization::PointCloudColorHandlerCustom<PointT> view(cloud, 0, 0, 255);
	viewer->addPointCloud<PointT>(cloud, view, "Raw point clouds");
	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "Raw point clouds");
	pcl::visualization::PointCloudColorHandlerCustom<PointT> viewFiltered(cloudFiltered, 255, 0, 0);
	viewer->addPointCloud<PointT>(cloudFiltered, viewFiltered, "filtered point clouds");
	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "filtered point clouds");
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}
//��ʾ����--˫����
void visualizeFilteredCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	//PCL���ӻ�����
	boost::shared_ptr<pcl::visualization::PCLVisualizer> view(new pcl::visualization::PCLVisualizer("ShowClouds"));
	int v1 = 0, v2 = 0;
	view->createViewPort(.0, .0, .5, 1., v1);
	view->setBackgroundColor(0., 0., 0., v1);
	view->addText("Raw point clouds", 10, 10, "text1", v1);
	view->createViewPort(.5, .0, 1., 1., v2);
	view->setBackgroundColor(0., 0., 0., v2);
	view->addText("filtered point clouds", 10, 10, "text2", v2);

	view->addPointCloud<pcl::PointXYZ>(cloud, "Raw point clouds", v1);
	view->addPointCloud<pcl::PointXYZ>(cloudFiltered, "filtered point clouds", v2);
	//���õ��Ƶ���Ⱦ����,string &id = "cloud"�൱�ڴ���ID
	view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 1, 0, "Raw point clouds", v1);//���õ�����ɫ
	view->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 1, 0, "filtered point clouds", v2);
	//view->addCoordinateSystem(1.0);
	//view->initCameraParameters();
	while (!view->wasStopped())
	{
		view->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}
//��ʾ����--˫����
void visualizeFilteredCloud2(pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
	pcl::PointCloud<pcl::PointXYZ>::Ptr& cloudFiltered)
{
	pcl::visualization::PCLVisualizer viewer("Cloud Viewer");
	//--------����������ʾ���ڲ����ñ�����ɫ------------
	viewer.setBackgroundColor(0, 0, 0);
	//-----------�����������ɫ-------------------------
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> in_h(cloud, 0, 255, 0);
	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> no_h(cloudFiltered, 255, 0, 0);
	//----------��ӵ��Ƶ���ʾ����----------------------
	viewer.addPointCloud(cloud, in_h, "cloud_in");
	viewer.addPointCloud(cloudFiltered, no_h, "cloud_out");

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}


//------------------------------------------------------------------------------------------------//

int maininstance()
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloudFiltered(new pcl::PointCloud<pcl::PointXYZ>);

	string cloudPath = "../bunny.pcd";
	if (pcl::io::loadPCDFile(cloudPath, *cloud) != 0)
		return -1;
	//pcl::PCDReader reader;
	//reader.read(cloudPath, *cloud);	
	cout << "���ص���������" << cloud->points.size() << "��" << endl;

	//------------------------------��������---------------------------------------------------
	//������ȡ
	//vector<int> extr{ 0, 1, 2, 3, 4, 5, 6, 7 };
	//extractPointIdxs(extr, cloud, cloudFiltered);
	//ֱͨ�˲���
	//filterPassThrough(cloud, cloudFiltered);
	//�����˲���
	//filterVoxelGrid(cloud, cloudFiltered);
	//filterApproximateVoxelGrid(cloud, cloudFiltered);
	//�Ľ�K���������˲���
	//filterVoxelGridEx(cloud, cloudFiltered);
	//ͳ���˲���
	//filterStddevMulThresh(cloud, cloudFiltered);
	//�뾶�˲���
	//filterRadius(cloud, cloudFiltered);
	//�����˲���
	//filterCondition(cloud, cloudFiltered);
	//ģ���˲�
	//filterModel(cloud, cloudFiltered);
	//ͶӰ�˲���
	//filterProject(cloud, cloudFiltered);
	//filterProjectEx(cloud, cloudFiltered);
	//��˹�˲�
	//filterConvolution3D(cloud, cloudFiltered);
	//GenerateGaussNoise(cloud, cloudFiltered);
	//˫���˲�
	//filterBilateral(cloud, cloudFiltered);
	//����˫���˲�
	//filterBilateralEx(cloud, cloudFiltered);
	//���߳̿���˫���˲�
	//filterBilateralExOMP(cloud, cloudFiltered);
	//��ֵ�˲�
	//filterMedian(cloud, cloudFiltered);
	//�Ƴ���Ե��������
	filterShadowPoints(cloud, cloudFiltered);
	//-------------------------------------------------------------------------------------

	cout << "�˲�����������" << cloudFiltered->points.size() << "��" << endl;
	//pcl::io::savePCDFile("output.pcd", *cloudFiltered);
	//pcl::PCDWriter writer;
	//writer.write<pcl::PointXYZ>("output.pcd", *cloudFiltered, false);

	visualizeFilteredCloud(cloud, cloudFiltered);
	//visualizeFilteredCloud2(cloud, cloudFiltered);
	//visualizeFilteredCloudSingle(cloud, cloudFiltered);

	return 0;
}