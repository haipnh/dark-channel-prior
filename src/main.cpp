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

//#define MAX(x,y) ((x)<=(y)? (y):(x))
//#define MIN(x,y) ((x)<=(y)? (x):(y))

unsigned char **Malloc2D_uchr(int wt, int ht,unsigned char init)
{
    int h,w;
    unsigned char **m;

    m=(unsigned char **)malloc(sizeof(void *)*ht);
    if(m==NULL){
        printf("malloc failed (%s,%i)\n",__FILE__,__LINE__);
        exit(-1);
    }
    m[0]=(unsigned char *)malloc(sizeof(unsigned char)*ht*wt);
    if(m[0]==NULL){
        printf("malloc failed (%s,%i)\n",__FILE__,__LINE__);
        exit(-1);
    }

    for(h=1;h<ht;h++){
        m[h]=m[h-1]+wt;
    }
    for(h=0;h<ht;h++){
        for(w=0;w<wt;w++){
            m[h][w]=init;
        }
    }

    return m;
}

// void split(cv::Mat input, unsigned char )

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

    int cn = image.channels();

    unsigned char** dark_channel = Malloc2D_uchr(img_width, img_height, 0);

    //Scalar_<uint8_t> bgrPixel;
    unsigned char bgrPixel[3];

    int min_ri=255, min_gi=255, min_bi=255;
    int lh=0, lw=0;

    unsigned char* rowPtrRed   = channels[0].ptr<unsigned char>(0); // init pointer
    unsigned char* rowPtrGreen = channels[1].ptr<unsigned char>(0); // init pointer
    unsigned char* rowPtrBlue  = channels[2].ptr<unsigned char>(0); // init pointer

    // unsigned char* data = (unsigned char *)malloc(sizeof(unsigned char)*img_height*img_width);


    // calculate dark channel
    for(int h = 0; h < image.rows; h++)
    {
        //rowPtrRed   = channels[0].ptr<unsigned char>();
        //rowPtrGreen = channels[1].ptr<unsigned char>();
        //rowPtrBlue  = channels[2].ptr<unsigned char>(h);
        for(int w = 0; w < image.cols; w++)
        {   /* //Method 1
            bgrPixel.val[0] = rowPtr[j*cn + 0]; // B
            bgrPixel.val[1] = rowPtr[j*cn + 1]; // G
            bgrPixel.val[2] = rowPtr[j*cn + 2]; // R */
            // Method 2
            // bgrPixel[0] = rowPtr[j*cn + 0];
            // bgrPixel[1] = rowPtr[j*cn + 1];
            // bgrPixel[2] = rowPtr[j*cn + 2];
            min_ri=255;
            min_gi=255;
            min_bi=255;

            // data[img_width*(h+lh)+(w+lw)] = channels[2].data[img_width*(h+lh)+(w+lw)];
            // assert(data[img_width*(h+lh)+(w+lw)]==rowPtrBlue[w]);

            for(lh=-2;lh<=2;lh++){ // -2 -> +2
                for(lw=-2;lw<=2;lw++){ // -2 -> +2
                    if((w+lw)>=0&&(w+lw)<img_width-1&&(h+lh)>=0&&(h+lh)<img_height-1){ // ignore out-of-frame pixels
                        min_ri=MIN(min_ri, channels[0].data[img_width*(h+lh)+(w+lw)]);
                        min_gi=MIN(min_gi, channels[1].data[img_width*(h+lh)+(w+lw)]);
                        min_bi=MIN(min_bi, channels[2].data[img_width*(h+lh)+(w+lw)]);
                    }
                }
            }
            // dark_channel[h][w] = rowPtr[w*cn+2];
            // assert(dark_channel[h][w]==rowPtrBlue[w]);
            // cout << to_string(bgrPixel[0]) << " " << to_string(bgrPixel[1]) << " " << to_string(bgrPixel[2]) << '\n';
        }
    }


    // Mat output(img_height, img_width, CV_8UC1, dark_channel);
    Mat output(img_height, img_width, CV_8UC1, data);
    cout << output.size();
    cout << channels[2].size();
    // std::memcpy(output.data, dark_channel, img_height*img_width*sizeof(unsigned char));

    // namedWindow("dark_channel",WINDOW_AUTOSIZE);
    // imshow("dark_channel", output);

    namedWindow("CopiedBlue",WINDOW_AUTOSIZE);
    imshow("CopiedBlue", output);

    namedWindow("Blue",WINDOW_AUTOSIZE);
    imshow("Blue", channels[2]);

     /*
         cout << img_width << "\n";
    cout << image.size[1] << "\n";

    cout << img_height << "\n";
    cout << image.size[0] << "\n";
     namedWindow( "Original Image", WINDOW_AUTOSIZE );
     imshow( "Original Image", image);
     namedWindow("Blue",WINDOW_AUTOSIZE);
     imshow("Red", channels[0]);
     namedWindow("Green",WINDOW_AUTOSIZE);
     imshow("Green", channels[1]);
     namedWindow("Red",WINDOW_AUTOSIZE);
     imshow("Blue", channels[2]);
     */



     waitKey(0);

    // compute sum of positive matrix elements
    // (assuming that M isa double-precision matrix)
    /*
    double sum=0;
    for(int i = 0; i < image.rows; i++)
    {
        const double* Mi = image.ptr<double>(i);
        for(int j = 0; j < image.cols; j++)
            sum += max(Mi[j], 0.);
    }
    cout << sum << "\n";
    double omega=0.95;
    double t0=0.01;
    double p=0.1;

    unsigned int* in;
    unsigned int* out;
    */

    // dehaze(in, out, img_width, img_height, omega, t0, p);
    cout << "Done!\n";

    return 0;
}
