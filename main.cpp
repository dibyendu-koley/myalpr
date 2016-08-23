/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 17 August, 2016, 10:55 AM
 */

#include <cstdlib>
#include <dirent.h>

#include "alpr.h"
#include "CmdLine.h"
#include "DetectRegions.h"

using namespace std;
int keyboard;
/*
 * 
 */
vector<string> listFile(const char* dir_name){
DIR *dpdf;
struct dirent *epdf;
vector<string> ImgFileName;
dpdf = opendir(dir_name);
if (dpdf != NULL){
   while (epdf = readdir(dpdf)){
        if( strstr( epdf->d_name, ".jpg" ) ){
                ImgFileName.push_back(epdf->d_name);
        }
   }
}
return (ImgFileName);
}
void readme()
  { std::cout << " Usage: ./executable_file <country_code> <state_code> <path_of_dir> " << std::endl;
   std::cout << " country_code example ind for india " << std::endl;
   std::cout << " state_code example wb for westbengal " << std::endl;
   std::cout << "ind wb /home/dibyendu/Desktop/car/back/1.jpg"<< std::endl;
  }
int main(int argc, char** argv) {
    int count_cascade=0,count_region=0;
    float height_reduce_percentage=.7;
    float y_increase_percentage=1.05;
    if( argc != 4 )
    { readme(); return -1; }
    cmdLine cmd;
    DetectRegions detectRegions;
    cmd.setData(argv);
    //cout<<"country:: "<<cmd.getData(1)<<" state:: "<<cmd.getData(2)<<" img_path:: "<<cmd.getData(3)<<"\n";
    Mat image;
    std::string dir_name = cmd.getData(3);
    vector<string> ImgFileName;
    ImgFileName =  listFile(cmd.getData(3));
    //read input data. ESC or 'q' for quitting
    string file_name;
    for(vector<string>::const_iterator i = ImgFileName.begin(); i != ImgFileName.end(); ++i) {
        file_name=*i;
        //cout << file_name << "\n";    
        //const char* img_path = cmd.getData(3);
        image = imread(dir_name+*i, CV_LOAD_IMAGE_COLOR); 
        //imshow("Original Image", image);
        //vector<Plate> posible_regions= detectRegions.run( image );
        //detect rigion by haar cascade
        vector<Rect> posible_regions_by_cascade= detectRegions.segment_by_cascade(image);
        if(!posible_regions_by_cascade.empty())
            count_cascade++;
        //cout<<posible_regions_by_cascade.empty()<<"\n";
            for(vector<Rect>::const_iterator j = posible_regions_by_cascade.begin(); j != posible_regions_by_cascade.end(); ++j) {
                //Mat img_cascade = image(Rect(j->x,j->y*y_increase_percentage,j->width,j->height*height_reduce_percentage));
                Mat img_cascade = image(Rect(j->x,j->y,j->width,j->height));
                imshow(file_name+"cascad", img_cascade);
                imwrite("cascad"+file_name, img_cascade );
                detectRegions.mySegment(img_cascade);
                //after haar cacade send the rois to the segmentation process. this increase the localization rate
//                vector<Plate> posible_regions_after_cascade= detectRegions.run( img_cascade );
//                if(!posible_regions_after_cascade.empty())
//                    count_region++;
//                for(int i=0; i< posible_regions_after_cascade.size(); i++)
//                {
//                    Mat img_posible_regions_after_cascade=posible_regions_after_cascade[i].plateImg;
//                    //detectRegions.segment_char(img_posible_regions_after_cascade);
////                    imshow(file_name+"cascad", img_posible_regions_after_cascade);
//                    waitKey();
//                
//                }
                
            }
//            for(int i=0; i< posible_regions.size(); i++)
//            {
//                Mat img=posible_regions[i].plateImg;
//                //detectRegions.segment_char(img);
//                //imshow(file_name, img);
//                //waitKey();
//                
//            }
        destroyWindow(file_name+"cascad");
        destroyWindow(file_name);
    }
    cout<<"Number of image detected by cascade:: "<<count_cascade<<"\n";
    //cout<<"Number of image detected by cascade and segmented:: "<<count_region<<"\n";
    return 0;
}

