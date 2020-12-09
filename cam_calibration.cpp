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

Scalar color( rand()&255, rand()&255, rand()&255 );

static void Calibrate(int, void*)
{
    blur( src_gray, detected_edges, Size(3,3) );
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*3, kernel_size );
    dst = Scalar::all(0);
    src.copyTo( dst, detected_edges);
    
    // Copy edges to the images that will display the results in BGR
    cvtColor(detected_edges, cdst, COLOR_GRAY2BGR);
    cdstP = cdst.clone();
    
    // Standard Hough Line Transform
    vector<Vec2f> lines; // will hold the results of the detection
    HoughLines(detected_edges, lines, 1, CV_PI/180, 150, 0, 0 ); // runs the actual detection
    

    
    imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
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

    //Breaking the while loop at the end of the video
    if (bSuccess == false) 
    {
      cout << "Could not open video stream!\n" << endl;
      return -1;
    }
    
    Mat m = src.clone();
    cvtColor(m,m,COLOR_BGR2GRAY);
    blur(m,m,Size(3,3));
    threshold(m,m,140,255,1);
    //erode(m,m,Mat(),Point(-1,-1),n_erode_dilate);
    //dilate(m,m,Mat(),Point(-1,-1),n_erode_dilate);
    
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<cv::Point> points;
    findContours(m, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );
	
    //for (size_t i=0; i<contours.size(); i++) 
    //{
	//for (size_t j = 0; j < contours[i].size(); j++) 
	//{
	//    Point p = contours[i][j];
	 //   points.push_back(p);
	//}
    //}
    
    // iterate through all the top-level contours,
    // draw each connected component with its own random color

    
    drawContours( src, contours, -1, color);

	
    //if(points.size() > 0)
    //{
	//Rect brect = boundingRect(Mat(points).reshape(2));
	//rectangle(src, brect.tl(), brect.br(), Scalar(100, 100, 200), 2, LINE_AA);
    //}
  
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