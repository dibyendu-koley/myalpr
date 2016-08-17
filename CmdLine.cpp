/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CmdLine.h"
cmdLine::cmdLine(void){
    country_code="";
    state_code="";
    image_path="";
}
cmdLine::~cmdLine(void){
    
}

void cmdLine::setData(char** argv)
{
    country_code=argv[1];
    state_code=argv[2];
    image_path=argv[3];
}
const char* cmdLine::getData(int data_number)
{
    if(data_number==1)
        return country_code;
    else if(data_number==2)
        return state_code;
    else if(data_number==3)
        return image_path;
    else
        return 0;
}