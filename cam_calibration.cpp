#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

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
    CannyThreshold(0, 0);

    if (waitKey(10) == 27)
    {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }
}
