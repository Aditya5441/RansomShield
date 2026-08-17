/**
 * @file    system_stm32wbxx.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "stm32wbxx.h"

#if !defined  (HSE_VALUE)
#define HSE_VALUE    (32000000UL)
#endif

#if !defined  (MSI_VALUE)
#define MSI_VALUE    (4000000UL)
#endif

#if !defined  (HSI_VALUE)
#define HSI_VALUE    (16000000UL)
#endif

#if !defined  (LSI_VALUE)
#define LSI_VALUE  (32000UL)
#endif

#if !defined  (LSE_VALUE)
#if defined(STM32WB5Mxx)
  #define LSE_VALUE    32774U
#else
  #define LSE_VALUE    32768U
#endif
#endif

#if defined(USER_VECT_TAB_ADDRESS)

#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM1_BASE
#if !defined(VECT_TAB_OFFSET)
#define VECT_TAB_OFFSET         0x00000000U
#endif

#else
#define VECT_TAB_BASE_ADDRESS   FLASH_BASE
#if !defined(VECT_TAB_OFFSET)
#define VECT_TAB_OFFSET         0x00000000U
#endif

#endif
#endif

uint32_t SystemCoreClock = 4000000UL ;

const uint32_t AHBPrescTable[16UL] = {1UL, 3UL, 5UL, 1UL, 1UL, 6UL, 10UL, 32UL, 2UL, 4UL, 8UL, 16UL, 64UL, 128UL, 256UL, 512UL};

const uint32_t APBPrescTable[8UL]  = {0UL, 0UL, 0UL, 0UL, 1UL, 2UL, 3UL, 4UL};

const uint32_t MSIRangeTable[16UL] = {100000UL, 200000UL, 400000UL, 800000UL, 1000000UL, 2000000UL, \
                                      4000000UL, 8000000UL, 16000000UL, 24000000UL, 32000000UL, 48000000UL, 0UL, 0UL, 0UL, 0UL
                                     };

#if defined(STM32WB55xx) || defined(STM32WB5Mxx) || defined(STM32WB35xx) || defined (STM32WB15xx) || defined (STM32WB1Mxx)
const uint32_t SmpsPrescalerTable[4UL][6UL] = {{1UL, 3UL, 2UL, 2UL, 1UL, 2UL}, \
  {2UL, 6UL, 4UL, 3UL, 2UL, 4UL}, \
  {4UL, 12UL, 8UL, 6UL, 4UL, 8UL}, \
  {4UL, 12UL, 8UL, 6UL, 4UL, 8UL}
};
#endif

/**
 * @brief   TODO: describe what SystemInit() does
 */
void SystemInit(void)
{
#if defined(USER_VECT_TAB_ADDRESS)

  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
#endif


#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << (10UL * 2UL)) | (3UL << (11UL * 2UL)));
#endif



  RCC->CR |= RCC_CR_MSION;


  RCC->CFGR = 0x00070000U;


  RCC->CR &= (uint32_t)0xFAF6FEFBU;


  RCC->CSR &= (uint32_t)0xFFFFFFFAU;


  RCC->CRRCR &= (uint32_t)0xFFFFFFFEU;


  RCC->PLLCFGR = 0x22041000U;

#if defined(STM32WB55xx) || defined(STM32WB5Mxx)

  RCC->PLLSAI1CFGR = 0x22041000U;
#endif


  RCC->CR &= 0xFFFBFFFFU;


  RCC->CIER = 0x00000000;
}

/**
 * @brief   TODO: describe what SystemCoreClockUpdate() does
 */
void SystemCoreClockUpdate(void)
{
  uint32_t tmp, msirange, pllvco, pllr, pllsource, pllm;




  msirange = MSIRangeTable[(RCC->CR & RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos];


  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
    case 0x00:
      SystemCoreClock = msirange;
      break;

    case 0x04:

      SystemCoreClock = HSI_VALUE;
      break;

    case 0x08:
      SystemCoreClock = HSE_VALUE;
      break;

    case 0x0C:

      pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC);
      pllm = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos) + 1UL ;

      if (pllsource == 0x02UL)
      {
        pllvco = (HSI_VALUE / pllm);
      }
      else if (pllsource == 0x03UL)
      {
        pllvco = (HSE_VALUE / pllm);
      }
      else
      {
        pllvco = (msirange / pllm);
      }

      pllvco = pllvco * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
      pllr = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos) + 1UL);

      SystemCoreClock = pllvco / pllr;
      break;

    default:
      SystemCoreClock = msirange;
      break;
  }



  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];


  SystemCoreClock = SystemCoreClock / tmp;
}

