Operation : Low-luminosity Enhancement

File name : Intensity_enh.c

Steps : 
	1. Open file "Intensity_enh.c".
	2. Change the arguments in line 13 and 14. The first argumet of function "luminosity_enh" is the path of input image, the second one is the path of output image, and the third one is to define the method you would like to use for low-luminosity enhancement (where mode=0 is Histogram equalization and mode=1 is gamma transformation). As the followings, 
		luminosity_enh ("../input1.bmp", "output1_1.bmp", mode = 0); 
		luminosity_enh ("../input1.bmp", "output1_2.bmp", mode = 1);
	3. Press compile and run.


