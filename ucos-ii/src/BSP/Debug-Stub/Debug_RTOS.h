/*
 * Debug_RTOS.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */

#ifndef DEBUG_RTOS_H_
#define DEBUG_RTOS_H_




/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include "Debug_Types.h"
#include "ucos_ii.h"

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/
#ifdef Debug_RTOS_IMPORT
	#define EXTERN
#else
	#define EXTERN extern
#endif

/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/

#define Debug_RTOS_THREADS_MAX		(2+20)		/*(OS_N_SYS_TASKS + OS_MAX_TASKS */

#define Debug_RTOS_TCB_SPOffset     (0)        /*offset of SP (task/thread stack pointer) member in RTOS Task Control Block
                                                 offset is in word size*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

typedef OS_TCB Debug_TCB;	     				/*Debug Thread Control Block TCB is the same type as underlying RTOS Task Contol Block TCB */

EXTERN Debug_MemWidth *Debug_RTOS_ThreadListPtr;     /*pointer to TCB list Table , accessed by Thread-ID/priority*/


EXTERN Debug_TID_t current_thread_OF_Focus  ;    /* ID of current debugged thread,i.e. thread priority*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

EXTERN void Debug_RTOS_Init(void);
EXTERN Debug_TID_t Debug_RTOS_GET_HPT(void);
EXTERN Debug_Block_Message_t Debug_RTOS_StubPend(void);
EXTERN void Debug_RTOS_StubPost(void *PostMessage);
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


#undef Debug_RTOS_IMPORT
#undef EXTERN

#endif /* DEBUG_RTOS_H_ */
