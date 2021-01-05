#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace cv;
using namespace std;

Mat src, src_gray;
Mat dst, detected_edges, dilated;
Mat cdst, cdstP;

double ppum;

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
    
    dataFile << "New file" << endl;
    dataFile << "1" << endl;
    dataFile << "2" << endl;
    dataFile << "3" << endl;
    

	
    while (true)
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

        if (waitKey(10) == 27)
        {
            cout << "Esc key is pressed by user. Stoppig the video" << endl;
            break;
        }
    }
}
