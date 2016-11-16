
// ヘッダーファイル
#include <opencv2/opencv.hpp>

// バージョン取得
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

// ビルドモード
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif

// ライブラリのリンク（不要な物はコメントアウト）
#pragma comment(lib, "opencv_core"          CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_highgui"       CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_imgproc"		CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_calib3d"		CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_viz"			CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_objdetect"	CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_gpu"			CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_features2d"	CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_flann"		CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_contrib"		CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_ml"			CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_legacy"		CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_nonfree"		CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_photo"		CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_stitching"	CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_videostab"	CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_videoio"	CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_imgcodecs"	CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_video"	CV_VERSION_STR CV_EXT_STR)

////vtk
//
//#pragma comment(lib, "vtkCommonCore-7.0.lib")
//#pragma comment(lib, "vtkRenderingCore-7.0.lib")
//#pragma comment(lib, "vtkalglib-7.0.lib")
//#pragma comment(lib, "vtkIOGeometry-7.0.lib")
//#pragma comment(lib, "vtkCommonExecutionModel-7.0.lib")
////#pragma comment(lib, "vtk-7.0.lib")


