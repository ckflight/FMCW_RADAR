
#include "CK_MCP4022.h"
#include "CK_GPIO.h"
#include "CK_TIME_HAL.h"

#define MCP4022_GPIO		GPIOC
#define MCP4022_CS_PIN		2
#define MCP4022_UD_PIN		3

#define MCP4022_MAXVALUE	0x3F

uint8_t current_value = 0;

void MCP4022_Init(void){

	// MCP4022 has 64 resistor levels in it which can be incremented or decremented
	// with CS pin is pulled low and sending 0x00 to 0x3F values with UD pin
	// to select one resistor level.

	CK_GPIO_Init(MCP4022_GPIO, MCP4022_UD_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_HIGH, CK_GPIO_NOPUPD);
	MCP4022_UD_Low();

	CK_GPIO_Init(MCP4022_GPIO, MCP4022_CS_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_HIGH, CK_GPIO_NOPUPD);
	MCP4022_CS_Low();

}

// Because of NPN transistor when send high it will be connected to GND
// and will be high when send 3v3
void MCP4022_UD_High(void){
	CK_GPIO_ClearPin(MCP4022_GPIO, MCP4022_UD_PIN);
}

void MCP4022_UD_Low(void){
	CK_GPIO_SetPin(MCP4022_GPIO, MCP4022_UD_PIN);
}

void MCP4022_CS_High(void){
	CK_GPIO_ClearPin(MCP4022_GPIO, MCP4022_CS_PIN);
}

void MCP4022_CS_Low(void){
	CK_GPIO_SetPin(MCP4022_GPIO, MCP4022_CS_PIN);
}

void MCP4022_SetValue(uint8_t new_value){

	// Device has 64 steps new value is hex value max 0x3F = 63
	// Rwb = (Rab * new_value) / 63;

	if(new_value > MCP4022_MAXVALUE){
		new_value = MCP4022_MAXVALUE;
	}

	uint8_t val = 0;
	if(new_value > current_value){
		val = new_value - current_value;
		MCP4022_Increase(val);
	}
	else if(new_value < current_value){
		val = current_value - new_value;
		MCP4022_Decrease(val);
	}

}

void MCP4022_Increase(uint8_t value){

	// Increment mode UD high prior to CS low
	// Every subsequent rising edge increments one value
	// Min value of pot. is 0x00 max is 0x3F
	// Incremented value is not written to EEPROM in this config.
	// To write it to EEPROM, during connection UD pull Low and force CS high. (not used now)
	// If reaches 0x3F it ignores new increments.

	MCP4022_UD_High();
	MCP4022_CS_Low();
	CK_TIME_DelayMicroSec(5);

	for(int i = 0; i < value; i++){
		MCP4022_UD_Low();
		CK_TIME_DelayMicroSec(3);
		MCP4022_UD_High();
		CK_TIME_DelayMicroSec(3);
	}

	CK_TIME_DelayMicroSec(5);
	MCP4022_UD_High();
	MCP4022_CS_High();

}

void MCP4022_Decrease(uint8_t value){

	// Decrement mode UD low prior to CS low
	// Every subsequent rising edge decrements one value

	MCP4022_UD_Low();
	MCP4022_CS_Low();
	CK_TIME_DelayMicroSec(5);

	for(int i = 0; i < value; i++){
		MCP4022_UD_High();
		CK_TIME_DelayMicroSec(3);
		MCP4022_UD_Low();
		CK_TIME_DelayMicroSec(3);
	}

	CK_TIME_DelayMicroSec(5);
	MCP4022_UD_High();
	MCP4022_CS_High();

}













