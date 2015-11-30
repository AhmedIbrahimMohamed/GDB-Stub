/*
 * Debug_IO.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */


/*
 * template.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */


/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          Debug IO Module
*
*                                         Xilinx Zynq XC7Z020
*                                               on the
*
*                                             Xilinx ZC702
*                                          Evaluation Board
*
* Filename      : Debug_IO.c
* Version       : V1.00
* Programmer(s) :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define Debug_IO_IMPORT
#include "Debug_IO.h"
#include <csp.h>
#include "platform.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************

*/
/*For Test only SW TxFIFO/RxFIFO used only by debug_IO Module for testing UART Rx,Tx functionality*/
static CPU_INT08U Debug_IO_TxFIFO[10] = {'H','E','L','O','W','O','R','L','D'};
static CPU_INT08U Debug_IO_RxFIFO[10];

static CPU_INT08U *Debug_IO_RSPBufferPtr ;
/*Instanse of XUARTPs used for initializing xilinx UART and Rx/Tx functions*/
static XUartPs         Uart_Ps_0;
/*instanse of Debug_IO_port data structure  as declared in Debug_IO.h*/



/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static CPU_INT08U Debug_UART_Read_char();
static void Debug_UART_Write_char(CPU_INT08U c);
static void  Init_UART_Port(void);
static CPU_INT08U Debug_UART_Read_Buffer (CPU_INT08U* buf,CPU_INT08U count);
static CPU_INT08U Debug_UART_Write_Buffer(CPU_INT08U* buf,CPU_INT08U count);
static CPU_INT08U Debug_UART_Read_TillChar(CPU_INT08U* buf , CPU_INT08U StopChar);
static CPU_INT08U Debug_UART_Read_IgnoreTillChar(CPU_INT08U);

/*only for testing UART */
static void debug_uart_handlerTest(void *);
//static void Debug_UART_Test(void);
/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/
/*********************************************************************************************************
*                                               Debug_UART_Read_char()
*
* Description : A blocking reading of one character from UART
*
* Argument(s) : pointer to character to be read
*
* Return(s)   : Number of characters read
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Read_char function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for reading characters
*
*               (2) blocking reading is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Read_char() in Debug_IO_Port structure
*
*               (4) driver in this Debug_IO module means Xilinx xuartps driver, it should be adapted -according to target platform- to other driver
*********************************************************************************************************
*/

static CPU_INT08U Debug_UART_Read_char()
{
	CPU_INT32U ImrRegister = XUartPs_ReadReg(Uart_Ps_0.Config.BaseAddress,
		 				  XUARTPS_IMR_OFFSET);
			CPU_INT08U RxChar;
	while((XUartPs_Recv(&Uart_Ps_0,&RxChar,1)) == 0);

	return RxChar;
}
/*********************************************************************************************************
*                                               Debug_UART_Write_char()
*
* Description :A blocking writing of one character to UART
*
* Argument(s) : character to be sent
*
* Return(s)   : none.
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Write_char function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for writing characters
*
*               (2) blocking writing is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Write_char() in Debug_IO_Port structure
*
*
*********************************************************************************************************
*/

static void Debug_UART_Write_char(CPU_INT08U c)
		{
			while(XUartPs_Send(&Uart_Ps_0,&c,1) == 0);
		}
/*********************************************************************************************************
*                                               Debug_UART_Read_Buffer()
*
* Description : blocking reading of buffer from UART
*
* Argument(s) : buf : pointer to buffer character to read in
*               count: number of bytes to be read
*
* Return(s)   : Number of character read
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Read_Buffer function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for reading characters
*
*               (2) blocking reading is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Read_Buffer() in Debug_IO_Port structure
*
*********************************************************************************************************
*/
static CPU_INT08U Debug_UART_Read_Buffer (CPU_INT08U* buf,CPU_INT08U count)
{
	CPU_INT08U RxCharCount = 0;
	RxCharCount = XUartPs_Recv( &Uart_Ps_0, buf, count );
	while( RxCharCount < count)
	{
			/*recalling Receive function with updated buf pointer with already received #bytes */

		RxCharCount += XUartPs_Recv( &Uart_Ps_0, (buf + RxCharCount), (count - RxCharCount) );

	}

    return RxCharCount;
}
/*********************************************************************************************************
*                                               Debug_UART_Write_Buffer()
*
* Description :blocking reading of buffer from UART
*
* Argument(s) : buf : pointer to buffer character to sent from
*               count: number of bytes to be sent
*
* Return(s)   : Number of character sent
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Write_Buffer function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for writing characters
*
*               (2) blocking writing is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Write_Buffer() in Debug_IO_Port structure
**********************************************************************************************************
*/

static CPU_INT08U Debug_UART_Write_Buffer(CPU_INT08U* buf,CPU_INT08U count)
{
	CPU_INT08U TxCharCount = 0;

        TxCharCount = XUartPs_Send( &Uart_Ps_0,buf, count );
		while( TxCharCount < count)
		{
			TxCharCount +=  XUartPs_Send( &Uart_Ps_0,buf+TxCharCount, (count-TxCharCount) );
		}

    return TxCharCount;
}

/*********************************************************************************************************
*                                               Debug_UART_Read_TillChar()
*
* Description : blocking reading of buffer from UART till reach a specified character
*
* Argument(s) : buf : pointer to buffer character to read in
*               StopChar: character upon its appearance , stop receive
*
* Return(s)   : Number of character read including StopChar
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Read_TillChar function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for reading characters
*
*               (2) blocking reading is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Read_TillChar() in Debug_IO_Port structure
*
*********************************************************************************************************
*/

static CPU_INT08U Debug_UART_Read_TillChar(CPU_INT08U* buf , CPU_INT08U StopChar)
		{
	CPU_INT08U RxCharCount = 0;
	RxCharCount = XUartPs_Recv( &Uart_Ps_0, buf, 1 );
		while( *(buf+(RxCharCount-1)) != StopChar)
		{
				/*recalling Receive function with updated buf pointer with already received #bytes */

			RxCharCount += XUartPs_Recv( &Uart_Ps_0, (buf + RxCharCount), 1 );

		}

	    return RxCharCount;


		}
/*********************************************************************************************************
*                                               Debug_UART_Read_IgnoreTillChar()
*
* Description : blocking reading of characters from UART till reach a specified character
*
* Argument(s) : StopChar: character upon its appearance , stop receive
*
* Return(s)   : Number of character have been skipped/ignored
*
* Caller(s)   : Debug_RSP module routines via Debug_Port.Debug_Read_IgnoreTillChar function pointer
*
* Note(s)     : (1) this Function is just based on non-blocking driver function for reading characters
*
*               (2) blocking reading is needed for Debug stub as it is by nature waiting for further debugger requests and has nothing else to do.
*
*               (3) Registerd as callback for Debug_Read_IgnoreTillChar() in Debug_IO_Port structure
*
*               (4)this function is like Debug_UART_Read_TillChar()except that it ignores the characters before StopChar
*
*********************************************************************************************************
*/

 CPU_INT08U Debug_UART_Read_IgnoreTillChar(CPU_INT08U StopChar)
{



	 CPU_INT08U  charsink;
	 CPU_INT08U RxSkippedCharCount = 0;
    RxSkippedCharCount = XUartPs_Recv( &Uart_Ps_0, &charsink, 1 );
    while(charsink != StopChar)
		  {
			  RxSkippedCharCount += XUartPs_Recv( &Uart_Ps_0, &charsink, 1 );
		  }


   return RxSkippedCharCount;

}
/*********************************************************************************************************
 *                                               Init_UART_Port()
 *
 * Description :this should be part of Debug_IO_init() . specified in only initializing the IO_port ,but  currently its code is merged in Debug_IO_init()
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : if defined/implemented , Debug_IO_init() should call it.
 *
 * Note(s)     : (1)it is based on driver initialization steps  mentioned in driver documentation
 *
 *               (2)
 *********************************************************************************************************
 */

static void  Init_UART_Port(void)
{

}

/*
*********************************************************************************************************
*                                               Debug_IO_PortHandler()
*
* Description : Work as application UART HAndler from Derive point of view. Works as low-level UART-Handler from RSP-Module Point of view
*               After handling UART, it calls RSP-Handler for high-level processing of incoming/outcoming packets
*
* Argument(s) : CallBackRef : Callback reference for event handler
*               Event       : what UART Event caused this interrupt to fire, detailed in xuartps.h
*               EventData   : information provided to handler according to Event Value.
* Return(s)   : none.
*
* Caller(s)   : .
*
* Note(s)     : (1)this handler takes the same parameters as defined by low-level driver function for registering UART-Handlers
*
*               (2)
*********************************************************************************************************
*/


static void   Debug_IO_PortHandler(void *CallBackRef, CPU_INT32U Event,
		CPU_INT32U EventData)
{

	CPU_INT08U * RxCount;
switch(Event)
 {

			case XUARTPS_EVENT_RECV_DATA :
				/*Here EventData is number of bytes received*/

				//RxCount = &EventData;
				//print("In Debug_IO Handler\n");
				//print("Handling Receive event  Event\n");
				//print("Receive count of : ");
				//print(RxCount);
				/*Reset Requested and remaining bytes*/
				Uart_Ps_0.ReceiveBuffer.RequestedBytes = Uart_Ps_0.ReceiveBuffer.RemainingBytes = Debug_IO_DefaultNumBytesRxedINT;
				/*TODO ::
				 * should be updated to point to  Uart_Ps_0.ReceiveBuffer.NextBytePtr = Debug_IO_RSPBufferPtr->nextptr*/
				Uart_Ps_0.ReceiveBuffer.NextBytePtr = &Debug_IO_RSPBufferPtr[0];
			  /*after handling driver-related interrupt , call RSP-related Handler for further high-level handling of packet*/

				Debug_Port.Debug_RSPCallback((void *) 0);



			break;

			default:
				//print("In Debug_IO HAndler\n");
				//print("Handling unknown Event\n");

			break;
  }

}





/*
*********************************************************************************************************
*                                               Debug_IO_init()
*
* Description : this function initialize the Low-level Debu_IO module.
*               It initializes driver port to be ready for Tx, Rx and interrupt processing
*               It initializes the Debug_Port instance for callback functions and basic attributes to be used by high-level module RSP
*
* Argument(s) : RSPPort_Handler : Handler routine implemented in Debug_RSP module to be called from IO-handler routine(i.e. Debug_IO_PortHandler()) for further packet buffer handling/proocessing.
*               INT_RxCount     : The default Number of bytes that  RSP module needs from IO module when receive interrupt occurs.
*               RSPRxBufferPtr  : Apointer to RSP module's Receive buffer,
*                                 used for interrupt handling store received characters directly in RSP RxBuffer
* Return(s)   : none.
*
* Caller(s)   : Debug_RSP_Init() function in Debug_RSP module
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/

void Debug_IO_init(Debug_IO_RSPHandler RSPPort_Handler, CPU_INT08U INT_RxCount, CPU_INT08U *RSPRxBufferPtr)
{
							/*********** Initialize port **************/

				/*** In this case , we need to initialize Xilinx xuartps module ****/

	   XUartPs_Config *Config_0;

	    /*XUartPs_Handler APPUartHandler = BSP_UART_Handler;*/

	        /*
	    	 * Initialize the UART driver so that it's ready to use
	    	 * Look up the configuration in the config table, then initialize it.
	    	 */
	   Config_0 = XUartPs_LookupConfig(UART_DEVICE_ID);
	   XUartPs_CfgInitialize(&Uart_Ps_0, Config_0, Config_0->BaseAddress);
	   XUartPs_SetBaudRate(&Uart_Ps_0, 115200);


	   	   	   	   /*************Port Interrupt-Related Configuration***********/
	    /*
	     * 1- register  Xilinx-uart-common-handler to CSP vector table
	     * */

	   CSP_IntVectReg((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
	                   (CSP_DEV_NBR    )CSP_INT_SRC_NBR_UART_01,
	                   (CPU_FNCT_PTR   )XUartPs_InterruptHandler,
                        &Uart_Ps_0);

	   Debug_IO_DefaultNumBytesRxedINT =   INT_RxCount;
	   Debug_IO_RSPBufferPtr            =   RSPRxBufferPtr;
		/*
		 * 2- configure the Receive Buffer attributes (set number of requested bytes,remaining bytes , NextBytePtr)
		 * */

	   Debug_IO_Port_RxBuffer_Attrs_Init(Debug_IO_DefaultNumBytesRxedINT,Debug_IO_RSPBufferPtr);
	  // Uart_Ps_0.ReceiveBuffer.RequestedBytes = Uart_Ps_0.ReceiveBuffer.RemainingBytes = Debug_IO_DefaultNumBytesRxedINT;
	   	   /*TODO :::
	     * should be updated to point to  Uart_Ps_0.ReceiveBuffer.NextBytePtr = Debug_IO_RSPBufferPtr->nextptr*/
	  // Uart_Ps_0.ReceiveBuffer.NextBytePtr = &Debug_IO_RSPBufferPtr[0];

	    /*
	     * 3- register the Debug-related Handler for Port interrupt to be called from Global Xilinx Handler
	     * */

	   XUartPs_SetHandler(&Uart_Ps_0, Debug_IO_PortHandler ,
	    	    		    				   (void *)0);

		/*
		 *  4- need explicitly to unify the Rx-Tigger-level of UART registers and the RSP-needed Rx Trigger Level
		 *  */
	   XUartPs_WriteReg(Config_0->BaseAddress,
	    	  	   	    			   XUARTPS_RXWM_OFFSET, Debug_IO_DefaultNumBytesRxedINT);
	    /*
	     * 5- Enable the individual UART interrupts masks so interrupts will occur,
	     * */

	   CPU_INT32U TestIntMask = XUARTPS_IXR_RXOVR;
	   XUartPs_SetInterruptMask(&Uart_Ps_0, TestIntMask);

		/*
		 * 6- Enable GLobal UART interrupt
		 * */
	   CSP_IntEn(CSP_INT_CTRL_NBR_MAIN,CSP_INT_SRC_NBR_UART_01);


		    /***** Initialize port callback functions and other attributes*********/

       Debug_Port.Debug_Read_char           = Debug_UART_Read_char;
       Debug_Port.Debug_Write_char          = Debug_UART_Write_char;
       Debug_Port.Debug_Read_Buffer         = Debug_UART_Read_Buffer;
       Debug_Port.Debug_Write_Buffer        = Debug_UART_Write_Buffer;
       Debug_Port.Debug_Read_TillChar       = Debug_UART_Read_TillChar;
       Debug_Port.Init_Port                 = Init_UART_Port;
       Debug_Port.Debug_RSPCallback         = RSPPort_Handler;
       Debug_Port.Port_type                 = Debug_IO_UART;
       Debug_Port.Debug_Read_IgnoreTillChar = Debug_UART_Read_IgnoreTillChar;


                             	    					/******TODO ::
                             	    					 * I Think we need to Flush UART-FIFOs here******/
/*need reading from TRM , https://lkml.org/lkml/2015/1/16/245 , http://www.ece.drexel.edu/courses/ECE-C490SBC/Notes/AudioCodec_base_Design/main.c*/


}

/*
*********************************************************************************************************
*                                               Debug_IO_Port_Buffer_Attrs_Init()
*
* Description : this function
*
* Argument(s) :
*
* Return(s)   : none.
*
* Caller(s)   :
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/
void Debug_IO_Port_RxBuffer_Attrs_Init(CPU_INT32U RequestedBytes ,CPU_INT08U *BufPtr )
{
	Uart_Ps_0.ReceiveBuffer.RequestedBytes = Uart_Ps_0.ReceiveBuffer.RemainingBytes = RequestedBytes;
	Uart_Ps_0.ReceiveBuffer.NextBytePtr = BufPtr;
}

/*
*********************************************************************************************************
*                                               Debug_IO_Port_InterruptDisable()
*
* Description : this function
*
* Argument(s) :
*
* Return(s)   : none.
*
* Caller(s)   :
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/
 CPU_INT32U Debug_IO_Port_InterruptDisable()
 {
	 /*
	 		 * Disable all the interrupts.
	 		 * This stops a previous operation that may be interrupt driven
	 		 */
	 		CPU_INT32U Port_IntState = XUartPs_ReadReg(Uart_Ps_0.Config.BaseAddress,
	 					  XUARTPS_IMR_OFFSET);
	 		XUartPs_WriteReg(Uart_Ps_0.Config.BaseAddress, XUARTPS_IDR_OFFSET,
	 			XUARTPS_IXR_MASK);
	 		return Port_IntState;
 }

 /*
 *********************************************************************************************************
 *                                               Debug_IO_Port_InterruptRestore()
 *
 * Description : this function
 *
 * Argument(s) :
 *
 * Return(s)   : none.
 *
 * Caller(s)   :
 *
 * Note(s)     : (1)
 *
 *               (2)
 *********************************************************************************************************
 */
 void Debug_IO_Port_InterruptRestore(CPU_INT32U IntState)
 {
	 /* Restore the interrupt state	 */
	 	XUartPs_WriteReg(Uart_Ps_0.Config.BaseAddress, XUARTPS_IER_OFFSET,
	 			IntState);
 }


/*Just for unit test*/
static void debug_uart_handlerTest(void *packetInfo)
{


print("In Uart Handler\n");
print ("Debug_IO_RxFIFO is ");
print(Debug_IO_RxFIFO);
print("\n");
print("Debug_IO_TxFIFO is ");
print (Debug_IO_TxFIFO);

}
/*Just for unit test
 * */
 void Debug_UART_Test(void)
{


	 CPU_INT08U *c;
	//Debug_IO_TxFIFO = {'H','E','L','O','W','O','R','L','D'};
	Debug_IO_init(debug_uart_handlerTest,10,Debug_IO_RxFIFO);
	print("In Debug UART test\n");
	/*c = Debug_Port.Debug_Read_char();
	Debug_Port.Debug_Write_char(*c);
	Debug_Port.Debug_Read_Buffer(&Debug_IO_RxFIFO[0] , 5);
	Debug_Port.Debug_Write_Buffer(&Debug_IO_TxFIFO[0],10);
	*/
	CPU_INT08U count = Debug_Port.Debug_Read_TillChar(Debug_IO_RxFIFO , '#');
	count = Debug_Port.Debug_Read_IgnoreTillChar('$');

}

