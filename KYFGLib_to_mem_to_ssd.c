/************************************************************************ 
*	File: KYFGLib_Example.cpp
*	Sample Predator Frame Grabber API application
*
*	KAYA Instruments Ltd.
*************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
//#include <dir.h>
#include "KYFGLib.h"
#include <time.h> 
#include <math.h> 


#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif
int f;
char fileName[1000];
int createFile = 0;
int flagPrint = 0;;
long long savedFrame = 0;
long long maxFrame = 10;
long long totalFrames = 0;
long long buffSize = 0;
time_t ticks_beg, ticks_end;
static int firstFlag = 1;
void* memdata;
long long width=1280, height=720, offsetx=-1, offsety=-1; // тут храним числа


void Stream_callback_func(void* userContext, STREAM_HANDLE streamHandle)
{
	static void* data = 0;
	static KYBOOL copyingDataFlag = KYFALSE;
	static clock_t ticks1, ticks_c;
	int buffIndex;
	void* buffData;

    if(0 == streamHandle)		// callback with indicator for acquisition stop
        return;

	totalFrames = KYFG_GetGrabberValueInt(streamHandle, "RXFrameCounter");
	buffSize = KYFG_StreamGetSize(streamHandle);			// get buffer size
	buffIndex = KYFG_StreamGetFrameIndex(streamHandle);
    buffData = KYFG_StreamGetPtr(streamHandle, buffIndex);		// get pointer of buffer data

//    printf("CallBack!\n");

	if (createFile == 1)
	{
		if ((savedFrame<maxFrame)&&(1==1))
		{
	//		 fwrite(buffData, 1, buffSize, f);
	//		write(f, buffData, buffSize);
//			printf("savedFrame = %lld\n", savedFrame);

	        memcpy(memdata+(savedFrame*width*height), buffData, (size_t)buffSize);			// copy data to local buffer
//	        memcpy(memdata, buffData, (size_t)buffSize);			// copy data to local buffer
//			printf("memdata = %p, memdata+hws = %p, hws = %lld\n", memdata, memdata+(savedFrame*width*height), savedFrame*width*height);


			savedFrame ++;
			if (savedFrame == 1)
				ticks_c = clock();
			if (savedFrame > 1)
			{
				if (((float)(clock()-ticks_c))/CLOCKS_PER_SEC >= 1)
				{
					// printf("fileName = %s, number saved frames %d, delta t, %f                               \r", fileName, savedFrame, ((float)(clock()-ticks1))/CLOCKS_PER_SEC);
					printf("fileName = %s, number saved frames %lld, total frames:%lld                           \r", fileName, savedFrame, totalFrames);
					// printf("\n");
					ticks_c = clock();
				}

				flagPrint = 1;
			}
			// ticks1 = clock();
		}
//								printf("done! \n");

	}

}

/*void Stream_callback_func(void* userContext, STREAM_HANDLE streamHandle)
{
    static void* data = 0;
    static KYBOOL copyingDataFlag = KYFALSE;
    long long buffSize = 0;
    int buffIndex;
    void* buffData;
    printf("CallBack!\n");

    if(0 == streamHandle)		// callback with indicator for acquisition stop
    {
        copyingDataFlag = KYFALSE;
        return;
    }

    totalFrames = KYFG_GetGrabberValueInt(streamHandle, "RXFrameCounter");
    buffSize = KYFG_StreamGetSize(streamHandle);			// get buffer size
    buffIndex = KYFG_StreamGetFrameIndex(streamHandle);
    buffData = KYFG_StreamGetPtr(streamHandle, buffIndex);		// get pointer of buffer data

    if(KYFALSE == copyingDataFlag)
    {
		savedFrame ++;
        copyingDataFlag = KYTRUE;
        data = (void*)realloc(data, (size_t)buffSize); 		// allocate size for local buffer
        if(0 == data)
        {
            return;
        }
        printf("\rGood callback buffer handle:%X, current index:%" PRISTREAM_HANDLE ", total frames:%lld        ", streamHandle, buffIndex, savedFrame);
        memcpy(data, buffData, (size_t)buffSize);			// copy data to local buffer
        //... Show Image with data ...
        copyingDataFlag = KYFALSE;
    }
}
*/

#define MAXBOARDS 4
FGHANDLE handle[MAXBOARDS];

int connectToGrabber(unsigned int grabberIndex)
{
    if ((handle[grabberIndex] = KYFG_Open(grabberIndex)) != -1)										// connect to selected device
    {
        printf("Good connection to grabber #%d, handle=%X\n", grabberIndex, handle[grabberIndex] );
        return 0;
    }
    else
    {
        printf("Could not connect to grabber #%d\n", grabberIndex);
        fflush(stdin);
        getchar();
        return -1;
    }
}

int main(int argc, char* argv[])
{
	unsigned int* info = 0;
	unsigned int infosize = 0, grabberIndex = 0, cameraIndex = 0, i;
    STREAM_HANDLE streamHandle = 0;
//	BUFFHANDLE buffHandle;
	CAMHANDLE camHandleArray[MAXBOARDS][4];						// there are maximum 4 cameras
	int detectedCameras[MAXBOARDS];
	char c = 0;
	// long long buffSize = 0;
	int buffIndex;
	char *opts = "w:h:l:b:r:o:m:c:x:y:a:g:G:e:s:"; // доступные опции, каждая принимает аргумент
    char cp[1000];
    char op; // а тут оператор
    int opt; 
    double framerate=50, exposuretime=-1;
    long long aoffset=0, gain2=800;
    long long gain = 1, seek = 0;
    struct timespec time_beg;
    struct timespec time_end;



    for (i=0; i<1000; i++)
		cp[i]=0;
    //strcpy(cp, '/media/fill/Fast/cp70/');
   // mkdir(c);
	printf("Hi! Ver 0.1!\n");

	if(argc == 1) { // если запускаем без аргументов, выводим справку
        printf("cp70grabber\n");
        printf("usage:\n");
        printf("cp70grabber -w n -h m -l s\n");
        printf("example:\n");
        printf(" $ opts -w 1280 -h 720 -o /media/fill/Fast/cp70/'-'\n");
        printf(" 323 — 23 = 300\n");
        return 0;
    }
    while((opt = getopt(argc, argv, opts)) != -1) { // вызываем getopt пока она не вернет -1
        switch(opt) {
            case 'w': // если опция -a, преобразуем строку с аргументом в число
                width = atoi(optarg);
                // printf("w=%lld\n", w);
                break;
            case 'h': // если опция -a, преобразуем строку с аргументом в число
                height = atoi(optarg);
                break;
            case 'x': // если опция -a, преобразуем строку с аргументом в число
                offsetx = atoi(optarg);
                break;
            case 'y': // если опция -a, преобразуем строку с аргументом в число
                offsety = atoi(optarg);
                break;
            case 'r': // если опция -a, преобразуем строку с аргументом в число
                framerate = atoi(optarg);
                break;
            case 'a': // если опция -a, преобразуем строку с аргументом в число
                aoffset = atoi(optarg);
                break;
            case 'g': // если опция -a, преобразуем строку с аргументом в число
                gain = atoi(optarg);
                break;
            case 'G': // если опция -a, преобразуем строку с аргументом в число
                gain2 = atoi(optarg);
                break;
            case 'm': // если опция -a, преобразуем строку с аргументом в число
                maxFrame = atoi(optarg);
                break;
            case 'e': // если опция -a, преобразуем строку с аргументом в число
                exposuretime = atoi(optarg);
                break;
            case 's': // если опция -a, преобразуем строку с аргументом в число
                seek = atoll(optarg);
                break;

            case 'o': // в op сохраняем оператор
			    for (i=0; i<1000; i++)
					fileName[i]=0;
			    strcpy(fileName, optarg);
			    printf("fileName=%s\n", fileName);
                break;
        }
    }

    if(maxFrame*width*height>40000000000)
	{
		printf("Too much frame: maxFrame*width*height > 40 Gb\n");
		exit(0);
	}
//    KYFG_GrabberExecuteCommand(handle[grabberIndex], "CxpPoCxpAuto");
	KY_DeviceScan(&infosize);						// First scan for device to retrieve the number of
    printf("Number of devices found: %d\n", infosize);
												// virtual and hardware devices connected to PC
	if(infosize == 0)
	{
		printf("Info allocation failed\n");
		fflush(stdin);
		getchar();
		return 0;
	}

	for(i = 0; i < infosize; i ++)
	{
		printf("Device %d: ", i);
		printf("%s\n",KY_DeviceDisplayName(i));											// Show assigned name to each pid
	}


//	if (0 ==
	 connectToGrabber(grabberIndex);
//	{
                // register a callback which will be returned upon each acquired frame
//                KYFG_CallbackRegister(handle[grabberIndex], Stream_callback_func, 0);

//	}

	grabberIndex = 0;
//	if(FGSTATUS_OK != KYFG_CameraScan(handle[grabberIndex], camHandleArray[grabberIndex], &detectedCameras[grabberIndex]))
//	{	
//		return 0;								// no cameras were detected
//	}
//	printf("Found %d cameras.\n", detectedCameras[grabberIndex]);
	// open a connection to chosen camera
    int detectionCount = _countof(camHandleArray);
	if(FGSTATUS_OK != KYFG_UpdateCameraList(handle[grabberIndex], camHandleArray[grabberIndex], &detectionCount))
    {
        return 0;;
    }
    printf("Found %d cameras.\n", detectionCount);
    if(detectionCount < 1)
    {
        return 0;; // no cameras were detected
    }
    detectedCameras[grabberIndex] = detectionCount;
    printf("Found %d cameras.\n", detectedCameras[grabberIndex]);

	if(FGSTATUS_OK == KYFG_CameraOpen2(camHandleArray[grabberIndex][0], 0))
	{
		printf("Camera 0 was connected successfully\n");
	}
	else
	{
		printf("Camera isn't connected\n");
		return 0;
	}
    KYFG_CameraCallbackRegister(camHandleArray[grabberIndex][0], Stream_callback_func, 0);

//	printf("aoffset = %lld\n", aoffset);

	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Width", width);							// set camera width to 1280
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Height", height);							// set camera height to 720
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Gain2", gain2);							// set camera height to 720
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "AOffset", aoffset);							// set camera height to 720
	if (gain == 1)
		KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "Gain", "x1");		// set camera pixel format to Mono8
	if (gain == 3)
		KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "Gain", "x1.5");		// set camera pixel format to Mono8
	if (gain == 2)
		KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "Gain", "x2");		// set camera pixel format to Mono8
	if (gain == 4)
		KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "Gain", "x4");		// set camera pixel format to Mono8
	if (offsetx!=-1)
		KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetX", offsetx);							// set camera width to 1280
	if (offsety!=-1)
		KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetY", offsety);							// set camera height to 720
	printf("gain = %lld\n", gain);

	gain2 = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "Gain2");							// set camera height to 720
	printf("gain2 = %lld\n", gain2);
	aoffset = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "AOffset");							// set camera height to 720
	printf("aoffset = %lld\n", aoffset);

	height = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "Height");							// set camera height to 720
	printf("height = %lld\n", height);
	width = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "Width");							// set camera height to 720
	printf("width = %lld\n", width);
	printf("OffsetX = %ld\n", KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetX"));
	printf("OffsetY = %ld\n", KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetY"));
	KYFG_SetCameraValueBool(camHandleArray[grabberIndex][0], "AcquisitionFrameRateEnable", KYTRUE);							// set camera height to 720
	KYFG_SetCameraValueFloat(camHandleArray[grabberIndex][0], "AcquisitionFrameRate", framerate);							// set camera height to 720
	if (exposuretime != -1)
		KYFG_SetCameraValueFloat(camHandleArray[grabberIndex][0], "ExposureTime", exposuretime);							// set camera height to 720
	KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "PixelFormat", "Mono8");		// set camera pixel format to Mono8
	printf("ExposureTime = %f\n", KYFG_GetCameraValueFloat(camHandleArray[grabberIndex][0], "ExposureTime"));
	framerate = KYFG_GetCameraValueFloat(camHandleArray[grabberIndex][0], "AcquisitionFrameRate");							// set camera height to 720
//	printf("test0!");

	printf("framerate = %f\n", framerate);
//	printf("test1!\n");

    if(FGSTATUS_OK != KYFG_StreamCreateAndAlloc(camHandleArray[grabberIndex][0], &streamHandle , 16, 0))
    {
        printf("Failed to allocate buffer.\n");
    }

   	printf("Creating mem buffer %2.3f Gb... \n", (float)(height*width*maxFrame)/1e9);
    memdata = (void*)malloc((size_t)(height*width*maxFrame)); 		// allocate size for local buffer
	printf("done!\n");


	printf("Prestart.\n");
	KYFG_CameraStart(camHandleArray[grabberIndex][0], streamHandle, 0);
	printf("Prestop.\n");
	KYFG_CameraStop(camHandleArray[grabberIndex][0]);

			
	if (createFile == 0)
	{
		createFile = 1;

		f = open(fileName, O_RDWR);
//		printf("lseek!\n");

		lseek(f, seek, SEEK_SET);
//		printf("Done!\n");
		printf("Created file.\n");

	}

	firstFlag = 0;
    clock_gettime(CLOCK_REALTIME, &time_beg);
	KYFG_CameraStart(camHandleArray[grabberIndex][0], streamHandle, 0);
	printf("Camera started!\n");
    while(savedFrame<maxFrame);

//	do
//	{
//	}while( (totalFrames < maxFrame)||(buffSize<=0));

    clock_gettime(CLOCK_REALTIME, &time_end);
	KYFG_CameraStop(camHandleArray[grabberIndex][0]);
	printf("\nStop! Done %lld frames in %1.3f s!\n", savedFrame, (double)(time_end.tv_sec-time_beg.tv_sec)+(double)(time_end.tv_nsec-time_beg.tv_nsec)/1e9);

	printf("Write to SSD... \n");
	if (height*width*savedFrame < 2.1e9)
		write(f, memdata, (size_t)(height*width*savedFrame));
	else
	{
		long long num_pack;
		num_pack = (long long)ceill((long double)(height*width*savedFrame)/2.1e9);
		printf("Number of pack is %lld \n", num_pack);
		for (long long jj=0;jj<num_pack-1;jj++)
		{
			printf("Current number is %lld \n", jj);
			write(f, memdata+((long long)2.1e9)*jj, 2.1e9);
		}
		printf("Final pack is writting");
		write(f, memdata, (size_t)(height*width*savedFrame-2.1e9*num_pack));
	}

//    while(savedFrame<maxFrame);
	free(memdata);
 //   KYFG_GrabberExecuteCommand(handle[grabberIndex], "CxpPoCxpTurnOff");
    if(KYFG_Close(handle[grabberIndex]) != FGSTATUS_OK)												// Close the selected device and unregisters all associated routines
    {
        printf("wasn't able to close grabber #%d\n", grabberIndex);
    }
	if (createFile == 1)
    	close(f);	
    printf("Bye!\n");

	return 0;
}

