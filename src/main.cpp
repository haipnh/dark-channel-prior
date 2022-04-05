#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace cv;
using namespace std;

#define DRK_CH_WND_WT 5 //should be an odd number
#define DRK_CH_WND_HT 5 //should be an odd number

int main( int argc, char** argv )
{
    Mat image;
    Mat channels[3];

    image = imread("images/1.png", IMREAD_UNCHANGED); // Read as BGR matrix, bit-depth: CV_8U
    if( image.empty() ) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::split(image, channels); // split from BGR image to RGB channels

    int img_width = image.size().width;
    int img_height = image.size().height;

    unsigned char* dark_channel = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);

    int min_ri=255, min_gi=255, min_bi=255;
    int lh=0, lw=0;

    // calculate dark channel
    int filter_w = DRK_CH_WND_WT/2;
    int filter_h = DRK_CH_WND_HT/2;

    for(int h = 0; h < image.rows; h++)
    {
        for(int w = 0; w < image.cols; w++)
        {   min_ri=255;
            min_gi=255;
            min_bi=255;
            for(lh=-filter_h;lh<=filter_h;lh++){ // -2 -> +2
                for(lw=-filter_w;lw<=filter_w;lw++){ // -2 -> +2
                    if((w+lw)>=0&&(w+lw)<img_width-1&&(h+lh)>=0&&(h+lh)<img_height-1){ // ignore out-of-frame pixels
                        min_ri=MIN(min_ri, channels[0].data[img_width*(h+lh)+(w+lw)]);
                        min_gi=MIN(min_gi, channels[1].data[img_width*(h+lh)+(w+lw)]);
                        min_bi=MIN(min_bi, channels[2].data[img_width*(h+lh)+(w+lw)]);
                    }
                }
            }
            dark_channel[img_width*h+w] = MIN(MIN(min_ri,min_gi),min_bi);
        }
    }

    // To do:


    Mat dark_channel_mat(img_height, img_width, CV_8UC1, dark_channel);
    namedWindow("DarkChannel",WINDOW_AUTOSIZE);
    imshow("DarkChannel", dark_channel_mat);
    waitKey(0);

    cout << "Done!\n";

    return 0;
}
