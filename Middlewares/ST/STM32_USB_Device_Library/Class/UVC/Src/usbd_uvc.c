/**
  ******************************************************************************
  * @file    usbd_uvc.c
  * @author  MCD Application Team
  * @brief   This file provides the UVC core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                UVC Class  Description
  *          ===================================================================
 *           This driver manages the UVC Class 1.0 following the "USB Device Class Definition for
  *           UVC Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 UVC Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 UVC Streaming Endpoint
  *             - 1 UVC Terminal Input (1 channel)
  *             - UVC Class-Specific AC Interfaces
  *             - UVC Class-Specific AS Interfaces
  *             - UVCControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - UVC Feature Unit (limited to Mute control)
  *             - UVC Synchronization type: Asynchronous
  *             - Single fixed UVC sampling rate (configurable in usbd_conf.h file)
  *          The current UVC class version supports the following UVC features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz.
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 DUVITECH.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "usbd_uvc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_UVC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_UVC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_UVC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_UVC_Private_Macros
  * @{
  */
	
/**
  * @}
  */




/** @defgroup USBD_UVC_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_UVC_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx);

static uint8_t  USBD_UVC_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx);

static uint8_t  USBD_UVC_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_UVC_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_UVC_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_UVC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_UVC_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_UVC_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_UVC_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_UVC_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_UVC_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_UVC_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static void UVC_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static void UVC_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

/**
  * @}
  */

/** @defgroup USBD_UVC_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_UVC =
{
  USBD_UVC_Init,
  USBD_UVC_DeInit,
  USBD_UVC_Setup,
  USBD_UVC_EP0_TxReady,
  USBD_UVC_EP0_RxReady,
  USBD_UVC_DataIn,
  USBD_UVC_DataOut,
  USBD_UVC_SOF,
  USBD_UVC_IsoINIncomplete,
  USBD_UVC_IsoOutIncomplete,
  USBD_UVC_GetCfgDesc,
  USBD_UVC_GetCfgDesc,
  USBD_UVC_GetCfgDesc,
  USBD_UVC_GetDeviceQualifierDesc,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_UVC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

#define USB_VIDEO_DESC_SIZ 159
/* USB UVC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_UVC_CfgDesc[] __ALIGN_END =
{
  /* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,               // bLength                  9
  USB_CONFIGURATION_DESCRIPTOR_TYPE,         // bDescriptorType          2
#ifndef VIDEO_USES_ISOC_EP
  WBVAL((USB_VIDEO_DESC_SIZ-9)),
#else
  WBVAL(USB_VIDEO_DESC_SIZ),	
#endif
  0x02,                                      // bNumInterfaces           2
  0x01,                                      // bConfigurationValue      1 ID of this configuration
  0x00,                                      // iConfiguration           0 no description available
  USB_CONFIG_BUS_POWERED ,                   // bmAttributes          0x80 Bus Powered
  USB_CONFIG_POWER_MA(500),                  // bMaxPower              500 mA
  
  /* Interface Association Descriptor */
  UVC_INTERFACE_ASSOCIATION_DESC_SIZE,       // bLength                  8
  USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, // bDescriptorType         11
  0x00,                                      // bFirstInterface          0
  0x02,                                      // bInterfaceCount          2
  CC_VIDEO,                                  // bFunctionClass          14 Video
  SC_VIDEO_INTERFACE_COLLECTION,             // bFunctionSubClass        3 Video Interface Collection
  PC_PROTOCOL_UNDEFINED,                     // bInterfaceProtocol       0 (protocol undefined)
  0x02,                                      // iFunction                2
    
  /* VideoControl Interface Descriptor */
  
  
  /* Standard VC Interface Descriptor  = interface 0 */
  USB_INTERFACE_DESC_SIZE,                   // bLength                  9
  USB_INTERFACE_DESCRIPTOR_TYPE,             // bDescriptorType          4
  USB_UVC_VCIF_NUM,                          // bInterfaceNumber         0 index of this interface (VC)
  0x00,                                      // bAlternateSetting        0 index of this setting
  0x00,                                      // bNumEndpoints            0 no endpoints
  CC_VIDEO,                                  // bInterfaceClass         14 Video
  SC_VIDEOCONTROL,                           // bInterfaceSubClass       1 Video Control
  PC_PROTOCOL_UNDEFINED,                     // bInterfaceProtocol       0 (protocol undefined)
  0x02,                                      // iFunction                2
  
  
  /* Class-specific VC Interface Descriptor */
  UVC_VC_INTERFACE_HEADER_DESC_SIZE(1),      // bLength                 13 12 + 1 (header + 1*interface
  CS_INTERFACE,                              // bDescriptorType         36 (INTERFACE)
  VC_HEADER,                                 // bDescriptorSubtype      1 (HEADER)
  WBVAL(UVC_VERSION),                        // bcdUVC                  1.10 or 1.00
  WBVAL(VC_TERMINAL_SIZ),                    // wTotalLength            header+units+terminals
  DBVAL(48000000),                         // dwClockFrequency  			48.000000 MHz
  0x01,                                      // bInCollection            1 one streaming interface
  0x01,                                      // baInterfaceNr( 0)        1 VS interface 1 belongs to this VC interface
  
  
  /* Input Terminal Descriptor (Camera) */
  UVC_CAMERA_TERMINAL_DESC_SIZE(2),          // bLength                 17 15 + 2 controls
  CS_INTERFACE,                              // bDescriptorType         36 (INTERFACE)
  VC_INPUT_TERMINAL,                         // bDescriptorSubtype       2 (INPUT_TERMINAL)
  0x01,                                      // bTerminalID              1 ID of this Terminal
  WBVAL(ITT_CAMERA),                         // wTerminalType       0x0201 Camera Sensor
  0x00,                                      // bAssocTerminal           0 no Terminal associated
  0x00,                                      // iTerminal                0 no description available
  WBVAL(0x0000),                             // wObjectiveFocalLengthMin 0
  WBVAL(0x0000),                             // wObjectiveFocalLengthMax 0
  WBVAL(0x0000),                             // wOcularFocalLength       0
  0x02,                                      // bControlSize             2
  0x00, 0x00,                                // bmControls          0x0000 no controls supported
  
  /* Output Terminal Descriptor */
  UVC_OUTPUT_TERMINAL_DESC_SIZE(0),          // bLength                  9
  CS_INTERFACE,                              // bDescriptorType         36 (INTERFACE)
  VC_OUTPUT_TERMINAL,                        // bDescriptorSubtype       3 (OUTPUT_TERMINAL)
  0x02,                                      // bTerminalID              2 ID of this Terminal
  WBVAL(TT_STREAMING),                       // wTerminalType       0x0101 USB streaming terminal
  0x00,                                      // bAssocTerminal           0 no Terminal assiciated
  0x01,                                      // bSourceID                1 input pin connected to output pin unit 1
  0x00,                                      // iTerminal                0 no description available
  
  
  /* Video Streaming (VS) Interface Descriptor */
  
  /* Standard VS Interface Descriptor  = interface 1 */
  // alternate setting 0 = Zero Bandwidth
  USB_INTERFACE_DESC_SIZE,                   // bLength                  9
  USB_INTERFACE_DESCRIPTOR_TYPE,             // bDescriptorType          4
  USB_UVC_VSIF_NUM,                          // bInterfaceNumber         1 index of this interface
  0x00,                                      // bAlternateSetting        0 index of this setting
	
#ifndef VIDEO_USES_ISOC_EP
  0x01,                                      // bNumEndpoints            0 no EP used
#else
	0x00,                                        /* bNumEndpoints - none, no bandwidth used */
#endif

  CC_VIDEO,                                  // bInterfaceClass         14 Video
  SC_VIDEOSTREAMING,                         // bInterfaceSubClass       2 Video Streaming
  PC_PROTOCOL_UNDEFINED,                     // bInterfaceProtocol       0 (protocol undefined)
  0x00,                                      // iInterface               0 no description available

#ifndef VIDEO_USES_ISOC_EP
  /* Standard VS Isochronous Video data Endpoint Descriptor */
  USB_ENDPOINT_DESC_SIZE,                   // bLength                  7
  USB_ENDPOINT_DESCRIPTOR_TYPE,             // bDescriptorType          5 (ENDPOINT)
  USB_ENDPOINT_IN(USB_UVC_ENDPOINT),        // bEndpointAddress      0x83 EP 3 IN
  USB_ENDPOINT_TYPE_BULK,            				// bmAttributes             Bulk transfer type
  WBVAL(VIDEO_PACKET_SIZE),                 // wMaxPacketSize
  0x00,                                      // bInterval                
#endif
  
  /* Class-specific VS Header Descriptor (Input) */
  UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1),// bLength               14 13 + (1*1) (no specific controls used)
  CS_INTERFACE,                              // bDescriptorType         36 (INTERFACE)
  VS_INPUT_HEADER,                           // bDescriptorSubtype       1 (INPUT_HEADER)
  0x01,                                      // bNumFormats              1 one format descriptor follows
  WBVAL(VC_HEADER_SIZ),
  USB_ENDPOINT_IN(USB_UVC_ENDPOINT),         // bEndPointAddress      0x83 EP 3 IN
  0x00,                                      // bmInfo                   0 no dynamic format change supported
  0x02,                                      // bTerminalLink            2 supplies terminal ID 2 (Output terminal)
  0x02,                                      // bStillCaptureMethod      0 NO supports still image capture
  0x01,                                      // bTriggerSupport          0 HW trigger supported for still image capture
  0x00,                                      // bTriggerUsage            0 HW trigger initiate a still image capture
  0x01,                                      // bControlSize             1 one byte bmaControls field size
  0x00,                                      // bmaControls(0)           0 no VS specific controls
  
  /* Class-specific VS Format Descriptor  */
  VS_FORMAT_UNCOMPRESSED_DESC_SIZE,     /* bLength 27*/
  CS_INTERFACE,                         /* bDescriptorType : CS_INTERFACE */
  VS_FORMAT_MJPEG,                      /* bDescriptorSubType : VS_FORMAT_MJPEG subtype */
  0x01,                                 /* bFormatIndex : First (and only) format descriptor */
  0x01,                                 /* bNumFrameDescriptors : One frame descriptor for this format follows. */
  0x01,                                 /* bmFlags : Uses fixed size samples.. */
  0x01,                                 /* bDefaultFrameIndex : Default frame index is 1. */
  0x00,                                 /* bAspectRatioX : Non-interlaced stream not required. */
  0x00,                                 /* bAspectRatioY : Non-interlaced stream not required. */
  0x00,                                 /* bmInterlaceFlags : Non-interlaced stream */
  0x00,                                 /* bCopyProtect : No restrictions imposed on the duplication of this video stream. */
  
  /* Class-specific VS Frame Descriptor */
  VS_FRAME_COMPRESSED_DESC_SIZE,      	/* bLength 2A */
  CS_INTERFACE,                         /* bDescriptorType : CS_INTERFACE */
  VS_FRAME_MJPEG,                				/* bDescriptorSubType : VS_FRAME_UNCOMPRESSED */
  0x01,                                 /* bFrameIndex : First (and only) frame descriptor */
  0x00,                                 /* bmCapabilities : Still images using capture method 0 are supported at this frame setting.D1: Fixed frame-rate. */
  WBVAL(WIDTH),                         /* wWidth (2bytes): Width of frame is 128 pixels. */
  WBVAL(HEIGHT),                        /* wHeight (2bytes): Height of frame is 64 pixels. */
  DBVAL(MIN_BIT_RATE),                  /* dwMinBitRate (4bytes): Min bit rate in bits/s  */ // 128*64*16*5 = 655360 = 0x000A0000 //5fps
  DBVAL(MAX_BIT_RATE),                  /* dwMaxBitRate (4bytes): Max bit rate in bits/s  */ // 128*64*16*5 = 655360 = 0x000A0000
  DBVAL(MAX_FRAME_SIZE),                /* dwMaxVideoFrameBufSize (4bytes): Maximum video or still frame size, in bytes. */ // 128*64*2 = 16384 = 0x00004000
  DBVAL(INTERVAL),				        			/* dwDefaultFrameInterval : 1,000,000 * 100ns -> 10 FPS */ // 5 FPS -> 200ms -> 200,000 us -> 2,000,000 X 100ns = 0x001e8480
  0x00,                                 /* bFrameIntervalType : Continuous frame interval */
  DBVAL(INTERVAL),                      /* dwMaxFrameInterval : 1,000,000 ns  *100ns -> 10 FPS */
  DBVAL(INTERVAL),                      /* dwMFrameInterval : 1,000,000 ns  *100ns -> 10 FPS */
  0x00, 0x00, 0x00, 0x00,               /* dwFrameIntervalStep : No frame interval step supported. */
  
  /* Color Matching Descriptor */
  VS_COLOR_MATCHING_DESC_SIZE,          /* bLength */
  CS_INTERFACE,                         /* bDescriptorType : CS_INTERFACE */
  0x0D,                                 /* bDescriptorSubType : VS_COLORFORMAT */
  0x00,                                 /* bColorPrimarie : 1: BT.709, sRGB (default) */
  0x00,                                 /* bTransferCharacteristics : 1: BT.709 (default) */
  0x00,                                 /* bMatrixCoefficients : 1: BT. 709. */
  
#ifdef VIDEO_USES_ISOC_EP  
  /* Standard VS Interface Descriptor  = interface 1 */
  // alternate setting 1 = operational setting
  USB_INTERFACE_DESC_SIZE,                   // bLength                  9
  USB_INTERFACE_DESCRIPTOR_TYPE,             // bDescriptorType          4
  USB_UVC_VSIF_NUM,                          // bInterfaceNumber         1 index of this interface
  0x01,                                      // bAlternateSetting        1 index of this setting
  0x01,                                      // bNumEndpoints            1 one EP used
  CC_VIDEO,                                  // bInterfaceClass         14 Video
  SC_VIDEOSTREAMING,                         // bInterfaceSubClass       2 Video Streaming
  PC_PROTOCOL_UNDEFINED,                     // bInterfaceProtocol       0 (protocol undefined)
  0x00,                                      // iInterface               0 no description available
  
  /* Standard VS Isochronous Video data Endpoint Descriptor */
  USB_ENDPOINT_DESC_SIZE,                   // bLength                  7
  USB_ENDPOINT_DESCRIPTOR_TYPE,             // bDescriptorType          5 (ENDPOINT)
  USB_ENDPOINT_IN(USB_UVC_ENDPOINT),        // bEndpointAddress      0x83 EP 3 IN
  USB_ENDPOINT_TYPE_ISOCHRONOUS | USB_ENDPOINT_SYNC_ASYNCHRONOUS,            // bmAttributes             1 isochronous transfer type
  WBVAL(VIDEO_PACKET_SIZE),                 // wMaxPacketSize
  0x01,                                      // bInterval                1 one frame interval
#endif
};

//data array for Video Probe and Commit
VideoControl    videoCommitControl =
{
  {0x00,0x00},                      // bmHint
  {0x01},                           // bFormatIndex
  {0x01},                           // bFrameIndex
  {DBVAL(INTERVAL),},          // dwFrameInterval
  {0x00,0x00,},                     // wKeyFrameRate
  {0x00,0x00,},                     // wPFrameRate
  {0x00,0x00,},                     // wCompQuality
  {0x00,0x00,},                     // wCompWindowSize
  {0x00,0x00},                      // wDelay
  {DBVAL(MAX_FRAME_SIZE)},    // dwMaxVideoFrameSize
  {DBVAL(VIDEO_PACKET_SIZE)},         // dwMaxPayloadTransferSize
  {DBVAL(48000000)},         // dwClockFrequency
  {0x00},                           // bmFramingInfo
  {0x00},                           // bPreferedVersion
  {0x00},                           // bMinVersion
  {0x00},                           // bMaxVersion
};

VideoControl    videoProbeControl =
{
  {0x00,0x00},                      // bmHint
  {0x01},                           // bFormatIndex
  {0x01},                           // bFrameIndex
  {DBVAL(INTERVAL),},          // dwFrameInterval
  {0x00,0x00,},                     // wKeyFrameRate
  {0x00,0x00,},                     // wPFrameRate
  {0x00,0x00,},                     // wCompQuality
  {0x00,0x00,},                     // wCompWindowSize
  {0x00,0x00},                      // wDelay
  {DBVAL(MAX_FRAME_SIZE)},    // dwMaxVideoFrameSize
  {DBVAL(VIDEO_PACKET_SIZE)},          // dwMaxPayloadTransferSize
  {DBVAL(48000000)},         // dwClockFrequency
  {0x00},                           // bmFramingInfo
  {0x00},                           // bPreferedVersion
  {0x00},                           // bMinVersion
  {0x00},                           // bMaxVersion
};

static void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \r\n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \r\n", ascii);
			}
		}
	}
}

static uint32_t  usbd_video_AltSet = 0;//number of current interface alternative setting

/**
  * @}
  */

/** @defgroup USBD_UVC_Private_Functions
  * @{
  */

/**
  * @brief  USBD_UVC_Init
  *         Initialize the UVC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_UVC_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	printf("%s\r\n", __func__);
	
#ifndef VIDEO_USES_ISOC_EP		
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
		      USB_ENDPOINT_IN(USB_UVC_ENDPOINT),		      
          USBD_EP_TYPE_BULK,
					VIDEO_PACKET_SIZE);

  /* Initialize the Video Hardware layer */
  USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(USB_UVC_ENDPOINT));

#else
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
		      USB_ENDPOINT_IN(USB_UVC_ENDPOINT),		      
          USBD_EP_TYPE_ISOC,
					VIDEO_PACKET_SIZE);

  /* Initialize the Video Hardware layer */
  USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(USB_UVC_ENDPOINT));
#endif	
  return USBD_OK;
}

/**
  * @brief  USBD_UVC_Init
  *         DeInitialize the UVC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_UVC_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
	// printf("%s\r\n", __func__);
  USBD_LL_CloseEP (pdev , USB_ENDPOINT_IN(USB_UVC_ENDPOINT));
  
  /* DeInitialize the Audio output Hardware layer */

  return USBD_OK;
}

/**
  * @brief  USBD_UVC_Setup
  *         Handle the UVC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_UVC_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
// printf("%s\r\n", __func__);
  uint16_t len;
  uint8_t  *pbuf;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :   
		// printf("CLASS REQ\r\n");		
    switch (req->bRequest)
    {
    case GET_CUR:
			printf("GET_CUR\r\n");		
      UVC_REQ_GetCurrent(pdev, req);
      break;
    case GET_DEF:
			printf("GET_DEF\r\n");	
      UVC_REQ_GetCurrent(pdev, req);
      break;
    case GET_MIN:
			printf("GET_MIN\r\n");	
      UVC_REQ_GetCurrent(pdev, req);
      break;
    case GET_MAX:
			printf("GET_MAX\r\n");	
      UVC_REQ_GetCurrent(pdev, req);
      break;

    case SET_CUR:
			printf("SET_CUR\r\n");
    	UVC_REQ_SetCurrent(pdev, req);
      break;

    default:
			printf("USBD_CtlError\r\n");
      USBD_CtlError (pdev, req);
		
      return USBD_FAIL;
    }
    break;

    
    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
		// printf("STD REQ\r\n");	
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
			// printf("USB_REQ_GET_DESCRIPTOR\r\n");
      if( (req->wValue >> 8) == CS_DEVICE)
      {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = usbd_video_Desc;   
#else
        pbuf = USBD_UVC_CfgDesc + 18;
#endif 
        len = MIN(USB_VIDEO_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, pbuf, len);
      break;
      
    case USB_REQ_GET_INTERFACE :
			printf("USB_REQ_GET_INTERFACE\r\n");
      USBD_CtlSendData (pdev, (uint8_t *)&usbd_video_AltSet, 1);
      break;
      
    case USB_REQ_SET_INTERFACE :
			printf("USB_REQ_SET_INTERFACE\r\n");
      if ((uint8_t)(req->wValue) < VIDEO_TOTAL_IF_NUM)
      {
        usbd_video_AltSet = (uint8_t)(req->wValue);

        if (usbd_video_AltSet == 1) {
					printf("EP Enabled\r\n");
					// clear the circular buffer					
					//camera_desired_state = 1;
        	//play_status = 1;
        } else {
					printf("EP Disabled\r\n");
					//camera_desired_state = 0;
        	USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(USB_UVC_ENDPOINT));
        	//play_status = 0;
        }
      }
      else
      {
        /* Call the error management function (command will be nacked */
				printf("USBD_CtlError\r\n");
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}


/**
  * @brief  USBD_UVC_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_UVC_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{
	printf("%s\r\n", __func__);
  /* Only OUT data are processed */
  return USBD_OK;
}

/**
  * @brief  USBD_UVC_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_UVC_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
	printf("%s\r\n", __func__);
	USBD_SetupReqTypedef sReq = pdev->request;
	
	printf("usbd_video bmRequest: 0x%02X bRequest: 0x%02X wIndex: 0x%04X wValue: 0x%04X wLength %d\r\n", sReq.bmRequest,sReq.bRequest,sReq.wIndex,sReq.wValue,sReq.wLength);
	
  return USBD_OK;
}
/**
  * @brief  USBD_UVC_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_UVC_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
	printf("%s\r\n", __func__);
  /* Only OUT control data are processed */
  return USBD_OK;
}
/**
  * @brief  USBD_UVC_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_UVC_SOF (USBD_HandleTypeDef *pdev)
{
	//printf("%s\r\n", __func__);
  return USBD_OK;
}

/**
  * @brief  USBD_UVC_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_UVC_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	printf("%s\r\n", __func__);
  return USBD_OK;
}
/**
  * @brief  USBD_UVC_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_UVC_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	printf("%s\r\n", __func__);
  return USBD_OK;
}
/**
  * @brief  USBD_UVC_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_UVC_DataOut (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{
	printf("%s\r\n", __func__);
  return USBD_OK;
}

/**
  * @brief  UVC_Req_GetCurrent
  *         Handles the GET_CUR UVC control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void UVC_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	printf("%s\r\n", __func__);  USBD_LL_FlushEP (pdev, USB_ENDPOINT_OUT(0));

  if(req->wValue == 256)
  {
	  //Probe Request
	  USBD_CtlSendData (pdev, (uint8_t*)&videoProbeControl, req->wLength);
  }
  else if (req->wValue == 512)
  {
	  //Commit Request
	  USBD_CtlSendData (pdev, (uint8_t*)&videoCommitControl, req->wLength);
  }else{
	  USBD_CtlSendData (pdev, (uint8_t*)&videoCommitControl, req->wLength);		
	}
}

/**
  * @brief  UVC_Req_SetCurrent
  *         Handles the SET_CUR UVC control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void UVC_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	printf("%s\r\n", __func__);  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */


	  if(req->wValue == 256)
	  {
		  //Probe Request
		  USBD_CtlPrepareRx (pdev, (uint8_t*)&videoProbeControl, req->wLength);
	  }
	  else if (req->wValue == 512)
	  {
		  //Commit Request
		  USBD_CtlPrepareRx (pdev, (uint8_t*)&videoCommitControl, req->wLength);
	  }else{
		  USBD_CtlPrepareRx (pdev, (uint8_t*)&videoCommitControl, req->wLength);
			
		}

  }
}


/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_UVC_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_UVC_DeviceQualifierDesc);
	printf("%s Len: %d\r\n", __func__, *length);
  return USBD_UVC_DeviceQualifierDesc;
}

/**
  * @brief  USBD_UVC_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_UVC_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_UVC_CfgDesc);
	printf("%s Len: %d\r\n", __func__, *length);
  return USBD_UVC_CfgDesc;
}


/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT Duvitech *****END OF FILE****/
