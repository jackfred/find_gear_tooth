// find_gear_tooth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>  
#include <direct.h>  
#include <stdlib.h>  
#include <stdio.h>  

#include <visp3/core/vpConfig.h>
#include <visp3/blob/vpDot2.h>

using namespace cv;
using namespace std;

int change = 71;

//Good way to save vector<vector<Point>> in bin format(fast)
void writeVectorOfVector(FileStorage &fs, string name, vector<vector<Point>> &vov)
{
	fs << name;
	fs << "{";
	for (int i = 0; i < vov.size(); i++)
	{
		fs << name + "_" + to_string(i);
		vector<Point> tmp = vov[i];
		fs << tmp;
	}
	fs << "}";
}

//Good way to read from vector<vector<Point>> in bin format(fast)
void readVectorOfVector(FileStorage &fns, string name, vector<vector<Point>> &vov)
{
	vov.clear();
	FileNode fn = fns[name];
	if (fn.empty()){
		return;
	}

	FileNodeIterator current = fn.begin(), it_end = fn.end(); // Go through the node
	for (; current != it_end; ++current)
	{
		vector<Point> tmp;
		FileNode item = *current;
		item >> tmp;
		vov.push_back(tmp);
	}
}

bool sketch_gear_tooth(Mat src,vector<Point> contour_in,int thickness=1)
{
	if (contour_in.empty()){
		return false;
	}
	else{
		Point pre_point, cur_point;
		for (size_t i = 0; i < contour_in.size(); i++){
			cur_point = contour_in[i];
			if (pre_point != Point(0, 0)){
				line(src, pre_point, cur_point, Scalar(242, 181, 121), thickness, 8);
			}
			waitKey(1);
			imshow("Happy Source", src);
			pre_point = cur_point;
		}
		return true;
	}
}

bool drawContourCenter(Mat &src, vector<Point> contour)
{
	// center of gravity
	Moments mo = moments(contour);
	Point center = Point(mo.m10 / mo.m00, mo.m01 / mo.m00);
	if (contour.size()<100)
	{
		return false;
	}

	/*
	// Detect circle blob to find the center precisely
	vpImage<unsigned char> I;
	vpImageConvert::convert(src, I);

	vpDot2 blob;
	blob.setGraphics(true);
	blob.setGraphicsThickness(1);
	vpImagePoint germ;
	bool init_done = false;
	germ.set_i(center.y);
	germ.set_j(center.x);

	vpDisplay::display(I);
	for (int i = 0; i < 2; i++){
		if (!init_done) {
			blob.initTracking(I, germ);
			init_done = true;
		}
		else {
			blob.track(I);
		}
	}
	center.x = blob.getCog().get_j();
	center.y = blob.getCog().get_i();
	cout << center.x << " " << center.y << endl;
	vpImageConvert::convert(I, src);
	*/
	circle(src, center, 10, Scalar(255, 0, 0), 5, CV_AA, 0);

	return true;
}

int main(int argc, char** argv)
{
	int largest_area = 0;
	int largest_contour_index = 0;
	vector<Rect> bounding_rect;
	VideoCapture cap(0);
	Mat src;

	while (1){
		cap >> src;

		namedWindow("threshold");
		createTrackbar("threshold", "thr", &change, 255);

		Mat thr(src.rows, src.cols, CV_8UC1);
		Mat dst(src.rows, src.cols, CV_8UC3, Scalar::all(0));
		Mat test_for_read_contours_from_file(src.rows, src.cols, CV_8UC3, Scalar::all(0));
		cvtColor(src, thr, CV_BGR2GRAY); // Convert to gray
		imshow("gray", thr);
		threshold(thr, thr, change, 255, THRESH_BINARY_INV); // Threshold the gray for dark object and light background
		//threshold(thr, thr, change, 255, THRESH_BINARY); // Threshold the gray for light object and dark background
		imshow("thr", thr);

		vector<vector<Point>> contours,contours_for_center; // Vector for storing contour
		vector<Vec4i> hierarchy, hierarchy_for_center;

		findContours(thr, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image

		Scalar color(0, 0, 255);
		Scalar color2(255, 0, 0);
		for (int i = 0; i < contours.size(); i++) // Iterate through each contour
		{
				drawContours(dst, contours, i, color, CV_FILLED, 8, hierarchy);
				drawContours(dst, contours, i, color2, 2, 8, hierarchy);
				rectangle(src, boundingRect(contours[i]), Scalar(0, 255, 0), 1, 8, 0);
				
		}	
		Mat dst_for_center;
		cvtColor(dst, dst_for_center, CV_BGR2GRAY);
		threshold(dst_for_center, dst_for_center, 5, 255, THRESH_BINARY);
		imshow("dst_for_center", dst_for_center);
		findContours(dst_for_center, contours_for_center, hierarchy_for_center, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image
		for (int i = 0; i < contours_for_center.size(); i++) // Iterate through each contour
		{
			drawContourCenter(src, contours_for_center[i]);
		}

		_mkdir("points");
	
		vector<vector<Point>> contours_from_file;
		FileStorage fs_out("points//contour_points.yml", FileStorage::WRITE);
		writeVectorOfVector(fs_out, "one", contours);
		fs_out.release();
		FileStorage fs_in("points//contour_points.yml", FileStorage::READ);
		readVectorOfVector(fs_in, "one", contours_from_file);
		fs_in.release();
		
		/*
		//Test for yml input/output
		color = Scalar(0, 128, 128);
		color2 = Scalar(128, 128, 0);
		for (int i = 0; i < contours_from_file.size(); i++) // Iterate through each contour
		{
			drawContours(test_for_read_contours_from_file, contours_from_file, i, color, CV_FILLED, 8, hierarchy);
			drawContours(test_for_read_contours_from_file, contours_from_file, i, color2, 2, 8, hierarchy);
			rectangle(src, boundingRect(contours_from_file[i]), Scalar(0, 255, 0), 1, 8, 0);
		}
		*/

		//Sketch gear tooth
		for (int i = 0; i < contours_from_file.size(); i++)
		{
			//sketch_gear_tooth(src, contours_from_file[i]);
		}
		
		//imshow("Happy Contour from file", test_for_read_contours_from_file);
		imshow("Happy Thread", thr);
		imshow("Happy Source", src);
		imshow("Happy Largest Contour", dst);

		// Wait for a keystroke in the window
		waitKey(30);

	}
	return 0;
}


