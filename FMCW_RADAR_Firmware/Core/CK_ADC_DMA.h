
#ifndef INC_CK_ADC_DMA_H_
#define INC_CK_ADC_DMA_H_

void CK_ADC_DMA_Init(void);

void CK_ADC_DMA_Start(void);

void CK_ADC_DMA_DMAStop(void);

void CK_ADC_DMA_DMAStart(void);

void CK_ADC_DMA_ADCStop(void);

void CK_ADC_DMA_ADCStart(void);

int CK_ADC_DMA_IsTxComplete(void);

void CK_ADC_DMA_TransferSamples(void);

#endif /* INC_CK_ADC_DMA_H_ */
