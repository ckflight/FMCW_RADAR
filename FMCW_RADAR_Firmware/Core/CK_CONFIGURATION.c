
#include "CK_CONFIGURATION.h"

#include "USBD_CDC/CK_USBD_INTERFACE.h"
#include "CK_TIME_HAL.h"

#include "CK_TQP5523.h"
#include "CK_ADF4158.h"

#define	CHECK_FREQ				10 // 10Hz
#define PYTHON_BUFFER_SIZE		64

uint8_t isStarted = 0;

uint8_t config_buffer[PYTHON_BUFFER_SIZE];
uint16_t config_index;

uint32_t record_time_counter = 0;
uint8_t record_time = 0;
uint32_t sweep_time_config = 0;
uint32_t sweep_gap_config = 0;
uint32_t sampling_frequency = 0;
uint32_t number_of_samples = 0;
uint32_t sweep_start_freq = 0;
uint32_t sweep_bw = 0;
uint8_t tx_mode = 0;
uint8_t gain_value = 0;
uint8_t sweep_type = 0;

typedef enum{

	IDLE,
	SEND_TX_POWER,
	READ_RECORD_TIME

}CONFIG_STATES_E;

CONFIG_STATES_E config_state = IDLE;

void CK_CONFIGURATION_Init(void){

	for(int i = 0; i < PYTHON_BUFFER_SIZE; i++){
		config_buffer[i] = 0;
	}

	config_index = 0;

	record_time_counter = 0;

	record_time = 0;

	sweep_time_config = 0;

	sweep_gap_config = 0;

	sampling_frequency = 0;

	number_of_samples = 0;

	sweep_start_freq = 0;

	sweep_bw = 0;

	isStarted = 0;

	tx_mode = 0;

	gain_value = 0;

}

void CK_CONFIGURATION_InitHardware(uint8_t mode){

	CK_CONFIGURATION_Init();

	uint8_t rx_data = 0;

	if(mode == 1){
		// Python will send start charcters to Radar
		while(isStarted == 0){

			// Read received data over usb
			while(CK_USBD_ReadData(&rx_data)){
				// Firs 2 bytes are indicator sign decode rest of the numbers.
				if(config_index >= 2){
					config_buffer[config_index++] = CK_CONFIGURATION_DecodeData(rx_data);
				}
				else{
					config_buffer[config_index++] = rx_data;
				}

				CK_TIME_DelayMilliSec(1);
			}

			// HOST will start state machine according to the data received.
			if(config_buffer[0] == '=' && config_buffer[1] == '=' && config_state == IDLE){

				uint8_t sw_t_msb = (config_buffer[2] << 4) | config_buffer[3];
				uint8_t sw_t_lsb = (config_buffer[4] << 4) | config_buffer[5];
				sweep_time_config = (sw_t_msb << 8) | (sw_t_lsb);

				uint8_t sw_g_msb = (config_buffer[6] << 4) | config_buffer[7];
				uint8_t sw_g_lsb = (config_buffer[8] << 4) | config_buffer[9];
				sweep_gap_config = (sw_g_msb << 8) | (sw_g_lsb);

				record_time = (config_buffer[10] << 4) | config_buffer[11];
				uint32_t sweep_p_microsec = sweep_time_config + sweep_gap_config;
				record_time_counter = (record_time * 1000.0f) / ((float)sweep_p_microsec / 1000.0f);

				uint8_t fs_msb = (config_buffer[12] << 4) | config_buffer[13];
				uint8_t fs_lsb = (config_buffer[14] << 4) | config_buffer[15];
				sampling_frequency = (fs_msb << 8) | (fs_lsb);

				uint8_t num_sample_msb = (config_buffer[16] << 4) | config_buffer[17];
				uint8_t num_sample_lsb = (config_buffer[18] << 4) | config_buffer[19];
				number_of_samples = (num_sample_msb << 8) | (num_sample_lsb);

				uint8_t sweep_start_msb = (config_buffer[20] << 4) | config_buffer[21];
				uint8_t sweep_start_lsb = (config_buffer[22] << 4) | config_buffer[23];
				sweep_start_freq = (sweep_start_msb << 8) | (sweep_start_lsb);

				uint8_t sweep_bw_msb = (config_buffer[24] << 4) | config_buffer[25];
				uint8_t sweep_bw_lsb = (config_buffer[26] << 4) | config_buffer[27];
				sweep_bw = (sweep_bw_msb << 8) | (sweep_bw_lsb);

				tx_mode = (config_buffer[28] << 4) | config_buffer[29];

				gain_value = (config_buffer[30] << 4) | config_buffer[31];

				sweep_type = (config_buffer[32] << 4) | config_buffer[33];

				config_index = 0;

				config_state = SEND_TX_POWER;

			}

			switch(config_state){

			case IDLE:

				break;

			case SEND_TX_POWER:


				CK_USBD_WriteTxCircularBuffer('=');
				CK_USBD_WriteTxCircularBuffer('=');

				CK_TQP5523_Enable();

				CK_TQP5523_CheckOutputPower();
				int dbm_power = CK_TQP5523_ReadDetectorOutputDBM();

				CK_TQP5523_Disable();

				CK_USBD_WriteTxCircularBuffer(dbm_power);
				CK_USBD_Transmit();

				config_state = READ_RECORD_TIME;

				break;

			case READ_RECORD_TIME:

				isStarted = 1;

				break;

			}
		}

	}
	else if(mode == 2){

		sweep_time_config = 2030;

		sweep_gap_config = 700;

		record_time = 10;
		uint32_t sweep_p_microsec = sweep_time_config + sweep_gap_config;
		record_time_counter = (record_time * 1000.0f) / ((float)sweep_p_microsec / 1000.0f);

		sampling_frequency = 250;

		number_of_samples = 500;

		sweep_start_freq = 580;

		sweep_bw = 200;

		sweep_type = 0; // 0 for Sawtooth, 1 for Triangular

		tx_mode = 2; // 0 for continuous tx, 1 for on off with tx, 2 for off

		gain_value = 1;

		config_index = 0;

	}
	else if(mode == 3){

		sweep_time_config = 1000;

		sweep_gap_config = 0;

		record_time = 20;
		uint32_t sweep_p_microsec = sweep_time_config + sweep_gap_config;
		record_time_counter = (record_time * 1000.0f) / ((float)sweep_p_microsec / 1000.0f);

		sampling_frequency = 250;

		number_of_samples = 250;

		sweep_start_freq = 570; //5.7GHz

		sweep_bw = 300;

		sweep_type = 1; // 0 for Sawtooth, 1 for Triangular

		tx_mode = 1; // 0 for continuous tx, 1 for on off with tx, 2 for off

		gain_value = 1;

		config_index = 0;

	}

}

uint32_t CK_CONFIGURATION_GetRecordTimeCounter(void){

	return record_time_counter;

}

uint8_t CK_CONFIGURATION_GetRecordTime(void){

	return record_time;

}

uint32_t CK_CONFIGURATION_GetSweepTime(void){

	return sweep_time_config;

}

uint32_t CK_CONFIGURATION_GetSweepGap(void){

	return sweep_gap_config;

}

uint32_t CK_CONFIGURATION_GetSamplingFrequency(void){

	return sampling_frequency;

}

uint32_t CK_CONFIGURATION_GetSampleNumber(void){

	return number_of_samples;

}

uint32_t CK_CONFIGURATION_GetSweepStartFrequency(void){

	return sweep_start_freq;

}

uint32_t CK_CONFIGURATION_GetSweepBandwith(void){

	return sweep_bw;

}

uint8_t CK_CONFIGURATION_GetTXMode(void){

	return tx_mode;

}

uint8_t CK_CONFIGURATION_SweepType(void){

	return sweep_type;

}

uint8_t CK_CONFIGURATION_GetGainValue(void){

	return gain_value;

}

uint8_t CK_CONFIGURATION_DecodeData(uint8_t rx_data){

	// Received data is ascii coded 0-9 -> 48-57, a-f -> 97-102
	// if data is 0x2a received data will be 50(2) and 97(a)
	uint8_t data_decoded = 0;
	if(rx_data >= 48 && rx_data <= 57){
		data_decoded = rx_data - 48;
	}
	else if(rx_data >= 97 && rx_data <= 102){
		if(rx_data == 97){
			data_decoded = 0xa;
		}
		else if(rx_data == 98){
			data_decoded = 0xb;
		}
		else if(rx_data == 99){
			data_decoded = 0xc;
		}
		else if(rx_data == 100){
			data_decoded = 0xd;
		}
		else if(rx_data == 101){
			data_decoded = 0xe;
		}
		else if(rx_data == 102){
			data_decoded = 0xf;
		}
	}

	return data_decoded;
}




















