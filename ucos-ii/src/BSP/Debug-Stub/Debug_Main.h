/*
 * Debug_Main.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */
/*
*********************************************************************************************************
*
*                                      BOARD SUPPORT PACKAGE (BSP)
*
*                        Xilinx Zynq-7000 ZC702 Cortex-A9 MPCore DEVELOPMENT KIT
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP present pre-processor macro definition.
*********************************************************************************************************/

#ifndef DEBUG_MAIN_H_
#define DEBUG_MAIN_H_

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include "Debug_RSP.h"
#include "Debug_HAL.h"
/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

# ifdef Debug_Main_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif
/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
EXTERN void Debug_Main_Init();
EXTERN CPU_INT08U Debug_Main_Report_Halt_Reason(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Read_Register(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Write_Register(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Read_Registers(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Write_Registers(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Read_memory(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Write_memory(Debug_TID_t ThreadID,void *CommandParam);

EXTERN CPU_INT08U Debug_Main_Step_machine_instruction(Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_Resume( Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Dbug_Main_InsertBreakPoint( Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT08U Debug_Main_RemoveBreakPoint( Debug_TID_t ThreadID,void *CommandParam);
EXTERN CPU_INT32U Activate_Sw_BreakPoints();
/*
*********************************************************************************************************
*                                          CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/
#undef EXTERN

#endif /* DEBUG_MAIN_H_ */
