/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CmdLine.h
 * Author: root
 *
 * Created on 17 August, 2016, 11:13 AM
 */

#ifndef CMDLINE_H
#define CMDLINE_H
class cmdLine 
{
public:
    cmdLine(void);
    ~cmdLine(void);
    void setData(char** argv);
    const char* getData(int data_number);
private:
    const char* country_code;
    const char* state_code;
    const char* image_path;
    
};


#endif /* CMDLINE_H */

