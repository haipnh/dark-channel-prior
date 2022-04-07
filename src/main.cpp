#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace cv;
using namespace std;
using namespace std::chrono;

#define DRK_CH_WND_WT 5 //should be an odd number
#define DRK_CH_WND_HT 5 //should be an odd number

struct pix_t { int w; int h; };

#define DISPLAY 1

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

    int img_width = image.size().width;
    int img_height = image.size().height;

    // INIT: dark channel calculation
    unsigned char* dark_channel = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);

    int h=0, w=0;
    int i=0, j=0;
    unsigned char pix_r=0, pix_g=0, pix_b=0;
    unsigned char min_ri=255, min_gi=255, min_bi=255;
    int lh=0, lw=0;
    
    struct pix_t center;
    center.h = DRK_CH_WND_HT/2;
    center.w = DRK_CH_WND_WT/2;

    // INIT: pick up the top p% brightest pixels in the dark channel
    float p=0.1;
    int max_val=255;
    int num_of_pix=(int)((img_height)*(img_width)*p)/100;
    struct pix_t *array_pix = (struct pix_t *)malloc((num_of_pix)*sizeof(struct pix_t));

    // INIT: find the airlight which has the hightest intensity in the input image
    unsigned char* yuv_y = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);
    unsigned char* yuv_u = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);
    unsigned char* yuv_v = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);
    int max_y=0;
    struct pix_t pos;
    unsigned char airlight_r;
    unsigned char airlight_g;
    unsigned char airlight_b;

    // INIT: compute the transmission map
    float omega=0.95;
    float min_r,min_g,min_b,min_t;
    float *tmap = (float*)malloc(sizeof(float)*img_height*img_width);

    // INIT: compute the haze free image
    float t0=0.01;
    float r,g,b,t_;
    unsigned char* output = (unsigned char*)malloc(sizeof(unsigned char)*img_height*img_width*3);
    Mat dehaze_mat(img_height, img_width, CV_8UC3, output);

    

    cv::split(image, channels); // split from BGR image to RGB channels

    // Begin: Pre-Processing
    auto start = high_resolution_clock::now();

    // calculate dark channel
    for(int h = 0; h < image.rows; h++)
    {
        for(int w = 0; w < image.cols; w++)
        {   min_ri=255;
            min_gi=255;
            min_bi=255;
            for(lh=-center.h;lh<=center.h;lh++){ // -2 -> +2
                for(lw=-center.w;lw<=center.w;lw++){ // -2 -> +2
                    if((w+lw)>=0&&(w+lw)<img_width-1&&(h+lh)>=0&&(h+lh)<img_height-1){ // ignore out-of-frame pixels
                        pix_r = channels[0].data[img_width*(h+lh)+(w+lw)];
                        pix_g = channels[1].data[img_width*(h+lh)+(w+lw)];
                        pix_b = channels[2].data[img_width*(h+lh)+(w+lw)];

                        min_ri=MIN(min_ri, pix_r);
                        min_gi=MIN(min_gi, pix_g);
                        min_bi=MIN(min_bi, pix_b);
                    }
                }
            }
            dark_channel[img_width*h+w] = MIN(MIN(min_ri,min_gi),min_bi);
        }
    }
    
    // pick up the top p% brightest pixels in the dark channel
    for(i=0;i<num_of_pix;){
        for(h=0;h<img_height;h++){
            for(w=0;w<img_width;w++){
                if(dark_channel[img_width*h+w]==max_val){
                    array_pix[i].h=h;
                    array_pix[i].w=w;
                    i++;
                }
                if(i==num_of_pix)
                    break;
            }
            if(i==num_of_pix)
                break;
        }
        if(i==num_of_pix)
            break;
        max_val--;
    }
    
    // convert rgb to yuv
    for(h=0;h<img_height;h++){
        for(w=0;w<img_width;w++){
            yuv_y[img_width*h+w]=(( 66*channels[0].data[img_width*h+w]+129*channels[1].data[img_width*h+w] +25*channels[2].data[img_width*h+w]+128)>>8)+16;
            yuv_u[img_width*h+w]=((-38*channels[0].data[img_width*h+w] -74*channels[1].data[img_width*h+w]+112*channels[2].data[img_width*h+w]+128)>>8)+128;
            yuv_v[img_width*h+w]=((112*channels[0].data[img_width*h+w] -94*channels[1].data[img_width*h+w] -18*channels[2].data[img_width*h+w]+128)>>8)+128;
        }
    }

    // find the airlight which has the hightest intensity in the input image
    for(i=0;i<num_of_pix;i++){
        if ( max_y < yuv_y[img_width * array_pix[i].h + array_pix[i].w]){
            pos=array_pix[i];
            max_y=yuv_y[img_width*pos.h+pos.w];
        }
    }
    airlight_r = channels[0].data[img_width*pos.h+pos.w];
    airlight_g = channels[1].data[img_width*pos.h+pos.w];
    airlight_b = channels[2].data[img_width*pos.h+pos.w];
    
    // compute the transmission map
    for(h=0;h<img_height;h++){
        for(w=0;w<img_width;w++){
            min_r=255.0;
            min_g=255.0;
            min_b=255.0;
            for(lh=-center.h;lh<=center.h;lh++){
                for(lw=-center.w;lw<=center.w;lw++){
                    if((lw+w)>=0&&(w+lw)<img_width-1&&(h+lh)>=0&&(h+lh)<img_height-1){
                        min_r=MIN(min_r,(float)channels[0].data[img_width*(h+lh)+(w+lw)]/airlight_r);
                        min_g=MIN(min_g,(float)channels[1].data[img_width*(h+lh)+(w+lw)]/airlight_g);
                        min_b=MIN(min_b,(float)channels[2].data[img_width*(h+lh)+(w+lw)]/airlight_b);
                    }
                }
            }
            min_t=MIN(MIN(min_r,min_g),min_b);
            tmap[img_width*h+w]=1-omega*min_t;
        }
    }

    // compute the haze free image
    for(h=0;h<img_height;h++){
        unsigned char *rowPtr = dehaze_mat.ptr<unsigned char>(h);
        for(w=0;w<img_width;w++){
            t_=MAX(tmap[img_width*h+w],t0);

            r=(((float)channels[0].data[img_width*h+w]-airlight_r)/t_)+airlight_r;
            g=(((float)channels[1].data[img_width*h+w]-airlight_g)/t_)+airlight_g;
            b=(((float)channels[2].data[img_width*h+w]-airlight_b)/t_)+airlight_b;

            rowPtr[w*3+0] = (unsigned char)MIN(MAX(r,0),255); // R
            rowPtr[w*3+1] = (unsigned char)MIN(MAX(g,0),255); // G
            rowPtr[w*3+2] = (unsigned char)MIN(MAX(b,0),255); // B
        }
    }

    // End: Pre-Processing
    auto stop = high_resolution_clock::now();
    float duration = duration_cast<microseconds>(stop - start).count();
    cout << "Processing time: " << to_string(duration) << " us ~ " << to_string(1/(duration/1000000)) << " FPS" << endl;
    cout << "Expected       : 16667 us ~ 60 FPS" << endl;

#if DISPLAY
    namedWindow("1-Input", WINDOW_AUTOSIZE);
    imshow("1-Input", image);

    Mat dark_channel_mat(img_height, img_width, CV_8UC1, dark_channel);
    namedWindow("2-DarkChannel", WINDOW_AUTOSIZE);
    imshow("2-DarkChannel", dark_channel_mat);

    Mat tmap_mat(img_height, img_width, CV_32FC1, tmap);
    namedWindow("3-TransmissionMap", WINDOW_AUTOSIZE);
    imshow("3-TransmissionMap", tmap_mat);

    namedWindow("4-Dehaze", WINDOW_AUTOSIZE);
    imshow("4-Dehaze", dehaze_mat);

    waitKey(0);
#endif
    return 0;
}
