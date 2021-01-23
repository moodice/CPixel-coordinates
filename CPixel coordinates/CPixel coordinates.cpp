/******************************/
/*        ����ƥ��Ͳ��        */
/******************************/

#include <opencv2/opencv.hpp>  
#include <iostream>  
#include "opencv2/imgcodecs/legacy/constants_c.h"
#include <opencv2\imgproc\types_c.h>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

const int imageWidth = 1024;                             //����ͷ�ķֱ���  
const int imageHeight = 768;
Size imageSize = Size(imageWidth, imageHeight);

Mat rgbImageL, grayImageL;
Mat rgbImageR, grayImageR;
Mat rectifyImageL, rectifyImageR;

Rect validROIL;//ͼ��У��֮�󣬻��ͼ����вü��������validROI����ָ�ü�֮�������  
Rect validROIR;

Mat mapLx, mapLy, mapRx, mapRy;     //ӳ���  
Mat Rl, Rr, Pl, Pr, Q;              //У����ת����R��ͶӰ����P ��ͶӰ����Q
Mat xyz;              //��ά����

Point origin;         //��갴�µ���ʼ��
Rect selection;      //�������ѡ��
bool selectObject = false;    //�Ƿ�ѡ�����

int blockSize = 0, uniquenessRatio = 0, numDisparities = 0;
Ptr<StereoBM> bm = StereoBM::create(16, 9);

/*
���ȱ궨�õ�����Ĳ���
fx 0 cx
0 fy cy
0 0  1
*/
Mat cameraMatrixL = (Mat_<double>(3, 3) << 
    3043.965495, 1.816886326, 646.6591164,
    0, 3062.429893, 318.6606484,
    0, 0, 1
    );
/*3043.965495,0,0,
1.816886326,3062.429893,0,
646.6591164,318.6606484,1
3043.965495, 1.816886326,646.6591164,
    0,3062.429893,318.6606484,
    0,0,1*/


//����(k1,k2,p1,p2,k3)
Mat distCoeffL = (Mat_<double>(5, 1) << 1, 0, 0, 0, 0
    );
//0.611568177,-119.2323681,6980.461906,-0.009666281,0.009310385
//0.611568177,-119.2323681, -0.009666281,0.009310385,6980.461906
//-0.009666281,0.009310385,0.611568177,-119.2323681,6980.461906



Mat cameraMatrixR = (Mat_<double>(3, 3) << 
    3048.550122, -9.738518169, 544.1579293,
    0, 3068.422702, 230.6478814,
    0, 0, 1
    );
/*3048.550122,0,0,
-9.738518169,3068.422702,0,
544.1579293,230.6478814,1
3048.550122, -9.738518169,544.1579293,
    0,3068.422702,230.6478814,
    0,0,1
*/
Mat distCoeffR = (Mat_<double>(5, 1) << 0.3285412648190094, -13.96276464161761, -0.03000457328682077, -0.02636697924848531, 130.1467536974576
    );
//0.096951793,-1.340593865,-32.78274225,-0.01879748,0.006495002 
//0.096951793,-1.340593865, -0.01879748,0.006495002,-32.78274225
//-0.01879748,0.006495002,0.096951793,-1.340593865,-32.78274225


Mat T = (Mat_<double>(3, 1) << -914.5883933, -60.02969331, 14.41187854);//Tƽ������ -914.5883933 ,-60.02969331,14.41187854
//Mat rec = (Mat_<double>(3, 1) << -0.00306, -0.03207, 0.00206);//rec��ת����
Mat R=(Mat_<double>(3,3)<< 
    0.997221835, -0.065171435, 0.036073491,
    0.066350242, 0.997266768, 0.032505968,
    -0.033856433, -0.034809146, 0.998820337
    );//R ��ת����
/*0.997221835,0.065171435,-0.036073491,
    -0.066350242,0.997266768,-0.032505968,
    0.033856433,0.034809146,0.998820337
    1,0,0,
    0,1,0,
    0,0,1*/

/*****����ƥ��*****/
//void stereo_match(int, void*)
//{
//    bm->setBlockSize(2 * blockSize + 5);     //SAD���ڴ�С��5~21֮��Ϊ��
//    bm->setROI1(validROIL);
//    bm->setROI2(validROIR);
//    bm->setPreFilterCap(31);
//    bm->setMinDisparity(0);  //��С�ӲĬ��ֵΪ0, �����Ǹ�ֵ��int��
//    bm->setNumDisparities(numDisparities * 16 + 16);//�Ӳ�ڣ�������Ӳ�ֵ����С�Ӳ�ֵ֮��,���ڴ�С������16����������int��
//    bm->setTextureThreshold(10);
//    bm->setUniquenessRatio(uniquenessRatio);//uniquenessRatio��Ҫ���Է�ֹ��ƥ��
//    bm->setSpeckleWindowSize(100);
//    bm->setSpeckleRange(32);
//    bm->setDisp12MaxDiff(-1);
//    Mat disp, disp8;
//    bm->compute(rectifyImageL, rectifyImageR, disp);//����ͼ�����Ϊ�Ҷ�ͼ
//    disp.convertTo(disp8, CV_8U, 255 / ((numDisparities * 16 + 16) * 16.0));//��������Ӳ���CV_16S��ʽ
//    reprojectImageTo3D(disp, xyz, Q, true); //��ʵ�������ʱ��ReprojectTo3D������X / W, Y / W, Z / W��Ҫ����16(Ҳ����W����16)�����ܵõ���ȷ����ά������Ϣ��
//    xyz = xyz * 16;
//    imshow("disparity", disp8);
//}

/*****�������������ص�*****/
//static void onMouse(int event, int x, int y, int, void*)
//{
//    if (selectObject)
//    {
//        selection.x = MIN(x, origin.x);
//        selection.y = MIN(y, origin.y);
//        selection.width = std::abs(x - origin.x);
//        selection.height = std::abs(y - origin.y);
//    }
//
//    switch (event)
//    {
//    case EVENT_LBUTTONDOWN:   //�����ť���µ��¼�
//        origin = Point(x, y);
//        selection = Rect(x, y, 0, 0);
//        selectObject = true;
//        cout << origin << "in world coordinate is: " << xyz.at<Vec3f>(origin) << endl;
//        break;
//    case EVENT_LBUTTONUP:    //�����ť�ͷŵ��¼�
//        selectObject = false;
//        if (selection.width > 0 && selection.height > 0)
//            break;
//    }
//}


/*****������*****/
int main()
{
    /*
    ����У��
    */
    //Rodrigues(rec, R); //Rodrigues�任
    stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R, T, Rl, Rr, Pl, Pr, Q, CALIB_ZERO_DISPARITY,
        0, imageSize, &validROIL, &validROIR);
    initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pr, imageSize, CV_32FC1, mapLx, mapLy);
    initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

    /*
    ��ȡͼƬ
    */
    rgbImageL = imread("D:\\���\\CPixel coordinates\\Right\\4880.png", CV_LOAD_IMAGE_COLOR);//Right
    cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);
    rgbImageR = imread("D:\\���\\CPixel coordinates\\Left\\4880.png", CV_LOAD_IMAGE_COLOR);//Left
    cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

    imshow("ImageL Before Rectify", grayImageL);
    imshow("ImageR Before Rectify", grayImageR);

    /*
    ����remap֮�����������ͼ���Ѿ����沢���ж�׼��
    */
    remap(grayImageL, rectifyImageL, mapLx, mapLy, INTER_LINEAR);
    remap(grayImageR, rectifyImageR, mapRx, mapRy, INTER_LINEAR);

    /*
    ��У�������ʾ����
    */
    Mat rgbRectifyImageL, rgbRectifyImageR;
    cvtColor(rectifyImageL, rgbRectifyImageL, CV_GRAY2BGR);  //α��ɫͼ CV_GRAY2BGR
    cvtColor(rectifyImageR, rgbRectifyImageR, CV_GRAY2BGR);//CV_GRAY2BGR
    
    //������ʾ
   /* rectangle(rgbRectifyImageL, validROIL, Scalar(0, 0, 255), 3, 8);
    rectangle(rgbRectifyImageR, validROIR, Scalar(0, 0, 255), 3, 8);*/
    imshow("ImageL After Rectify", rgbRectifyImageL);
    imshow("ImageR After Rectify", rgbRectifyImageR);

    //��ʾ��ͬһ��ͼ��
    Mat canvas;
    double sf;
    int w, h;
    sf =  600.0 / MAX(imageSize.width, imageSize.height);
    w = cvRound(imageSize.width * sf);
    h = cvRound(imageSize.height * sf);
    canvas.create(h, w * 2, CV_8UC3);   //ע��ͨ��

    //��ͼ�񻭵�������
    Mat canvasPart = canvas(Rect(w * 0, 0, w, h));                                //�õ�������һ����  
    resize(rgbRectifyImageL, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);     //��ͼ�����ŵ���canvasPartһ����С  
    Rect vroiL(cvRound(validROIL.x * sf), cvRound(validROIL.y * sf),                //��ñ���ȡ������    
        cvRound(validROIL.width * sf), cvRound(validROIL.height * sf));
    //rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);                      //����һ������  
    cout << "Painted ImageL" << endl;

    //��ͼ�񻭵�������
    canvasPart = canvas(Rect(w, 0, w, h));                                      //��û�������һ����  
    resize(rgbRectifyImageR, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
    Rect vroiR(cvRound(validROIR.x * sf), cvRound(validROIR.y * sf),
        cvRound(validROIR.width * sf), cvRound(validROIR.height * sf));
    //rectangle(canvasPart, vroiR, Scalar(0, 0, 255), 3, 8);
    cout << "Painted ImageR" << endl;

    //���϶�Ӧ������
    for (int i = 0; i < canvas.rows; i += 16)
        line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);
    imshow("rectified", canvas);

    /*
    ����ƥ��
    */
   /* namedWindow("disparity", CV_WINDOW_AUTOSIZE);
     //����SAD���� Trackbar
    createTrackbar("BlockSize:\n", "disparity", &blockSize, 8, stereo_match);
     //�����Ӳ�Ψһ�԰ٷֱȴ��� Trackbar
    createTrackbar("UniquenessRatio:\n", "disparity", &uniquenessRatio, 50, stereo_match);
     //�����Ӳ�� Trackbar
    createTrackbar("NumDisparities:\n", "disparity", &numDisparities, 16, stereo_match);
    //�����Ӧ����setMouseCallback(��������, ���ص�����, �����ص������Ĳ�����һ��ȡ0)
    setMouseCallback("disparity", onMouse, 0);
    stereo_match(0, 0);*/

    waitKey(0);
    return 0;
}
//#include "pch.h"
//#include <iostream>
//#include <math.h>
//#include <opencv2/opencv.hpp>
//
//using namespace std;
//using namespace cv;
//
//int main()
//{
//    Mat img1 = imread("D:\\���\\CPixel coordinates\\Left\\4890.png");
//    Mat img2 = imread("D:\\���\\CPixel coordinates\\Right\\4890.png");
//    if (img1.empty() || img2.empty())
//    {
//        cout << "����ͼƬʧ�ܣ������Ӧ·��ͼƬ�Ƿ���ڣ�" << endl;
//        return 1;
//    }
//    imshow("src1", img1);
//    imshow("src2", img2);
//    int w1 = img1.cols; int h1 = img1.rows;
//    int w2 = img2.cols; int h2 = img2.rows;
//    int width = w1 + w2; int height = max(h1, h2);
//    Mat  resultImg = Mat(height, width, CV_8UC3, Scalar::all(0));
//    Mat ROI_1 = resultImg(Rect(0, 0, w1, h1));
//    Mat ROI_2 = resultImg(Rect(w1, 0, w2, h2));
//    img1.copyTo(ROI_1);
//    img2.copyTo(ROI_2);
//    imshow("result", resultImg);
//    imwrite("result.jpg", resultImg);
//    waitKey(0);
//    return 0;
//}