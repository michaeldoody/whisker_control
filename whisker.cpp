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
    threshold(m, m, 137, 255, 1);
    erode(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    dilate(m, m, Mat(), Point(-1,-1), n_erode_dilate);
    
    namedWindow( "RasPi Cam", WINDOW_AUTOSIZE );
    imshow("RasPi Cam", src);

    if (waitKey(10) == 27)
    {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }
}
