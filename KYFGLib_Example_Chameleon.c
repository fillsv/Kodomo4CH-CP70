/************************************************************************ 
*	File: KY_Simulation_Example.c
*	Sample Chameleon Camera simulator API application
*
*	KAYA Instruments Ltd.
*************************************************************************/

#ifdef _WIN32
#include "stdafx.h"
#define KY_MAX_PATH MAX_PATH
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define KY_MAX_PATH PATH_MAX
#endif

#include "KYFGLib.h"


#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


static void* rawBuffers[8];
static KYBOOL bufferIsReplaced[8];
static const int FRAMES_COUNT = sizeof(rawBuffers)/sizeof(rawBuffers[0]);
static CAMHANDLE cameraHandle = INVALID_CAMHANDLE;
static STREAM_HANDLE cameraStreamHandle = INVALID_STREAMHANDLE;
static int	frames_sent = 0;// Total number of frames sent


void LoadRawFile(char* rawFilePath, int frameIndex)
{
	FILE* file;
	int frame_size = 0;
	int read_size = 0;

	if((file = fopen(rawFilePath, "rb")) == NULL)
	{
		printf("Couldn't open file %s\n\r", rawFilePath);
		return;
	}
	fseek(file, 0, SEEK_END);													// seek end of binary file
	frame_size = ftell(file);														// give current position
	rewind(file);																// get back to the beginning of the file

	rawBuffers[frameIndex] = malloc(frame_size);
	if(NULL == rawBuffers[frameIndex])
	{
		printf("Couldn't allocate memory to read file %s\n\r", rawFilePath);
		return;
	}
	read_size = fread(rawBuffers[frameIndex], 1, frame_size, file);
	if (read_size != frame_size)
	{
	    printf("Failed to read %d bytes from file %s\n\r", frame_size, rawFilePath);
	}
	
	fclose(file);
}

void PrepareRawBuffers()
{
	int i;
	char fileName[KY_MAX_PATH];
	for (i = 0; i < FRAMES_COUNT; i++)
	{
		sprintf(fileName, "../Data/%d.raw", i);
		LoadRawFile(fileName, i);
		bufferIsReplaced[i] = KYFALSE;
	}
}


void StartGeneration()
{
  frames_sent = 0; // Reset frames counter when starting new session
  FGSTATUS status = KYFG_CameraStart(cameraHandle, cameraStreamHandle, 0);	// Starts the simulation
  printf("KYFG_CameraStartAcquire returned: 0x%X\n", status);
}

void KYDeviceEventCallBackImpl(void* UNUSED(userContext), KYDEVICE_EVENT* pEvent)
{
  switch(pEvent->eventId)
  {
      case KYDEVICE_EVENT_CAMERA_START_REQUEST:
      {
        CAMHANDLE eventCameraHandle = ((KYDEVICE_EVENT_CAMERA_START*)pEvent)->camHandle;
        if (eventCameraHandle == cameraHandle)
        {
            printf("\nDetected remote request to start generation\n");
            StartGeneration();
        }
      }
      break;
  }
}

void CameraCallbackImpl (void* UNUSED(userContext), STREAM_HANDLE streamHandle)
{
	uint32_t	currentIndex;			// Indicates the Nth frame that was currently send.
	void		*ptr;					// Pointer to current frame the data
	uint32_t	size;					// Size of each frame in bytes

	if (!streamHandle)
	{
	    // callback with streamHandle == 0 indicates that stream generation has stopped
	    // any data retrieved using this handle (frame index, buffer pointer, etc.) won't be valid
	    printf("\nStream generation stopped\n");
	    return;
	}

	currentIndex = KYFG_StreamGetFrameIndex(streamHandle);

	frames_sent++;
	printf("\nCallback call N %d, frame N %u                      ",frames_sent, currentIndex);

	ptr = KYFG_StreamGetPtr(streamHandle, currentIndex);
	size = KYFG_StreamGetSize(streamHandle);
	// after frame has been generated replace its memory with a different raw content:
	if (frames_sent >= FRAMES_COUNT // start "on-the-fly" buffer replacement after x frames were sent - just as an example...
	    &&
	    bufferIsReplaced[currentIndex] == KYFALSE)
	{
	    memcpy(ptr, rawBuffers[currentIndex], size);
	    bufferIsReplaced[currentIndex] = KYTRUE;
	    printf(", content of this frame has been replaced for next generation cycle");
	}
}

// Helper function to get single printable char as user input
int get_printable_char()
{
  int c;
  fflush(stdin);
  do
      c = getchar();
  while (isspace(c));
  return c;
}

int main(int UNUSED(argc), char* UNUSED(argv[]))
{
	FGHANDLE deviceHandle = 0;
    CAMHANDLE cameraHandles[KY_MAX_CAMERAS];
    int detectedCameras = KY_MAX_CAMERAS;
	FGSTATUS status;
	unsigned int* info = NULL;
	int infosize = 0;
	int infoIndex = -1;
	int i;
	char c = 0;

	PrepareRawBuffers();

	infosize = KYFG_Scan(NULL, 0);													// First scan for device to retrieve the number of
																					// virtual and hardware devices connected to PC
	if((info= (unsigned int*)malloc(sizeof(unsigned int)*infosize)) !=NULL)
	{
		infosize = KYFG_Scan(info,infosize);			// Scans for simulators cuttently connected to PC . Returns array with each ones pid
	}
	else
	{
		printf("Info allocation failed\n");
		get_printable_char();
		return 0;
	}
	printf("Number of scan results: %d\n", infosize);
	
	for(i = 0; i < infosize; i++)
	{
        KY_DEVICE_INFO deviceInfo;
        KY_DeviceInfo(i, &deviceInfo);
        printf("Device %d: ", i);
        printf("%s\n", deviceInfo.szDeviceDisplayName);		// Show assigned name to each device
	}

	if (infosize > 0)
	{
		do{
			printf("\nEnter which device to connect (0-%d): ", infosize - 1);
			infoIndex = get_printable_char();
			infoIndex = infoIndex - '0'; // translate user entered char to int number

		} while (!(infoIndex >= 0 && infoIndex <= infosize - 1));
		// Conenct to specific simulator
		if ((deviceHandle = KYFG_OpenEx(infoIndex, 0)) != INVALID_FGHANDLE)	// open the selected device
		{
			printf("Good connection, deviceHandle == 0x%X\n", deviceHandle);
		}
		else
		{
			printf("Not connected\n");
			get_printable_char();
			return -1;
		}
	}
	else
	{
		printf("Exiting\n");
		get_printable_char();
		return -1;
	}

    status = KYFG_UpdateCameraList(deviceHandle, cameraHandles, &detectedCameras);
    if (FGSTATUS_OK != status)
    {
		printf("Camera scan failed\n");
		get_printable_char();
		return -1;
    }
	
	// Currently only one camera is implemented by Chameleon Simulator
	if (detectedCameras < 1)
	{
		printf("No camera detected\n");
		get_printable_char();
		return 0;
	}
	// Also, in this example we are working with only first camera
	cameraHandle = cameraHandles[0];
	status = KYFG_CameraOpen2(cameraHandle, 0); // if 'xml_file_path' is 0 then only internal XML will be used providing minimal set of mandatory camera parameters
    if ( FGSTATUS_OK != status)
    {
		printf("Failed to open camera, status 0x%X\n", status);
		get_printable_char();
		return -1;
    }
	
	// Register callback function for the camera
	status = KYFG_CameraCallbackRegister(cameraHandle, CameraCallbackImpl, 0);
    if ( FGSTATUS_OK != status)
    {
		printf("Failed to register camera callback, status 0x%X\n", status);
		get_printable_char();
		return -1;
    }

    // Register callback function for device
    status = KYDeviceEventCallBackRegister(deviceHandle, KYDeviceEventCallBackImpl, 0);
    if ( FGSTATUS_OK != status)
    {
        printf("Failed to register device callback, status 0x%X\n", status);
        get_printable_char();
        return -1;
    }

	// Optional step - re-load the selected XML file 
	/*
	if(KYFG_LoadCameraConfiguration(cameraHandle, "../Data/KAYA_Chameleon.xml", 0) != FGSTATUS_OK) // KAYA additional file values - not implemented yet, second parameter must be 0
	{
		printf("Load of camera configuration file failed\n");
		printf("Press to exit");
		get_printable_char();
		return 0;
	}
	*/

	// Setting the camera parameters
	KYFG_SetCameraValueInt(cameraHandle, "Width", 640);
	KYFG_SetCameraValueInt(cameraHandle, "Height", 480);
	KYFG_SetCameraValueEnum_ByValueName(cameraHandle, "PixelFormat" , "RGB8");
	KYFG_SetCameraValueFloat(cameraHandle, "AcquisitionFrameRate", 60.0);

	printf("Creating stream...\n");
	if(FGSTATUS_OK == KYFG_StreamCreateAndAlloc(cameraHandle,
                                           &cameraStreamHandle,
                                           FRAMES_COUNT,
                                           0))// streamIndex currently must be 0
	{
		printf("Stream was created successfuly\n");
	}
	else
	{
		printf("Stream creation has failed\n");
		printf("Press to exit");
		get_printable_char();
		return 0;
	}

	printf("Loading File...\n");
	if(FGSTATUS_OK == KYFG_LoadFileData(cameraStreamHandle, "../Data/example_rgb_8bit.bmp", "bmp", FRAMES_COUNT)) // load an image(s) file to the simulator
	{
		printf("File was loaded successfuly\n");
	}
	else
	{
		printf("File load has failed\n");
		printf("Press to exit");
		get_printable_char();
		return 0;
	}

	while (c != 'e')
	{
		printf("\nEnter choice: (s-start)(t-stop)(e-exit)\n");
		while ((c = get_printable_char()) == -1);

		if (c == 't')
		{
			KYFG_CameraStop(cameraHandle);							// Stops the simulation
		}
		else if (c == 's')
		{
		    StartGeneration();
		}
	}
	printf("\nExiting...\n");

	if(KYFG_Close(deviceHandle) != FGSTATUS_OK)	// Close the selected device and releases all associated resources - cameraHandle, cameraStreamHandle; also unregisteres callbacks
	{
		printf("wasn't able to close correctly\n");
	}

	printf("Press any key to exit");
	fflush(stdin);
	getchar();
	return 0;
}

