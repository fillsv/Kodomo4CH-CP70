/************************************************************************ 
*	File: KYFGLib_Example.cpp
*	Sample Predator Frame Grabber API application
*
*	KAYA Instruments Ltd.
*************************************************************************/

#ifdef _WIN32
#include "stdafx.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "KYFGLib.h"

#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#define MAXBOARDS 4
FGHANDLE handle[MAXBOARDS];
unsigned int currentGrabberIndex;

#ifdef __GNUC__ // _aligned_malloc() implementation for gcc
void* _aligned_malloc(size_t size, size_t alignment)
{
    size_t pageAlign = size % 4096;
    if(pageAlign)
    {
        size += 4096 - pageAlign;
    }

#if(GCC_VERSION <= 40407)
    void * memptr = 0;
    posix_memalign(&memptr, alignment, size);
    return memptr;
#else
    return aligned_alloc(alignment, size);
#endif
}
#endif // #ifdef __GNUC__

void Stream_callback_func(STREAM_BUFFER_HANDLE streamBufferHandle, void* userContext)
{
    unsigned char* pFrameMemory = 0;
    int64_t totalFrames = -1;
    uint32_t frameId = 0;
 
	if (!streamBufferHandle)
	{
		// this callback indicates that acquisition has stopped
		return;
	}
            
	//
    //
	//

    // process data associated with given stream buffer
    KYFG_BufferGetInfo(streamBufferHandle, 
                        KY_STREAM_BUFFER_INFO_BASE, 
                        &pFrameMemory, 
                        NULL, 
                        NULL);

    KYFG_BufferGetInfo(streamBufferHandle, 
                        KY_STREAM_BUFFER_INFO_ID, 
                        &frameId, 
                        NULL, 
                        NULL);

    totalFrames = KYFG_GetGrabberValueInt(handle[currentGrabberIndex], "RXFrameCounter");

    printf(//"\n" // Uncomment to print on new line eacg time
            "\rGood callback stream's buffer handle:%lX, ID:%d, total frames:%ld        ", streamBufferHandle, frameId, totalFrames);

    // return stream buffer to input queue
    KYFG_BufferToQueue(streamBufferHandle, KY_ACQ_QUEUE_INPUT);
}

int connectToGrabber(unsigned int grabberIndex)
{
	int64_t dmadQueuedBufferCapable;

    if ((handle[grabberIndex] = KYFG_Open(grabberIndex)) != -1)										// connect to selected device
    {
        printf("Good connection to grabber #%d, handle=%X\n", grabberIndex, handle[grabberIndex] );
    }
    else
    {
        printf("Could not connect to grabber #%d\n", grabberIndex);
        fflush(stdin);
        getchar();
        return -1;
    }

	dmadQueuedBufferCapable = KYFG_GetGrabberValueInt(handle[grabberIndex], DEVICE_QUEUED_BUFFERS_SUPPORTED);
	if (1 != dmadQueuedBufferCapable)
	{
        printf("grabber #%d does not support queued buffers\n", grabberIndex);
        fflush(stdin);
        getchar();
        return -1;
	}

    currentGrabberIndex = grabberIndex;

    return 0;
}

CAMHANDLE camHandleArray[MAXBOARDS][KY_MAX_CAMERAS];						// there are maximum 4 cameras
STREAM_HANDLE cameraStreamHandle;
size_t frameDataSize, frameDataAligment;
STREAM_BUFFER_HANDLE streamBufferHandle[16] = {0};
int iFrame;

int startCamera(unsigned int grabberIndex, unsigned int cameraIndex)
{
    // put all buffers to input queue
    KYFG_BufferQueueAll(cameraStreamHandle, KY_ACQ_QUEUE_UNQUEUED, KY_ACQ_QUEUE_INPUT);    

	// start acquisition
	KYFG_CameraStart(camHandleArray[grabberIndex][cameraIndex], cameraStreamHandle, 0);

	return 0;
}

int main(int argc, char* argv[])
{
	unsigned int* info = 0;
    unsigned int infosize = 0, grabberIndex = 0, cameraIndex = 0, i;
    int detectedCameras[MAXBOARDS];
	char c = 0;

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
        KY_DEVICE_INFO deviceInfo;
        KY_DeviceInfo(i, &deviceInfo);
        printf("Device %d: ", i);
        printf("%s\n",deviceInfo.szDeviceDisplayName);	// Show name of each device
	}

    c = 'x';
    while (c != 'e')
	{
        if (c != '\n')
        {
            printf("\nEnter choice: ([0-4]-select grabber) (o-open frabber) (c-connect to camera)(s-start)(t-stop)(e-exit)\n");
        }
		fflush(stdin);
        while ((c = getchar()) == -1);

        if( c>= '0' && c <= ('0' + _countof(handle) - 1) )
        {
            grabberIndex = c - '0';
            printf("Selected grabber #%d\n", grabberIndex);
        }
        else
        if( c== 'c')
        {
            // scan for connected cameras
            detectedCameras[grabberIndex] = KY_MAX_CAMERAS;
            if(FGSTATUS_OK != KYFG_UpdateCameraList(handle[grabberIndex], camHandleArray[grabberIndex], &detectedCameras[grabberIndex]))
			{
				continue;								// no cameras were detected
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
				continue;
			}
			
            //Š// update camera/grabber buffer dimensions parameters before stream creation
            KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Width", 640);							// set camera width 
            KYFG_SetCameraValueInt(camHandleArray[grabberIndex][0], "Height", 480);							// set camera height
            KYFG_SetCameraValueEnum_ByValueName(camHandleArray[grabberIndex][0], "PixelFormat", "RGB8");	// set camera pixel format
    
	        // create stream and assign appropriate runtime acquisition callback function
            KYFG_StreamCreate(camHandleArray[grabberIndex][cameraIndex], &cameraStreamHandle, 0);
            KYFG_StreamBufferCallbackRegister(cameraStreamHandle, Stream_callback_func, NULL);
       
            // Retrieve information about required frame buffer size and alignment 
            KYFG_StreamGetInfo(cameraStreamHandle, 
                                KY_STREAM_INFO_PAYLOAD_SIZE, 
                                &frameDataSize, 
                                NULL, NULL);

            KYFG_StreamGetInfo(cameraStreamHandle,
                                KY_STREAM_INFO_BUF_ALIGNMENT, 
                                &frameDataAligment, 
                                NULL, NULL);

            // allocate memory for desired number of frame buffers
            for (iFrame = 0; iFrame < _countof(streamBufferHandle); iFrame++)
            {
                    void * pBuffer = _aligned_malloc(frameDataSize, frameDataAligment);
                    KYFG_BufferAnnounce(cameraStreamHandle,
                                        pBuffer, 
                                        frameDataSize, 
                                        NULL, 
                                        &streamBufferHandle[iFrame]);
            }
		}
        else if(c == 'o')
        {
            connectToGrabber(grabberIndex);
        }
        else if(c == 't')
        {
            KYFG_CameraStop(camHandleArray[grabberIndex][0]);
		}
		else if (c == 's')
		{
			startCamera(grabberIndex, 0);
		}

	}
	printf("\nExiting...\n");

    for (grabberIndex = 0; grabberIndex < infosize; grabberIndex++)
    {
        if(KYFG_Close(handle[grabberIndex]) != FGSTATUS_OK)												// Close the selected device and unregisters all associated routines
        {
            printf("wasn't able to close grabber #%d\n", grabberIndex);
        }
    }

	printf("Press to exit");
	fflush(stdin);
    getchar();
	return 0;
}

