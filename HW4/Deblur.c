
#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
#include <complex.h>

//--------------------------------------------------//
//                   Main                           //
//--------------------------------------------------//
int main() {
    /*
        Restoration (path to input file, path to output file);
    */
    Restoration ("input1.bmp", "output1.bmp");
    Restoration ("input2.bmp", "output2.bmp");
    printf("------ Finish!!\n");
 }

//--------------------------------------------------//
//                   Function                       //
//--------------------------------------------------//
int Restoration (const char *fname_s, const char *fname_t) {

/*
    Args :
        *fname_s : the path to the input file
        *fname_t : the path to the output file

    Returns :
        0 or 1

*/

//-----------------------------  Declaration ----------------------------------//

    FILE          *fp_s = NULL;    // source file handler
    FILE          *fp_t = NULL;    // target file handler
    unsigned int  x,y,c;             // for loop counter
    unsigned int  width, height, n_channel;   // image width, image height
    unsigned short int bitperpixel;// bit per pixel
    unsigned char *image_s = NULL; // source image array
    unsigned char *image_t = NULL; // target image array
    unsigned char channel[4] ;     // channels per pixel
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

//--------------------------- Readout input bmp -------------------------------------//
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
    printf(">> Info of input image : \n", width);
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

//-------------------- Build two memory array to store bmp data -------------------------//
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


//---------------------------- FFT of Input images -------------------------------------//

    // Declaration
    fftw_complex *in_r, *in_g, *in_b;
    fftw_complex *out_r, *out_g, *out_b;
    fftw_plan plan_r, plan_g, plan_b;

    in_r = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);
    out_r = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);

    in_g = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);
    out_g = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);

    in_b = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);
    out_b = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);

    // Initialize for FFT
    plan_r = fftw_plan_dft_2d(height,width,  in_r, out_r, FFTW_FORWARD, FFTW_ESTIMATE);
    plan_g = fftw_plan_dft_2d(height,width,  in_g, out_g, FFTW_FORWARD, FFTW_ESTIMATE);
    plan_b = fftw_plan_dft_2d(height,width,  in_b, out_b, FFTW_FORWARD, FFTW_ESTIMATE);

    // Define the input value for FFT
    for(y = 0; y != height; ++y) {
            for(x = 0; x != width; ++x) {
                        in_r[width * y + x][0] = (double)*(image_s + n_channel * (width * y + x) + 2);
                        in_r[width * y + x][1] = 0;

                        in_g[width * y + x][0] = (double)*(image_s + n_channel * (width * y + x) + 1);

                        in_g[width * y + x][1] = 0;

                        in_b[width * y + x][0] = (double)*(image_s + n_channel * (width * y + x) + 0);
                        in_b[width * y + x][1] = 0;
            }
    }



    // Do FFT
    fftw_execute(plan_r);
    fftw_destroy_plan(plan_r);

    fftw_execute(plan_g);
    fftw_destroy_plan(plan_g);

    fftw_execute(plan_b);
    fftw_destroy_plan(plan_b);


    //---------------------------- Definition of H -------------------------------------//
    int lines;
    FILE *fp_mot_real, *fp_mot_imag, *fp_gau_real, *fp_gau_imag;
    int i;

    if (width==400 && height==300){
        fp_mot_real = fopen("./H/H_mot_real_1.txt", "r");
        fp_mot_imag = fopen("./H/H_mot_imag_1.txt", "r");
        fp_gau_real = fopen("./H/H_gau_real_1.txt", "r");
        fp_gau_imag = fopen("./H/H_gau_imag_1.txt", "r");

    }
    else if (width==1600 && height==1200){
        fp_mot_real = fopen("./H/H_mot_real_2.txt", "r");
        fp_mot_imag = fopen("./H/H_mot_imag_2.txt", "r");
        fp_gau_real = fopen("./H/H_gau_real_2.txt", "r");
        fp_gau_imag = fopen("./H/H_gau_imag_2.txt", "r");
    }

    // Motion OTF in real part
    float *H_mot_real=(float *) malloc(sizeof(float)*width*height);
    if(fp_mot_real == NULL)
        return ;
    lines =0;

    while(lines < height)
    {
        for(i = 0; i < width; i ++){
            fscanf(fp_mot_real, "%f",(H_mot_real+lines*width+i));
        }

        if(feof(fp_mot_real)) break;
        lines++;

    }
    fclose(fp_mot_real);

    // Motion OTF in image part
    float *H_mot_imag=(float *) malloc(sizeof(float)*width*height);
    if(fp_mot_imag == NULL)
        return;
    lines =0;
    while(lines < height)
    {
        for(i = 0; i < width; i ++){
            fscanf(fp_mot_imag, "%f",(H_mot_imag+lines*width+i));
        }

        if(feof(fp_mot_imag)) break;
        lines++;

    }
    fclose(fp_mot_imag);

    // Guassian OTF in real part
    float *H_gua_real=(float *) malloc(sizeof(float)*width*height);
    if(fp_gau_real == NULL)
        return;
    lines =0;
    while(lines < height)
    {
        for(i = 0; i < width; i ++){
            fscanf(fp_gau_real, "%f",(H_gua_real+lines*width+i));
        }

        if(feof(fp_gau_real)) break;
        lines++;

    }
    fclose(fp_gau_real);

    // Guassian OTF in image part
    float *H_gua_imag=(float *) malloc(sizeof(float)*width*height);
    if(fp_gau_imag == NULL)
        return;
    lines =0;
    while(lines < height)
    {
        for(i = 0; i < width; i ++){
            fscanf(fp_gau_imag, "%f",(H_gua_imag+lines*width+i));
        }

        if(feof(fp_gau_imag)) break;
        lines++;

    }
    fclose(fp_gau_imag);
    //---------------------------- Wiener Filtering -------------------------------------//
    for(y = 0; y != height; ++y) {
                for(x = 0; x != width; ++x) {

                        complex double temp_r, temp_g, temp_b;
                        complex double H_r, H_g, H_b;
                        double k;

                        temp_r = out_r[width * y + x][0] + out_r[width * y + x][1] * _Complex_I;
                        temp_g = out_g[width * y + x][0] + out_g[width * y + x][1] * _Complex_I;
                        temp_b = out_b[width * y + x][0] + out_b[width * y + x][1] * _Complex_I;

                        // Wiener Filtering based on motion otf
                        k = 0.02;
                        H_r = *(H_mot_real+width * (height-y-1) + x) + *(H_mot_imag+width * (height-y-1) + x) * _Complex_I;
                        H_g = H_r; H_b = H_r;
                        temp_r = ( conj(H_r) / ((fabs( H_r*conj(H_r) )) + k) ) * temp_r;
                        temp_g = ( conj(H_g) / ((fabs( H_g*conj(H_g) )) + k) ) * temp_g;
                        temp_b = ( conj(H_b) / ((fabs( H_b*conj(H_b) )) + k) ) * temp_b;

                        // Wiener Filtering based on Gaussian otf
                        k = 0.05;
                        H_r = *(H_gua_real+width * (height-y-1) + x) + *(H_gua_imag+width * (height-y-1) + x) * _Complex_I;
                        H_g = H_r; H_b = H_r;
                        temp_r = ( conj(H_r) / ((fabs( H_r*conj(H_r) )) + k) ) * temp_r;
                        temp_g = ( conj(H_g) / ((fabs( H_g*conj(H_g) )) + k) ) * temp_g;
                        temp_b = ( conj(H_b) / ((fabs( H_b*conj(H_b) )) + k) ) * temp_b;


                        out_r[width * y + x][0] = creal(temp_r); out_r[width * y + x][1] = cimag(temp_r);
                        out_g[width * y + x][0] = creal(temp_g); out_g[width * y + x][1] = cimag(temp_g);
                        out_b[width * y + x][0] = creal(temp_b); out_b[width * y + x][1] = cimag(temp_b);
                }
    }


    //------------------------------ Inverse FFT -----------------------------------------//
    // Declaration
    fftw_complex *outt_r, *outt_g, *outt_b;
    fftw_plan plan_rr, plan_gg, plan_bb;

    outt_r = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);
    outt_g = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);
    outt_b = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * height);

    // Initialize for FFT
    plan_rr = fftw_plan_dft_2d(height,width,  out_r, outt_r, FFTW_BACKWARD, FFTW_ESTIMATE);
    plan_gg = fftw_plan_dft_2d(height,width,  out_g, outt_g, FFTW_BACKWARD, FFTW_ESTIMATE);
    plan_bb = fftw_plan_dft_2d(height,width,  out_b, outt_b, FFTW_BACKWARD, FFTW_ESTIMATE);


    // Do FFT
    fftw_execute(plan_rr);
    fftw_execute(plan_gg);
    fftw_execute(plan_bb);


    //------------------------------ Normalization -----------------------------------------//
    double max[3] = {0, 0 , 0};
    double min[3] = {1000000000000, 1000000000000 , 1000000000000};


    for(y = 0; y != height; ++y) {
            for(x = 0; x != width; ++x) {

                        if (fabs(outt_r[width * y + x][0]) >max[2]) max[2] = fabs(outt_r[width * y + x][0]);
                        if (fabs(outt_r[width * y + x][0]) <min[2]) min[2] = fabs(outt_r[width * y + x][0]);

                        if (fabs(outt_g[width * y + x][0]) >max[1]) max[1] = fabs(outt_g[width * y + x][0]);
                        if (fabs(outt_g[width * y + x][0]) <min[1]) min[1] = fabs(outt_g[width * y + x][0]);

                        if (fabs(outt_b[width * y + x][0]) >max[0]) max[0] = fabs(outt_b[width * y + x][0]);
                        if (fabs(outt_b[width * y + x][0]) <min[0]) min[0] = fabs(outt_b[width * y + x][0]);
            }
    }

    double max_fin = 0;
    double min_fin = 1000000000000;
    for(c = 0; c != 3; ++c) {

            if (max[c]>max_fin) max_fin=max[c];
            if (min[c]<min_fin) min_fin=min[c];
    }

        for(y = 0; y != height; ++y) {
                for(x = 0; x != width; ++x) {

                        *(image_t + n_channel * (width * y + x) + 2) = ( abs(outt_r[width * y + x][0]) - min_fin ) * ( 1 / (max_fin-min_fin) ) * 255;
                        *(image_t + n_channel * (width * y + x) + 1) = ( abs(outt_g[width * y + x][0]) - min_fin ) * ( 1 / (max_fin-min_fin) ) * 255;
                        *(image_t + n_channel * (width * y + x) + 0) = ( abs(outt_b[width * y + x][0]) - min_fin ) * ( 1 / (max_fin-min_fin) ) * 255;

                }
        }


    //--------------------------- Write back to a new bmp and output ----------------------------------//
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


