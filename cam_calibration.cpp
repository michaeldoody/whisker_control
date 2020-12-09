#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

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
      return -1;
    }
    
    int largest_area=0;
    int largest_contour_index=0;
    Rect bounding_rect;
    
    Mat m = src.clone();
    cvtColor(m,m,COLOR_BGR2GRAY);
    blur(m,m,Size(3,3));
    threshold(m, m, 140, 255, 1);
    erode(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    dilate(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    
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
    
    double height = bounding_rect.height;
    double width = bounding_rect.width;
    
    double pixels_per_um = (int) round((height/101 + width/101)/2);
    
    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;
    cout << "Pixels Per Micrometer: " << pixels_per_um << endl;
    cout << "\n\n" << endl;

    // Outline the calibration crosshairs and draw a rectangle around it
    drawContours( src, contours, largest_contour_index, Scalar( 0, 255, 0 ), 1);
    rectangle(src, bounding_rect.tl(), bounding_rect.br(), Scalar(100, 100, 200), 2, LINE_AA);
  
    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    namedWindow( "RasPi Cam", WINDOW_AUTOSIZE );
    imshow("RasPi Cam", src);
    // Calibrate(0, 0);

    if (waitKey(10) == 27)
    {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }
}
