/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "DetectRegions.h"

void DetectRegions::setFilename(string s) {
        filename=s;
}

DetectRegions::DetectRegions(){
    //showSteps=true;
    showSteps=false;
    saveRegions=false;
}

bool DetectRegions::verifySizes(RotatedRect mr){

    float error=0.4;
    //Spain car plate size: 52x11 aspect 4,7272
    float aspect=4.7272;
    //Set a min and max area. All other patchs are discarded
    int min= 15*aspect*15; // minimum area
    int max= 125*aspect*125; // maximum area
    //Get only patchs that match to a respect ratio.
    float rmin= aspect-aspect*error;
    float rmax= aspect+aspect*error;

    int area= mr.size.height * mr.size.width;
    float r= (float)mr.size.width / (float)mr.size.height;
    if(r<1)
        r= (float)mr.size.height / (float)mr.size.width;

    if(( area < min || area > max ) || ( r < rmin || r > rmax )){
        return false;
    }else{
        return true;
    }

}

Mat DetectRegions::histeq(Mat in)
{
    Mat out(in.size(), in.type());
    if(in.channels()==3){
        Mat hsv;
        vector<Mat> hsvSplit;
        cvtColor(in, hsv, CV_BGR2HSV);
        split(hsv, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, hsv);
        cvtColor(hsv, out, CV_HSV2BGR);
    }else if(in.channels()==1){
        equalizeHist(in, out);
    }

    return out;

}

vector<Rect> DetectRegions::segment_by_cascade(Mat image)
{
    Mat gray_image;
    cvtColor( image, gray_image, CV_BGR2GRAY );
    std::vector<Rect> rois;
    // Load Face cascade (.xml file)
    CascadeClassifier face_cascade;
    face_cascade.load( "cascade.xml" );
    // Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale( gray_image, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(10, 20));
    Mat roi;
    for( int i = 0; i < faces.size(); i++ )
    {
        roi = image(faces[i]);
        rois.push_back(faces[i]);
    }
    return rois;
}
Mat DetectRegions::refine_segment(Mat inputImage, Mat inputImageRGB, bool use_y_axis)
{
     //imshow("test", inputImageRGB);
    int max_col_size = 0;

    int columnCount;

    if (use_y_axis)
    {
      // Calculate the histogram for vertical stripes
      for (int col = 0; col < inputImage.cols; col++)
      {
        columnCount = 0;

        for (int row = 0; row < inputImage.rows; row++)
        {
          //if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
          if (inputImage.at<uchar>(row, col) > 0)
            columnCount++;
        }

        colHeights.push_back(columnCount);

        if (columnCount > max_col_size)
          max_col_size = columnCount;
      }
    }
    else
    {
      // Calculate the histogram for horizontal stripes
      for (int row = 0; row < inputImage.rows; row++)
      {
        columnCount = 0;

        for (int col = 0; col < inputImage.cols; col++)
        {
          //if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
          if (inputImage.at<uchar>(row, col) > 0)
            columnCount++;
        }

        colHeights.push_back(columnCount);

        if (columnCount > max_col_size)
          max_col_size = columnCount;
      }
    }
    int histo_width = colHeights.size();
    int histo_height = max_col_size;
    histoImg = Mat::zeros(Size(histo_width, histo_height), CV_8U);
    
    // Draw the columns onto an Mat image
    for (unsigned int col = 0; col < histoImg.cols; col++)
    {
      if (col >= colHeights.size())
        break;

      int columnCount = colHeights[col];
      for (; columnCount > 0; columnCount--)
        histoImg.at<uchar>(histo_height - columnCount, col) = 255;
    }
    //----------segment the plate image
    float percent=.1;
    int temp_count=0;
    int temp_loc_x=0;
    int temp_loc_y=0;
    if (1)
    {
        //
      temp_count=0;
      // Calculate the histogram for vertical stripes
      for (int col = histoImg.cols/2; col > 0; col--)
      {
          temp_count=0;
        for (int row = 0; row < histoImg.rows; row++)
        {
            //cout<<"pixel1::"<<static_cast<int>(inputImage.at<uchar>(row, col))<<"\n";
          if (histoImg.at<uchar>(row, col) > 0){
          //if ( inputImage.at<cv::Vec3b>(row, col) == cv::Vec3b(255,255,255) ){
            temp_count++;
            //cout<<"pixel1::"<<static_cast<int>(inputImage.at<uchar>(row, col))<<"\n";
          }
        }
        cout<<max_col_size*percent<<"::"<<temp_count<<":col "<<col<<"\n";
        if (temp_count < max_col_size*percent ){
        //if (temp_count == 0 ){
            if(col-2 < 0)
                temp_loc_x=col-1;
            else
                temp_loc_x=col-2;
          break;
        }
      }
      temp_count=0;
      for (int col = histoImg.cols/2; col < histoImg.cols; col++)
      {
          temp_count=0;
        for (int row = 0; row < histoImg.rows; row++)
        {
           if (histoImg.at<uchar>(row, col) > 0){
           // if ( inputImage.at<cv::Vec3b>(row, col) == cv::Vec3b(255,255,255) )
                temp_count++;
                //cout<<"pixel2::"<<static_cast<int>(inputImage.at<uchar>(row, col))<<"\n";
           }
        }
        cout<<max_col_size*percent<<"--"<<temp_count<<":col "<<col<<"\n";
        if (temp_count < max_col_size*percent){
        //if (temp_count == 0 ){
          temp_loc_y=col-2;
          break;
        }
      }
    }
    //cout<<inputImageRGB.cols<<":"<<inputImageRGB.rows<<"::"<<histoImg.cols<<"\n";
    //cout<<Rect(temp_loc_x, 0, inputImageRGB.cols, temp_loc_x)<<"\n";
    //img[y: y + h, x: x + w]
    cv::Rect roi;
    roi.x = 0;
    roi.y = temp_loc_x;
//    if(temp_loc_x > 0)
//        roi.y = temp_loc_x - 1;
//    else
//        roi.y = temp_loc_x;
    if(temp_loc_y==0)
        temp_loc_y=1;
    roi.width = inputImageRGB.cols;
    roi.height = temp_loc_y-temp_loc_x+2;
    cout<<roi<<"\n";
    circle(inputImageRGB, Point(roi.x,roi.y),2, Scalar(255,0,0),CV_FILLED, 2,0);        //blue
    circle(inputImageRGB, Point(roi.width,roi.height),2, Scalar(0,255,0),CV_FILLED, 2,0);
    imshow("input img", inputImage);
    //imshow("input", inputImageRGB);
    imshow("input crop", inputImage(roi));
    //return(inputImageRGB(Rect(temp_loc_x, 0, inputImageRGB.cols, temp_loc_x)));
    //return(inputImageRGB(Rect(x, y, w, h)));
//    else
//    {
//      // Calculate the histogram for horizontal stripes
//      for (int row = 0; row < histoImg.rows; row++)
//      {
//        columnCount = 0;
//
//        for (int col = 0; col < histoImg.cols; col++)
//        {
//          //if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
//          if (inputImage.at<uchar>(row, col) > 0)
//            columnCount++;
//        }
//
//        colHeights.push_back(columnCount);
//
//        if (columnCount > max_col_size)
//          max_col_size = columnCount;
//      }
//    }
    

}
//incompleat---------------
Rect DetectRegions::refinePlate(Mat inputImage)
{
    Rect roi;
    int loc_first,loc_last;
    for (int col = 0; col < inputImage.cols; col++)
    {
        //if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
        //if (inputImage.at<uchar>(inputImage.rows, col) > 0)
        //columnCount++;
    }
    return roi;
}
void DetectRegions::segment_char(Mat input)
//vector<Plate> DetectRegions::segment_char(Mat input)
{
    //imshow("test", input);
    vector<Plate> output;
    //convert image to gray
    Mat img_gray;
    cvtColor(input, img_gray, CV_BGR2GRAY);
    //imshow("gray", input);
    //blur(input, input, Size(5,5));    

    //Finde vertical lines. Car plates have high density of vertical lines
    Mat img_sobel;
    //Sobel(input, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    if(showSteps)
        imshow("Sobel", img_sobel);

    //threshold image
    Mat img_threshold;
    threshold(img_gray, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    if(0)
        imshow("Threshold", img_threshold);
    Mat img_threshold_invert;
    bitwise_not ( img_threshold, img_threshold_invert );
    if(0)
        imshow("Threshold Invert", img_threshold_invert);
    //---------erosion is done for removing small patches-------------start
    int erosion_size = 1;   
    cv::Mat element_img_threshold = cv::getStructuringElement(cv::MORPH_CROSS,
                      cv::Size(1 * erosion_size + 1, 1 * erosion_size + 1), 
                      cv::Point(erosion_size, erosion_size) );

    cv::erode(img_threshold_invert, img_threshold_invert, element_img_threshold); 
    cv::erode(img_threshold_invert, img_threshold_invert, element_img_threshold); 
    //---------erosion is done for removing small patches-------------end

    if(0)
        imshow("Threshold Invert after dialate", img_threshold_invert);
    //---------------clear colHeights and histoImg on every count
    histoImg.release();
    colHeights.clear();
//    //create histogram image virtical 
//    refine_segment(img_threshold_invert,true);
//    imshow("Histogram Image", histoImg);
    //create histogram image horizontal
    Mat refine_plate;
    
    refine_segment(img_threshold_invert, input,false);
    if(1)
        imshow("Histogram Image", histoImg);
    //imshow("Refine plate", refine_plate);
    
    //skeletonization the inverted image-----------start
    cv::Mat skel(img_threshold_invert.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp;
    cv::Mat eroded;
 
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
 
    bool done;		
    do
    {
        cv::erode(img_threshold_invert, eroded, element);
        cv::dilate(eroded, temp, element); // temp = open(img)
        cv::subtract(img_threshold_invert, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        eroded.copyTo(img_threshold_invert);
        
        done = (cv::countNonZero(img_threshold_invert) == 0);
    } while (!done);
    //skeletonization the inverted image-----------end

    if(1)
        imshow("Threshold Invert after keletonization", skel);
    
    
}
vector<Plate> DetectRegions::segment(Mat input){
    vector<Plate> output;

    //convert image to gray
    Mat img_gray;
    cvtColor(input, img_gray, CV_BGR2GRAY);
    blur(img_gray, img_gray, Size(5,5));    

    //Finde vertical lines. Car plates have high density of vertical lines
    Mat img_sobel;
    Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    if(showSteps)
        imshow("Sobel", img_sobel);

    //threshold image
    Mat img_threshold;
    threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    if(showSteps)
        imshow("Threshold", img_threshold);

    //Morphplogic operation close
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3) );
    morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
    if(showSteps)
        imshow("Close", img_threshold);

    //Find contours of possibles plates
    vector< vector< Point> > contours;
    findContours(img_threshold,
            contours, // a vector of contours
            CV_RETR_EXTERNAL, // retrieve the external contours
            CV_CHAIN_APPROX_NONE); // all pixels of each contours

    //Start to iterate to each contour founded
    vector<vector<Point> >::iterator itc= contours.begin();
    vector<RotatedRect> rects;

    //Remove patch that are no inside limits of aspect ratio and area.    
    while (itc!=contours.end()) {
        //Create bounding rect of object
        RotatedRect mr= minAreaRect(Mat(*itc));
        if( !verifySizes(mr)){
            itc= contours.erase(itc);
        }else{
            ++itc;
            rects.push_back(mr);
        }
    }

    // Draw blue contours on a white image
    cv::Mat result;
    input.copyTo(result);
    cv::drawContours(result,contours,
            -1, // draw all contours
            cv::Scalar(255,0,0), // in blue
            1); // with a thickness of 1
    //imshow("Result", result);
    
    for(int i=0; i< rects.size(); i++){

        //For better rect cropping for each posible box
        //Make floodfill algorithm because the plate has white background
        //And then we can retrieve more clearly the contour box
        circle(result, rects[i].center, 3, Scalar(0,255,0), -1);
        //get the min size between width and height
        float minSize=(rects[i].size.width < rects[i].size.height)?rects[i].size.width:rects[i].size.height;
        minSize=minSize-minSize*0.5;
        //initialize rand and get 5 points around center for floodfill algorithm
        //srand ( time(NULL) );         //------------joy comment it out
        //Initialize floodfill parameters and variables
        Mat mask;
        mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
        mask= Scalar::all(0);
        int loDiff = 30;
        int upDiff = 30;
        int connectivity = 4;
        int newMaskVal = 255;
        int NumSeeds = 10;
        Rect ccomp;
        int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
        for(int j=0; j<NumSeeds; j++){
            Point seed;
            seed.x=rects[i].center.x+rand()%(int)minSize-(minSize/2);
            seed.y=rects[i].center.y+rand()%(int)minSize-(minSize/2);
            //cout<<seed.x<<"::"<<seed.y<<"\n";
            circle(result, seed, 1, Scalar(0,255,255), -1);
            int area = floodFill(input, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
        }
        if(showSteps)
            imshow("MASK", mask);
        //cvWaitKey(0);

        //Check new floodfill mask match for a correct patch.
        //Get all points detected for get Minimal rotated Rect
        vector<Point> pointsInterest;
        Mat_<uchar>::iterator itMask= mask.begin<uchar>();
        Mat_<uchar>::iterator end= mask.end<uchar>();
        for( ; itMask!=end; ++itMask)
            if(*itMask==255)
                pointsInterest.push_back(itMask.pos());

        RotatedRect minRect = minAreaRect(pointsInterest);

        if(verifySizes(minRect)){
            // rotated rectangle drawing 
            Point2f rect_points[4]; minRect.points( rect_points );
            for( int j = 0; j < 4; j++ )
                line( result, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,255), 1, 8 );    

            //Get rotation matrix
            float r= (float)minRect.size.width / (float)minRect.size.height;
            float angle=minRect.angle;    
            if(r<1)
                angle=90+angle;
            Mat rotmat= getRotationMatrix2D(minRect.center, angle,1);

            //Create and rotate image
            Mat img_rotated;
            warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);

            //Crop image
            Size rect_size=minRect.size;
            if(r < 1)
                swap(rect_size.width, rect_size.height);
            Mat img_crop;
            getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);
            
            Mat resultResized;
            resultResized.create(33,144, CV_8UC3);
            resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
            //imshow("test", img_crop);
            //Equalize croped image
            Mat grayResult;
            cvtColor(resultResized, grayResult, CV_BGR2GRAY); 
            blur(grayResult, grayResult, Size(3,3));
            grayResult=histeq(grayResult);
            if(saveRegions){ 
                stringstream ss(stringstream::in | stringstream::out);
                ss << "tmp/" << filename << "_" << i << ".jpg";
                imwrite(ss.str(), grayResult);
            }
            //cout<<minRect.boundingRect()<<endl;
            //output.push_back(Plate(grayResult,minRect.boundingRect()));     //store the gray histogram equalized image
            output.push_back(Plate(resultResized,minRect.boundingRect()));     //store rgb image
        }
    }       
    if(showSteps)
    //if(1) 
        imshow("Contours", result);

    return output;
}

vector<Plate> DetectRegions::run(Mat input){
    
    //Segment image by white 
    vector<Plate> tmp=segment(input);

    //return detected and posibles regions
    return tmp;
}

