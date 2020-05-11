
#include "CK_UART.h"
#include "CK_GPIO.h"


void CK_UART_Init1(int baudRate){

	/*
	 * Default 8bit 1 start 1 stop bit, no parity
	 * USART1,6 Clocked by PCLK2
	 * USART2,3,4,5 Clocked by PCLK1
	 */
	int baud = baudRate;

	int frequency_pclk2 = HAL_RCC_GetPCLK2Freq();//90MHz or 84MHz
	frequency_pclk2 = 90000000;//90MHz or 84MHz

	int baudNum = frequency_pclk2/(8*baud);

	/*
	 * USART1 PA9=TX,PA10=RX
	 * RX Not Used
	 */
	RCC->APB2ENR |= CK_RCC_APB2ENR_USART1;
	CK_GPIO_ClockEnable(GPIOA);
	CK_GPIO_Init(GPIOA,9,CK_GPIO_AF,CK_GPIO_AF7,CK_GPIO_PUSHPULL,CK_GPIO_HIGH,CK_GPIO_NOPUPD);
	CK_GPIO_Init(GPIOA,10,CK_GPIO_AF,CK_GPIO_AF7,CK_GPIO_PUSHPULL,CK_GPIO_HIGH,CK_GPIO_NOPUPD);

	USART1->BRR = baudNum<<4;//First 4 bit is fraction and not affecting
	USART1->CR1 |= CK_USART_CR1_UE | CK_USART_CR1_OVER8;

}

void CK_Usart1Send(uint8_t chr[],int size){

	USART1->CR1 |= CK_USART_CR1_TE;
	for(int i=0;i<size;i++){
		USART1->DR = chr[i];
		while((USART1->SR & CK_USART_SR_TXE) == 0);
	}
	while((USART1->SR & CK_USART_SR_TC) == 0);
}

void CK_IntPrintln(int num){
	int idx = 0;
	if(num < 0){
		CK_TXBuffer_int[idx++] = 45;//ASCII '-'
		num *= -1;
	}
	int tmp[6];
	int numOfDigits = 0;
	int limit = 6;

	for(int i=0; i < limit; i++){
		if(num>=10){
			tmp[i] = (int)num%10;
			num = num-tmp[i];
			num = num/10;
		}
		else{
			tmp[i] = num;
			numOfDigits = i;
			i = limit;
		}
	}
	for(int i=numOfDigits; i >= 0; i--){
		CK_TXBuffer_int[idx++] = tmp[i]+48;//for ASCII code of number
	}
	CK_TXBuffer_int[idx++] = 0x0A;	//ASCII line feed
	CK_TXBuffer_int[idx++] = 0x0D;	//ASCII carriage return


	CK_Usart1Send(CK_TXBuffer_int,idx);

}

void CK_IntPrint(int num){
	int idx = 0;
	if(num < 0){
		CK_TXBuffer_int[idx++] = 45;//ASCII '-'
		num *= -1;
	}
	int tmp[6];
	int numOfDigits = 0;
	int limit = 6;

	for(int i=0; i < limit; i++){
		if(num>=10){
			tmp[i] = (int)num%10;
			num = num-tmp[i];
			num = num/10;
		}
		else{
			tmp[i] = num;
			numOfDigits = i;
			i = limit;
		}
	}
	for(int i=numOfDigits; i >= 0; i--){
		CK_TXBuffer_int[idx++] = tmp[i]+48;//for ASCII code of number
	}

	CK_Usart1Send(CK_TXBuffer_int,idx);

}

void CK_Int16Println(int16_t num){
	int idx = 0;
	int16_t numUnsigned = num;
	if(num < 0){
		CK_TXBuffer_int16[idx++] = 45;//ASCII '-'
		numUnsigned = ~(num) + 1; //2's Complement
	}
	int tmp[6];
	int numOfDigits = 0;
	int limit = 6;


	for(int i=0; i < limit; i++){
		if(numUnsigned>=10){
			tmp[i] = (int)numUnsigned%10;
			numUnsigned = numUnsigned-tmp[i];
			numUnsigned = numUnsigned/10;
		}
		else{
			tmp[i] = numUnsigned;
			numOfDigits = i;
			i = limit;
		}
	}
	for(int i=numOfDigits; i >= 0; i--){
		CK_TXBuffer_int16[idx++] = tmp[i]+48;//for ASCII code of number
	}
	CK_TXBuffer_int16[idx++] = 10;//ASCII new line

	CK_Usart1Send(CK_TXBuffer_int16,idx);

}

void CK_Int16Print(int16_t num){
	int idx = 0;
	int16_t numUnsigned = num;
	if(num < 0){
		CK_TXBuffer_int16[idx++] = 45;//ASCII '-'
		numUnsigned = ~(num) + 1; //2's Complement
	}
	int tmp[6];
	int numOfDigits = 0;
	int limit = 6;


	for(int i=0; i < limit; i++){
		if(numUnsigned>=10){
			tmp[i] = (int)numUnsigned%10;
			numUnsigned = numUnsigned-tmp[i];
			numUnsigned = numUnsigned/10;
		}
		else{
			tmp[i] = numUnsigned;
			numOfDigits = i;
			i = limit;
		}
	}
	for(int i=numOfDigits; i >= 0; i--){
		CK_TXBuffer_int16[idx++] = tmp[i]+48;//for ASCII code of number
	}

	CK_Usart1Send(CK_TXBuffer_int16,idx);

}

void CK_FloatPrintln(float num){
	int idx = 0;
	int flag = 0;
	num = num*100;//2 digit after comma
	int intnum = (int)num;
	const int limit = 10;//10 digit number max
	if(intnum<0){
		CK_TXBuffer_float[idx++] = 45;//ASCII '-'
		intnum *= -1;
	}
	if(intnum<10){//0.00
		flag = 1;
	}
	else if(intnum>=10 && intnum<=99){//0.01 to 0.99
		flag = 2;
	}

	int tmp[limit];
	int numOfDigits = 0;
	for(int i=0;i<limit;i++){
		if(intnum>=10){
			tmp[i] = (int)intnum%10;
			intnum = intnum-tmp[i];
			intnum = intnum/10;
		}
		else{
			tmp[i] = intnum;
			numOfDigits = i;//get num of digits
			i = limit;//end loop
		}
	}
	if(flag==1){//make 0.00
		tmp[1] = 0;tmp[2] = 0;
		numOfDigits = 2;
	}
	if(flag==2){//add 0 for 0.01 to 0.99 numbers
		tmp[++numOfDigits] = 0;
	}

	for(int i=numOfDigits; i >= 0; i--){
		if(i>1){
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
		else if(i==1){
			CK_TXBuffer_float[idx++] = 46;//for ASCII code of '.'
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
		else{
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
	}
	CK_TXBuffer_float[idx++] = 10;//ASCII new line
	CK_Usart1Send(CK_TXBuffer_float,idx);

}

void CK_FloatPrint(float num){
	int idx = 0;
	int flag = 0;
	num = num*100;//2 digit after comma
	int intnum = (int)num;
	const int limit = 10;//10 digit number max
	if(intnum<0){
		CK_TXBuffer_float[idx++] = 45;//ASCII '-'
		intnum *= -1;
	}
	if(intnum<10){//0.00
		flag = 1;
	}
	else if(intnum>=10 && intnum<=99){//0.01 to 0.99
		flag = 2;
	}

	int tmp[limit];
	int numOfDigits = 0;
	for(int i=0;i<limit;i++){
		if(intnum>=10){
			tmp[i] = (int)intnum%10;
			intnum = intnum-tmp[i];
			intnum = intnum/10;
		}
		else{
			tmp[i] = intnum;
			numOfDigits = i;//get num of digits
			i = limit;//end loop
		}
	}
	if(flag==1){//make 0.00
		tmp[1] = 0;tmp[2] = 0;
		numOfDigits = 2;
	}
	if(flag==2){//add 0 for 0.01 to 0.99 numbers
		tmp[++numOfDigits] = 0;
	}

	for(int i=numOfDigits; i >= 0; i--){
		if(i>1){
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
		else if(i==1){
			CK_TXBuffer_float[idx++] = 46;//for ASCII code of '.'
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
		else{
			CK_TXBuffer_float[idx++] = tmp[i]+48;//for ASCII code of number
		}
	}
	CK_Usart1Send(CK_TXBuffer_float,idx);

}

void CK_StringPrintln(const char str[]){

	int size = CK_getArraySize(str);
	uint8_t tmp[CK_BufferSize];
	for(int i=0;i<size;i++){
		tmp[i] = str[i];
	}
	tmp[size++] = 10;
	CK_Usart1Send(tmp,size);

}
void CK_StringPrint(const char str[]){

	int size = CK_getArraySize(str);
	uint8_t tmp[CK_BufferSize];
	for(int i=0;i<size;i++){
		tmp[i] = str[i];
	}
	CK_Usart1Send(tmp,size);

}

int CK_getArraySize(const char  arry[]){
	int s= 0;
	while(arry[s] != 0){//Null
		s++;
	}
	return s;
}




