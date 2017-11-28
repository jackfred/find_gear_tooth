// find_gear_tooth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>  

using namespace cv;
using namespace std;
int main(int argc, char** argv)
{
	int largest_area = 0;
	int largest_contour_index = 0;
	vector<Rect> bounding_rect;
	VideoCapture cap(0);
	Mat src;

	while (1){
		if (!(cap.read(src))) //get one frame form video   
			break;

		Mat thr(src.rows, src.cols, CV_8UC1);
		Mat dst(src.rows, src.cols, CV_8UC3, Scalar::all(0));
		cvtColor(src, thr, CV_BGR2GRAY); // Convert to gray
		imshow("gray", thr);
		threshold(thr, thr, 60, 255, THRESH_BINARY_INV); // Threshold the gray
		imshow("thr", thr);

		vector<vector<Point>> contours; // Vector for storing contour
		vector<Vec4i> hierarchy;

		findContours(thr, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image

		Scalar color(0, 0, 255);
		Scalar color2(255, 0, 0);
		for (int i = 0; i < contours.size(); i++) // Iterate through each contour
		{
				drawContours(dst, contours, i, color, CV_FILLED, 8, hierarchy);
				drawContours(dst, contours, i, color2, 2, 8, hierarchy);
				rectangle(src, boundingRect(contours[i]), Scalar(0, 255, 0), 1, 8, 0);
		}	
		for (size_t i = 0; i < contours.size(); i++)
		{
			std::fstream outputFile;			
			outputFile.open("points//contour_points_"+to_string(i)+".txt", std::ios::out);
			for (size_t ii = 0; ii < contours[i].size(); ++ii)
				outputFile << contours[i][ii].x << " " << contours[i][ii].y << std::endl;
			outputFile.close();
		}
		imshow("Happy Thread", thr);
		imshow("Happy Source", src);
		imshow("Happy Largest Contour", dst);

		// Wait for a keystroke in the window
		waitKey(30);

	}
	return 0;
}