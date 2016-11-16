// 20160926 cv::line�Ō��Ǐd��

#include<myopencv.h>
#include<fstream>
#include<numeric>

using namespace std;
using namespace cv;

#define IMAGE_VALUE_THRESOLD 5 //���x臒l
#define MARKERPOINT_BUF 15 //imageCoodinate_Marker�o�b�t�@��

//--------------------
//�}�E�X�R�[���o�b�N�֐�
//�N���b�N�����_��ǐՊJ�n
//--------------------
void onMouse(int event, int x, int y, int flags, void* param) {

	vector<Point2f>* seed_point = (vector<Point2f>*)param;

	// ���{�^���N���b�N�ŒǐՊJ�n
	if (event == CV_EVENT_LBUTTONDOWN) {
		seed_point->push_back(Point2f(x, y));
	}

}


//--------------------
//�̈�g���̊֐�
//��q�_�̎���25�_�A�̈�̏d�S��Ԃ�
//--------------------
Point2f RegionGrowing(Point2f seed, Mat* img) {

	Mat hsv_img;
	cvtColor(*img, hsv_img, CV_BGR2HSV);

	//�}�X�N�摜
	Mat mask(hsv_img.rows, hsv_img.cols, CV_8UC1);
	mask = Scalar(0);

	//���ړ_�̃X�^�b�N
	vector<Point2f> stack;
	stack.push_back(seed);

	//������f�A�N�Z�X�pdd
	vector<Point2f> dd;
	for (int a = 0; a < 5; a++) {
		for (int b = 0; b < 5; b++) {
			dd.push_back(Point2f(a - 2, b - 2));
		}
	}

	Vec3b p_hsv;
	p_hsv = hsv_img.at<Vec3b>(seed);
	Point2f c_point = Point2f(0, 0);
	int c_count = 0;

	// �X�^�b�N����ɂȂ�܂ŌJ��Ԃ�
	while (!stack.empty()) {

		// ���ړ_�����o��
		Point2f p = stack.back();
		stack.pop_back();

		//�܂��}�[�J�����Ă��Ȃ��ꍇ
		if (mask.at<unsigned char>(p) < 255) {

			//�}�[�J������
			mask.at<unsigned char>(p) = 255;

			c_point += p;
			c_count++;

#ifdef _OPENMP
#pragma omp parallel for
#endif

			// ����25�_�Ɋg��
			for (int i = 0; i < dd.size(); i++) {

				// �����𖞂����Ƃ��C�X�^�b�N�Ɋi�[
				Point2f pp = p + dd.at(i);
				if (pp.x < 0 || pp.y < 0 || pp.x >= hsv_img.cols || pp.y >= hsv_img.rows) continue;
				if (mask.at<unsigned char>(pp) == 255) continue;
				Vec3b r = hsv_img.at<Vec3b>(pp);

				if (r[0] > p_hsv[0] - IMAGE_VALUE_THRESOLD && r[0] < p_hsv[0] + IMAGE_VALUE_THRESOLD) {
					stack.push_back(pp);
				}
			}
		}
	}


	// �̈�̏d�S�_��Ԃ�
	c_point = c_point / c_count;
	if (c_point.x < 0 || c_point.y < 0 || c_point.x >= hsv_img.cols || c_point.y >= hsv_img.rows) {
		return seed;
	}
	else {
		return c_point;
	}

}


//--------------------
//���C���֐�
//--------------------
int main(void) {

	//----------
	//�O���t�@�C���ǂݍ���
	//----------

	string buf;
	
	//�J���������p�����[�^�A�c�ݓǂݍ���
	Mat intrinsic(3, 3, CV_32FC1);
	Mat distCoefs(1, 5, CV_32FC1);

	ifstream ifs_cameraParam("mycamera.txt");
	if (!ifs_cameraParam) {
		cout << "�J���������p�����[�^�t�@�C���̓ǂݍ��݂Ɏ��s" << endl;
		cin.get();
		exit(1);
	}

	buf.clear();
	getline(ifs_cameraParam, buf);
	getline(ifs_cameraParam, buf);
	sscanf_s(buf.data(), "[%f,%f,%f", &intrinsic.at<float>(0,0), &intrinsic.at<float>(0,1), &intrinsic.at<float>(0,2));
	getline(ifs_cameraParam, buf);
	sscanf_s(buf.data(), "%f,%f,%f", &intrinsic.at<float>(1, 0), &intrinsic.at<float>(1, 1), &intrinsic.at<float>(1, 2));
	getline(ifs_cameraParam, buf);
	sscanf_s(buf.data(), "%f,%f,%f]", &intrinsic.at<float>(2, 0), &intrinsic.at<float>(2, 1), &intrinsic.at<float>(2, 2));
	getline(ifs_cameraParam, buf);
	getline(ifs_cameraParam, buf);
	sscanf_s(buf.data(), "[%f,%f,%f,%f,%f]", &distCoefs.at<float>(0, 0), &distCoefs.at<float>(0, 1), &distCoefs.at<float>(0, 2), &distCoefs.at<float>(0, 3), &distCoefs.at<float>(0, 4));

	cout << "INTRINSIC" << endl << intrinsic << endl;
	cout << "DISTCOEFS" << endl << distCoefs << endl;

	ifs_cameraParam.close();


	//�������(CT)���W�ǂݍ���
	vector<Point3f> worldCoodinate_Marker, worldCoodinate_Cancer, worldCoodinate_Vein;
	vector<Point2i> linePairs_Vein;

	ifstream ifs_worldCoodinate("worldCoodinate20160929.txt");
	if (!ifs_worldCoodinate) {
		cout << "���E���W�t�@�C���̓ǂݍ��݂Ɏ��s" << endl;
		cin.get();
		exit(1);
	}

	int num, i;
	Point3f coodinate3_buf;
	Point2i pairs_buf;

	buf.clear();
	getline(ifs_worldCoodinate, buf);
	getline(ifs_worldCoodinate, buf);
	sscanf_s(buf.data(), "%d", &num);
	for (int count = 0; count < num; count++) {
		getline(ifs_worldCoodinate, buf);
		sscanf_s(buf.data(), "%d,%f,%f,%f", &i, &coodinate3_buf.x, &coodinate3_buf.y, &coodinate3_buf.z);
		coodinate3_buf.z = coodinate3_buf.z *0.63993;
		worldCoodinate_Marker.push_back(coodinate3_buf);
	}
	getline(ifs_worldCoodinate, buf);
	getline(ifs_worldCoodinate, buf);
	sscanf_s(buf.data(), "%d", &num);
	for (int count = 0; count < num; count++) {
		getline(ifs_worldCoodinate, buf);
		sscanf_s(buf.data(), "%d,%f,%f,%f", &i, &coodinate3_buf.x, &coodinate3_buf.y, &coodinate3_buf.z);
		coodinate3_buf.z = coodinate3_buf.z *0.63993;
		worldCoodinate_Cancer.push_back(coodinate3_buf);
	}
	getline(ifs_worldCoodinate, buf);
	getline(ifs_worldCoodinate, buf);
	sscanf_s(buf.data(), "%d", &num);
	for (int count = 0; count < num; count++) {
		getline(ifs_worldCoodinate, buf);
		sscanf_s(buf.data(), "%d,%f,%f,%f", &i, &coodinate3_buf.x, &coodinate3_buf.y, &coodinate3_buf.z);
		coodinate3_buf.z = coodinate3_buf.z *0.63993;
		worldCoodinate_Vein.push_back(coodinate3_buf);
	}
	getline(ifs_worldCoodinate, buf);
	getline(ifs_worldCoodinate, buf);
	sscanf_s(buf.data(), "%d", &num);
	for (int count = 0; count < num; count++) {
		getline(ifs_worldCoodinate, buf);
		sscanf_s(buf.data(), "%d,%d,%d", &i, &pairs_buf.x, &pairs_buf.y);
		linePairs_Vein.push_back(pairs_buf);
	}

	cout << "worldCoodinate_Marker" << endl << worldCoodinate_Marker << endl;
	cout << "worldCoodinate_Cancer" << endl << worldCoodinate_Cancer << endl;
	cout << "worldCoodinate_Vein" << endl << worldCoodinate_Vein << endl;

	ifs_worldCoodinate.close();


	//----------
	//�C���^�[�t�F�[�X����
	//----------

	//�J�����ڑ�
	int cam_num;

	cout << "�J�����ԍ��F";
	cin >> cam_num;
	VideoCapture cap(cam_num);
	if (!cap.isOpened()) {
		cout << "�J�����ڑ��Ɏ��s" << endl;
		cin.get();
		exit(1);
	}


	//�\����ʐ���
	vector<Point2f> seed_point;
	
	namedWindow("Image Superimposer");
	setMouseCallback("Image Superimposer", onMouse, &seed_point);


	//----------
	//���[�v����
	//----------
	
	vector<Point2f> imageCoodinate_Marker, imageCoodinate_Cancer, imageCoodinate_Vein;
	vector<vector<Point2f>> buf_imageCoodinate_Marker;


	int key = 0;
	while (key != 27) {

		Mat input_img;
		cap >> input_img;

		//�}�[�J�[�ǐ�
		if (seed_point.size() > worldCoodinate_Marker.size()) {
			seed_point.resize(worldCoodinate_Marker.size());
		}
		imageCoodinate_Marker.resize(seed_point.size());
		buf_imageCoodinate_Marker.resize(seed_point.size());
		for (i = 0; i < seed_point.size(); i++) {
			seed_point[i] = RegionGrowing(seed_point[i], &input_img);
			buf_imageCoodinate_Marker[i].push_back(seed_point[i]);
			Point2f sum_point;
			for (int j = 0; j < buf_imageCoodinate_Marker[i].size(); j++) {
				sum_point += buf_imageCoodinate_Marker[i][j];
			}
			imageCoodinate_Marker[i] = sum_point / (float)buf_imageCoodinate_Marker[i].size();
			if (buf_imageCoodinate_Marker[i].size() > MARKERPOINT_BUF) {
				buf_imageCoodinate_Marker[i].erase(buf_imageCoodinate_Marker[i].begin());
			}

			// �}�[�J�[�ʒu�̕`��
			circle(input_img, imageCoodinate_Marker[i], 10, Scalar(0, 255, 255), 2);
			cv::putText(input_img, format("%d", i + 1), imageCoodinate_Marker[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0), 1, CV_AA);
		}

		//��ᇈʒu�A���ǈʒu�`��
		if (imageCoodinate_Marker.size() == worldCoodinate_Marker.size()) {
			Mat rvec, tvec;
			solvePnPRansac(worldCoodinate_Marker, imageCoodinate_Marker, intrinsic, distCoefs, rvec, tvec, false);
			
			//����
			projectPoints(worldCoodinate_Vein, rvec, tvec, intrinsic, distCoefs, imageCoodinate_Vein);
			for (int b = 0; b < linePairs_Vein.size(); b++) {
				line(input_img, imageCoodinate_Vein[linePairs_Vein[b].x], imageCoodinate_Vein[linePairs_Vein[b].y], Scalar(255, 0, 0), 7 );
			}

			//���
			projectPoints(worldCoodinate_Cancer, rvec, tvec, intrinsic, distCoefs, imageCoodinate_Cancer);
			for (int a = 0; a < imageCoodinate_Cancer.size(); a++) {
				circle(input_img, imageCoodinate_Cancer[a], 15, Scalar(0, 0, 255), -1);
			}
		}


		

		//��ʕ\��
		imshow("Image Superimposer",input_img);

		//�L�[����
		key = waitKey(1);
		switch (key) {
		case 'b':
			if(!seed_point.empty())
			seed_point.pop_back();
			break;
		default:
			break;
		}
	}

	

	return 0;
}