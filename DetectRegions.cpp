/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "DetectRegions.h"

void DetectRegions::setFilename(string s) {
        filename=s;
}
keepers::keepers(vector<Point> contour1,Rect box1){
    contour=contour1;
    box=box1;
}
DetectRegions::DetectRegions(){
    showSteps=true;
    //showSteps=false;
    saveRegions=false;
}

bool DetectRegions::verifySizes(RotatedRect mr){

    float error=0.2;
    //Spain car plate size: 52x11 aspect 4,7272
    //float aspect=4.7272;
    float aspect=4.3636;
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
void DetectRegions::plateTopBottom(Mat src)
{
    Mat dst, cdst;
    Canny(src, dst, 50, 200, 3);
    cvtColor(dst, cdst, CV_GRAY2BGR);
 
    vector<Vec2f> lines;
      HoughLines(dst, lines, 1, CV_PI/180, 150, 0, 0 );
    // detect lines
    //HoughLines(dst, lines, 1, cv::CV_PI/180, 150, 0, 0 );
 
    // draw lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        double angle = theta * (180 / CV_PI);
         
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        if ( (angle > 70 && angle < 110) || (angle > 250 && angle < 290))
        {
        line( cdst, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
        }
    }
 
    imshow("source", src);
    imshow("detected lines", cdst);
 
    waitKey();
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
    face_cascade.detectMultiScale( gray_image, faces, 1.1, 15, 0|CV_HAAR_SCALE_IMAGE, Size(10, 5));
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
        //cout<<max_col_size*percent<<"::"<<temp_count<<":col "<<col<<"\n";
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
        //cout<<max_col_size*percent<<"--"<<temp_count<<":col "<<col<<"\n";
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
    //cout<<roi<<"\n";
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
void DetectRegions::mySegment(Mat input)
{
    //int img_y,img_x;
    vector<float> bg_int;
    int x_, y_, width, height,bg,fg;
    Mat img,blue_edges,green_edges,red_edges,edges,processed,rejected;
    //vector<vector<Point> > contours;
    mycontours.clear();
    kps.clear();
    img_y=img_x=0;
    vector<Vec4i> hierarchys;
    Vec4i hierarchy;
    Rect bounding_rect;
    //copyMakeBorder( input, img, 25, 25, 25, 25, BORDER_ISOLATED);
    copyMakeBorder( input, img, 50, 50, 50, 50, BORDER_ISOLATED);
    if(showSteps)
        imshow("Border Plate", img);
    //Calculate the width and height of the image
    img_y = input.cols;
    img_x = input.rows;
    Mat bgr[3];   //destination array   //B=bgr[0], G=bgr[1], R=bgr[2]
    split(img,bgr);//split source  
    //Run canny edge detection on each channel
    Canny(bgr[0], blue_edges, 200, 250);
    Canny(bgr[1], green_edges, 200, 250);
    Canny(bgr[2], red_edges, 200, 250);

    //Join edges back into image
    edges = blue_edges | green_edges | red_edges;
    edges.copyTo(processed);
    edges.copyTo(rejected);
            
    if(showSteps)
        imshow("Edge", edges);
    //Find the contours
    findContours( edges, mycontours, hierarchys, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );

    for( int i = 0; i< mycontours.size(); i++ )
    {
        
        
        bounding_rect=boundingRect(mycontours[i]);
        //# Check the contour and it's bounding box
      
        std::ostringstream index ;
        index << i;
        //cout<<hierarchys[i]<<"\n";
        //cout<<hierarchys[hierarchys.size()-3]<<"\n";
        //cout<<hierarchys[hierarchys.size()-4]<<"\n";
        //cout<<keep(mycontours[i])<<"::"<<include_box(i, hierarchys, mycontours[i])<<"\n";
        //cout<<mycontours[i]<<"\n";
        if (keep(mycontours[i]) and include_box(i, hierarchys, mycontours[i]))    
        {
            cout<<mycontours[i]<<"\n";
            kps.push_back(keepers(mycontours[i],bounding_rect));
            rectangle( processed, bounding_rect.tl(), bounding_rect.br(), Scalar(100, 100, 100), 3, 8, 0 );
            //cv2.putText(rejected, str(index_), (x, y - 5), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255))
            putText(processed, index.str(), Point2f(bounding_rect.x, bounding_rect.y - 5), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);
            //rectangle(processed, bounding_rect.tl(),bounding_rect.br(), Scalar(100, 100, 100), 1);
            //cout<<"------------"<<"\n";  
        }
        else
        {
            rectangle(rejected, bounding_rect.tl(),bounding_rect.br(), Scalar(100, 100, 100), 1, 8, 0);
            putText(rejected, index.str(), Point2f(bounding_rect.x, bounding_rect.y - 5), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);
        }
            
    }
    Mat new_image(edges.rows,edges.cols, CV_8UC3, Scalar(255,255,255));
    if(showSteps)
        imshow("processed Plate", processed);
    if(showSteps)
        imshow("rejected Plate", rejected);
    if(showSteps)
        imshow("input Plate", input);
    //------------dibyendu upto this point it is working perfect
    for( int i = 0; i< kps.size(); i++ )
    {
//        # Find the average intensity of the edge pixels to
//# determine the foreground intensity
        //cout<<kps[i].contour<<"\n";
        //cout<<kps[i].box<<"\n";
        double fg_int = 0.0,bg_int1 = 0.0;
        for( int j = 0; j< kps[i].contour.size();j++)
        {
            //---------tag
            //cout << kps[i].contour[j].x<<","<< kps[i].contour[j].y<<"\n";
            //cout << img_x <<":"<<img_y<<"\n";
            fg_int += ii(kps[i].contour[j].x, kps[i].contour[j].y,input);
        }
        //fg_int = fg_int/double(kps[i].contour.size());
        //cout << fg_int<<"\n";
        Rect(x_, y_, width, height) = kps[i].box;
//         //# bottom left corner 3 pixels
//        //cout<<kps[i].box.x<< kps[i].box.y<< kps[i].box.width<< kps[i].box.height<<"\n";
//        bg_int.push_back(ii(kps[i].box.x - 1, kps[i].box.y - 1, input));
//        bg_int.push_back(ii(kps[i].box.x - 1, kps[i].box.y, input));
//        bg_int.push_back(ii(kps[i].box.x, kps[i].box.y - 1, input));
//         //# bottom right corner 3 pixels
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width + 1, kps[i].box.y - 1, input));
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width, kps[i].box.y - 1, input));
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width + 1, kps[i].box.y, input));
//         //# top left corner 3 pixels
//        bg_int.push_back(ii(kps[i].box.x - 1, kps[i].box.y + kps[i].box.height + 1, input));
//        bg_int.push_back(ii(kps[i].box.x - 1, kps[i].box.y + kps[i].box.height, input));
//        bg_int.push_back(ii(kps[i].box.x, kps[i].box.y + kps[i].box.height + 1, input));
//         //# top right corner 3 pixels
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width + 1, kps[i].box.y + kps[i].box.height + 1, input));
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width, kps[i].box.y + kps[i].box.height + 1, input));
//        bg_int.push_back(ii(kps[i].box.x + kps[i].box.width + 1, kps[i].box.y + kps[i].box.height, input));
//        bg_int1 = CalcMHWScore(bg_int);
//        //# Determine if the box should be inverted
//        if(fg_int >= bg_int1){
//            fg = 255;
//            bg = 0;
//        }
//        else{
//            fg = 0;
//            bg = 255;
//        }
////        # Loop through every pixel in the box and color the
////        # pixel accordingly
//        for (int k=kps[i].box.x;k<(kps[i].box.x + kps[i].box.width); k++){
//            for (int l=kps[i].box.y;l<(kps[i].box.y + kps[i].box.height); l++){
//                if (k >= img_y or l >= img_x)
//                    cout<<"pixel out of bounds"<<"\n";
//                if (ii(k, l,img) > fg_int)
//                    new_image.at<uchar>(k,l) = bg;
//                else
//                    new_image.at<uchar>(k, l) = fg;
//            }
//        }
       
    }
    if(showSteps)
        imshow("newimage Plate", new_image);
    waitKey();
}
float DetectRegions::CalcMHWScore(vector<float> scores)
{
  float median;
  size_t size = scores.size();

  sort(scores.begin(), scores.end());
  //cout<<scores[size / 2]<<"\n";
  if (size  % 2 == 0)
  {
      median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
  }
  else 
  {
      median = scores[size / 2];
  }

  return median;
}
//void DetectRegions::mySegment(Mat input)
////vector<Plate> DetectRegions::segment_char(Mat input)
//{
//    if(showSteps)
//        imshow("Original Plate", input);
//    vector<Plate> output;
//    //convert image to gray
//    Mat img_gray;
//    cvtColor(input, img_gray, CV_BGR2GRAY);
//    //Finde vertical lines. Car plates have high density of vertical lines
//    Mat img_sobel;
//    Sobel(input, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
//    if(showSteps)
//        imshow("Sobel", img_sobel);
//
//    //threshold image
//    Mat img_threshold;
//    threshold(img_gray, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
//    if(showSteps)
//        imshow("Threshold", img_threshold);
//    Mat img_threshold_invert;
//    bitwise_not ( img_threshold, img_threshold_invert );
//    if(showSteps)
//        imshow("Threshold Invert", img_threshold_invert);
//    //---------erosion is done for removing small patches-------------start
//    int erosion_size = 1;   
//    cv::Mat element_img_threshold = cv::getStructuringElement(cv::MORPH_CROSS,
//                      cv::Size(1 * erosion_size + 1, 1 * erosion_size + 1), 
//                      cv::Point(erosion_size, erosion_size) );
//
//    cv::erode(img_threshold_invert, img_threshold_invert, element_img_threshold); 
//    cv::erode(img_threshold_invert, img_threshold_invert, element_img_threshold); 
//    //---------erosion is done for removing small patches-------------end
//
//    if(0)
//        imshow("Threshold Invert after dialate", img_threshold_invert);
//    //---------------clear colHeights and histoImg on every count
//    histoImg.release();
//    colHeights.clear();
////    //create histogram image virtical 
////    refine_segment(img_threshold_invert,true);
////    imshow("Histogram Image", histoImg);
//    //create histogram image horizontal
//    Mat refine_plate;
//    
//    //refine_segment(img_threshold_invert, input,false);
//    if(showSteps)
//        imshow("Histogram Image", histoImg);
//    //imshow("Refine plate", refine_plate);
//    
//    //skeletonization the inverted image-----------start
//    cv::Mat skel(img_threshold_invert.size(), CV_8UC1, cv::Scalar(0));
//    cv::Mat temp;
//    cv::Mat eroded;
// 
//    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
// 
//    bool done;		
//    do
//    {
//        cv::erode(img_threshold_invert, eroded, element);
//        cv::dilate(eroded, temp, element); // temp = open(img)
//        cv::subtract(img_threshold_invert, temp, temp);
//        cv::bitwise_or(skel, temp, skel);
//        eroded.copyTo(img_threshold_invert);
//        
//        done = (cv::countNonZero(img_threshold_invert) == 0);
//    } while (!done);
//    //skeletonization the inverted image-----------end
//
//    if(showSteps)
//        imshow("Threshold Invert after keletonization", skel);
//    waitKey();
//    
//}
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
//# Determine pixel intensity
//# Apparently human eyes register colors differently.
//# TVs use this formula to determine
//# pixel intensity = 0.30R + 0.59G + 0.11B
double DetectRegions::ii(int xx,int yy,Mat img)
{
    //Point3_<uchar>* p;
    if(yy >= img_y or xx >= img_x){
        //cout<<"point outside the image\n";
        return 0.0;
    }
    Point3_<uchar>* p = img.ptr<Point3_<uchar> >(xx,yy);
    // pixel intensity = 0.30R + 0.59G + 0.11B      //p->x :B p->y :G p->z :R
    //cout<<"test"<<(0.30 * p->z + 0.59 * p->y + 0.11 * p->x)<<"\n";
    return (0.30 * p->z + 0.59 * p->y + 0.11 * p->x);
}

//----------------------------------
//# Count the number of relevant siblings of a contour
int DetectRegions::count_siblings(int index, vector<Vec4i> h_, vector<Point> contour, bool inc_children = false)
{
    int p_,n,count;
    if (inc_children)
        count = count_children(index, h_, contour);
    else
        count = 0;
//     Look ahead
    p_ = h_[index][0];
    while (p_ > 0){
        if (keep(mycontours[p_]))
            count += 1;
        if (inc_children)
            count += count_children(p_, h_, contour);
        p_ = h_[p_][0];
    }
//    # Look behind
    n = h_[index][1];
    while (n > 0){
        if (keep(mycontours[n]))
            count += 1;
        if (inc_children)
            count += count_children(n, h_, contour);
        n = h_[n][1];
    }
return count;
}
//# Whether we care about this contour

bool DetectRegions::keep(vector<Point> contour)
{
    //cout<<keep_box(contour)<<"::"<<connected(contour)<<"\n";
    if(keep_box(contour) and connected(contour))
        return true;
    else
        return false;
}
//# Count the number of real children
//---------------------------
bool DetectRegions::include_box(int index, vector<Vec4i> h_, vector<Point> contour)
{
    int cc,p;
    p=get_parent(index, h_);
    //cout<<"count_children parent:"<<p<<"\n";
    //cc = count_children(get_parent(index, h_), h_, contour);
    //cout<<"count_children:"<<cc<<"\n";    
    if (is_child(index, h_) and count_children(get_parent(index, h_), h_, contour) <= 2){
        //cout<<"skipping: is an interior to a letter"<<"\n";
        return false;
    }
    if (count_children(index, h_, contour) > 2){
        //cout<<"skipping, is a container of letters"<<"\n";
        return false;
    }
    return true;
}
//# Whether we should keep the containing box of this
//# contour based on it's shape

bool DetectRegions::keep_box(vector<Point> contour)
{
    //cout<<img_x<<":"<<img_y<<"\n";
    //vector<Point>  contours_poly;
    float w_,h_;
    Rect boundRect;
    //approxPolyDP( contour, contours_poly, 3, true );
    boundRect = boundingRect( contour );
    w_ = boundRect.width * 1.0;
    h_ = boundRect.height * 1.0;
    //cout<<boundRect.x<<":"<<boundRect.y<<":"<<w_<<":"<<h_<<"\n";
//    # Test it's shape - if it's too oblong or tall it's
//    # probably not a real character
    if ((w_ / h_ < 0.1) or (w_ / h_ > 10))
    {
        return false;
    }
    else if ((w_ * h_) > ((img_x * img_y) / 5) or ((w_ * h_) < 15))
    {
         return false;
    }
//    else
//    {
    return true;
//    }
    
}
//# A quick test to check whether the contour is
//# a connected shape    
bool DetectRegions::connected(vector<Point> contour)
{
    Point first,last;
    first = contour.front();
    last = contour.back();
    //cout<< first <<":"<<last<<"\n";
    if(abs(first.x - last.x) <= 1 and abs(first.y - last.y) <= 1)
        return true;
    else
        return false;
}
//---------------------------
//# Count the number of real children
int DetectRegions::count_children(int index, vector<Vec4i> h_, vector<Point> contour)
{
    int count;
    if (h_[index][2] < 0)
        return 0;
    else
    {
        //        #If the first child is a contour we care about
        //        # then count it, otherwise don't
        if(keep(mycontours[h_[index][2]]))
            count = 1;
        else
            count = 0;
        count += count_siblings(h_[index][2], h_, contour, true);
        return count;
    }
}

//---------------------------
//# Get the first parent of the contour that we care about
int DetectRegions::get_parent(int index, vector<Vec4i> h_)
{
    int parent;
    parent = h_[index][3];
    //cout<<index<<":"<<parent<<"\n";
    vector<Point> test = mycontours[1];
//    if(keep(test))
//        cout<<"++++"<<"\n";
//    else
//        cout<<"****"<<"\n";
    //cout<<mycontours[parent]<<"\n";
    
    while(!keep(test) and parent > 0)
        parent = h_[parent][3];
    //cout<<parent<<"\n";
    return parent;
}

//---------------------------
//# Quick check to test if the contour is a child
bool DetectRegions::is_child(int index, vector<Vec4i> h_)
{
    if(get_parent(index, h_)>0)
        return true;
    else
        return false;
}



