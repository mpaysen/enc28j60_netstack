/* Includes ------------------------------------------------------------------*/
#include "main.h"
#define BUFFER_SIZE 550

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
arp_table table;
ether_types eth_types;
prtcl_types prot_types;
udp_serivces services;
static uint8_t buffer [BUFFER_SIZE + 1];
mac_address my_mac = {0xB8,0x37,0x4A,0x04,0x20,0x0b}; // MAC address: (b8:37:4a:04:20:0b)
ip_address my_ip = {0x00,0x00,0x00,0x00};
ip_address my_subnet = {0x00,0x00,0x00,0x00};
ip_address my_gateway = {0x00,0x00,0x00,0x00};
ip_address my_dhcp_server = {0x00,0x00,0x00,0x00};
uint8_t dhcp_rdy = 0x00;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void SPI1_Init(void);

/**
 * Initialisiert die erforderlichen Peripherieger�te, konfiguriert den Systemtakt, 
 * initialisiert Netzwerkkomponenten wie Ethernet, DHCP, ARP, ICMP und UDP, 
 * und �berwacht den Netzwerkverkehr.
 * Die Funktion verwendet die HAL-Bibliothek f�r die Mikrocontroller-Peripherie.
 */
int main(void) {
	
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
	
  /* Initialize all configured peripherals */
  GPIO_Init();
  SPI1_Init();
	enc28_init(my_mac); // Initialize eth_hw
	eth_init(&eth_types);// Initialize Layer 2
	ipv4_init(&prot_types);// Initialize Layer 3 (IPv4)
	udp_init(&services); // Initialize Layer 4 (UDP)
	dhcp_init(&my_ip, &my_subnet, &my_gateway, &my_dhcp_server, &dhcp_rdy, my_mac); // Initialize Layer 7 (DHCP)

HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //LED ON
	
  /* CODE BEGIN WHILE */

while (1) { // DHCP Loop
	if(
			my_ip.octet[0] == 0x00 &&
			my_ip.octet[1] == 0x00 &&
			my_ip.octet[2] == 0x00 &&
			my_ip.octet[3] == 0x00
	
	){
		send_dhcp_disc();
	}
	uint16_t length = enc28_packetReceive(BUFFER_SIZE, buffer);
	 if(length){
			eth_handler(buffer, length); //handel DHCP
	}
	 if(dhcp_rdy){
			dhcp_rdy = 0x00;
			break;
	}
}

arp_table_init(&table, &my_ip, my_mac); // Initialize ARP
icmp_init(&my_ip, &my_subnet, &my_gateway, my_mac); // Initialize ICMP
	
	
 while (1)
  {

	uint16_t length = enc28_packetReceive(BUFFER_SIZE, buffer);
if(length){
			eth_handler(buffer, length); //handel Netzwerkverkehr
	 }
	if(dhcp_rdy){
			dhcp_rdy = 0x00;
	}
	//send_icmp_req(my_ip);
	 ///HAL_Delay(2000);
  }
  /* CODE END */
}



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


static void SPI1_Init(void)
{
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
	

}


static void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
	
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}


void Error_Handler(void)
{

  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


