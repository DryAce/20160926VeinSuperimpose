// 20160926 cv::lineで血管重畳

#include<myopencv.h>
#include<fstream>
#include<numeric>

using namespace std;
using namespace cv;

#define IMAGE_VALUE_THRESOLD 5 //明度閾値
#define MARKERPOINT_BUF 15 //imageCoodinate_Markerバッファ数

//--------------------
//マウスコールバック関数
//クリックした点を追跡開始
//--------------------
void onMouse(int event, int x, int y, int flags, void* param) {

	vector<Point2f>* seed_point = (vector<Point2f>*)param;

	// 左ボタンクリックで追跡開始
	if (event == CV_EVENT_LBUTTONDOWN) {
		seed_point->push_back(Point2f(x, y));
	}

}


//--------------------
//領域拡張の関数
//種子点の周囲25点、領域の重心を返す
//--------------------
Point2f RegionGrowing(Point2f seed, Mat* img) {

	Mat hsv_img;
	cvtColor(*img, hsv_img, CV_BGR2HSV);

	//マスク画像
	Mat mask(hsv_img.rows, hsv_img.cols, CV_8UC1);
	mask = Scalar(0);

	//注目点のスタック
	vector<Point2f> stack;
	stack.push_back(seed);

	//差分画素アクセス用dd
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

	// スタックが空になるまで繰り返す
	while (!stack.empty()) {

		// 注目点を取り出す
		Point2f p = stack.back();
		stack.pop_back();

		//まだマーカがついていない場合
		if (mask.at<unsigned char>(p) < 255) {

			//マーカをつける
			mask.at<unsigned char>(p) = 255;

			c_point += p;
			c_count++;

#ifdef _OPENMP
#pragma omp parallel for
#endif

			// 周辺25点に拡張
			for (int i = 0; i < dd.size(); i++) {

				// 条件を満たすとき，スタックに格納
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


	// 領域の重心点を返す
	c_point = c_point / c_count;
	if (c_point.x < 0 || c_point.y < 0 || c_point.x >= hsv_img.cols || c_point.y >= hsv_img.rows) {
		return seed;
	}
	else {
		return c_point;
	}

}


//--------------------
//メイン関数
//--------------------
int main(void) {

	//----------
	//外部ファイル読み込み
	//----------

	string buf;
	
	//カメラ内部パラメータ、歪み読み込み
	Mat intrinsic(3, 3, CV_32FC1);
	Mat distCoefs(1, 5, CV_32FC1);

	ifstream ifs_cameraParam("mycamera.txt");
	if (!ifs_cameraParam) {
		cout << "カメラ内部パラメータファイルの読み込みに失敗" << endl;
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


	//現実空間(CT)座標読み込み
	vector<Point3f> worldCoodinate_Marker, worldCoodinate_Cancer, worldCoodinate_Vein;
	vector<Point2i> linePairs_Vein;

	ifstream ifs_worldCoodinate("worldCoodinate20160929.txt");
	if (!ifs_worldCoodinate) {
		cout << "世界座標ファイルの読み込みに失敗" << endl;
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
	//インターフェース準備
	//----------

	//カメラ接続
	int cam_num;

	cout << "カメラ番号：";
	cin >> cam_num;
	VideoCapture cap(cam_num);
	if (!cap.isOpened()) {
		cout << "カメラ接続に失敗" << endl;
		cin.get();
		exit(1);
	}


	//表示画面生成
	vector<Point2f> seed_point;
	
	namedWindow("Image Superimposer");
	setMouseCallback("Image Superimposer", onMouse, &seed_point);


	//----------
	//ループ処理
	//----------
	
	vector<Point2f> imageCoodinate_Marker, imageCoodinate_Cancer, imageCoodinate_Vein;
	vector<vector<Point2f>> buf_imageCoodinate_Marker;


	int key = 0;
	while (key != 27) {

		Mat input_img;
		cap >> input_img;

		//マーカー追跡
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

			// マーカー位置の描画
			circle(input_img, imageCoodinate_Marker[i], 10, Scalar(0, 255, 255), 2);
			cv::putText(input_img, format("%d", i + 1), imageCoodinate_Marker[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0), 1, CV_AA);
		}

		//腫瘍位置、血管位置描画
		if (imageCoodinate_Marker.size() == worldCoodinate_Marker.size()) {
			Mat rvec, tvec;
			solvePnPRansac(worldCoodinate_Marker, imageCoodinate_Marker, intrinsic, distCoefs, rvec, tvec, false);
			
			//血管
			projectPoints(worldCoodinate_Vein, rvec, tvec, intrinsic, distCoefs, imageCoodinate_Vein);
			for (int b = 0; b < linePairs_Vein.size(); b++) {
				line(input_img, imageCoodinate_Vein[linePairs_Vein[b].x], imageCoodinate_Vein[linePairs_Vein[b].y], Scalar(255, 0, 0), 7 );
			}

			//腫瘍
			projectPoints(worldCoodinate_Cancer, rvec, tvec, intrinsic, distCoefs, imageCoodinate_Cancer);
			for (int a = 0; a < imageCoodinate_Cancer.size(); a++) {
				circle(input_img, imageCoodinate_Cancer[a], 15, Scalar(0, 0, 255), -1);
			}
		}


		

		//画面表示
		imshow("Image Superimposer",input_img);

		//キー入力
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