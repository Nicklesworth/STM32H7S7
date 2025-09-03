/* SPDX-License-Identifier: Unlicense */
#include "psram.h"
#include "common.h"
#include "stm32.h"

/* Aps256xx APMemory memory */
  
/* Read Operations */
#define READ_CMD                                0x00
#define READ_LINEAR_BURST_CMD                   0x20
#define READ_HYBRID_BURST_CMD                   0x3F
  
/* Write Operations */
#define WRITE_CMD                               0x80
#define WRITE_LINEAR_BURST_CMD                  0xA0
#define WRITE_HYBRID_BURST_CMD                  0xBF
  
/* Reset Operations */
#define RESET_CMD                               0xFF
  
/* Registers definition */
#define MR0                                     0x00000000
#define MR1                                     0x00000001
#define MR2                                     0x00000002
#define MR3                                     0x00000003
#define MR4                                     0x00000004
#define MR8                                     0x00000008
  
/* Register Operations */
#define READ_REG_CMD                            0x40
#define WRITE_REG_CMD                           0xC0
  
/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ                 4
#define DUMMY_CLOCK_CYCLES_WRITE                4

/**
* @brief  Write mode register
* @param  Ctx Component object pointer
* @param  Address Register address
* @param  Value Register value pointer
* @retval error status
*/
uint32_t APS256_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value)
{
  XSPI_RegularCmdTypeDef sCommand1={0};
  
  /* Initialize the write register command */
  sCommand1.OperationType       = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand1.InstructionMode     = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand1.InstructionWidth    = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand1.InstructionDTRMode  = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand1.Instruction         = WRITE_REG_CMD;
  sCommand1.AddressMode         = HAL_XSPI_ADDRESS_8_LINES;
  sCommand1.AddressWidth        = HAL_XSPI_ADDRESS_32_BITS;
  sCommand1.AddressDTRMode      = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand1.Address             = Address;
  sCommand1.AlternateBytesMode  = HAL_XSPI_ALT_BYTES_NONE;
  sCommand1.DataMode            = HAL_XSPI_DATA_8_LINES;
  sCommand1.DataDTRMode         = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand1.DataLength          = 2;
  sCommand1.DummyCycles         = 0;
  sCommand1.DQSMode             = HAL_XSPI_DQS_DISABLE;
  
  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand1, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  /* Transmission of the data */
  if (HAL_XSPI_Transmit(Ctx, (uint8_t *)(Value), HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK;
}

/**
* @brief  Read mode register value
* @param  Ctx Component object pointer
* @param  Address Register address
* @param  Value Register value pointer
* @param  LatencyCode Latency used for the access
* @retval error status
*/
uint32_t APS256_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value, uint32_t LatencyCode)
{
  XSPI_RegularCmdTypeDef sCommand={0};
  
  /* Initialize the read register command */
  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = READ_REG_CMD;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = Address;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength         = 2;
  sCommand.DummyCycles        = (LatencyCode - 1U);
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;
  
  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  /* Reception of the data */
  if (HAL_XSPI_Receive(Ctx, (uint8_t *)Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK;
}

/**
* @brief  Switch from Octal Mode to Hexa Mode on the memory
* @param  None
* @retval None
*/
void PSRAM_Init(void)
{
  /* MR0 register for read and write */
  uint8_t regW_MR0[2]={0x24,0x8D}; /* To configure AP memory Latency Type and drive Strength */
  uint8_t regR_MR0[2]={0};
  
  /* MR8 register for read and write */
  uint8_t regW_MR8[2]={0x4B,0x08}; /* To configure AP memory Burst Type */
  uint8_t regR_MR8[2]={0};
  
  /*Read Latency */
  uint8_t latency=6;
  
  /* Configure Read Latency and drive Strength */
  if (APS256_WriteReg(&hxspi1, MR0, regW_MR0) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Check MR0 configuration */
  if (APS256_ReadReg(&hxspi1, MR0, regR_MR0, latency ) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Check MR0 configuration */
  if (regR_MR0 [0] != regW_MR0 [0])
  {
    Error_Handler() ;
  }
  
  /* Configure Burst Length */
  if (APS256_WriteReg(&hxspi1, MR8, regW_MR8) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Check MR8 configuration */
  if (APS256_ReadReg(&hxspi1, MR8, regR_MR8, 6) != HAL_OK)
  {
    Error_Handler();
  }
  
  if (regR_MR8[0] != regW_MR8[0])
  {
    Error_Handler() ;
  }

  XSPI_RegularCmdTypeDef sCommand = {0};

  sCommand.OperationType      = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = WRITE_CMD;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = 0x0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_16_LINES;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength         = 0;
  sCommand.DummyCycles        = DUMMY_CLOCK_CYCLES_WRITE;
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;
  
  if (HAL_XSPI_Command(&hxspi1, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  
  sCommand.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCommand.Instruction = READ_CMD;
  sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ;
  sCommand.DQSMode     = HAL_XSPI_DQS_ENABLE;
  
  if (HAL_XSPI_Command(&hxspi1, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  
  XSPI_MemoryMappedTypeDef sMemMappedCfg;
  sMemMappedCfg.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_ENABLE;
  sMemMappedCfg.TimeoutPeriodClock      = 0x34;
  
  
  if (HAL_XSPI_MemoryMapped(&hxspi1, &sMemMappedCfg) != HAL_OK)
  {
    Error_Handler();
  }

}