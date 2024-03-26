#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------//
//                   Main                           //
//--------------------------------------------------//
int main() {
    /*
        flip(path to input file, path to output file, mode);
        * mode=0: Gaussian smoothing
        * mode=1: Weighted-average filtering
    */
    int mode;
    Denoise ("../input3.bmp", "output3_1.bmp", mode=0);
    Denoise ("../input3.bmp", "output3_2.bmp", mode=1);
    printf("\n");
    printf("------ Finish!!\n");
 }

//--------------------------------------------------//
//                   Function                       //
//--------------------------------------------------//
int Denoise (const char *fname_s, const char *fname_t, int mode) {

/*
    Args :
        *fname_s : the path to the input file
        *fname_t : the path to the output file
        mode     : method for denoising
            - mode=0: Gaussian smoothing
            - mode=1: Weighted-average filtering

    Returns :
        0 or 1

*/

//--------- Declaration-------------------//
    FILE          *fp_s = NULL;    // source file handler
    FILE          *fp_t = NULL;    // target file handler
    unsigned int  x,y,c;             // for loop counter
    unsigned int  width, height, n_channel;   // image width, image height
    unsigned short int bitperpixel;// bit per pixel
    unsigned char *image_s = NULL; // source image array
    unsigned char *image_t = NULL; // target image array
    double channel[4] ;     // channels per pixel
    unsigned int x_avg;            // average of x axis
    unsigned int x_t;              // target of x axis

    unsigned char header[54] = {
    // File Header
        0x42, 0x4d,  // Signature : 'BM'
        0, 0, 0, 0,  // FileSize
        0, 0,        // Reserved1
        0, 0,        // Reserved2
        54, 0, 0, 0, // DataOffset
        // Info Header
        40, 0, 0, 0, // Info Header size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        24, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // image size
        0, 0, 0, 0,  // x resolution
        0, 0, 0, 0,  // y resolution
        0, 0, 0, 0,  // ncolours
        0, 0, 0, 0   // important colors
    };

    unsigned int file_size;           // file size
    unsigned int rgb_raw_data_offset; // RGB raw data offset

//----------- Readout input bmp-------------------//
    fp_s = fopen(fname_s, "rb");
    if (fp_s == NULL) {
        printf("fopen fp_s error\n");
        return -1;
    }

    // move offset to 10 to get Dataoffset
    fseek(fp_s, 10, SEEK_SET);
    fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s);

    // move offset to 18 to get width & height;
    fseek(fp_s, 18, SEEK_SET);
    fread(&width,  sizeof(unsigned int), 1, fp_s);
    fread(&height, sizeof(unsigned int), 1, fp_s);
    printf(">> Info of input image : \n");
    printf("Width : %d\n", width);
    printf("Height : %d\n", height);

    // move offset to 28 to get width & height;
    fseek(fp_s, 28, SEEK_SET);
    fread(&bitperpixel, sizeof(unsigned short int), 1, fp_s);
    n_channel = bitperpixel/8;
    printf("Bit per pixel : %d\n", bitperpixel);
    printf("Channel per pixel : %d\n", n_channel);

    // move offset to Dataoffset to get bmp raw data
    fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

//----------- Build two memory array to store bmp data-------------------//
    image_s = (unsigned char *)malloc((size_t)width * height * n_channel);
    if (image_s == NULL) {
        printf("malloc images_s error\n");
        return -1;
    }

    image_t = (unsigned char *)malloc((size_t)width * height * n_channel);
    if (image_t == NULL) {
        printf("malloc image_t error\n");
        return -1;
    }

    fread(image_s, sizeof(unsigned char), (size_t)(long)width * height * n_channel, fp_s);

//----------- Denoising operation-------------------//

    // Weighted-average filter
    double conv_a[9][4];
    double mask_a[3][3]={{0.11111 ,0.11111, 0.11111}, {0.11111, 0.11111, 0.11111}, {0.11111, 0.11111, 0.11111}};

    // Gaussian smoothing filter
    double conv_g[25][4];
    double mask_g[5][5]={{3.663e-3,  0.014652, 0.025641, 0.014652, 3.663e-3},
                          {0.014652, 0.058608, 0.095238, 0.058608, 0.014652},
                          {0.025641, 0.095238, 0.14652, 0.095238, 0.025641},
                          {0.014652, 0.058608, 0.095238, 0.058608, 0.014652},
                          {3.663e-3 ,0.014652, 0.025641, 0.014652, 3.663e-3}};

    // Gaussian smoothing
    if (mode==0){
        printf(">> Gaussian smoothing (5*5 kernel; sigma=1)...\n");
        for(y = 0; y != height; ++y) {
            for(x = 0; x != width; ++x) {
                for (c=0; c!=n_channel; ++c){

                        // Convolution
                        if (x<2 || y<2)                     conv_g[0][c] = 0;
                        else                                conv_g[0][c] =    (*(image_s + n_channel * (width * y + x-12)  + c)*mask_g[0][0]);

                        if (x<1 || y<2)                     conv_g[1][c] = 0;
                        else                                conv_g[1][c] =    (*(image_s + n_channel * (width * y + x-11)  + c)*mask_g[0][1]);

                        if(y<2)                             conv_g[2][c] = 0;
                        else                                conv_g[2][c] =    (*(image_s + n_channel * (width * y +  x-10 )  + c)*mask_g[0][2]);

                        if(x+1>=width || y<2)               conv_g[3][c] = 0;
                        else                                conv_g[3][c] =    (*(image_s + n_channel * (width * y +  x-9 )  + c)*mask_g[0][3]);

                        if(x+2>=width || y<2)               conv_g[4][c] = 0;
                        else                                conv_g[4][c] =    (*(image_s + n_channel * (width * y +  x-8 )  + c)*mask_g[0][4]) ;

                        if (x<2 || y<1)                     conv_g[5][c] = 0;
                        else                                conv_g[5][c] =    (*(image_s + n_channel * (width * y +  x-7 )  + c)*mask_g[1][0]);

                        if (x<1 || y<1)                     conv_g[6][c] = 0;
                        else                                conv_g[6][c] =    (*(image_s + n_channel * (width * y +  x-6 )  + c)*mask_g[1][1]);

                        if ( y<1)                           conv_g[7][c] = 0;
                        else                                conv_g[7][c] =    (*(image_s + n_channel * (width * y +  x-5 )  + c)*mask_g[1][2]);

                        if ( y<1 || x+1>=width)             conv_g[8][c] = 0;
                        else                                conv_g[8][c] =    (*(image_s + n_channel * (width * y +  x-4 )  + c)*mask_g[1][3]);

                        if ( y<1 || x+2>=width)             conv_g[9][c] = 0;
                        else                                conv_g[9][c] =    (*(image_s + n_channel * (width * y +  x-3 )  + c)*mask_g[1][4]) ;

                        if ( x<2)                           conv_g[10][c] = 0;
                        else                                conv_g[10][c] =    (*(image_s + n_channel * (width * y +  x-2)  + c)*mask_g[2][0]);

                        if ( x<1)                           conv_g[11][c] = 0;
                        else                                conv_g[11][c] =    (*(image_s + n_channel * (width * y +  x-1)  + c)*mask_g[2][1]);

                                                            conv_g[12][c] =    (*(image_s + n_channel * (width * y +  x )  + c)*mask_g[2][2]);

                        if (  x+1>=width)                   conv_g[13][c] = 0;
                        else                                conv_g[13][c] =    (*(image_s + n_channel * (width * y +  x+1 )  + c)*mask_g[2][3]);

                        if (  x+2>=width)                   conv_g[14][c] = 0;
                        else                                conv_g[14][c] =    (*(image_s + n_channel * (width * y +  x+2 )  + c)*mask_g[2][4]) ;

                        if (  x<2 || y+1>=height)           conv_g[15][c] = 0;
                        else                                conv_g[15][c] =    (*(image_s + n_channel * (width * y +  x+3)  + c)*mask_g[3][0]);

                        if (  x<1 || y+1>=height)           conv_g[16][c] = 0;
                        else                                conv_g[16][c] =    (*(image_s + n_channel * (width * y +  x+4)  + c)*mask_g[3][1]);

                        if (  y+1>=height)                  conv_g[17][c] = 0;
                        else                                conv_g[17][c] =    (*(image_s + n_channel * (width * y +  x+5 )  + c)*mask_g[3][2]);

                        if (  y+1>=height || x+1>=width)    conv_g[18][c] = 0;
                        else                                conv_g[18][c] =    (*(image_s + n_channel * (width * y +  x+6)  + c)*mask_g[3][3]);

                        if (  y+1>=height || x+2>=width)    conv_g[19][c] = 0;
                        else                                conv_g[19][c] =    (*(image_s + n_channel * (width * y +  x+7)  + c)*mask_g[3][4]) ;

                        if (  y+2>=height || x<2)           conv_g[20][c] = 0;
                        else                                conv_g[20][c] =    (*(image_s + n_channel * (width * y +  x+8)  + c)*mask_g[4][0]);

                        if (  y+2>=height || x<1)           conv_g[21][c] = 0;
                        else                                conv_g[21][c] =    (*(image_s + n_channel * (width * y +  x+9)  + c)*mask_g[4][1]);

                        if (  y+2>=height)                  conv_g[22][c] = 0;
                        else                                conv_g[22][c] =    (*(image_s + n_channel * (width * y +  x+10)  + c)*mask_g[4][2]);

                        if (  y+2>=height|| x+1>=width)     conv_g[23][c] = 0;
                        else                                conv_g[23][c] =    (*(image_s + n_channel * (width * y +  x+11 )  + c)*mask_g[4][3]);

                        if (  y+2>=height|| x+2>=width)     conv_g[24][c] = 0;
                        else                                conv_g[24][c] =    (*(image_s + n_channel * (width * y +  x+12 ) + c)*mask_g[4][4]) ;


                        channel[c] =   (conv_g[0][c]   + conv_g[1][c]  + conv_g[2][c]  + conv_g[3][c]  + conv_g[4][c] +
                                        conv_g[5][c]   + conv_g[6][c]  + conv_g[7][c]  + conv_g[8][c]  + conv_g[9][c] +
                                        conv_g[10][c]  + conv_g[11][c] + conv_g[12][c] + conv_g[13][c] + conv_g[14][c] +
                                        conv_g[15][c]  + conv_g[16][c] + conv_g[17][c] + conv_g[18][c] + conv_g[19][c] +
                                        conv_g[20][c]  + conv_g[21][c] + conv_g[22][c] + conv_g[23][c] + conv_g[24][c] ) ;
                        // Clamping
                        if(channel[c]>255)
                            *(image_t + n_channel * (width * y + x) + c) = 255;
                        else if(channel[c]<0)
                            *(image_t + n_channel * (width * y + x) + c) = *(image_s + n_channel * (width * y + x) + c)-0;
                        else
                            *(image_t + n_channel * (width * y + x) + c) = channel[c];

                }
            }
        }
    }
    // Weighted-average filtering
    else if(mode==1){
        printf(">> Weight-average filtering (3*3 kernel)...\n");
        for(y = 0; y != height; ++y) {
            for(x = 0; x != width; ++x) {
                for (c=0; c!=n_channel; ++c){

                        // Convolution
                        if (x==0 || y==0)               conv_a[0][c] = 0;
                        else                            conv_a[0][c] = (*(image_s + n_channel * ((width * y) + (x-4))  + c)*mask_a[0][0]);

                        if (y==0)                       conv_a[1][c] = 0;
                        else                            conv_a[1][c] =    (*(image_s + n_channel * ((width * y) + ( x-3 ))  + c)*mask_a[0][1]);

                        if(y==0 || x+1>=width)          conv_a[2][c] = 0;
                        else                            conv_a[2][c] =    (*(image_s + n_channel * ((width * y) + (x-2))  + c)*mask_a[0][2]);

                        if(x==0)                        conv_a[3][c] = 0;
                        else                            conv_a[3][c] =    (*(image_s + n_channel * (width * ( y ) + (x-1))  + c)*mask_a[1][0]);

                        if(x+1>=width)                  conv_a[5][c] = 0;
                        else                            conv_a[5][c] =    (*(image_s + n_channel * (width * (y) + (x+1))  + c)*mask_a[1][2]) ;

                        if(x==0 || y+1>=height)         conv_a[6][c] =  0;
                        else                            conv_a[6][c] =    (*(image_s + n_channel * ((width * y) + (x+2))  + c)*mask_a[2][0]);

                        if(y+1>=height)                 conv_a[7][c] = 0;
                        else                            conv_a[7][c] =    (*(image_s + n_channel * ((width * y) + ( x+3 ))  + c)*mask_a[2][1]) ;

                        if(x+1>=width || y+1>=height)   conv_a[8][c] = 0;
                        else                            conv_a[8][c] =    (*(image_s + n_channel * ((width * y) + (x+4))  + c)*mask_a[2][2]) ;

                        conv_a[4][c] =   *(image_s + n_channel * (width * ( y ) + ( x ))  + c)*mask_a[1][1] ;

                        channel[c] =    conv_a[0][c] + conv_a[1][c] + conv_a[2][c] + conv_a[3][c] + conv_a[4][c] +
                                        conv_a[5][c] + conv_a[6][c] + conv_a[7][c] + conv_a[8][c] ;


                        // Clamping
                        if(channel[c]>255)
                            *(image_t + n_channel * (width * y + x) + c) =  255;
                        else if(channel[c]<0)
                            *(image_t + n_channel * (width * y + x) + c) =  *(image_s + n_channel * (width * y + x) + c)-0;
                        else
                            *(image_t + n_channel * (width * y + x) + c) =  channel[c];

                    }
            }
        }
    }

//----------- Write back to a new bmp and output-------------------//

    fp_t = fopen(fname_t, "wb");
    if (fp_t == NULL) {
        printf("fopen fname_t error\n");
        return -1;
        }

    // file size
    file_size = width * height * n_channel + rgb_raw_data_offset;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8)  & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8)  & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    header[22] = height &0x000000ff;
    header[23] = (height >> 8)  & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    // bit per pixel
    header[28] = bitperpixel & 0x000000ff;
    header[29] = (bitperpixel >> 8) & 0x000000ff;

    // write header
    fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);
    // write image
    fwrite(image_t, sizeof(unsigned char), (size_t)(long)width * height * n_channel, fp_t);

    fclose(fp_s);
    fclose(fp_t);

    return 0;
 }


