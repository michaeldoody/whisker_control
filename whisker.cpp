#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include "tic.hpp"
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <chrono>

/*
Tic Input and Motor Settings in Pololu Tic Control Center GUI
Max Speed: 500000000
Starting Speed: 2500
Max Acceleration: 40000
Step Mode: 1/4 step
Current Limit: 2005 mA
*/

using namespace cv;
using namespace std;
using namespace std::chrono;

Mat src, src_gray, blur_gray;
Mat dst, detected_edges, dilated;
Mat cdst, cdstP;

float um, ppum; // Diameter of whisker in micrometers & Pixels per micrometer from ppum.txt
float linearPos, linearVel, expectedDia;
int32_t motorPos, startPos, motorVel;
int32_t EXPECTED_START_POS = 32500; // Position motor resets to at beginning of each trial. Update only if motor has stalled

int baseDia = 1700; // Base diameter in microns
int tipDia = 25; // Tip diameter in microns
int arcLen = 170; // Whisker arc length in mm 
int timeLimit = 120000; // Max amount of time whisker drawing process will take in ms

int lowThreshold = 190;
int n_erode_dilate = 1;
const int kernel_size = 5;

int vpi = 0; // velocity profile index
int velProfile[5] = {500000, 1000000, 1500000, 2000000, 3000000};
int velProfileTime[5] = {24000, 48000, 72000, 96000, 12000};

static void WhiskerDiameter(int, void*)
{
    
    Mat m = src.clone();
    
    GaussianBlur(src_gray, blur_gray, Size(kernel_size, kernel_size), 0, 0);
    Canny(blur_gray, detected_edges, lowThreshold, lowThreshold*3, kernel_size );
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
    
    
    if (lines.size() < 2) // Whisker isn't in full view
    {
        cout << "Whisker is not centered / in view of camera" << endl;
    }
    else
    {
        // Set default line positions
        Point tA, tB, tC, bA, bB, bC; // t = top line, b = bottom line 
        tA.x = 0;
        tA.y = 0;
        tC.x = 650;
        tC.y = 0;
        bA.x = 0;
        bA.y = 480;
        bC.x = 650;
        bC.y = 480;
    
        
        // variables for the top whisker edge
        double aT, bT, xT, yT;
        
        // Tracks avg height of top and bottom line
        int highest = -100000;
        int lowest = 100000;
        
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
                bA.x = pt1.x;
                bA.y = pt1.y;
                bC.x = pt2.x;
                bC.y = pt2.y;
                
            }
            if (avgHeight < lowest && pt1.x < 400)
            {
                lowest = avgHeight;
                tA.x = pt1.x;
                tA.y = pt1.y;
                tC.x = pt2.x;
                tC.y = pt2.y;
                
                aT = a;
                bT = b;
                xT = x0;
                yT = y0;
            }
        }
        
        // Draw the two whisker edges
        line( cdst, tA, tC, Scalar(255,0,0), 2, LINE_AA);
        line( cdst, bA, bC, Scalar(0,0,255), 2, LINE_AA);
        
        // Draw perpendicular lines from top whisker edge
        float vX, vY;
        double sum = 0.0;
        int num_lines = 0;
        for(int w = 125; w < 625; w += 100)
        {
            tB.x = cvRound(xT - w*(-bT));
            tB.y = cvRound(yT - w*(aT));
            
            // Get the direction vector going from A to B
            vX = (float)(tB.x - tA.x);
            vY = (float)(tB.y - tA.y);
            
            // Normalize the vector
            float mag = sqrt(vX*vX + vY*vY);
            vX = vX / mag;
            vY = vY / mag;
            
            // Rotate the vector 90 degrees
            float temp = vX;
            vX = -vY;
            vY = temp;
            
            // Create perpendicular line
            bB.x = tB.x + vX*100;
            bB.y = tB.y + vY*100;
            
            // Line bAbC represented as a1x + b1y = c1
            double a1 = bC.y - bA.y;
            double b1 = bA.x - bC.x;
            double c1 = a1*bA.x + b1*bA.y;
            
            // Line tBbB represented as a2x + b2y = c2
            double a2 = bB.y - tB.y;
            double b2 = tB.x - bB.x;
            double c2 = a2*tB.x + b2*tB.y;
            
            double determinant = a1*b2 - a2*b1;
            
            bB.x = (b2*c1 - b1*c2)/determinant; 
            bB.y = (a1*c2 - a2*c1)/determinant;
            
            // Create perpendicular line
            line( cdst, tB, bB, Scalar(0,255,0), 2, LINE_AA);
            
            double xLength = bB.x - tB.x;
            double yLength = bB.y - tB.y;
            double lineLength = sqrt(xLength*xLength + yLength*yLength);
            
            //cout << lineLength << endl;
            
            sum += lineLength;
            num_lines++;
        }
        
        float avgDia = sum / num_lines;
        float um =round(avgDia / ppum);
        cout << "Whisker Diameter in Micrometers:  " << um << endl;
    }
    
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

    strftime(buffer,80,"%F-%T_",timeinfo);	
    return buffer;	
}

int main( int argc, char** argv )
{
	tic::handle handle;
	tic::variables vars;
	
	try
	{
		// Open tic handle and reset actuator to start position
		handle = open_handle();
		handle.exit_safe_start();
		handle.set_target_position(EXPECTED_START_POS);
		vars = handle.get_variables();
		startPos = vars.get_current_position();
	}
	catch (const std::exception & error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	
	// Wait until actuator reaches start position
	while(startPos < EXPECTED_START_POS*0.99 && startPos > EXPECTED_START_POS*1.01)
	{
		vars = handle.get_variables();
		startPos = vars.get_current_position();
	}
	
    cout << "Clamp the polymer filament in place, and allow it to heat to its glass temperature. Press the Enter Key when ready to draw." << endl;
	cin.get();

	
    // Open the video file for reading
    VideoCapture cap(0); 
  
    // If no success opening camera, exit program
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
    //TODO 2021-01-20-11-28-30_1700D_25d_170-1S.csv
    //time, linear Position, target velocity, actual velocity (adjusted from feedback), target diameter, actual diameter
    string filename = "data/" + datetime() + to_string(baseDia) + "D_" + to_string(tipDia) + "d_" + to_string(arcLen) + "S.csv";
    std::ofstream dataFile(filename);
    
    // Write the data file column headers
    dataFile << "Time(ms)," << "TargetVelocity(mm/s)," << "ActualVelocity(mm/s)," << "TargetWhiskerDiameter(um)," << "ActualWhiskerDiameter(um)" << endl;
	
	auto timeStart = std::chrono::high_resolution_clock::now();
	auto timeCurrent = std::chrono::high_resolution_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(timeCurrent - timeStart).count();
 
    
    while (1)
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
        
        if(timeDiff > velProfileTime[vpi])
        {
            if(vpi < 5) //TODO change to a variable
            {
                vpi++;
            }
            else {break;}
        }
        
        
        // Set motor velocity according to velocity profile
        motorVel = timeDiff*timeDiff/800;
        linearVel = (double)motorVel/1000000.0;
        cout << "Setting target linear velocity to " << linearVel << " mm/s" << endl;
        vars = handle.get_variables();
        motorPos = vars.get_current_position();
        linearPos = (-7 * (double)motorPos / 688) + (7 * (double)EXPECTED_START_POS / 688); // Equation converts motor position to linear actuator position (mm) 
        cout << "Current actuator position is " << linearPos << " mm" << endl;
        cout << endl;
        cout << endl;
        
        // Equation converting current arc length to current expected whisker diameter from whisker geometry profile
		expectedDia = (tipDia - baseDia) * linearPos / arcLen + baseDia;
        
        // Limit switch
        if(linearPos > 325)
        {
			cout << "Actuator limit reached... Stopping motor" << endl;
			handle.set_target_velocity(0);
			break;
		}	
        
        //TODO add time switch and arc length switch to stop motor
        
        handle.exit_safe_start();
		handle.set_target_velocity(-motorVel);
       
        
        // Write timestamp, whisker diameter, and actuator linear velocity to csv file
        //time, steps, target velocity, actual velocity (adjusted from feedback), target diameter, actual diameter

		dataFile << timeDiff << "," << linearVel << "," << linearVel << "," << expectedDia << "," << um << endl;

        if (waitKey(100) == 27)
        {
            cout << "Esc key is pressed by user. Stopping the video" << endl;
            handle.set_target_velocity(0);
            break;
        }
    }
    
    handle.set_target_velocity(0);
    dataFile.close();
    return 0;
}
