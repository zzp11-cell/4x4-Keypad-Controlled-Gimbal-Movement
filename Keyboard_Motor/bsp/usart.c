#include "usart.h"

/**********************************************************
***	Emm_V5.0�����ջ���������
***	��д���ߣ�ZHANGDATOU
***	����֧�֣��Ŵ�ͷ�ջ��ŷ�
***	�Ա����̣�https://zhangdatou.taobao.com
***	CSDN���ͣ�http s://blog.csdn.net/zhangdatou666
***	qq����Ⱥ��262438510
**********************************************************/

__IO uint8_t rxCmd[FIFO_SIZE] = {0};
__IO uint8_t rxCount = 0;

/**
	* @brief   UART_0�жϺ���
	* @param   ��
	* @retval  ��
	*/
void UART_3_INST_IRQHandler(void)
{
/**********************************************************
***	���ڽ����ж�
**********************************************************/
	if(DL_UART_getPendingInterrupt(UART_3_INST) == DL_UART_IIDX_RX)
	{
		// δ���һ֡���ݽ��գ����ݽ��뻺�����
		fifo_enQueue((uint8_t)DL_UART_Main_receiveData(UART_3_INST));
	}

	// ������ڽ����ж�
	DL_UART_clearInterruptStatus(UART_3_INST, DL_UART_IIDX_RX);
}

/**
	* @brief   ��ȡ��������
	* @param   ��
	* @retval  ��
	*/
void usart_getCmd(void)
{
	__IO uint16_t i = 0;
	
	// ��ȡһ֡��������
	rxCount = fifo_queueLength(); for(i=0; i < rxCount; i++) { rxCmd[i] = fifo_deQueue(); }
}

/**
	* @brief   USART���Ͷ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;
	
	for(i=0; i < len; i++) { usart_SendByte(cmd[i]); }
}

/**
	* @brief   USART����һ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart_SendByte(uint16_t data)
{
	__IO uint16_t t0 = 0;

	//������0æ��ʱ��ȴ�����æ��ʱ���ٷ��ʹ��������ַ�
	while(DL_UART_isBusy(UART_3_INST) == true)
	{
		++t0; if(t0 > 8000)	{	return; }	// ��ʱ�˳�����ֹ����
	}
	
	//���͵����ַ�
	DL_UART_Main_transmitData(UART_3_INST, data);
}




