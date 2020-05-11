
#ifndef __USBD_IOREQ_H
#define __USBD_IOREQ_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "USBD_CDC/usbd_def.h"
#include  "USBD_CDC/usbd_core.h"

USBD_StatusTypeDef  USBD_CtlSendData (USBD_HandleTypeDef *pdev,
                               uint8_t *pbuf,
                               uint16_t len);

USBD_StatusTypeDef  USBD_CtlContinueSendData (USBD_HandleTypeDef  *pdev,
                               uint8_t *pbuf,
                               uint16_t len);

USBD_StatusTypeDef USBD_CtlPrepareRx (USBD_HandleTypeDef  *pdev,
                               uint8_t *pbuf,
                               uint16_t len);

USBD_StatusTypeDef  USBD_CtlContinueRx (USBD_HandleTypeDef  *pdev,
                              uint8_t *pbuf,
                              uint16_t len);

USBD_StatusTypeDef  USBD_CtlSendStatus (USBD_HandleTypeDef  *pdev);

USBD_StatusTypeDef  USBD_CtlReceiveStatus (USBD_HandleTypeDef  *pdev);

uint32_t  USBD_GetRxCount (USBD_HandleTypeDef *pdev, uint8_t ep_addr);

#ifdef __cplusplus
}
#endif

#endif
