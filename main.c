
/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"
#include "ht32_board_config.h"
#include "MPU6500.h"
#include "buzzer_pwm.h"

/* Private constants ---------------------------------------------------------------------------------------*/
#define I2C_MASTER_ADDRESS     0x0A
#define I2C_SLAVE_ADDRESS      0x68
#define ClockSpeed             400000

/* Private function prototypes -----------------------------------------------------------------------------*/
void Buzzer_PlayTable(void);
void Buzzer_Init(u32 uFrequency);
u32 times=0;
int Delay(int);
float abs(float);
void UxART_Configuration(void);

FlagStatus _button1 = RESET;
FlagStatus TmpStatus = RESET;

void CKCU_Configuration(void);
void GPIO_IN_Configuration(void);
void GPIO_OUT_Configuration(void);
void EXTI_WAKE_Configuration(void);
void I2CMaster_Configuration(void);
int Key_Process(void);
vu32 guKeyState[1];

void MPU6500_I2C_Write_OneByte(u8 reg_addr, u8 reg_value);

u8 MPU6500_I2C_Read_OneByte(u8 reg_addr);

u16 Get_HL_Value(u8 high_reg, u8 low_reg);

void Set_Acc_Scale_Select(u8 scale);

void Show_Acc_Scale_Select(void);

double Acc_Scale(u16 raw_value);


/* Private variables ---------------------------------------------------------------------------------------*/
u8 Who_Am_I_Value;
u16 Acc_Raw_Value[3];
int j=0;

/* Global functions ----------------------------------------------------------------------------------------*/
u32 i;
u16 gBee_Scale[] =
{
  1396,
  1048,1396,1048,1396,1048,1396,1048,1396,
	1048,1396,1048,1396,1048,1396,1048,1396,
	1048,1396,1048,1396,1048,1396,1048
};
/*********************************************************************************************************//**
  * @brief  Main program.
  * @retval None
  ***********************************************************************************************************/

int main(void)
{
  RETARGET_Configuration();	
  I2CMaster_Configuration();
	/* 配置I2C、USART */
	CKCU_Configuration();               /* System Related configuration                                       */
  /* Configure WAKEUP, KEY1, KEY2 pins as the input function                                                */
  GPIO_IN_Configuration();
  /* Configure LED1, LED2, LED3 pins as output function                                                     */
  GPIO_OUT_Configuration();
	EXTI_WAKE_Configuration();
  Buzzer_Init(0);
	UxART_Configuration();
	
	
	Who_Am_I_Value = MPU6500_I2C_Read_OneByte(0x75);
	printf("Who am I Value : %02x\r\n", Who_Am_I_Value); 
	while(!(Who_Am_I_Value ==  0x70));
	/* 設定加速度量程為2G */
	Set_Acc_Scale_Select(mpu6500_range_2G);
	Show_Acc_Scale_Select();
  while (1){ 
	/* 得到加速度的X軸、Z軸放入陣列內	*/
		Acc_Raw_Value[0] = Get_HL_Value(AccXH_Reg , AccXL_Reg);
		Acc_Raw_Value[1] = Get_HL_Value(AccXH_Reg , AccYL_Reg);
		Acc_Raw_Value[2] = Get_HL_Value(AccZH_Reg , AccZL_Reg);
		
		printf("Acc%c --> Raw_Value_DEC: %05d , Raw_Value_HEX: %04x , Acc_Scale: %f\r\n",
							Axis[0], Acc_Raw_Value[0], Acc_Raw_Value[0], Acc_Scale(Acc_Raw_Value[0]));
		printf("Acc%c --> Raw_Value_DEC: %05d , Raw_Value_HEX: %04x , Acc_Scale: %f\r\n",
							Axis[1], Acc_Raw_Value[1], Acc_Raw_Value[1], Acc_Scale(Acc_Raw_Value[1]));
		printf("Acc%c --> Raw_Value_DEC: %05d , Raw_Value_HEX: %04x , Acc_Scale: %f\r\n\n\n",
							Axis[2], Acc_Raw_Value[2], Acc_Raw_Value[2], Acc_Scale(Acc_Raw_Value[2]));
		Delay(100);

		if(abs(Acc_Scale(Acc_Raw_Value[0])) > 1.2 || abs(Acc_Scale(Acc_Raw_Value[1])) > 1.2 || Acc_Scale(Acc_Raw_Value[2]) < -1.7)
		{
			printf("someone is falling down!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r");
			Buzzer_PlayTable(); 
		}
	}
}

float abs(float a)
{
	if(a >= 0) return a;
	else return a * (-1);
}

void Buzzer_PlayTable(void)
{
  static u32 i = 0;

  /* Bee 1 times, gBee_Scale[i] Hz, active 250 ms, inactive 250 ms                                          */
  if (Buzzer_IsFinish() == TRUE)
  {
		printf("Playing audio.......\n\r");
		for(i = 0; i < 24; i++)
		{
			printf("i = %d\n\r", i);
			Buzzer_Start(1, gBee_Scale[i], 250, 250);
			if(Delay(100) == 1)
			{
				printf("i stop playing..........................\n\r");
				return;
			}
		}
		printf("Audio finish.........\n\r");
  }
	
}
int Key_Process(void)
{
	//guKeyState[0] = FALSE;
	//printf("i am in key\n\r");
  if (guKeyState[0] == TRUE)
  {
		guKeyState[0] = FALSE;
		printf("guKeyState[0] == TRUE\n\r");
		return 1;
  }
}
void CKCU_Configuration(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
	
	CKCUClock.Bit.PA = 1;
  CKCUClock.Bit.PB = 1;
	CKCUClock.Bit.AFIO       = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
	 
}

void GPIO_IN_Configuration(void)
{
  /* Configure WAKEUP, KEY1, KEY2 pins as the input function                                                */
  /* Configure AFIO mode of input pins                                                                      */
 
  AFIO_GPxConfig(GPIO_PA,AFIO_PIN_3,AFIO_MODE_1);


  /* Configure GPIO direction of input pins                                                                 */
  GPIO_DirectionConfig(HT_GPIOA,GPIO_PIN_3,GPIO_DIR_IN);


  /* Configure GPIO pull resistor of input pins                                                             */
  GPIO_PullResistorConfig(HT_GPIOA,GPIO_PIN_3,GPIO_PR_DOWN);


  GPIO_InputConfig(HT_GPIOA,GPIO_PIN_3,ENABLE);
}
void GPIO_OUT_Configuration(void)
{
  /* Configure LED1, LED2, LED3 pins as output function                                                     */
  /* Configure AFIO mode of output pins                                                                     */
  AFIO_GPxConfig(GPIO_PB,AFIO_PIN_2,AFIO_MODE_1);
  /* Configure GPIO direction of output pins                                                                */
  GPIO_DirectionConfig(HT_GPIOB,GPIO_PIN_2,GPIO_DIR_OUT);
  
}

int Delay(int x)
{
	//printf("i am in delay\n\r");
	for(j = 0; j < x * 5000; j++)
	{
		//printf("i am delay...................\n\r");
		if(Key_Process() == 1)
		{
			printf("i break delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r");
			return 1;
		}//else printf("i am not press...............\n\r");
	}
}

//PIN B12
void EXTI_WAKE_Configuration(void)
{
	
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
    CKCUClock.Bit.AFIO       = 1;
    CKCUClock.Bit.EXTI       = 1;
    CKCUClock.Bit.HTCFG_WAKE_GPIO_CK = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

  /* Configure AFIO mode of input pins                                                                      */
  AFIO_GPxConfig(HTCFG_WAKE_GPIO_ID, HTCFG_WAKE_AFIO_PIN, AFIO_FUN_GPIO);

  /* Enable GPIO Input Function                                                                             */
  GPIO_InputConfig(HTCFG_WAKE_GPIO_PORT, HTCFG_WAKE_GPIO_PIN, ENABLE);

  /* Configure GPIO pull resistor of input pins                                                             */
  GPIO_PullResistorConfig(HTCFG_WAKE_GPIO_PORT, HTCFG_WAKE_GPIO_PIN, GPIO_PR_UP);

  /* Select Port as EXTI Trigger Source                                                                     */
  AFIO_EXTISourceConfig(HTCFG_WAKE_AFIO_EXTI_CH, HTCFG_WAKE_AFIO_ESS);

  { /* Configure EXTI Channel n as rising edge trigger                                                      */

    /* !!! NOTICE !!!
       Notice that the local variable (structure) did not have an initial value.
       Please confirm that there are no missing members in the parameter settings below in this function.
    */
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Channel = HTCFG_WAKE_EXTI_CH;
    EXTI_InitStruct.EXTI_Debounce = EXTI_DEBOUNCE_ENABLE;
    EXTI_InitStruct.EXTI_DebounceCnt = 0;
    EXTI_InitStruct.EXTI_IntType = EXTI_NEGATIVE_EDGE;
    EXTI_Init(&EXTI_InitStruct);
  }

  /* Enable EXTI & NVIC line Interrupt                                                                      */
  EXTI_IntConfig(HTCFG_WAKE_EXTI_CH, ENABLE);
  NVIC_EnableIRQ(HTCFG_WAKE_EXTI_IRQn);
}




/*********************************************************************************************************//**
  * @brief  Configure the I2C.
  * @retval None
  ***********************************************************************************************************/
void I2CMaster_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
	
  /* 配置系統時鐘 */
	CKCUClock.Bit.I2C0 = 1;
  CKCUClock.Bit.AFIO = 1;
	CKCUClock.Bit.PB   = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  /* 配置AFIO	*/
  AFIO_GPxConfig(GPIO_PB, AFIO_PIN_0, AFIO_MODE_7);
  AFIO_GPxConfig(GPIO_PB, AFIO_PIN_1, AFIO_MODE_7);
	/* 配置上拉電阻 */
	GPIO_PullResistorConfig(HT_GPIOB, GPIO_PIN_0, GPIO_PR_UP);
	GPIO_PullResistorConfig(HT_GPIOB, GPIO_PIN_1, GPIO_PR_UP);
	
	/* 配置I2C暫存器 */
  
  I2C_InitStructure.I2C_GeneralCall = DISABLE;
  I2C_InitStructure.I2C_AddressingMode = I2C_ADDRESSING_7BIT;
  I2C_InitStructure.I2C_Acknowledge = DISABLE;
  I2C_InitStructure.I2C_OwnAddress = I2C_MASTER_ADDRESS;
  I2C_InitStructure.I2C_Speed = ClockSpeed;
  I2C_InitStructure.I2C_SpeedOffset = 0;
  I2C_Init(HT_I2C0, &I2C_InitStructure);
  
	/* 啟用I2C1 */
  I2C_Cmd(HT_I2C0, ENABLE);
}


u8 MPU6500_I2C_Read_OneByte(u8 reg_addr)
{
	u8 receive_data = 0;
	
	/* 等待閒置狀態 */
  while (I2C_ReadRegister(HT_I2C0, I2C_REGISTER_SR)&0x80000);
	/* 啟用I2C發送ACK信號 */
  I2C_AckCmd(HT_I2C0, ENABLE);
	/* 發送START信號、目標設備位址，寫入模式 */
  I2C_TargetAddressConfig(HT_I2C0, I2C_SLAVE_ADDRESS, I2C_MASTER_WRITE);
	/* 檢查START信號是否傳輸完成 */
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_SEND_START));
	/* 檢查目標設備位址、讀寫模式位是否傳輸完成 */
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_TRANSMITTER_MODE));
	/* 檢查發送模式下數據暫存器是否為空 */
	while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_TX_EMPTY));
	/* 發送暫存器位址 */
	I2C_SendData(HT_I2C0, reg_addr); 
	/* 發送START信號、目標設備位址，接收模式 */
	I2C_TargetAddressConfig(HT_I2C0, I2C_SLAVE_ADDRESS, I2C_MASTER_READ);
	/* 檢查START信號是否傳輸完成 */
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_SEND_START));
	/* 檢查目標設備位址、讀寫模式位是否傳輸完成 */
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_RECEIVER_MODE));
	/* 關閉I2C發送ACK信號 */
  I2C_AckCmd(HT_I2C0, DISABLE);
	/* 檢查接收模式下數據暫存器是否為空 */
	while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_RX_NOT_EMPTY));
	/* 接收數據 */
	receive_data = I2C_ReceiveData(HT_I2C0);
	/* 發送I2C的停止信號 */
	I2C_GenerateSTOP(HT_I2C0);
	return receive_data;
}

void MPU6500_I2C_Write_OneByte(u8 reg_addr, u8 register_value)
{
	/* 等待閒置狀態 */
	while (I2C_ReadRegister(HT_I2C0, I2C_REGISTER_SR)&0x80000);
	/* 啟用I2C發送ACK信號 */
	I2C_AckCmd(HT_I2C0, ENABLE);
	/* 傳送START信號、目標設備位址，寫入模式 
	Send I2C START & I2C slave address for write*/
  I2C_TargetAddressConfig(HT_I2C0, I2C_SLAVE_ADDRESS, I2C_MASTER_WRITE);
	/* 檢查START信號是否傳輸完成 
	Check on Master Transmitter STA condition and clear it*/
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_SEND_START));
	/* 檢查目標設備位址、讀寫模式位是否傳輸完成 
	Check on Master Transmitter ADRS condition and clear it*/
  while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_TRANSMITTER_MODE));
	/* 檢查發送模式下數據暫存器是否為空
	Check on Master Transmitter TXDE condition*/
	while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_TX_EMPTY));
	/* 發送暫存器位址 */
	I2C_SendData(HT_I2C0, reg_addr); 
	/* 檢查發送模式下數據暫存器是否為空 */
	while (!I2C_CheckStatus(HT_I2C0, I2C_MASTER_TX_EMPTY));
	/* 發送數據 */
	I2C_SendData(HT_I2C0, register_value); 
	/* 發送I2C的停止信號 */
	I2C_GenerateSTOP(HT_I2C0);
}

void Set_Acc_Scale_Select(u8 scale)
{
	u8 scale_data;
	
	scale_data = (MPU6500_I2C_Read_OneByte(Acc_Config_Reg) & 0xE7) | scale;
	MPU6500_I2C_Write_OneByte(Acc_Config_Reg, scale_data); 
}

void Show_Acc_Scale_Select(void)
{
	u8 scale;
	
	scale = MPU6500_I2C_Read_OneByte(Acc_Config_Reg) ;
	
	switch(scale)
	{
		case mpu6500_range_2G:
			printf("Acc_Scale_Status: +-2G\r\n");
			break;
		case mpu6500_range_4G:
			printf("Acc_Scale_Status: +-4G\r\n");
			break;
		case mpu6500_range_8G:
			printf("Acc_Scale_Status: +-8G\r\n");
			break;
		case mpu6500_range_16G:
			printf("Acc_Scale_Status: +-16G\r\n");
			break;
		default:
			printf("Acc_Scale_Status: error\r\n");
			break;
	}
}

u16 Get_HL_Value(u8 high_reg, u8 low_reg)
{
	u8 h_Value, l_Value;
	
	h_Value = MPU6500_I2C_Read_OneByte(high_reg);
	l_Value = MPU6500_I2C_Read_OneByte(low_reg);
	
	/* h_value left 8 bits or l_value = (h_value+l_value)*/
	return (((u16)h_Value << 8) | l_Value);
}

double Acc_Scale(u16 raw_value)
{
	u8 scale;
	double per_digit;
	
	scale = MPU6500_I2C_Read_OneByte(Acc_Config_Reg) & 0x18;
	switch(scale)
	{
		case mpu6500_range_2G:
			per_digit = 2.0/0x8000;
			break;
		case mpu6500_range_4G:
			per_digit = 4.0/0x8000;
			break;
		case mpu6500_range_8G:
			per_digit = 8.0/0x8000;
			break;
		case mpu6500_range_16G:
			per_digit = 16.0/0x8000;
			break;
		default:
			break;
	}
	return ((signed short)raw_value * per_digit);
}

void UxART_Configuration(void)
{
  #if 0 // Use following function to configure the IP clock speed.
  // The UxART IP clock speed must be faster 16x then the baudrate.
  CKCU_SetPeripPrescaler(CKCU_PCLK_UxARTn, CKCU_APBCLKPRE_DIV2);
  #endif

  { /* Enable peripheral clock of AFIO, UxART                                                               */
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    CKCUClock.Bit.AFIO                   = 1;
    CKCUClock.Bit.HTCFG_UART_RX_GPIO_CLK = 1;
    CKCUClock.Bit.HTCFG_UART_IPN         = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
  }

  /* Turn on UxART Rx internal pull up resistor to prevent unknow state                                     */
  GPIO_PullResistorConfig(HTCFG_UART_RX_GPIO_PORT, HTCFG_UART_RX_GPIO_PIN, GPIO_PR_UP);

  /* Config AFIO mode as UxART function.                                                                    */
  AFIO_GPxConfig(HTCFG_UART_TX_GPIO_ID, HTCFG_UART_TX_AFIO_PIN, AFIO_FUN_USART_UART);
  AFIO_GPxConfig(HTCFG_UART_RX_GPIO_ID, HTCFG_UART_RX_AFIO_PIN, AFIO_FUN_USART_UART);

  {
    /* UxART configured as follow:
          - BaudRate = 9600 baud
          - Word Length = 8 Bits
          - One Stop Bit
          - None parity bit
    */

    /* !!! NOTICE !!!
       Notice that the local variable (structure) did not have an initial value.
       Please confirm that there are no missing members in the parameter settings below in this function.
    */
    USART_InitTypeDef USART_InitStructure = {0};
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
    USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
    USART_InitStructure.USART_Parity = USART_PARITY_NO;
    USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
    USART_Init(HTCFG_UART_PORT, &USART_InitStructure);
  }

  /* Enable UxART interrupt of NVIC                                                                         */
  NVIC_EnableIRQ(HTCFG_UART_IRQn);

  /* Enable UxART Rx interrupt                                                                              */
  USART_IntConfig(HTCFG_UART_PORT, USART_INT_RXDR, ENABLE);

  /* Enable UxART Tx and Rx function                                                                        */
  USART_TxCmd(HTCFG_UART_PORT, ENABLE);
  USART_RxCmd(HTCFG_UART_PORT, ENABLE);
}

/*********************************************************************************************************//**
  * @brief  Configure the I2C.
  * @retval None
  ***********************************************************************************************************/


/*********************************************************************************************************//**
  * @brief  The I2C master-slave state is checked by interrupt and data is sent from the master to the slave.
  * @retval None
  ***********************************************************************************************************/


/*********************************************************************************************************//**
  * @brief  Compare two buffers.
  * @param  Buffer1, Buffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval SUCCESS or ERROR
  ***********************************************************************************************************/


#if (HT32_LIB_DEBUG == 1)
/*********************************************************************************************************//**
  * @brief  Report both the error name of the source file and the source line number.
  * @param  filename: pointer to the source file name.
  * @param  uline: error line source number.
  * @retval None
  ***********************************************************************************************************/
void assert_error(u8* filename, u32 uline)
{
  /*
     This function is called by IP library that the invalid parameters has been passed to the library API.
     Debug message can be added here.
     Example: printf("Parameter Error: file %s on line %d\r\n", filename, uline);
  */

  while (1)
  {
  }
}
#endif


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
