/**
  ******************************************************************************
  * @file    usbd_uvc_if_template.c
  * @author  MCD Application Team
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 Duvitech.
  * All rights reserved.</center></h2>
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_uvc_if_template.h"

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

static int8_t  TEMPLATE_Init         (uint32_t  UVCFreq, uint32_t Volume, uint32_t options);
static int8_t  TEMPLATE_DeInit       (uint32_t options);
static int8_t  TEMPLATE_UVCCmd     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t  TEMPLATE_VolumeCtl    (uint8_t vol);
static int8_t  TEMPLATE_MuteCtl      (uint8_t cmd);
static int8_t  TEMPLATE_PeriodicTC   (uint8_t cmd);
static int8_t  TEMPLATE_GetState     (void);

USBD_UVC_ItfTypeDef USBD_UVC_Template_fops =
{
  TEMPLATE_Init,
  TEMPLATE_DeInit,
  TEMPLATE_UVCCmd,
  TEMPLATE_VolumeCtl,
  TEMPLATE_MuteCtl,
  TEMPLATE_PeriodicTC,
  TEMPLATE_GetState,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  TEMPLATE_Init
  *         Initializes the UVC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_Init(uint32_t  UVCFreq, uint32_t Volume, uint32_t options)
{
  /*
     Add your initialization code here
  */
  return (0);
}

/**
  * @brief  TEMPLATE_DeInit
  *         DeInitializes the UVC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_DeInit(uint32_t options)
{
  /*
     Add your deinitialization code here
  */
  return (0);
}


/**
  * @brief  TEMPLATE_UVCCmd
  *         UVC command handler
  * @param  Buf: Buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_UVCCmd (uint8_t* pbuf, uint32_t size, uint8_t cmd)
{

  return (0);
}

/**
  * @brief  TEMPLATE_VolumeCtl
  * @param  vol: volume level (0..100)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_VolumeCtl (uint8_t vol)
{

  return (0);
}

/**
  * @brief  TEMPLATE_MuteCtl
  * @param  cmd: vmute command
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_MuteCtl (uint8_t cmd)
{

  return (0);
}

/**
  * @brief  TEMPLATE_PeriodicTC
  * @param  cmd
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_PeriodicTC (uint8_t cmd)
{

  return (0);
}

/**
  * @brief  TEMPLATE_GetState
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t TEMPLATE_GetState (void)
{

  return (0);
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

