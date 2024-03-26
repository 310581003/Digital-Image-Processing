#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------//
//                   Main                           //
//--------------------------------------------------//
int main() {
    /*
        resolution(path to input file, path to output file, bits per pixel in  one channel of output bmp);
    */
   resolution("../input2.bmp", "output2_3.bmp", 2);
 }

//--------------------------------------------------//
//                   Function                       //
//--------------------------------------------------//
int resolution(const char *fname_s, const char *fname_t, int Quanti_level) {

/*
    Args :
        *fname_s : the path to the input file
        *fname_t : the path to the output file
        Quanti_level : bits per pixel in one channel of output bmp
    Returns :
        0 or 1

*/

//--------- Declaration-------------------//
    FILE          *fp_s = NULL;    // source file handler
    FILE          *fp_t = NULL;    // target file handler
    unsigned int  x,y,c;             // for loop counter
    unsigned int  width, height, n_channel;   // image width, image height
    unsigned short int bitperpixel;
    unsigned int shift_level;
    unsigned char *image_s = NULL; // source image array
    unsigned char *image_t = NULL; // target image array
    unsigned char channel[4] ;     // channels per pixel

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
    printf(">> Input BMP Info ...\n");
    printf("Width : %d\n", width);
    printf("Height : %d\n", height);

    // move offset to 28 to get bit per pixel;
    fseek(fp_s, 28, SEEK_SET);
    fread(&bitperpixel, sizeof(unsigned short int), 1, fp_s);
    n_channel = bitperpixel/8;
    printf("Bits per pixel : %d", bitperpixel);

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

//----------- Quantization Operation -------------------//
    shift_level = 8- Quanti_level;

    for(y = 0; y != height; ++y) {
        for(x = 0; x != width; ++x) {
                for (c=0; c!=n_channel; ++c){

                    channel[c] = *(image_s + n_channel * (width * y + x) + c);

                    *(image_t + n_channel * (width * y + x) + c) = (channel[c] >> shift_level) << shift_level;
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



