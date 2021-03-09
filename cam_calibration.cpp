#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
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

int lowThreshold = 190;
int n_erode_dilate = 1;
const int kernel_size = 5;

string img_name = "whisker.jpg";

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
    
    cout << "Focus the camera lens on the calibration slide reticle. Press Space or S to capture the current video frame." << endl;
    cout << "Press Esc to exit the program." << endl;
  
    while (true)
    {
	bool bSuccess = cap.read(src); // read a new frame from video 

	// Breaking the while loop
	if (bSuccess == false) 
	{
	cout << "Could not open video stream!\n" << endl;
	}
    
	namedWindow( "Whisker Calibration", WINDOW_AUTOSIZE );
	imshow("Whisker Calibration", src);
	

	int key = waitKey(10);
    
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
	    cout << "Esc key is pressed by user. Exiting Camera Calibration" << endl;
	    return 0;
	}
    }
    
  
    // Read captured image
    Mat img = imread(img_name, 1);
  
    // User drags ROI box tightly around the calibration slide reticle
    Rect2d rect = selectROI(img);
    
    // Crop image 
    Mat imgCrop = img(rect);
    
    // Calculate pixels per micrometer
    double height = rect.height; //bounding_rect.height;
    double width = rect.width; //bounding_rect.width;
    
    double pixels_per_um = round((height/101.0 + width/101.0) * 1000.0 /2) / 10000.0;
    
    // Display height, width, and pixels per micrometer		  
    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;
    cout << "Pixels Per Micrometer: " << pixels_per_um << endl;
    cout << "\n\n" << endl;
  
    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    namedWindow( "Cropped Whisker Calibration", WINDOW_AUTOSIZE );
    imshow("Cropped Whisker Calibration", imgCrop);
    
    cout << "Press Space or S to save calibration measurement" << endl;
    int key = waitKey(0);
    
    // Save pixels_per_um to txt file if Space or S key is pressed
    if (key == 32 || key == 115)
    {
	ofstream myfile;
    	myfile.open ("ppum.txt");
	myfile << to_string(pixels_per_um);
	myfile.close();
    }
    
    // Exit on Esc key press
    if (key == 27)
    {
	cout << "Esc key is pressed by user. Exiting Camera Calibration" << endl;
	return 0;
    }
    
    return 0;
}
