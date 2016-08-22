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

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

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
        
        vector<Plate> run(Mat input);
        vector<Rect> segment_by_cascade(Mat image);
        Mat refine_segment(Mat image,Mat inputImageRGB,bool use_y_axis);
        Rect refinePlate(Mat inputImage);

        //vector<Plate> segment_char(Mat input);
        void segment_char(Mat input);

    private:
        vector<Plate> segment(Mat input);
//        vector<Rect> segment_by_cascade(Mat image);

        bool verifySizes(RotatedRect mr);
        Mat histeq(Mat in);
};
#endif /* DETECTREGIONS_H */

