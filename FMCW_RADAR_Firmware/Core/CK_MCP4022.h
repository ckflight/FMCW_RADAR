
#ifndef CK_MCP4022_H_
#define CK_MCP4022_H_

#include "stm32f4xx.h"

void MCP4022_Init(void);

void MCP4022_UD_High(void);

void MCP4022_UD_Low(void);

void MCP4022_CS_High(void);

void MCP4022_CS_Low(void);

void MCP4022_SetValue(uint8_t new_value);

void MCP4022_Increase(uint8_t value);

void MCP4022_Decrease(uint8_t value);


#endif
