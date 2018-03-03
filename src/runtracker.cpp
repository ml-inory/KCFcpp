#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"

#include <dirent.h>

using namespace std;
using namespace cv;

// Frame readed
Mat frame, moving_frame;

// 左上角点和右下角点
cv::Point tl_point, br_point;

// 窗口名称
std::string window_name("frame");

// 图像大小
cv::Size window_size(640, 360);

// 初始框设置完成标记
bool is_init = false;

// 鼠标回调
void on_mouse(int event, int x, int y, int flags, void* params)
{
	if (frame.empty())
		return;

	cv::Point cur_point = cv::Point(x, y);
	// cout << "x:" << x << endl;
	// cout << "y:" << y << endl;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		tl_point = cur_point;
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		moving_frame = frame.clone();
		cv::rectangle(moving_frame, tl_point, cur_point, Scalar(0,255,0), 2);
		// cv::imshow(window_name, moving_frame);
	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		br_point = cur_point;
		is_init = true;
	}
}

int main(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	// bool SILENT = true;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "fixed_window") == 0 )
			FIXEDWINDOW = true;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "show") == 0 )
			SILENT = false;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;
		}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}
	
	// Create KCFTracker object
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

	// Tracker results
	Rect result;

    // Video Capture
    cv::VideoCapture capture(0);

    if (!capture.isOpened())
    {
        cout << "Cannot open camera" << endl;
        return -1;
    }

    cv::namedWindow(window_name);
    // 设置回调
    cv::setMouseCallback(window_name, on_mouse);

    // 读图，等待1秒让isp调好
    for (int i = 0; i < 2; ++i)
    {
    	bool ret = capture.read(frame);
		if (!ret)
			return -1;
		cv::waitKey(500);
    }
    
	cv::resize(frame, frame, window_size);
	moving_frame = frame.clone();

    // 未设置好初始框则循环
    while (!is_init)
    {
    	cv::imshow(window_name, moving_frame);
    	if (cv::waitKey(40) == 'q')
            return -1;
    }

    cv::Rect init_rect(tl_point, br_point);
    // 初始化跟踪器
    tracker.init(init_rect, frame);

    while (capture.isOpened())
    {
        bool ret = capture.read(frame);
        if (!ret)
            break;
        cv::resize(frame, frame, window_size);

        // 跟踪器更新
        cv::Rect result = tracker.update(frame);

        cv::rectangle(frame, result, Scalar(0,255,0), 2);

        cv::imshow(window_name, frame);
        if (cv::waitKey(40) == 'q')
            break;
    }

    return 1;
}
