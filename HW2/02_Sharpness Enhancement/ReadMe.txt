Operation : Sharpness Enhancement

File name : Sharpness_enh.c

Steps : 
	1. Open file "Sharpness_enh.c".
	2. Change the arguments in line 14 and 15. The first argumet of function "sharp" is the path of input image, the second one is the path of output image, and the third one is to define the kernel you would like to use for sharpness enhancement (where the kernel is {{0, -1, 0},{ -1, 5, -1},{ 0, -1, 0}} when mode=0 and the kernel is {{-1 ,-1, -1}, {-1, 9, -1}, {-1, -1, -1}} when mode=1). As the followings, 
		sharp("../input2.bmp", "output2_1.bmp", mode=0);
		sharp("../input2.bmp", "output2_2.bmp", mode=1);
	3. Press compile and run.


