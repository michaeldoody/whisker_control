#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include "tic/include/tic.hpp"
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <chrono>

using namespace cv;
using namespace std;
using namespace std::chrono;

Mat src, src_gray;
Mat dst, detected_edges, dilated;
Mat cdst, cdstP;

double um, ppum; // Diameter of whisker in micrometers & Pixels per micrometer from ppum.txt
int motorVel = 10000;

int lowThreshold = 7;
const int max_lowThreshold = 210;
int n_erode_dilate = 1;
const int kernel_size = 3;

static void WhiskerDiameter(int, void*)
{
    Mat m = src.clone();
    
    blur( src_gray, detected_edges, Size(3,3) );
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*3, kernel_size );
    dilate(detected_edges, dilated, Mat(), Point(-1,-1), n_erode_dilate);
    erode(dilated, dilated, Mat(), Point(-1,-1), n_erode_dilate);
    dst = Scalar::all(0);
    src.copyTo(dst, dilated);
    
    // Copy edges to the images that will display the results in BGR
    cvtColor(dilated, cdst, COLOR_GRAY2BGR);
    cdstP = cdst.clone();
    
    // Standard Hough Line Transform
    vector<Vec2f> lines; // will hold the results of the detection
    HoughLines(dilated, lines, 1, CV_PI/180, 150, 0, 0 ); // runs the actual detection
    

    int highest = -100000;
    int lowest = 100000;
    
    // Set default line positions
    Point ptHigh1, ptHigh2, ptLow1, ptLow2;
    ptHigh1.x = 0;
    ptHigh1.y = 480;
    ptHigh2.x = 650;
    ptHigh2.y = 480;
    ptLow1.x = 0;
    ptLow1.y = 0;
    ptLow2.x = 650;
    ptLow2.y = 0;
    
    // Select the uppermost and bottommost lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 650*(-b));
        pt1.y = cvRound(y0 + 650*(a));
        pt2.x = cvRound(x0 - 650*(-b));
        pt2.y = cvRound(y0 - 650*(a));
        
        int avgHeight = (pt1.y + pt2.y)/2 ;
        
        if (avgHeight > highest && pt1.x < 400)
        {
            highest = avgHeight;
            ptHigh1.x = pt1.x;
            ptHigh1.y = pt1.y;
            ptHigh2.x = pt2.x;
            ptHigh2.y = pt2.y;
            
        }
        else if (avgHeight < lowest && pt1.x < 400)
        {
            lowest = avgHeight;
            ptLow1.x = pt1.x;
            ptLow1.y = pt1.y;
            ptLow2.x = pt2.x;
            ptLow2.y = pt2.y;
        }
    }
    
    
    // Draw the two lines
    line( cdst, ptHigh1, ptHigh2, Scalar(0,0,255), 2, LINE_AA);
    line( cdst, ptLow1, ptLow2, Scalar(0,0,255), 2, LINE_AA);
    
    // Average distance between the lines
    int dist1, dist2, avgDist;
    dist1 = ptHigh1.y - ptLow1.y;
    dist2 = ptHigh2.y - ptLow2.y;
    avgDist = (dist1 + dist2) / 2;
    double um =round(avgDist / ppum);
    cout << "Whisker Diameter in Micrometers:  " << um << endl;
    Point avg1, avg2;
    avg1.x = 325;
    avg1.y = abs(ptLow2.y - ptLow1.y)/2 + min(ptLow1.y, ptLow2.y);
    avg2.x = 325;
    avg2.y = avg1.y + avgDist;
    
    line( cdst, avg1, avg2, Scalar(0,255,0), 2, LINE_AA);

    
    imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
}

// Opens a handle to a Tic that can be used for communication.
//
// To open a handle to any Tic:
//   tic_handle * handle = open_handle();
// To open a handle to the Tic with serial number 01234567:
//   tic_handle * handle = open_handle("01234567");
tic::handle open_handle(const char * desired_serial_number = nullptr)
{
  // Get a list of Tic devices connected via USB.
  std::vector<tic::device> list = tic::list_connected_devices();
 
  // Iterate through the list and select one device.
  for (const tic::device & device : list)
  {
    if (desired_serial_number &&
      device.get_serial_number() != desired_serial_number)
    {
      // Found a device with the wrong serial number, so continue on to
      // the next device in the list.
      continue;
    }
 
    // Open a handle to this device and return it.
    return tic::handle(device);
  }
 
  throw std::runtime_error("No device found.");
}

string datetime()	
{	
    time_t rawtime;	
    struct tm * timeinfo;	
    char buffer[80];	

    time (&rawtime);	
    timeinfo = localtime(&rawtime);	

    strftime(buffer,80,"data%d%m%Y_%H%M%S",timeinfo);	
    return buffer;	
}

int main( int argc, char** argv )
{
	
	try
	{
		tic::handle handle = open_handle();
 
		tic::variables vars = handle.get_variables();
 
		int32_t position = vars.get_current_position();
		std::cout << "Current position is " << position << ".\n";
 
		int32_t new_target_vel = -2500000;
		std::cout << "Setting target velocity to " << new_target_vel << ".\n";
 
		handle.exit_safe_start();
		handle.set_target_velocity(new_target_vel);
		waitKey(0);
		handle.set_target_velocity(0);
	}
	catch (const std::exception & error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	
	
    //open the video file for reading
    VideoCapture cap(0); 
  
    // if not success, exit program
    if (cap.isOpened() == false)  
    {
        cout << "Cannot open the camera" << endl;
        cin.get(); //wait for any key press
        return -1;
    }
    
    // Retrieve pixels per micrometer from whiskers.txt 
    string ppum_text;
    ifstream infile;
    infile.open ("ppum.txt");
    getline(infile, ppum_text); // Saves the text file line
	infile.close();
    
    ppum = atof(ppum_text.c_str());

    // Create a csv file for whisker drawing data
    string filename = datetime() + ".csv";
    std::ofstream dataFile(filename);
    
    // Write the data file column headers
    dataFile << "Time(ms)" << "," << "WhiskerDiameter(um)" << "," << "MotorVelocity" << endl;
	
	auto timeStart = std::chrono::high_resolution_clock::now();
	auto timeCurrent = std::chrono::high_resolution_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(timeCurrent - timeStart).count();
    
    while (timeDiff < 120000) // stop after 2 mins 
    {
        bool bSuccess = cap.read(src); // read a new frame from video 

        // Break the while loop if there is no video
        if (bSuccess == false) 
        {
            cout << "Could not open video stream!\n" << endl;
            return -1;
        }
    
        dst.create( src.size(), src.type() );
        cvtColor( src, src_gray, COLOR_BGR2GRAY );
        namedWindow( "RasPi Cam", WINDOW_AUTOSIZE );
        imshow("RasPi Cam", src);
        WhiskerDiameter(0, 0);
        
        auto timeCurrent = std::chrono::high_resolution_clock::now();
        timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(timeCurrent - timeStart).count();
        
        // Write data to file
		dataFile << timeDiff << "," << um << "," << 20 << endl;

        if (waitKey(100) == 27)
        {
            cout << "Esc key is pressed by user. Stoppig the video" << endl;
            break;
        }
    }
    
    dataFile.close();
    return 0;
}
