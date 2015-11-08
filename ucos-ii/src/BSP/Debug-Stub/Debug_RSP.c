/*
 * Debug_RSP.c
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
*                                        Debug Remote Serial Protocol (Debug_RSP) module
*
*                                         Xilinx Zynq XC7Z020
*                                               on the
*
*                                             Xilinx ZC702
*                                          Evaluation Board
*
* Filename      : Debug_RSP.c
* Version       : V1.00
* Programmer(s) :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define Debug_RSP_IMPORT
#include "Debug_RSP.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*Macro to Reset a character array Buf of size sz bytes  to zeros*/

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
typedef struct _Debug_RSP_Packet
{
	Debug_RDP_header_t header;
	Debug_RSP_trailer_t trailer;
	CPU_INT08U *Payload_ptr;
	CPU_INT08U CheckSum;
}Debug_RSP_Packet;

Debug_RSP_Packet RSP_Packet;
typedef enum {
               Debug_RSP_Memory,
               Debug_RSP_Register,
               Debug_RSP_Exec,
               Debug_RSP_Breakpoint,
               Debug_RSP_Query
 }Debug_RSP_CommandClasses ;

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

#define Debug_RSP_Max_BufferLen  50                          /*Maximum Transmit/Recive buffer Length in ??? */


/* Packet buffers hold the Transmitted/Received packet Raw information */
static CPU_INT08U Debug_RSP_SendBuffer[Debug_RSP_Max_BufferLen];
static CPU_INT08U Debug_RSP_ReceiveBuffer[Debug_RSP_Max_BufferLen];




static const CPU_INT08U Debug_RSP_Byte2Hex_Lookup[] = "0123456789abcdef";

/*Memory Buffer used for reading/writing memory values*/


/*Register Buffer used for reading/writing memory values*/

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*asynchronous Receiving*/
static void Debug_RSP_Port_IntHandler(void *packetInfo);
static void Debug_RSP_Register_port_Handler(void);
static void Debug_RSP_InitCmmands_FunctionsList(void);
static void Debug_RSP_Unknown_Packet(void);
static void Debug_RSP_Acks(void);
static void Debug_RSP_Nacks(void);

/*Helper functions*/
static void Debug_Reg2Hex(void);
static void Debug_Hex2Reg(void);

static CPU_INT08U Debug_Mem2Hex(Debug_MemWidth *start_address, Debug_Len_t ByteCount,CPU_INT08U *HexBuffer);
static CPU_INT08U Debug_Hex2Mem(CPU_INT08U *HexBuffer,CPU_INT08U *MemByteBuffer, Debug_Len_t ByteCount);
static CPU_INT08U Debug_RSP_Byte2Hex(CPU_INT08U halfbyte);
static CPU_INT08U Debug_RSP_Hex2byte(CPU_INT08U Hex);
static CPU_INT08U Debug_Hex2Word(CPU_INT08U **HexStream, Debug_MemWidth * word);
static void Debug_Word2Hex(Debug_MemWidth * word, CPU_INT08U **HexStream);
static void Debug_RSP_resetBuffer(CPU_INT08U * bufptr);
static CPU_INT08U IsBreakPointValid(CPU_INT08U * ptr,Debug_MemWidth * bPAddress,CPU_INT32U * bPLength,CPU_INT08U * bPType );
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





/*
*********************************************************************************************************
*                                               Debug_RSP_Init()
*
* Description : Initialize Debug_RSP module
*               - call Debug_IO (low-level module) initialize function
*               - RSP module register its own high-level handler to Debug_IO handler
*                 so that it can be called from Debug_IO port-events' handler
*               - initialize command function array to its default handling routine which is Debug_RSP_Unknown_Packet()
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : should be Debug_Main Module
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/



void Debug_RSP_Init(void)
 {

	/*Call Debug_IO module initialization function */

	Debug_IO_init(Debug_RSP_Port_IntHandler,Debug_RSP_DefaultNumBytesRxedINT,Debug_RSP_ReceiveBuffer);

	Debug_RSP_InitCmmands_FunctionsList();

	RSP_Packet.header = '$';
	RSP_Packet.trailer = '#';
	RSP_Packet.CheckSum = 0;
	RSP_Packet.Payload_ptr = Debug_RSP_Payload_InBuf;

	Debug_Block_Message = Debug_Exception_UserBKPT_Hit;
 }
/*
*********************************************************************************************************
*                                               Debug_RSP_Put_Packet()
*
* Description :This function encapsulate the data payload into RSP Packet format  $packet-data #checksum
*
* Argument(s) : PacketData : main data payload of RSP Packet
*               Command    : RSP command To which this packet is a response
*               this parameter is needed to convert the memory contents or register contents to Hex stream
*
* Return(s)   : none.
*
* Caller(s)   : Debug_Main Module after processing specific command
*
* Note(s)     : (1)gets information from the packet structure RSP_Packet
*
*               (2)
*********************************************************************************************************
*/
 void Debug_RSP_Put_Packet(CPU_INT08U *PacketData, Debug_RSP_Commands Command)
 {

	 CPU_INT08U Char;
     /*Do we need to Flush Transmit buffer first??*/

	 /*TODO::
	  * Discuss which is better
	  * is it to put the complete packet in buffer with all things header - trailor-cs then use writebuffer to send it
	  *
	  * or
	  *
	  * send it as separated characters as the following code
	  * */

	 	while(1)

	{

	 		Debug_Port.Debug_Write_char(RSP_Packet.header);
			 //reset the checksum
			RSP_Packet.CheckSum = 0;

			 RSP_Packet.Payload_ptr = PacketData;
			 /*Add 'O' to the checksum in case of Console Output packet */
			 if (Command == Debug_RSP_CONSOLE)
			 {
				 Debug_Port.Debug_Write_char('O');
				 RSP_Packet.CheckSum += 'O';
			 }

	 		//Write The Packet Payload
		// Char = RSP_Packet.Payload_ptr;

	 while(Char = *(RSP_Packet.Payload_ptr ++))
		 {
		       Debug_Port.Debug_Write_char(Char);
		       RSP_Packet.CheckSum += Char;

		 }
	 //Write the Packet trailer

	 Debug_Port.Debug_Write_char(RSP_Packet.trailer);

	 //write the calculated checksum
	 Debug_Port.Debug_Write_char(Debug_RSP_Byte2Hex((RSP_Packet.CheckSum >> 4) & 0xF));   /*Write higher half-byte of checksum*/
	 Debug_Port.Debug_Write_char(Debug_RSP_Byte2Hex(RSP_Packet.CheckSum & 0xF));
       /*Wait to get Ack*/

	 Char = Debug_Port.Debug_Read_char();

	 if (Char == '+')
		 return;
	 if(Char == '$')  /*What does this mean , GDB Reconnects? as mentioned in RT-Thread Stub*/

	 {

		 /*Nacks this packet*/
		 Debug_RSP_Nacks();
          /*TODO:: discuss :
           * Flush Transmit Buffer as RT-Thread Stub*/
          /*why we do not flush the Receiver too*/
          return;


	 }
	 /*else , retransmit packet! assume we receive a '-' NACKing packet we sent*/
 }


 }
 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Get_Packet()
 *
 * Description : Get RSP Packet format $<dataPayload>#<CheckSum>
 *               This function de-capsulate information received from debugger from RSP format
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Debug_Main Module
 *
 * Note(s)     : (1) this function uses a blocking sent function to send a full packet
 *
 *               (2)
 *********************************************************************************************************
 */
 void Debug_RSP_Get_Packet(void)
 {


	 //CPU_INT08U *Sinkptr;
       /*Ignore any characters till we get the packet header*/
      CPU_INT08U RxChar;
      CPU_INT08U CalcChecksum = 0;
      CPU_INT08U PacketCheckSum = 0;
      CPU_INT08U Rxcount = 0;

#if (Debug_NoCheckSumCheck == 0)

do{
#endif
	 Rxcount         = 0;            /*start from index 0 in input buffer to receive another packet*/
#if (Debug_NoCheckSumCheck == 0)
	 //CalcChecksum    = 0;            /*Reset the local Checksum*/
	 RSP_Packet.CheckSum = 0;
	/*return after receiving '$'*/
    Debug_Port.Debug_Read_IgnoreTillChar(RSP_Packet.header);

    /*Now receive the data payload till the packet trailer and calculate the Ckecksum*/

    /*TODO::
    	  *Need check size of receive buffer against received characters count*/

#endif

    while((RxChar = Debug_Port.Debug_Read_char()) != RSP_Packet.trailer)
   	 {

    	/*Receiving payload*/
   	 Debug_RSP_Payload_InBuf[Rxcount] = RxChar;
#if (Debug_NoCheckSumCheck == 0)

   	 //CalcChecksum += RxChar;
   	 RSP_Packet.CheckSum += RxChar ;
#endif
   	 Rxcount++;
   	 }

#if (Debug_NoCheckSumCheck == 0)

    /*Now Get the packet CheckSum*/
    PacketCheckSum  = Debug_RSP_Hex2byte(Debug_Port.Debug_Read_char()) << 4;    /*should be high half-byte of checksum*/
    PacketCheckSum  += Debug_RSP_Hex2byte(Debug_Port.Debug_Read_char());   /*should be Low half-byte of checksum*/


    /*Nack the Packet if Checksums are not equal*/
    if(PacketCheckSum != RSP_Packet.CheckSum)
    	Debug_RSP_Nacks();

    /*Ack the correct packet*/
    else
    	Debug_RSP_Acks();


}while(PacketCheckSum != RSP_Packet.CheckSum);/*re-receive <--> ack/Nack Cycle*/
#endif

//}while(PacketCheckSum != CalcChecksum);/*re-receive <--> ack/Nack Cycle*/

 }

 /*
  *********************************************************************************************************
  *                                               Debug_RSP_Process_Packet()
  *
  * Description : process the incoming packet for received commands and call its handling routine
  *
  * Argument(s) : none.
  *
  * Return(s)   : none.
  *
  * Caller(s)   : not known yet but may be stub task
  *
  * Note(s)     : (1)
  *
  *               (2)
  *********************************************************************************************************
  */
  CPU_INT08U Debug_RSP_Process_Packet(Debug_TID_t Thread_of_focus)
  {
	  /*TODO:: Reset the outbuffer to 0's to put the new response of the received command*/
	  /*This loop is too slow and suspicious as counter is reset or low value after high value 0->97->298->78 !!!*/
	  Debug_RSP_resetBuffer(Debug_RSP_Payload_OutBuf);

	  /*Chosoe a task ID here for a matter of testing*/
	 // CPU_INT08U taskID = 0 ;/*would be ignored for now by Debug_Main_xx() functions*/
	  CPU_INT08U *Cmd_Param_start =  &Debug_RSP_Payload_InBuf[1];
	  CPU_INT08U *Debug_INVALID = "";   /*Invalid reply string for invalid command parameters/options*/
	  //Debug_RSP_Info_pP *paramPtr = &Command_opts_pP;
	  Debug_MemWidth BpAddress;
	  CPU_INT32U BpLength;
	  CPU_INT08U BpType;
	  CPU_INT08U err = 0;
	  Debug_MemWidth Resume_Address = 0;
	  Debug_Block_Message_t stub_message;

	  switch (Debug_RSP_Payload_InBuf[0])
	  {
	  	         case '?':/* report stopped reason */
	  	        	/*TODO:: Need Test if we are here , does GDB Reconnecting*/
	  	            Debug_RSP_Payload_OutBuf[0]= 'S';
	  	        	(*Debug_RSP_Commands_Functions[Debug_RSP_ReportHaltReason])(Thread_of_focus,(void *)&Command_opts_HaltSig);
	  	        	Debug_RSP_Payload_OutBuf[1] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum >> 4);
	  	        	Debug_RSP_Payload_OutBuf[2] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum & 0xF);
	  	        	Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_ReportHaltReason );

	  	        	break;

	  	         case 'g':       /* return the value of the CPU registers */
                                 /*'g'*/
					    (*Debug_RSP_Commands_Functions[Debug_RSP_g])(Thread_of_focus,(void *)&Command_opts_gG);

					    /*TEST : see value of RegisterarrayPtr here*/
						if(Debug_Mem2Hex(Command_opts_gG.RegisterarrayPtr,Command_opts_gG.bytecount ,&Debug_RSP_Payload_OutBuf[0])
								== DEBUG_ERR_INOUTBUF_OVERFLOW)
						{/*TODO: Handle the overflow error*/}
						else
							/*TEST : see value of RegisterarrayPtr here ,should not change*/
							Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_g );
				 break;
	  	         case 'G':       /* set the value of the CPU registers - return OK */
	  	        	             /*‘G XX...’*/
						if (Debug_Hex2Mem(Cmd_Param_start, (CPU_INT08U *)Command_opts_gG.RegisterarrayPtr,Command_opts_gG.bytecount)
								== DEBUG_SUCCESS)
							{
							   (*Debug_RSP_Commands_Functions[Debug_RSP_G])(Thread_of_focus,(void *)&Command_opts_gG);

							   Debug_RSP_OK_Packet();
							}else
							{
								Debug_INVALID = "\nINVALID Hex Characters in register values in <G> command packet\n";
								//Debug_RSP_Put_Packet(Debug_INVALID, 0);
								Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
								break;
							}
	  	       break;
	  	         case 'm': /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
	  	        	        /* ‘m addr,length ’ - return length bytes of memory*/
	  	        	 /*TEST : see Value of Cmd_Param_start here*/

	  	        	 if (Debug_Hex2Word(&Cmd_Param_start, &Command_opts_mM.start_address) > 0 && *Cmd_Param_start++ == ',' &&
	  	 	  	        			Debug_Hex2Word(&Cmd_Param_start, &(Command_opts_mM.Count)) > 0) {
	  	 	  	        /*TEST : see Value of Cmd_Param_start here shouldbe advanced to after length field
	  	 	  	         *        see value of start address and count*/

	  	        		 if(Command_opts_mM.Count > (Debug_RSP_IN_OUTBUFMax >> 1)) /*if number of bytes needed < #hexDigits/2 , exceedes buffer*/
	  	        		 {
	  	        			Debug_INVALID = "\nRequired Number of bytes in <m>command packet exceeds target output buffer\n";
	  	        			//Debug_RSP_Put_Packet(Debug_INVALID, 0);
	  	        			Debug_RSP_Error_Packet(Debug_PacketErrorCode);
	  	        			break;
	  	        		 }

	  	        		 (*Debug_RSP_Commands_Functions[Debug_RSP_m])(Thread_of_focus,(void *)&Command_opts_mM);

						 if(Debug_Mem2Hex(Command_opts_mM.MemoryArrayptr,Command_opts_mM.Count, &Debug_RSP_Payload_OutBuf[0])
								== DEBUG_SUCCESS)
							Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_m );
						 else {
								//TODO:: how to handle the Overflow error
							break;
							 }
	  	 	  	        } else {
	  	 	  	        		    	Debug_INVALID = "\nINVALID Syntax or invalid Hex Characters in <m> command packet\n";
	  	 	  	        		    	//Debug_RSP_Put_Packet(Debug_INVALID, 0);
	  	 	  	        		        Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
	  	 	  	        		        break;
	  	 	  	        	    }
	  	 	  	             break;
	  	          case 'M':/* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
	  	        	       /* ‘M addr,length :XX...’ */

	  	        	 /*TEST : see Value of Cmd_Param_start here*/
	  	        	    if (Debug_Hex2Word(&Cmd_Param_start, &Command_opts_mM.start_address) > 0 && *Cmd_Param_start++ == ',' &&
						    Debug_Hex2Word(&Cmd_Param_start, &(Command_opts_mM.Count)) > 0 && *Cmd_Param_start++ == ':') {
	  	        	    	/*TEST : see Value of Cmd_Param_start here should be advanced to after length field
	  	        	    		  	 	  	         *        see value of start address and count*/
 						if (Debug_Hex2Mem(Cmd_Param_start, (CPU_INT08U *)Command_opts_mM.MemoryArrayptr,Command_opts_mM.Count) == DEBUG_SUCCESS)

						 {

 							if(Command_opts_mM.Count > (Debug_RSP_IN_OUTBUFMax >> 1)) /*if number of bytes needed < #hexDigits/2 , exceedes buffer*/
								 {
 								/*TODO::discussion
 								 * we can ignore this exceedence and write what we have*/
									Debug_INVALID = "\nRequired Number of bytes in <M>command packet exceeds target output buffer\n";
									//Debug_RSP_Put_Packet(Debug_INVALID, 0);
									Debug_RSP_Error_Packet(Debug_PacketErrorCode);
									break;
								 }

 							if( (*Debug_RSP_Commands_Functions[Debug_RSP_M])(Thread_of_focus,(void *)&Command_opts_mM) == DEBUG_SUCCESS)
								Debug_RSP_OK_Packet();
							else{
								Debug_INVALID = "\nCan not set memory values in <M> command packet\n";
								//Debug_RSP_Put_Packet(Debug_INVALID, 0);
								Debug_RSP_Error_Packet(DEBUG_MEM_ERROR);
								break;

							}


						 }else {
								Debug_INVALID = "\nInvalid Hex Characters in memory values in <M> command packet\n";
								//Debug_RSP_Put_Packet(Debug_INVALID, 0);
								Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
								break;
							 }
					    }else {
							Debug_INVALID = "\nINVALID syntax or invalid Hex Characters in <M> command packet\n";
							//Debug_RSP_Put_Packet(Debug_INVALID, 0);
							Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
							break;
						 }
				 break;
	  	         case 'p':       /* return the value of  a single CPU register */
	  	        	             /* ‘p n’ */

						 if (Debug_Hex2Word(&Cmd_Param_start, &(Command_opts_pP.RegID) )){

							 if( (*Debug_RSP_Commands_Functions[Debug_RSP_p])(Thread_of_focus,(void *)&Command_opts_pP)
									 == DEBUG_ERR_INVALID_REGID)
								{
								 Debug_INVALID = "\nRegno is outside supported IDs in <p> command packet\n";
								 //Debug_RSP_Put_Packet(Debug_INVALID, 0);
								 Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
								break;
								}
							 else
							 {
								 /*Convert the regvalue to HEX and Send it */
								if(Debug_Mem2Hex(&(Command_opts_pP.RegVal),sizeof(Debug_RegisterWidth),&Debug_RSP_Payload_OutBuf[0])
										== DEBUG_ERR_INOUTBUF_OVERFLOW)
								{//TODO:: how to handle the Overflow error
	  	 	  	        			break;}

								Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_p );
							 }
							 break;
						}else {
							Debug_INVALID = "\nINVALID Syntax or Invalid Hex Characters in Register ID in <p> command packet\n";
							//Debug_RSP_Put_Packet(Debug_INVALID, 0);
							Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
						   break;
						}

	  		  	 break;
	  	         case 'P':       /* set the value of a single CPU registers - return OK */
	  	        	             /* ‘P n...=r...’ */
	  	        	/*TEST : see Value of Cmd_Param_start here*/
						if (Debug_Hex2Word(&Cmd_Param_start, &Command_opts_pP.RegID) && *Cmd_Param_start++ == '='
								&& Debug_Hex2Mem(Cmd_Param_start, &(Command_opts_pP.RegVal) , sizeof(Debug_RegisterWidth)) == DEBUG_SUCCESS )

							/*TEST : see Value of Cmd_Param_start here should be advanced
							 *       see value of RegID and RegVal*/
						{
							if ((*Debug_RSP_Commands_Functions[Debug_RSP_P])(Thread_of_focus,(void *)&Command_opts_pP) == DEBUG_SUCCESS)
								Debug_RSP_OK_Packet();
							 else {
										Debug_INVALID = "\nRegno in <P> command packet is outside supported IDs\n";
										//Debug_RSP_Put_Packet(Debug_INVALID, 0);
										Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
										break;
									}
						}else {
								Debug_INVALID = "\nINVALID Syntax or INVALID Hex Characters in <P> command packet ID\n";
								//Debug_RSP_Put_Packet(Debug_INVALID, 0);
								Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);

							   break;
						}
	  		  	 break;


	  	         case 'D': /* Debugger detach */
	  	         case 'k': /* Debugger detach via kill */
	  	        	 //TODO:
	  	        	 //has an error reply
	  	        	 return Stub_Detach_Kill;
	  	             break;
	  	         case 's': /* sAA..AA    step form address AA..AA (optional) */

	  	        	 /* 1- get  an address to step from if GDB sent*/
	  	        	if(!Debug_Hex2Word(&Cmd_Param_start,&Resume_Address))//if no address is specified in <s> packet
	  	        	{
	  	        		/*2-  Insert breakpoint at next executed instruction address */
						err = (*Debug_RSP_Commands_Functions[Debug_RSP_s])(Thread_of_focus,(void *)Resume_Address);
						if (err == DEBUG_SUCCESS || err == DEBUG_Bad_Instruction )
						{
							/*3- activate all SW breakpoints to be hit when resumed*/
							//Activate_Sw_BreakPoints();

							/*4- Pend Stub Task so that program resumes */
							stub_message = (*Debug_Stub_PendPtr)();

							/*Here , Stub resume with the appropriate Exception Signal,
							 * Note , Stub will not wake up in case of the following Exceptions
							 * RST : system Reset
							 * RSVD : Reserved Exception Entry
							 * SVC  : supervisor call
							 *  */
							Debug_RSP_Payload_OutBuf[0]= 'S';
							Debug_RSP_Payload_OutBuf[1] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum >> 4);
							Debug_RSP_Payload_OutBuf[2] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum & 0xF);
							Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_s );
						}
						else /*Error in setting breakpoint*/
						{
							/**TODO:: in case of DEBUG_BRKPT_ERROR_UnableSET, will we
							 * 1- resume without breakpoint??
							 * 2- would we send Error Packet to gdb, no it is not in standard
							 * 3- send console packet with error to user and resume*/
						}

	 	        	}//if GDB wants to step from current address

	  	        	else{//step from  address

	  	        	}//if GDB need to step from  address

	  	        	 break;
	  	         case 'c': /* cAA..AA    Continue at address AA..AA (optional) */

					 /*  get  an address to resume from if GDB sent*/
					Debug_Hex2Word(&Cmd_Param_start,&Resume_Address);

					err = (*Debug_RSP_Commands_Functions[Debug_RSP_c])(Thread_of_focus,(void *)Resume_Address);

					/*pend on exception occurance */
					/*Pending mechanism*/
					stub_message = (*Debug_Stub_PendPtr)();

					/*Here , Stub resume with the appropriate Exception Signal,
					 * Note , Stub will not wake up in case of the following Exceptions
					 * RST : system Reset
					 * RSVD : Reserved Exception Entry
					 * SVC  : supervisor call
					 *  */
					Debug_RSP_Payload_OutBuf[0]= 'S';
					Debug_RSP_Payload_OutBuf[1] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum >> 4);
					Debug_RSP_Payload_OutBuf[2] = Debug_RSP_Byte2Hex(Command_opts_HaltSig.Signum & 0xF);
					Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],Debug_RSP_s );

				break;

	  	       case 'z':/* Break point remove */
	  	       case 'Z':/* Break point set */

	  	    	 err = IsBreakPointValid(Debug_RSP_Payload_InBuf,&BpAddress,&BpLength,&BpType);
				if( err == DEBUG_SUCCESS)
				 {
					Command_opts_znZn.BkptAddress=&BpAddress;
					Command_opts_znZn.Kind=BpType;
						if(Debug_RSP_Payload_InBuf[0]=='Z')
						{
							if((*Debug_RSP_Commands_Functions[Debug_RSP_Z0])(0,(void *)&Command_opts_znZn) == DEBUG_BRKPT_ERROR_UnableSET)
							{
								Debug_INVALID = "\nCan not set breakpoint \n";
								//Debug_RSP_Put_Packet(Debug_INVALID, 0);
								Debug_RSP_Error_Packet(DEBUG_BRKPT_ERROR_UnableSET);
								break;
							}
							else
								Debug_RSP_OK_Packet();
						}
							else if(Debug_RSP_Payload_InBuf[0]=='z')
							{//TODO: check error Code from Remove SoftWare BreakPoint
									if((*Debug_RSP_Commands_Functions[Debug_RSP_z0])(0,(void *)&Command_opts_znZn) == Debug_BKPT_Remove_AddressError)
								{
									Debug_INVALID = "\nCan not Remove or Not Exist breakpoint \n";
									//Debug_RSP_Put_Packet(Debug_INVALID, 0);
									Debug_RSP_Error_Packet(Debug_BKPT_Remove_AddressError);
									break;
								}
								else
									Debug_RSP_OK_Packet();
							}
				 }
				 else
					Debug_RSP_Error_Packet(err);
	  	       	 break;
	  	         default:
	  	        	Debug_RSP_Unknown_Packet();
	  	        	break;
	  	     }


	  /*reset the input buffer for further packet getting */
	  Debug_RSP_resetBuffer(Debug_RSP_Payload_InBuf);

	  return Stub_Active;
  }



 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Error_Packet()
 *
 * Description : send an Error Packet Format 'E'
 *
 * Argument(s) : ErrorNum :  Error code as described in Debug_Types.h
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)
 *
 *               (2)
 *********************************************************************************************************
 */
 void Debug_RSP_Error_Packet(CPU_INT08U ErrorNum)
 {
     /*An error has occurred; xx is a two-digit hexadecimal error number ($Exx#CS )*/

	 Debug_RSP_Payload_OutBuf[0] = 'E';
	 Debug_RSP_Payload_OutBuf[1] = Debug_RSP_Byte2Hex(ErrorNum >> 4);
	 Debug_RSP_Payload_OutBuf[2] = Debug_RSP_Byte2Hex(ErrorNum & 0xF);
	 Debug_RSP_Put_Packet(&Debug_RSP_Payload_OutBuf[0],0);

 }
 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Console_Packet()
 *
 * Description : sends an 'O' Packet to sends application specific output to debugger that should be displayes  as it is to user
 *
 * Argument(s) : PacketData: should be pointer to message to be printed by GDB Console.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : application print() function that is used across application,kernel,etc
 *
 * Note(s)     : (1)it is used to be able to multiplex a single medium port for transferring both
 *                  Protocol-specific commands and application-specific output
 *
 *               (2)
 *********************************************************************************************************
 */
 void Debug_RSP_Console_Packet(CPU_INT08U *PacketData)
 {
      Debug_RSP_Put_Packet(PacketData,Debug_RSP_CONSOLE);
 }
 /*
  *********************************************************************************************************
  *                                               Debug_RSP_OK_Packet()
  *
  * Description : sends an ok Packet
  *
  * Argument(s) : none.
  *
  * Return(s)   : none.
  *
  * Caller(s)   : Debug_Main module
  *
  * Note(s)     : (1)
  *               (2)
  *********************************************************************************************************
  */
 void Debug_RSP_OK_Packet(void)
 {

	 CPU_INT08U *OK = "OK";
	 Debug_RSP_Put_Packet(OK,0);
 }



 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Port_IntHandler()
 *
 * Description :High-level interrupt handler for Debug port events
 *
 * Argument(s) : packetInfo : any information that low-level IO module needs to deliver to RSP module
 *               not formally defined yet but may be pointer to received buffer of data based on specific interrupt
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Debug_IO_PortHandler() via Debug_RSPCallback in Debug_IO_Port structure
 *
 * Note(s)     : (1)
 *
 *               (2)
 *********************************************************************************************************
 */
 static void Debug_RSP_Port_IntHandler(void *packetInfo)
{

}
 /*
 *********************************************************************************************************
 *                                               Debug_RSP_InitCmmands_FunctionsList()
 *
 * Description :Initialize the RSP Commands' Function pointers to their default handling routine which is Debug_RSP_Unknown_Packet
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Debug_RSP_Init()
 *
 * Note(s)     : (1)
 *
 *               (2)
 *********************************************************************************************************
 */

 static void Debug_RSP_InitCmmands_FunctionsList(void)

 {
 	CPU_INT08U CommIndex ;
         for(CommIndex = 0; CommIndex < Debug_RSP_NumCommPackets;CommIndex++ )
              Debug_RSP_Commands_Functions[CommIndex] = Debug_RSP_Unknown_Packet ;
 }

 /*default function for processing unknown/unsupported command*/


 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Unknown_Packet()
 *
 * Description :sends an empty RSP packet in respond to unknown debugger command packet
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)
 *
 *               (2)
 *********************************************************************************************************
 */
 static void Debug_RSP_Unknown_Packet(void)
 {
	 /*For any command not supported by the stub, an empty response (‘$#00’) should
	 be returned*/
	 CPU_INT08U Empty[] = "";
	 Debug_RSP_Put_Packet(Empty, Debug_RSP_UNKNOWN);

 }

 static void Debug_RSP_Acks(void)
 {

	 Debug_Port.Debug_Write_char('+');
 }
 static void Debug_RSP_Nacks(void)
 {
	 Debug_Port.Debug_Write_char('-');
 }
 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Byte2Hex()
 *
 * Description : convert a 4-bit in byte to its corresponding HEX ASCII character
 *
 * Argument(s) : halfbyte : decimal value between 0 , 15.
 *
 * Return(s)   : Corresponding HEX character
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)Examples : input= 15  output= 'F'  , input = 4  output= '4' input=12  output='C'
 *
 *
 *********************************************************************************************************
 */
 static CPU_INT08U Debug_RSP_Byte2Hex(CPU_INT08U halfbyte)
 {
	 /*TODO::
	  * Should we assure that halfbyte Value < 16 decimal*/
	 if(halfbyte <= 0xF)
		 return Debug_RSP_Byte2Hex_Lookup[halfbyte];
	 else
		 return DEBUG_ERROR;

 }

 /*
 *********************************************************************************************************
 *                                               Debug_RSP_Hex2byte()
 *
 * Description : Convert HEX ASCII character to its corresponding  4-bit decimal value
 *
 * Argument(s) : hexbyte : decimal value between 0 , 15.
 *
 * Return(s)   : Corresponding decimal value between 0 , 15
 *               DEBUG_ERR_INVALID_HEXChar if got invalid HEX character
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)Examples : input= 'F'|'f'  output= 15  , input = '4'  output= 4 input='C'|'c'  output=12
 *
 *
 *********************************************************************************************************
 */
 static CPU_INT08U Debug_RSP_Hex2byte(CPU_INT08U hexbyte)
  {
	 if(hexbyte >= 'A' && hexbyte <= 'F')
		 return hexbyte - 'A'/*get the offset from ASCII*/ + 10 ;   /*10 is for the offset of literal hexs(a-f/A-F) in Debug_RSP_Byte2Hex_Lookup[]*/
	 else if(hexbyte >= 'a' && hexbyte <= 'f')
	 		 return hexbyte - 'a' + 10 ;
	 else if(hexbyte >= '0' && hexbyte <= '9')
	 		 return hexbyte - '0'/*get the offset from ASCII*/ ;
	 else   /* any other ASCII code is INVALID in RSP*/
		  return DEBUG_ERR_INVALID_HEXChar;
  }

 /*
 *********************************************************************************************************
 *                                               Debug_Mem2Hex()
 *
 * Description :This function converts a memory range words into a HexChar-series buffer
 *
 * Argument(s) : start_address  : start address of memory range to be converted
 *               ByteCount      : Number of memory bytes to be converted
 *               HexBuffer      : pointer to buffer to store the converted Hex characters stream
 *
 *
 * Return(s)   : DEBUG_SUCCESS  if no error
 *               DEBUG_ERR_INOUTBUF_OVERFLOW    if  ByteCount exceeds HexBuffer size or encounter invalid hex character
 *
 *
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)HexBuffer pointer will not be advanced by ByteCount*2
 *
 *Example usage:  if  we are handling m packet , addr and len is the start address and length we parsed from packet fields, then
 *
 *  Debug_Mem2Hex(addr, len, &Debug_RSP_Payload_OutBuf[currentptr])
 *
 *   loads the output buffer with Hex characters converted from memory range addr to addr+len
 *    ------------------------------------------------------------------------
 *    |     |        |         |         |        |        |        |        |
 *    |  0  |    1   |    2    |    3    |   4    |   5    |   .... |    m   |   ]--->m bytes of memory
 *    ------------------------------------------------------------------------
 *    <-Lowest byte->                                       <-Highest-byte->
 *
 *    -----------------------------------------------------------------------------------------------
 *    |       |         |           |         |            |        |         |        |        |
 *    |  0    |     1   |     2     |    3    |      4     |   5    |    ...  |    n-1 |    n   |   ]---> n converted hex characters  n = m*2
 *    ----------------------------------------------------------------------------------------------
 *     0-HHB    0-LHB     1-HHB        1-LHB     2-HHB        2-LHB                m-HHB    m-LHB
 *     <-Lowest byte->                                                         <-Highest-byte->
 *
 *  HHB = High Half Byte
 *  LHB = Low Half Byte
 *
 *********************************************************************************************************
 */
static CPU_INT08U Debug_Mem2Hex(Debug_MemWidth *start_address, Debug_Len_t ByteCount,CPU_INT08U *HexBuffer)
 {
	/*use byte by byte access*/
	CPU_INT08U *MemBytePtr = (CPU_INT08U *) start_address ;
	CPU_INT08U Count = 0;
	CPU_INT08U HexChar =0 ;

	/*TODO::
	 * need to check the limits of the hex buffer
	 * */
	while( Count < ByteCount &&  Count < (Debug_RSP_IN_OUTBUFMax >> 1))    /*Count not exceed #bytes in Debug_RSP_IN_OUTBUFMax which is Debug_RSP_IN_OUTBUFMax/2 */
	        {
	        	     if( (HexChar = Debug_RSP_Byte2Hex((*(MemBytePtr)  >> 4) & 0xF) ) == DEBUG_ERROR )       /*convert High Half byte*/
	        	        break ;

	        	     *(HexBuffer++) = HexChar;

	        	     if( (HexChar = Debug_RSP_Byte2Hex(*(MemBytePtr++) & 0xF) ) == DEBUG_ERROR)             /*convert Low Half byte */
                          break;

	        	     *(HexBuffer++) =	 HexChar;
	                	 Count ++ ;
	        }
	        if(Count == ByteCount) /*We are done*/
	        	return DEBUG_SUCCESS;
    return DEBUG_ERR_INOUTBUF_OVERFLOW ; /*converted memory exceeds limits of out buffer*/

 }
/*
*********************************************************************************************************
*                                               Debug_Hex2Mem()
*
* Description :This function converts a  HexChar-series buffer into memory buffer of BYTES
*
* Argument(s) : HexBuffer          : pointer to buffer to get Hex character stream from
*               MemByteBuffer      : Temp storage used to store the memory BYTES converted
*
*               ByteCount          : number of Hex characters/bytes in HexBuffer
*
*
* Return(s)   : DEBUG_SUCCESS
*               DEBUG_ERR_INVALID_HEXChar
*
*
* Caller(s)   : Debug_RSP_Process_Packet()
*
* Note(s)     : (1)MemByteBuffer will be used by caller to store them in target memory
*
*Example usage:  if  we are handling M packet ,len is the length field in packet and Temp_membuf is a temporary memory buffer
*
*                and ptr is pointer to Debug_RSP_Payload_InBuf[currentptr] then
*
*  Debug_Hex2Mem(ptr, Temp_membuf, len)
*
*   loads the Temp_membuf with words converted from hex stream in Debug_RSP_Payload_InBuf
*    ------------------------------------------------------------------------
 *    |     |        |         |         |        |        |        |        |
 *    |  0  |    1   |    2    |    3    |   4    |   5    |   .... |    n   |   ]---> n hex characters seen as one n/2 bytes
 *    ------------------------------------------------------------------------
 *    <-Lowest byte->                                       <-Highest-byte->
 *
 *
 *
 *    ------------------------------------
 *    |                 |                |
 *    |       0         |      1         |  ]---> two hex characters seen as one byte
 *    ------------------------------------
 *    <-high half-byte-> <-low half-byte->
*********************************************************************************************************
*/
 static CPU_INT08U Debug_Hex2Mem(CPU_INT08U *HexBuffer,CPU_INT08U *MemByteBuffer, Debug_Len_t ByteCount)
 {
	 //CPU_INT08U BytePtr = (CPU_INT08U *)MemBuffer;
	 CPU_INT08U Count    = 0;
	 CPU_INT08U Byte     =  0;
	 CPU_INT08U LHByte     =  0;
    /*we can either choose a temp storage for Words/bytes converted
     * or
     * use the passed HexBuffer as a temp storage as it will not be after calling this function*/
	 //CPU_INT08U TempMemBuff [ByteCount];

	 while(Count < ByteCount)
	 {
		 /*TODO::
		  *  need to check for INVALID HEX chars*/
		if( (Byte = Debug_RSP_Hex2byte(*HexBuffer)) == DEBUG_ERR_INVALID_HEXChar)                /*High HalfByte*/
		  break;
		HexBuffer++;
		Byte <<= 4;

		if((LHByte = Debug_RSP_Hex2byte(*HexBuffer)) == DEBUG_ERR_INVALID_HEXChar)                   /*Low Half byte*/
			break;
		HexBuffer++;
		Byte                      |=  LHByte ;
	    *(MemByteBuffer)          = Byte ;
		 MemByteBuffer ++;
         Count++ ;
    }

	 if(Count == ByteCount)  /*we are done*/
		 return DEBUG_SUCCESS ;
	 return DEBUG_ERR_INVALID_HEXChar ;
 }

 /*
*********************************************************************************************************
*                                               Debug_Hex2Word()
*
* Description :This function converts a HexChar-stream into single word
*
* Argument(s) : HexStream    : pointer to  Hex characters stream
*               Word         : pointer to converted  Word
*
*
* Return(s)   : Number of hex character (Half Bytes) converted
*
*
* Caller(s)   : Debug_RSP_Process_Packet()
*
* Note(s)     :  (1) this functions does not support negative sign sent in packets like $Hc-1#
*                (2)HexStream is advanced by (#bytes converted *2)
*
*Example usage:  if  we are handling p packet ,we need to get the value of Register number from hex stream in input buffer ,
*
*     if ptr is pointer to Debug_RSP_Payload_InBuf[currentptr],then
*   Debug_Hex2Word(ptr, &wosrd)
*   parse the register number and put into word variable
*
*   if hex stream is " "  word =      --> to be calculated
*    ------------------------------------------------------------------------
*    |     |        |         |         |        |        |        |        |
*    |  0  |    1   |    2    |    3    |   4    |   5    |   6    |    7   |   ]---> 8 hex characters seen as one word(4-bytes)
*    ------------------------------------------------------------------------
*    <-highest byte->                                       <-lowest-byte->
*
*
*
*    ------------------------------------
*    |                 |                |
*    |       0         |      1         |  ]---> two hex characters seen as one byte
*    ------------------------------------
*    <-high half-byte-> <-low half-byte->
*********************************************************************************************************
*/
static CPU_INT08U  Debug_Hex2Word(CPU_INT08U **HexStream, Debug_MemWidth * word)
{


	// CPU_INT08U **Hexptr = &HexStream ;
		 *word = 0;
		 CPU_INT08U HalfByte;
		 CPU_INT08U half_bytecount = 0;
		 	while(**HexStream)   /*while there is hex chars parse them byte-by-byte and put them in word
		 	 TODO:: ensure this is valid for all fields that use this function*/
		 		 {
		 		 /*parse  byte byte*/
		 		/*TODO::
		 		 * check if the hex character is valid :not(G,H,I,J,K,L,..)*/
		 		if ((HalfByte  = Debug_RSP_Hex2byte(**HexStream)) == DEBUG_ERR_INVALID_HEXChar) /*invalid HEX*/
		 					    break ;

		 			(*HexStream)++;
		 			*word = *word << 4 | HalfByte;
		 			half_bytecount ++;
		 		 }

return (half_bytecount);
}
 /*
 *********************************************************************************************************
 *                                               Debug_Word2Hex()
 *
 * Description :This function converts a single word into HexChar-stream
 *
 * Argument(s) : Word         : pointer to Word to be converted
 *               address of pointer to HexStream    : pointer to converted Hex character stream
 *
 *
 *
 * Return(s)   : none
 *
 *
 * Caller(s)   : Debug_RSP_Process_Packet()
 *
 * Note(s)     : (1)HexStream is advanced by (#bytes converted *2)
 *
 *Example usage:  if  we are handling P packet , we need to convert the register value into hex and send it in out buffer
 *                and if ptr is pointer to Debug_RSP_Payload_OutBuf[currentptr], then
 *                Debug_Hex2Word(&word,ptr)
 *                convert register value into Hex and put into Debug_RSP_Payload_OutBuf
 *    if  word =  , hex stream is " "  --> to be calculated
 *
 *    ------------------------------------
 *    |     |        |         |         |
 *    |  0  |    1   |    2    |    3    |   ]---> 4 byte word
 *    ------------------------------------
 *
 *
 *    ------------------------------------------------------------------------
 *    |     |        |         |         |        |        |        |        |
 *    |  0  |    1   |    2    |    3    |   4    |   5    |   6    |    7   |   ]---> 8 hex characters seen as one word(4-bytes)
 *    ------------------------------------------------------------------------
 *     3-HHB   3-LHB    2-HHB     2-LHB     1-HHB    1-LHB   0-HHB     0-LHB
 *    <-highest byte->                                       <-lowest-byte->

 *    HHB = high half-byte
 *    LHB = low half-byte
 *
 *
 *********************************************************************************************************
 */
 static void Debug_Word2Hex(Debug_MemWidth * word, CPU_INT08U **HexStream)
 {
	 /*this functinos will not be used now*/
    /*cast to character pointer to parse memory byte-by-byte*/

	 CPU_INT08U *byteptr = (CPU_INT08U *)word;
     //CPU_INT08U **HEXPtr = &HexStream;
	 CPU_INT08U count = 0;
	 while(count < sizeof(Debug_MemWidth))/*one iteration  convert one byte */
	 {
         *((*HexStream)++) = Debug_RSP_Byte2Hex(((*byteptr) >> 4) & 0xf);    /*convert high halfbyte first*/
         *((*HexStream)++) = Debug_RSP_Byte2Hex((*byteptr++) & 0xf);           /*convert low halfbyte */
         count++;

	 }
 }

static void Debug_RSP_resetBuffer(CPU_INT08U * bufptr)
{
	CPU_INT32U count;
		         for (count = 0; count < Debug_RSP_IN_OUTBUFMax ; count++)
		        	 bufptr[count] = 0;


}
static CPU_INT08U IsBreakPointValid(CPU_INT08U * ptr,Debug_MemWidth * bPAddress,CPU_INT32U * bPLength,CPU_INT08U * bPType )
{
		//0   1 2 3 4 5
		//Z Type,Add,Kind    --> Type(1[HW],0[SW]) , Address , Kind(2,4)bytes which is the length of the break point
		//depending on the HW Architecture.
	CPU_INT08U *BPType=&ptr[1];
	CPU_INT08U *BPData=&ptr[2];// Address & Kind
	Debug_MemWidth Address;
	CPU_INT32U Length;

	  if (*(BPData++) != ',') {
		 // Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);//Bad Command Args
					return DEBUG_Bad_COMMAND_ARGS;
		 }
	  if (!Debug_Hex2Word(&BPData, &Address)) {
		  //Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS_Address /*Invalid Add*/);
	         return DEBUG_Bad_COMMAND_ARGS_Address;
	     }
	  if (*(BPData++) != ','||!Debug_Hex2Word(&BPData, &Length)) {
		 // Debug_RSP_Error_Packet(DEBUG_Bad_COMMAND_ARGS);
	 	         return DEBUG_Bad_COMMAND_ARGS_Address;
	 	     }
	  *bPAddress=Address;
	  *bPLength=Length;
	  *bPType=*BPType - 48;
	  return DEBUG_SUCCESS;

}
