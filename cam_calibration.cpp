#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <ctime>

using namespace cv;
using namespace std;

Mat src, src_gray;
Mat dst, detected_edges;
Mat cdst, cdstP;

int lowThreshold = 8;
const int max_lowThreshold = 100;
double n_threshold = 128;
int n_erode_dilate = 1;
const int kernel_size = 3;

string img_name = "whisker.jpg";

string datetime()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"cam_cal%d%m%Y_%H%M%S",timeinfo);
    return buffer;
}

int main( int argc, char** argv )
{
    //open the video file for reading
    VideoCapture cap(0); 
  
    // if not success, exit program
    if (cap.isOpened() == false)  
    {
	cout << "Cannot open the camera" << endl;
	cin.get(); //wait for any key press
	return -1;
    }
  
    while (true)
    {
	bool bSuccess = cap.read(src); // read a new frame from video 

	// Breaking the while loop
	if (bSuccess == false) 
	{
	cout << "Could not open video stream!\n" << endl;
	}
    
    
	int key = waitKey(5);
    
	// Save image if Space or S key is pressed
	if (key == 32 || key == 115)
	{
	    imwrite(img_name, src);
	    cap.release();
	    break;
	}
    
	// Exit on Esc key press
	if (key == 27)
	{
	    cout << "Esc key is pressed by user. Stoppig the video" << endl;
	    break;
	}
    }
  
    // Read captured image
    Mat img = imread(img_name, 1);
  
    // Select ROI
    Rect2d rect = selectROI(img);
  
    // Crop image 
    Mat imgCrop = img(rect);
  
    int largest_area=0;
    int largest_contour_index=0;
    Rect bounding_rect;
    
    Mat m = src.clone();
    cvtColor(m,m,COLOR_BGR2GRAY);
    blur(m,m,Size(3,3));
    Canny(m, m, lowThreshold, lowThreshold*3, kernel_size );
    dilate(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    //erode(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<cv::Point> points;
    
    // Find all contours in image
    findContours(m, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );
	
    //	Find largest contour (the calibration slide's mm crosshairs)
    for(size_t i = 0; i < contours.size(); i++)
    {
	double area = contourArea(contours[i]);
	
	if(area > largest_area)
	{
	    largest_area = area;
	    largest_contour_index = i;
	    bounding_rect = boundingRect(contours[i]);
	}
    }
    
    
    // Calculate pixels per micrometer
    double height = bounding_rect.height;
    double width = bounding_rect.width;
    
    double pixels_per_um = round((height/101 + width/101) * 1000.0 /2) / 1000.0;
    
    // Display height, width, and pixels per micrometer		  
    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;
    cout << "Pixels Per Micrometer: " << pixels_per_um << endl;
    cout << "\n\n" << endl;

    // Outline the calibration crosshairs and draw a rectangle around it
    //drawContours( src, contours, largest_contour_index, Scalar( 0, 255, 0 ), 1);
    //rectangle(src, bounding_rect.tl(), bounding_rect.br(), Scalar(100, 100, 200), 2, LINE_AA);
  
    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    namedWindow( "RasPi Cam", WINDOW_AUTOSIZE );
    imshow("RasPi Cam", src);
<<<<<<< HEAD
    
    waitKey(0);
    return 0;
=======

    if (waitKey(10) == 27)
    {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }
>>>>>>> 36b69fe4cac0caa0f808fd85954625cc25fe7333
}
