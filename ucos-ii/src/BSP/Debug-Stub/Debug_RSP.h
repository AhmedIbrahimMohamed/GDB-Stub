/*
 * Debug_RSP.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */
/*
 * template.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */
/*
*********************************************************************************************************
*
*                                      Debug Remote Serial Protocol (Debug_RSP) module
*
*                        Xilinx Zynq-7000 ZC702 Cortex-A9 MPCore DEVELOPMENT KIT
*
* Filename      : Debug_RSP.h
* Version       : V1.00
* Programmer(s) :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               DEBUG_RSP present pre-processor macro definition.
*********************************************************************************************************/
#ifndef DEBUG_RSP_H_
#define DEBUG_RSP_H_



/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/



#include "Debug_IO.h"
#include "Debug_Types.h"
/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef Debug_RSP_IMPORT
	#define EXTERN
#else
	#define EXTERN extern
#endif

/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/
#define Debug_RSP_NumCommPackets  20                   /*Number of command Packets*/

#define  Debug_RSP_DefaultNumBytesRxedINT  16  /*This is the default Number of bytes that  RSP module needs from IO module
                                                             when receive interrupt occurs*/

//#define Debug_RSP_IN_OUTBUFMax			((NUMREGBYTES << 1) + 10)
#define Debug_RSP_IN_OUTBUFMax			400                  /*Maximum length of payload Buffer
                                                                 400 Hex chars = 200 bytes = 50 words*/

#define Debug_NoCheckSumCheck           0

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

/*Input and Output buffer to hold the DataPayload of an RSP Packet*/
EXTERN CPU_INT08U Debug_RSP_Payload_InBuf[Debug_RSP_IN_OUTBUFMax]
#ifdef    Debug_RSP_IMPORT
= {5}
#endif
;
EXTERN CPU_INT08U Debug_RSP_Payload_OutBuf[Debug_RSP_IN_OUTBUFMax]
#ifdef    Debug_RSP_IMPORT
= {5}
#endif
;
/*Array of Function  Pointers to all supported packet commands
 *
 * Each function pointer points to function handling a specific packet command (i.e. <m> packet , <g> packet ,etc)
 *function pointers are  initialized to Unknown_Packet handling Routine
 *and after that ,Debug_Main module assign each function pointer to its corresponding defined routine.(in Debug_Main_Init_CommandFunctions() )
 *
 * */

EXTERN CPU_INT08U (*Debug_RSP_Commands_Functions[Debug_RSP_NumCommPackets])(Debug_TID_t ThreadID, void *CommandParam);



/*
 *
 *
 * Each data structure of the following data structures holds information about options sent in the command packet
 * These options should be more than one option to make a struct for it named Debug_RSP_Info_<command>
 * for Example: the <m> packet for target memory-range reading has the form of  ($m addr,length #checksum).
 * here, addr,length are put in a structure and named "Debug_RSP_Info_m" . and so on for all commands
 *
 * These data structures are passed to their corresponding handling routine in Debug_Main module-after casting to (void *)- through calling
 * Debug_RSP_Commands_Functions[Debug_RSP_<command>]((void *)Debug_RSP_Info_<command>);
 * All other commands' options are passed as they are
 *
 *
 * */
/*information about <m> command options*/
typedef struct _Debug_RSP_Info_m
{
	Debug_addr_t start_address ;
	Debug_Len_t       Count;

}Debug_RSP_Info_m;

EXTERN Debug_RSP_Info_m Command_opts_m;

/*information about <g> and <G> command options*/
typedef struct _Debug_RSP_Info_gG
{
	/*pointer to Register Buffer used by low level module to get/set register*/
	Debug_RegisterWidth *RegisterarrayPtr;
	CPU_INT08U bytecount;
}Debug_RSP_Info_gG;

EXTERN Debug_RSP_Info_gG Command_opts_gG
#ifdef Debug_RSP_IMPORT
 = {0}
#endif
 ;



/*information about <M> command options*/
typedef struct _Debug_RSP_Info_mM
{
	Debug_MemWidth  *MemoryArrayptr;          /*Memory array ptr to write to the read memory range*/
	Debug_addr_t start_address ;              /*Start address of memory to write to/read from*/
	CPU_INT32U       Count;                   /*byte count of read/written memory range*/

}Debug_RSP_Info_mM;

EXTERN Debug_RSP_Info_mM Command_opts_mM;


/*information about <zn/Zn> command options*/
typedef struct _Debug_RSP_Info_znZn
{
	Debug_addr_t  *BkptAddress;
	CPU_INT08U       Kind;                /*kind is target-specific and typi-cally indicates the size of the breakpoint
	                                        in bytes that should be inserted. E.g.,
                                            the arm and mips can insert either a 2 or 4 byte breakpoint. Some architectures
                                            have additional meanings for kind; */
	/*CondList;*/                            /*Condition list*/
}Debug_RSP_Info_znZn;

EXTERN Debug_RSP_Info_znZn Command_opts_znZn;
/*information about <P> command options*/
typedef struct _Debug_RSP_Info_pP
{
	Debug_RegID_t  RegID;
	Debug_RegisterVal RegVal;                /*it is the value to be written in case of <P> and the value read in case of <p>*/
}Debug_RSP_Info_pP;

EXTERN Debug_RSP_Info_pP Command_opts_pP;

/*information about <?> command
 * would be extended to support <T> responsepacket with its parameters
 * */
typedef struct _Debug_RSP_Info_HaltSig
{
	CPU_INT08U  Signum;

}Debug_RSP_Info_HaltSig;

EXTERN Debug_RSP_Info_HaltSig Command_opts_HaltSig;
typedef  enum {
                Debug_RSP_UNKNOWN = 1, /*Unknown command */
                Debug_RSP_m ,  /*read memory range $m,<addr>,<len>#CS*/
                Debug_RSP_M ,  /*write memory range $M,<addr>,<len>#CS*/
                Debug_RSP_g ,
                Debug_RSP_G ,
                Debug_RSP_p ,
                Debug_RSP_P ,
                Debug_RSP_c ,
                Debug_RSP_s ,
                Debug_RSP_z0 , /*SW breakpoint*/
                Debug_RSP_Z0 , /**/
                Debug_RSP_z1 ,/*HW breakpoint*/
                Debug_RSP_Z1 ,
                Debug_RSP_ReportHaltReason,
                Debug_RSP_CONSOLE
 }Debug_RSP_Commands;

/*Debug_Block_Message :: this is the message  that is passed after Exception Handling to Stub Task when resuming
 * For Example , it is used here to pass the source of exception to stub task
 * */


EXTERN Debug_Block_Message_t Debug_Block_Message;

/*
 * This enumerates Sources of Exception that Stub-Handler trigger whenever an Exception occurs
 * */
enum {
	Debug_Exception_BKPT_Hit,
	Debug_Exception_UNDEFINED_INSTRUCTION,
	Debug_Exception_MemoryError,
	//what else can we provide

}Debug_Exception;


enum {

	Stub_Active,
	Stub_Blocked,
	Stub_Detach_Kill
}Stub_State;

/*Function Pointer points to pend routine provided by RTOS module*/
Debug_Block_Message_t (*Debug_Stub_PendPtr)(void);
void (*Debug_Stub_PostPtr)(void * Msg);

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
EXTERN void Debug_RSP_Init(void);


EXTERN void Debug_RSP_Get_Packet(void);
EXTERN CPU_INT08U Debug_RSP_Process_Packet(Debug_TID_t Thread_of_focus);
/*TODO::
 * Decide whether following 3 functions would be local or global
 * for now, Following */
EXTERN void Debug_RSP_Put_Packet(CPU_INT08U *PacketData,Debug_RSP_Commands Command) ;
EXTERN void Debug_RSP_Error_Packet(CPU_INT08U ErrorNum);
EXTERN void Debug_RSP_Console_Packet(CPU_INT08U *PacketData);

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


#undef Debug_RSP_IMPORT
#undef EXTERN
#endif /* DEBUG_RSP_H_ */
