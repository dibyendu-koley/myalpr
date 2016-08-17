/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   alpr.h
 * Author: root
 *
 * Created on 17 August, 2016, 10:59 AM
 */

#ifndef ALPR_H
#define ALPR_H
#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;
class ALPR
{
public:
        ALPR(void);
        ~ALPR(void);
        void extract_NEG(void);
        void extract_POS(void);
        void train_HOG_SVM(void);
        void convert_HOG_SVM(void);
        void test_HOG_SVM(const char* dir_name);
        void test_HOG_SVM_after_preprocess(Mat roi);
        bool verifySizes(RotatedRect mr);
        void testPlate(Mat roi,bool showSteps);
private:
        int Y_MIN;
        int Y_MAX;
        int Cr_MIN;
        int Cr_MAX;
        int Cb_MIN;
        int Cb_MAX;
};

#endif /* ALPR_H */

