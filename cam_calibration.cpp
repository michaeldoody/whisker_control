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
const int kernel_size = 3;

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
  
    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    namedWindow( "RasPi Cam", WINDOW_AUTOSIZE );
    imshow("RasPi Cam", src);
    Calibrate(0, 0);

    if (waitKey(10) == 27)
    {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }
}
