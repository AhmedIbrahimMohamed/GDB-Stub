/*
 * Debug_IO.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */
/*
*********************************************************************************************************
*
*                                     Debug Input/Output Module
*
*                        Xilinx Zynq-7000 ZC702 Cortex-A9 MPCore DEVELOPMENT KIT
*
* Filename      : Debug_IO.h
* Version       : V1.00
* Programmer(s) :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               DEBUG_IO present pre-processor macro definition.
*********************************************************************************************************/


#ifndef DEBUG_IO_H_
#define DEBUG_IO_H_




/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include "xuartps.h"
#include "cpu.h"
/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef Debug_IO_IMPORT
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
/*This is the default Number of bytes that  RSP module needs from IO module when receive interrupt occurs*/
EXTERN CPU_INT08U Debug_IO_DefaultNumBytesRxedINT;

typedef enum {
	                        Debug_IO_UART ,
	                        Debug_IO_TCPIP
}Debug_IO_port_types;

typedef   void (*Debug_IO_RSPHandler)(void *) ;


typedef enum {
         FlushRxPath,
         FlushTxPath,
         FlushBothPaths

}FlushWhat_t;
/*
 *
 * Debug_IO_Port : this data structure holds information about active debug port (UART/TCPIP/etc) that abstracts the details of low-level medium port
 * Debug_IO_Port is used by high-level module Debug_RSP to manipulate its protocol packets
 * information included :
 * 1- call-back functions for primary Port functions (Initialize/Read/Write/Flush)
 * these functions in-turn calls the low-level drivers routines
 * 2-port-type is the type of port according to used medium. Could be UART, TCPIP port, Bluetooth channel,etc
 *   enumarated values are in Debug_IO_port_types
 * 3-state : port is ready or not. based on  state variable in low-level driver.
 *
 * 4- port ID : ID of the current port as given by low-level driver based on #instances
 *5- Debug_RSPCallback : Abstract/High-level handler of Debug-port-events
 * Debug_IO_PortHandler() calls this callback function
 *5-
 *5-
 * */
 typedef struct _Debug_IO_Port
{
	 /*Synchronous Transmission and Receiving*/
	CPU_INT08U (*Debug_Read_char)();
	void (*Debug_Write_char)(CPU_INT08U c);
	void  (*Init_Port)(void);
	CPU_INT08U (*Debug_Read_Buffer)(CPU_INT08U* buf,CPU_INT08U count);
	CPU_INT08U (*Debug_Read_TillChar)(CPU_INT08U* buf , CPU_INT08U StopChar);
	CPU_INT08U (*Debug_Write_Buffer)(CPU_INT08U* buf,CPU_INT08U count);
	CPU_INT08U (*Debug_Read_IgnoreTillChar)(CPU_INT08U StopChar);
	CPU_INT08U (*Debug_Flush)(enum FlushWhat_t FlushWhat);

	Debug_IO_port_types Port_type;
	CPU_INT08U State;
	CPU_INT08U Port_id;
	Debug_IO_RSPHandler Debug_RSPCallback ;
}Debug_IO_Port;


EXTERN Debug_IO_Port Debug_Port;

/*EXTERN Debug_IO_arcHandler Debug_IO_arc_Port_Handler;*/

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
EXTERN void Debug_IO_init(Debug_IO_RSPHandler RSPPort_Handler, CPU_INT08U INT_RxCount, CPU_INT08U *RSPRxBufferPtr,int RSPRXBufferSize);
EXTERN void Debug_IO_Port_RxBuffer_Attrs_Init(CPU_INT32U RequestedBytes ,CPU_INT08U *BufPtr );
EXTERN CPU_INT32U Debug_IO_Port_InterruptDisable();
EXTERN void Debug_IO_Port_InterruptRestore(CPU_INT32U IntState);

/*For testing only*/
EXTERN  void Debug_UART_Test(void);
EXTERN unsigned int  Rec_char( CPU_INT08U *Ptr,int NReqchar,CPU_INT08U IsIntChar );

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
#undef Debug_IO_IMPORT
#undef EXTERN

#endif /* DEBUG_IO_H_ */
