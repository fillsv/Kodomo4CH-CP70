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
//#include <dir.h>
#include "KYFGLib.h"
#include <time.h> 


#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif
FILE* f;
char fileName[1000];
int createFile = 0;
int flagPrint = 0;;
long long savedFrame = 0;
long long maxFrame = 10;
long long totalFrames = 0;
long long buffSize = 0;

void Stream_callback_func(BUFFHANDLE buffHandle, void* userContext)
{
	static void* data = 0;
	static KYBOOL copyingDataFlag = KYFALSE;
	static clock_t ticks1, ticks_c;
	int buffIndex;
//	char fileName[]="tmp0001.dat            ";
	void* buffData;


	// if(0 == buffHandle)		// callback with indicator for acquisition stop
	// {
	// 	copyingDataFlag = KYFALSE;
	// 	return;
	// }
//	for(int i = 11; i<23; i++)
//		fileName[i] = 0;
	totalFrames = KYFG_GetGrabberValueInt(buffHandle, "RXFrameCounter");
	buffSize = KYFG_StreamGetSize(buffHandle);			// get buffer size
	buffIndex = KYFG_StreamGetFrameIndex(buffHandle);
	buffData = KYFG_StreamGetPtr(buffHandle, buffIndex);		// get pointer of buffer data
//	printf("\rCallback buffer handle:%X, current index:%d, total frames:%lld        ", buffHandle, buffIndex, totalFrames);
	// printf("$-#");

	// if(KYFALSE == copyingDataFlag)
	// {
//		copyingDataFlag = KYTRUE;
		//data = (void*)realloc(data, buffSize); 		// allocate size for local buffer
	//	if(0 == data)
	//	{
	//		return;
	//}
	//printf("Good callback buffer handle:%X, buff size %lld, current index:%d, total frames:%lld        \r", buffHandle, buffSize, buffIndex, totalFrames);

	if (createFile == 1)
		if ((savedFrame<maxFrame)&&(1==1))
		{
			 fwrite(buffData, 1, buffSize, f);
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

//	if(flagPrint == 0)
	// printf("Good callback buffer handle:%X, buff size %lld, current index:%d, total frames:%lld        \r", buffHandle, buffSize, buffIndex, totalFrames);
	// printf("\n");	

		//memcpy(data, buffData, buffSize);			// copy data to local buffer
		//... Show Image with data ...
		// copyingDataFlag = KYFALSE;
	// }
}

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
	BUFFHANDLE buffHandle;
	CAMHANDLE camHandleArray[MAXBOARDS][4];						// there are maximum 4 cameras
	int detectedCameras[MAXBOARDS];
	char c = 0;
	// long long buffSize = 0;
	int buffIndex;
	char *opts = "w:h:l:b:r:o:m:c:x:y:"; // доступные опции, каждая принимает аргумент
    long long w=1280, h=720, x=-1, y=-1; // тут храним числа
    char cp[1000];
    char op; // а тут оператор
    int opt; 
    double r=50;
  	clock_t ticks_beg, ticks_end;

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
                w = atoi(optarg);
                // printf("w=%lld\n", w);
                break;
            case 'h': // если опция -a, преобразуем строку с аргументом в число
                h = atoi(optarg);
                break;
            case 'x': // если опция -a, преобразуем строку с аргументом в число
                x = atoi(optarg);
                break;
            case 'y': // если опция -a, преобразуем строку с аргументом в число
                y = atoi(optarg);
                break;
            case 'r': // если опция -a, преобразуем строку с аргументом в число
                r = atoi(optarg);
                break;
            case 'm': // если опция -a, преобразуем строку с аргументом в число
                maxFrame = atoi(optarg);
                break;

            case 'o': // в op сохраняем оператор
			    for (i=0; i<1000; i++)
					fileName[i]=0;
			    strcpy(fileName, optarg);
			    printf("fileName=%s\n", fileName);
                break;
        }
    }


	infosize = KYFG_Scan(0,0);						// First scan for device to retrieve the number of
														// virtual and hardware devices connected to PC
	if((info= (unsigned int*)malloc(sizeof(unsigned int)*infosize)) != 0)
	{
        infosize = KYFG_Scan(info,infosize);			// Scans for frame grabbers currently connected to PC. Returns array with each ones pid
	}
	else
	{
		printf("Info allocation failed\n");
		fflush(stdin);
		getchar();
		return 0;
	}
	printf("Number of scan results: %d\n", infosize);
	for(i=0; i<infosize; i++)
	{
		printf("Device %d: ", i);
		printf("%s\n",KY_DeviceDisplayName(i));											// Show assigned name to each pid
	}

    /*
	if (infosize > 0)
	{
		do{
			printf("\nEnter which device to connect (0-%d): ", infosize - 1);
			fflush(stdin);
            grabberIndex = getchar();
            grabberIndex = grabberIndex - '0';

        } while (!(grabberIndex >= 0 && grabberIndex <= infosize - 1));
		
        connectToGrabber(grabberIndex);
	}
	else
	{
		printf("Exiting\n");
		fflush(stdin);
        getchar();
		return 0;
	}
    */

	if (0 == connectToGrabber(grabberIndex))
	{
                // register a callback which will be returned upon each acquired frame
                KYFG_CallbackRegister(handle[grabberIndex], Stream_callback_func, 0);
	}

	c = 'x';
	grabberIndex = 0;
	if(FGSTATUS_OK != KYFG_CameraScan(handle[grabberIndex], camHandleArray[grabberIndex], &detectedCameras[grabberIndex]))
	{	
		return 0;								// no cameras were detected
	}
	printf("Found %d cameras.\n", detectedCameras[grabberIndex]);
	// open a connection to chosen camera
	if(FGSTATUS_OK == KYFG_CameraOpen2(camHandleArray[grabberIndex][0], 0))
	{
		printf("Camera 0 was connected successfully\n");
	}
	else
	{
		printf("Camera isn't connected\n");
		return 0;
	}
			
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Width", w);							// set camera width to 1280
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Height", h);							// set camera height to 720
	if (x!=-1)
		KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetX", x);							// set camera width to 1280
	if (y!=-1)
		KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetY", y);							// set camera height to 720
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Gain2", 600);							// set camera height to 720
	KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "AOffset", 2000);							// set camera height to 720
	KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "Gain", "x2");		// set camera pixel format to Mono8
	x = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetX");							// set camera height to 720
	printf("x = %lld\n", x);
	y = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetY");							// set camera height to 720
	printf("x = %lld\n", y);
	h = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "Height");							// set camera height to 720
	printf("h = %lld\n", h);
	w = KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "Width");							// set camera height to 720
	printf("w = %lld\n", w);
	printf("OffsetX = %ld\n", KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetX"));
	printf("OffsetY = %ld\n", KYFG_GetCameraValueInt(camHandleArray[grabberIndex][0], "OffsetY"));
	// KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "AcquisitionFps", r);							// set camera height to 720
	KYFG_SetCameraValueBool(camHandleArray[grabberIndex][0], "AcquisitionFrameRateEnable", KYTRUE);							// set camera height to 720
	KYFG_SetCameraValueFloat(camHandleArray[grabberIndex][0], "AcquisitionFrameRate", r);							// set camera height to 720
	KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "PixelFormat", "Mono8");		// set camera pixel format to Mono8
	printf("ExposureTime = %f\n", KYFG_GetCameraValueFloat(camHandleArray[grabberIndex][0], "ExposureTime"));
	r = KYFG_GetCameraValueFloat(camHandleArray[grabberIndex][0], "AcquisitionFrameRate");							// set camera height to 720
	printf("r = %f\n", r);
	// allocate buffer large enough to get farmes and be able to process this data
	if(FGSTATUS_OK != KYFG_StreamCreateAndAlloc(camHandleArray[grabberIndex][0], &buffHandle , 512, 0))
	{
		printf("Buffer wasn't generated successfully");
	}
			
	KYFG_CameraStartAcquire(camHandleArray[grabberIndex][0], buffHandle, 0);
	KYFG_CameraStopAcquire(camHandleArray[grabberIndex][0]);

	printf("Start!");
	if (createFile == 0)
	{
		createFile = 1;
		f = fopen(fileName, "wb");
	}
	ticks_beg = clock();
	KYFG_CameraStartAcquire(camHandleArray[grabberIndex][0], buffHandle, 0);
	do
	{
		// totalFrames = KYFG_GetGrabberValueInt(buffHandle, "RXFrameCounter");
		// printf("Rr %lld fr!\n ", totalFrames);
	}while( (totalFrames < maxFrame)||(buffSize<=0));

	KYFG_CameraStopAcquire(camHandleArray[grabberIndex][0]);
	ticks_end = clock();
	// totalFrames = KYFG_GetGrabberValueInt(buffHandle, "RXFrameCounter");
	printf("\nStop! Done %lld frames in %f s!\n", savedFrame, ((float)(ticks_end-ticks_beg))/CLOCKS_PER_SEC);

    while(savedFrame<maxFrame);

// 	while (c != 'e')
// 	{
//         if (c != '\n')
// 		{
// 			printf("\nEnter choice: ([0-4]-select grabber) (o-open frabber) (c-connect to camera)(s-start)(t-stop)(e-exit)\n");
//     	}
// 		fflush(stdin);

// 		totalFrames = KYFG_GetGrabberValueInt(buffHandle, "RXFrameCounter");
// 		buffSize = KYFG_StreamGetSize(buffHandle);			// get buffer size
// 		buffIndex = KYFG_StreamGetFrameIndex(buffHandle);
// //		buffData = KYFG_StreamGetPtr(buffHandle, buffIndex);		// get pointer of buffer data
// 		printf("Buffer handle:%X, current index:%d, total frames:%lld        \n", buffHandle, buffIndex, totalFrames);
//         	while ((c = getchar()) == -1)
//         	{
// 				totalFrames = KYFG_GetGrabberValueInt(buffHandle, "RXFrameCounter");
// 		        if (totalFrames >= 16)
// 		        {
// 					KYFG_CameraStopAcquire(camHandleArray[grabberIndex][0]);
// 					printf("Stop! 16 frames!");
			
// 		        }

//         	}

// 		if(c == 't')
//     	{
// 			KYFG_CameraStopAcquire(camHandleArray[grabberIndex][0]);
// 		}
// 		else if (c == 's')
// 		{
// 			printf("Start!");
// 			if (createFile == 0)
// 			{
// 				createFile = 1;
// 				f = fopen(fileName, "wb");
// 			}
// 	    		KYFG_CameraStartAcquire(camHandleArray[grabberIndex][0], buffHandle, 0);
// 		}

// 	}
// 	printf("\nExiting...\n");

    // for (grabberIndex = 0; grabberIndex < infosize; grabberIndex++)
    // {
    if(KYFG_Close(handle[grabberIndex]) != FGSTATUS_OK)												// Close the selected device and unregisters all associated routines
    {
        printf("wasn't able to close grabber #%d\n", grabberIndex);
    }
    // }

// 	printf("Press to exit");
// 	fflush(stdin);
//     getchar();
	if (createFile == 1)
    	fclose(f);	
	printf("All time is %f s!\n", ((float)(clock()-ticks_beg))/CLOCKS_PER_SEC);
    printf("Bye!\n");

	return 0;
}

