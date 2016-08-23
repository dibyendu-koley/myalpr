/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DetectRegions.h
 * Author: root
 *
 * Created on 17 August, 2016, 2:13 PM
 */

#ifndef DETECTREGIONS_H
#define DETECTREGIONS_H
#include <string.h>
#include <vector>

#include "Plate.h"
//#include <cv.h>
//#include <highgui.h>
//#include <cvaux.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

using namespace std;
using namespace cv;

class DetectRegions{
    public:
        DetectRegions();
        string filename;
        void setFilename(string f);
        bool saveRegions;
        bool showSteps;
        Mat histoImg;
        vector<int> colHeights;
        vector<vector<Point> > mycontours;
        
        vector<Plate> run(Mat input);
        vector<Rect> segment_by_cascade(Mat image);
        Mat refine_segment(Mat image,Mat inputImageRGB,bool use_y_axis);
        Rect refinePlate(Mat inputImage);
        void plateTopBottom(Mat inputImage);
        //vector<Plate> segment_char(Mat input);
        void mySegment(Mat input);
        //---------------below function are used in the algorithm
        float ii(int xx,int yy,Mat input);
        bool keep(vector<Point> contour, Mat input);
        bool connected(vector<Point> contour);
        bool keep_box(vector<Point> contour, Mat input);
        bool is_child(int index, vector<Vec4i> h_);
        bool include_box(int index, vector<Vec4i> h_, vector<Point> contour);
        int get_parent(int index, vector<Vec4i> h_);
        int count_children(int index, vector<Vec4i> h_, vector<Point> contour);
        int count_siblings(int index, vector<Vec4i> h_, vector<Point> contour, bool inc_children=false);



        //int count_children(index, h_, contour);



    private:
        vector<Plate> segment(Mat input);
//        vector<Rect> segment_by_cascade(Mat image);

        bool verifySizes(RotatedRect mr);
        Mat histeq(Mat in);
};
#endif /* DETECTREGIONS_H */

