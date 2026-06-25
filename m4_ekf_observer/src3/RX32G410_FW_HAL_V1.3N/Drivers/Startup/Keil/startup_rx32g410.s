;/**************************************************************************//**
; * @file     startup_rx32g410.s
; * @brief    CMSIS Core Device Startup File for
; *           rx32g4xx Device
; * @version  V1.0.1
; * @date     23. July 2019
; ******************************************************************************/
;/*
; * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


;<h> Stack Configuration
;  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>

Stack_Size      EQU      0x00000400

                AREA     STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE    Stack_Size
__initial_sp


;<h> Heap Configuration
;  <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>

Heap_Size      EQU     0x200

                AREA     HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE    Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA     RESET, DATA, READONLY
                EXPORT   __Vectors
                EXPORT   __Vectors_End
                EXPORT   __Vectors_Size

__Vectors       DCD      __initial_sp                   ;     Top of Stack
                DCD      Reset_Handler                  ;     Reset Handler
                DCD      NMI_Handler                    ; -14 NMI Handler
                DCD      HardFault_Handler              ; -13 Hard Fault Handler
                DCD      MemManage_Handler              ; -12 MPU Fault Handler
                DCD      BusFault_Handler               ; -11 Bus Fault Handler
                DCD      UsageFault_Handler             ; -10 Usage Fault Handler
                DCD      0                              ;  -9 Reserved
                DCD      0                              ;  -8 Reserved
                DCD      0                              ;  -7 Reserved
                DCD      0                              ;  -6 Reserved
                DCD      SVC_Handler                    ;  -5 SVC Handler
                DCD      DebugMon_Handler               ;  -4 Debug Monitor Handler
                DCD      0                              ;  -3 Reserved
                DCD      PendSV_Handler                 ;  -2 PendSV Handler
                DCD      SysTick_Handler                ;  -1 SysTick Handler

                ; Interrupts
                DCD     WWDG_IRQHandler                 ;   0 Window Watchdog
                DCD     PVD_IRQHandler                  ;   1 PVD through EXTI Line detect
                DCD     TAMPER_IRQHandler               ;   2 Tamper
                DCD     RTC_IRQHandler                  ;   3 RTC
                DCD     FLASH_IRQHandler                ;   4 Flash
                DCD     RCC_IRQHandler                  ;   5 RCC
                DCD     EXTI0_IRQHandler                ;   6 EXTI Line 0
                DCD     EXTI1_IRQHandler                ;   7 EXTI Line 1
                DCD     EXTI2_IRQHandler                ;   8 EXTI Line 2
                DCD     EXTI3_IRQHandler                ;   9 EXTI Line 3
                DCD     EXTI4_IRQHandler                ;  10 EXTI Line 4
                DCD     DMA1_Channel1_IRQHandler        ;  11 DMA1 Channel 1
                DCD     DMA1_Channel2_IRQHandler        ;  12 DMA1 Channel 2
                DCD     DMA1_Channel3_IRQHandler        ;  13 DMA1 Channel 3
                DCD     DMA1_Channel4_IRQHandler        ;  14 DMA1 Channel 4
                DCD     DMA1_Channel5_IRQHandler        ;  15 DMA1 Channel 5
                DCD     DMA1_Channel6_IRQHandler        ;  16 DMA1 Channel 6
                DCD     DMA1_Channel7_IRQHandler        ;  17 DMA1 Channel 7
                DCD     ADC1_2_IRQHandler               ;  18 ADC1_2
                DCD     USB_HP_CAN1_TX_IRQHandler       ;  19 CAN1 TX
                DCD     USB_LP_CAN1_RX0_IRQHandler      ;  20 CAN1 RX0
                DCD     CAN1_RX1_IRQHandler             ;  21 CAN1 RX1
                DCD     CAN1_SCE_IRQHandler             ;  22 CAN1 SCE
                DCD     EXTI9_5_IRQHandler              ;  23 EXTI Line 9..5
                DCD     TIM1_BRK_TIM15_IRQHandler       ;  24 TIM1 Break, Transition error, Index error and TIM15
                DCD     TIM1_UP_TIM16_IRQHandler        ;  25 TIM1 Update and TIM16
                DCD     TIM1_TRG_COM_TIM17_IRQHandler   ;  26 TIM1 Trigger, Commutation, Direction change, Index and TIM17
                DCD     TIM1_CC_IRQHandler              ;  27 TIM1 Capture Compare
                DCD     TIM2_IRQHandler                 ;  28 TIM2
                DCD     TIM3_IRQHandler                 ;  29 TIM3
                DCD     TIM4_IRQHandler                 ;  30 TIM4
                DCD     I2C1_EV_IRQHandler              ;  31 I2C1 Event
                DCD     I2C1_ER_IRQHandler              ;  32 I2C1 Error
                DCD     I2C2_EV_IRQHandler              ;  33 I2C2 Event
                DCD     I2C2_ER_IRQHandler              ;  34 I2C2 Error
                DCD     SPI1_IRQHandler                 ;  35 SPI1
                DCD     SPI2_IRQHandler                 ;  36 SPI2
                DCD     UART1_IRQHandler                ;  37 UART1
                DCD     UART2_IRQHandler                ;  38 UART2
                DCD     UART3_IRQHandler                ;  39 UART3
                DCD     EXTI15_10_IRQHandler            ;  40 EXTI Line 15..10
                DCD     RTC_Alarm_IRQHandler            ;  41 RTC Alarm through EXTI Line
                DCD     USBWakeUp_IRQHandler            ;  42 Reserved
                DCD     TIM8_BRK_IRQHandler             ;  43 TIM8 Break
                DCD     TIM8_UP_IRQHandler              ;  44 TIM8 Update
                DCD     TIM8_TRG_COM_IRQHandler         ;  45 TIM8 Trigger and Commutation
                DCD     TIM8_CC_IRQHandler              ;  46 TIM8 Capture Compare
                DCD     ADC3_IRQHandler                 ;  47 ADC3
                DCD     0                               ;  48 Reserved
                DCD     0                               ;  49 Reserved
                DCD     TIM5_IRQHandler                 ;  50 TIM5
                DCD     0                               ;  51 Reserved
                DCD     UART4_IRQHandler                ;  52 UART4
                DCD     UART5_IRQHandler                ;  53 UART5
                DCD     TIM6_DAC_IRQHandler             ;  54 TIM6 and DAC1/3
                DCD     TIM7_DAC_IRQHandler             ;  55 TIM7 and DAC2
                DCD     DMA2_Channel1_IRQHandler        ;  56 DMA2 Channel 1
                DCD     DMA2_Channel2_IRQHandler        ;  57 DMA2 Channel 2
                DCD     DMA2_Channel3_IRQHandler        ;  58 DMA2 Channel 3
                DCD     DMA2_Channel4_IRQHandler        ;  59 DMA2 Channel 4
                DCD     DMA2_Channel5_IRQHandler        ;  60 DMA2 Channel 5
                DCD     0                               ;  61 Reserved
                DCD     0                               ;  62 Reserved
                DCD     0                               ;  63 Reserved
                DCD     COMP1_2_3_IRQHandler            ;  64 COMP1_2_3
                DCD     COMP4_IRQHandler                ;  65 COMP4
                DCD     0                               ;  66 Reserved
                DCD     HRTIM1_Master_IRQHandler        ;  67 HRTIM1 master timer
                DCD     HRTIM1_TIMA_IRQHandler          ;  68 HRTIM1 timer A
                DCD     HRTIM1_TIMB_IRQHandler          ;  69 HRTIM1 timer B
                DCD     HRTIM1_TIMC_IRQHandler          ;  70 HRTIM1 timer C
                DCD     HRTIM1_TIMD_IRQHandler          ;  71 HRTIM1 timer D
                DCD     HRTIM1_TIME_IRQHandler          ;  72 HRTIM1 timer E
                DCD     HRTIM1_FLT_IRQHandler           ;  73 HRTIM1 fault
                DCD     HRTIM1_TIMF_IRQHandler          ;  74 HRTIM1 timer F
                DCD     0                               ;  75 Reserved
                DCD     0                               ;  76 Reserved
                DCD     0                               ;  77 Reserved
                DCD     0                               ;  78 Reserved
                DCD     0                               ;  79 Reserved
                DCD     0                               ;  80 Reserved
                DCD     0                               ;  81 Reserved
                DCD     0                               ;  82 Reserved
                DCD     0                               ;  83 Reserved
                DCD     0                               ;  84 Reserved
                DCD     0                               ;  85 Reserved
                DCD     CAN2_TX_IRQHandler              ;  86 CAN2 TX
                DCD     CAN2_RX0_IRQHandler             ;  87 CAN2 RX0
                DCD     CAN2_RX1_IRQHandler             ;  88 CAN2 RX1
                DCD     CAN2_SCE_IRQHandler             ;  89 CAN2 SCE
                DCD     CLC1_IRQHandler                 ;  90 CLC1
                DCD     CLC2_IRQHandler                 ;  91 CLC2
                DCD     CLC3_IRQHandler                 ;  92 CLC3
                DCD     CLC4_IRQHandler                 ;  93 CLC4
                DCD     DMAMUX_OVR_IRQHandler           ;  94 DMAMUX Overrun
                DCD     0                               ;  95 Reserved
                DCD     DMA1_Channel8_IRQHandler        ;  96 DMA1 channel 8
                DCD     DMA2_Channel6_IRQHandler        ;  97 DMA2 channel 6
                DCD     DMA2_Channel7_IRQHandler        ;  98 DMA2 channel 7
                DCD     DMA2_Channel8_IRQHandler        ;  99 DMA2 channel 8
                DCD     CORDIC_IRQHandler               ; 100 CORDIC
                DCD     FMAC_IRQHandler                 ; 101 FMAC

__Vectors_End
__Vectors_Size  EQU      __Vectors_End - __Vectors


                AREA     |.text|, CODE, READONLY

; Reset handler
Reset_Handler   PROC
                EXPORT   Reset_Handler             [WEAK]
                IMPORT   SystemInit
                IMPORT   __main

                LDR      R0, =SystemInit
                BLX      R0
                LDR      R0, =__main
                BX       R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler          [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler           [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler           [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler             [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler            [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WWDG_IRQHandler            [WEAK]
                EXPORT  PVD_IRQHandler             [WEAK]
                EXPORT  TAMPER_IRQHandler          [WEAK]
                EXPORT  RTC_IRQHandler             [WEAK]
                EXPORT  FLASH_IRQHandler           [WEAK]
                EXPORT  RCC_IRQHandler             [WEAK]
                EXPORT  EXTI0_IRQHandler           [WEAK]
                EXPORT  EXTI1_IRQHandler           [WEAK]
                EXPORT  EXTI2_IRQHandler           [WEAK]
                EXPORT  EXTI3_IRQHandler           [WEAK]
                EXPORT  EXTI4_IRQHandler           [WEAK]
                EXPORT  DMA1_Channel1_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel2_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel3_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel4_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel5_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel6_IRQHandler   [WEAK]
                EXPORT  DMA1_Channel7_IRQHandler   [WEAK]
                EXPORT  ADC1_2_IRQHandler          [WEAK]
                EXPORT  USB_HP_CAN1_TX_IRQHandler  [WEAK]
                EXPORT  USB_LP_CAN1_RX0_IRQHandler [WEAK]
                EXPORT  CAN1_RX1_IRQHandler        [WEAK]
                EXPORT  CAN1_SCE_IRQHandler        [WEAK]
                EXPORT  EXTI9_5_IRQHandler         [WEAK]
                EXPORT  TIM1_BRK_TIM15_IRQHandler  [WEAK]
                EXPORT  TIM1_UP_TIM16_IRQHandler   [WEAK]
                EXPORT  TIM1_TRG_COM_TIM17_IRQHandler [WEAK]
                EXPORT  TIM1_CC_IRQHandler         [WEAK]
                EXPORT  TIM2_IRQHandler            [WEAK]
                EXPORT  TIM3_IRQHandler            [WEAK]
                EXPORT  TIM4_IRQHandler            [WEAK]
                EXPORT  I2C1_EV_IRQHandler         [WEAK]
                EXPORT  I2C1_ER_IRQHandler         [WEAK]
                EXPORT  I2C2_EV_IRQHandler         [WEAK]
                EXPORT  I2C2_ER_IRQHandler         [WEAK]
                EXPORT  SPI1_IRQHandler            [WEAK]
                EXPORT  SPI2_IRQHandler            [WEAK]
                EXPORT  UART1_IRQHandler           [WEAK]
                EXPORT  UART2_IRQHandler           [WEAK]
                EXPORT  UART3_IRQHandler           [WEAK]
                EXPORT  EXTI15_10_IRQHandler       [WEAK]
                EXPORT  RTC_Alarm_IRQHandler       [WEAK]
                EXPORT  USBWakeUp_IRQHandler       [WEAK]	
                EXPORT  TIM8_BRK_IRQHandler        [WEAK]
                EXPORT  TIM8_UP_IRQHandler         [WEAK]
                EXPORT  TIM8_TRG_COM_IRQHandler    [WEAK]
                EXPORT  TIM8_CC_IRQHandler         [WEAK]
                EXPORT  ADC3_IRQHandler            [WEAK]
                EXPORT  TIM5_IRQHandler            [WEAK]
                EXPORT  UART4_IRQHandler           [WEAK]	
                EXPORT  UART5_IRQHandler           [WEAK]
                EXPORT  TIM6_DAC_IRQHandler       [WEAK]
                EXPORT  TIM7_DAC_IRQHandler       [WEAK]
                EXPORT  DMA2_Channel1_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel2_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel3_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel4_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel5_IRQHandler   [WEAK]
                EXPORT  COMP1_2_3_IRQHandler       [WEAK]
                EXPORT  COMP4_IRQHandler           [WEAK]
                EXPORT  HRTIM1_Master_IRQHandler   [WEAK]	
                EXPORT  HRTIM1_TIMA_IRQHandler     [WEAK]
                EXPORT  HRTIM1_TIMB_IRQHandler     [WEAK]
                EXPORT  HRTIM1_TIMC_IRQHandler     [WEAK]
                EXPORT  HRTIM1_TIMD_IRQHandler     [WEAK]
                EXPORT  HRTIM1_TIME_IRQHandler     [WEAK]
                EXPORT  HRTIM1_FLT_IRQHandler      [WEAK]
                EXPORT  HRTIM1_TIMF_IRQHandler     [WEAK]
                EXPORT  CAN2_TX_IRQHandler         [WEAK]
                EXPORT  CAN2_RX0_IRQHandler        [WEAK]
                EXPORT  CAN2_RX1_IRQHandler        [WEAK]
                EXPORT  CAN2_SCE_IRQHandler        [WEAK]
                EXPORT  CLC1_IRQHandler            [WEAK]
                EXPORT  CLC2_IRQHandler            [WEAK]
                EXPORT  CLC3_IRQHandler            [WEAK]
                EXPORT  CLC4_IRQHandler            [WEAK]					
                EXPORT  DMAMUX_OVR_IRQHandler      [WEAK]
                EXPORT  DMA1_Channel8_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel6_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel7_IRQHandler   [WEAK]
                EXPORT  DMA2_Channel8_IRQHandler   [WEAK]
                EXPORT  CORDIC_IRQHandler          [WEAK]
                EXPORT  FMAC_IRQHandler            [WEAK]

WWDG_IRQHandler
PVD_IRQHandler
TAMPER_IRQHandler
RTC_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_IRQHandler
DMA1_Channel3_IRQHandler
DMA1_Channel4_IRQHandler
DMA1_Channel5_IRQHandler
DMA1_Channel6_IRQHandler
DMA1_Channel7_IRQHandler
ADC1_2_IRQHandler
USB_HP_CAN1_TX_IRQHandler
USB_LP_CAN1_RX0_IRQHandler
CAN1_RX1_IRQHandler
CAN1_SCE_IRQHandler
EXTI9_5_IRQHandler
TIM1_BRK_TIM15_IRQHandler
TIM1_UP_TIM16_IRQHandler
TIM1_TRG_COM_TIM17_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
I2C2_EV_IRQHandler
I2C2_ER_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
UART3_IRQHandler
EXTI15_10_IRQHandler
RTC_Alarm_IRQHandler
USBWakeUp_IRQHandler
TIM8_BRK_IRQHandler
TIM8_UP_IRQHandler
TIM8_TRG_COM_IRQHandler
TIM8_CC_IRQHandler
ADC3_IRQHandler
TIM5_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
TIM6_DAC_IRQHandler
TIM7_DAC_IRQHandler
DMA2_Channel1_IRQHandler
DMA2_Channel2_IRQHandler
DMA2_Channel3_IRQHandler
DMA2_Channel4_IRQHandler
DMA2_Channel5_IRQHandler
COMP1_2_3_IRQHandler
COMP4_IRQHandler
HRTIM1_Master_IRQHandler
HRTIM1_TIMA_IRQHandler
HRTIM1_TIMB_IRQHandler
HRTIM1_TIMC_IRQHandler
HRTIM1_TIMD_IRQHandler
HRTIM1_TIME_IRQHandler
HRTIM1_FLT_IRQHandler
HRTIM1_TIMF_IRQHandler
CAN2_TX_IRQHandler
CAN2_RX0_IRQHandler
CAN2_RX1_IRQHandler
CAN2_SCE_IRQHandler
CLC1_IRQHandler
CLC2_IRQHandler
CLC3_IRQHandler
CLC4_IRQHandler
DMAMUX_OVR_IRQHandler
DMA1_Channel8_IRQHandler
DMA2_Channel6_IRQHandler
DMA2_Channel7_IRQHandler
DMA2_Channel8_IRQHandler
CORDIC_IRQHandler
FMAC_IRQHandler


                B       .

                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                IF      :DEF:__MICROLIB           
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                
                ELSE
                
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
                 
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF

                END