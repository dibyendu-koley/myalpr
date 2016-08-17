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
    for(vector<string>::const_iterator i = ImgFileName.begin(); i != ImgFileName.end(); ++i) {
        cout << dir_name+*i << "\n";    
        //const char* img_path = cmd.getData(3);
        image = imread(dir_name+*i, CV_LOAD_IMAGE_COLOR); 
        imshow("Original Image", image);
        vector<Plate> posible_regions= detectRegions.run( image );
            for(int i=0; i< posible_regions.size(); i++)
            {
                Mat img=posible_regions[i].plateImg;
                imshow("Image", img);
                waitKey();
            }
    }
    return 0;
}

