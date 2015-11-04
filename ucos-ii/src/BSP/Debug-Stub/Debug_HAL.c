/*
 * Debug_HAL.c
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
*                                        BOARD SUPPORT PACKAGE
*
*                                         Xilinx Zynq XC7Z020
*                                               on the
*
*                                             Xilinx ZC702
*                                          Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define Debug_HAL_IMPORT
#include "Debug_HAL.h"



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

 _gdb_arch arch_gdb_ops = {
	//.gdb_bpt_instr		=  {0xfe, 0xde, 0xff, 0xe7}  //Little-Endian
	/*this is not a "BKPT" , what is this */
		 .gdb_bpt_instr		=  {0x70, 0x00, 0x20, 0xe1} //BKPT
};

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
 static Debug_MemWidth Get_Target_DP_Class(CPU_INT32U Instruction);

 static Debug_MemWidth Get_Target_LDSTR_Class(CPU_INT32U Instruction);

 static Debug_MemWidth Get_Target_Branch_Class(CPU_INT32U Instruction);

 static Debug_MemWidth Get_Target_COPSVC_Class(CPU_INT32U Instruction);

 static Debug_MemWidth Get_Target_LDSTR_Media(CPU_INT32U Instruction);
 static CPU_INT32U Shift_Shift_C(CPU_INT32U operand, CPU_INT08U shift_t, CPU_INT08U shift_n);
 static CPU_INT08U Count_NumRegsLoaded( CPU_INT16U CPU_RegList);


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
*                                               void Debug_HAL_init()
*
* Description :
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : .
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/

void Debug_HAL_init(void)
{

	/*Initialize Registers Buffer*/
	CPU_INT08U RegOffset;
	for(RegOffset =R0_Offset ; RegOffset< Debug_HAL_Regs_IDMAX ; RegOffset++)
		Debug_HAL_RegsBuffer[RegOffset] = 0;

}


/*
*********************************************************************************************************
*                                       Register Functions
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               void Debug_HAL_Regs_Readall()
*
* Description :Reads target registers that GDB needs in <g> command packet . Registers are Thread-specific and is stored in Debug_HAL_RegsBuffer[]
*
* Argument(s) : ThreadID : thread id for which this function read its register context.
*
* Return(s)   : none.
*
* Caller(s)   : Debug_Main_Read_Registers() in Debug_Main Module
*
* Note(s)     : (1) ThreadID could be thread priority , thread ID  depending on the target system(RTOS)
*
*               (2)refer to OSTaskStkInit() in os_cpu_c.c to know how registers are stored in task stack.
*********************************************************************************************************
*/

void Debug_HAL_Regs_Readall(Debug_TID_t ThreadID)
{

	/*ThreadID here means priority*/

	CPU_INT08U count ;
	//Debug_TID_t current_thread = current_thread_OF_Focus;

	Debug_TCB *Deubg_TCBCurr = (Debug_TCB *)Debug_RTOS_ThreadListPtr[ThreadID] ; /*get a pointer to the thread stack from the Task Control Block */
	Debug_RegisterWidth *Debug_SP = Deubg_TCBCurr->OSTCBStkPtr;

	/***************Note : Debug_SP is incremented or decremented based on stack-growth setting"OS_STK_GROWTH macro" in os_cpu.h *********************/
	Debug_HAL_RegsBuffer[SP_Offset] = Debug_SP;                                        /*Get the SP*/
	Debug_HAL_RegsBuffer[CPSR_Offset] = *(Debug_SP++);                                 /*Get the CPSR*/

	for(count = R0_Offset; count <= R12_Offset;count++)                                    /*get R0-R12*/
		Debug_HAL_RegsBuffer[count] = *(Debug_SP++);

	Debug_HAL_RegsBuffer[LR_Offset] = *(Debug_SP++);                                       /*Get R14=LR , R13=SP is skipped
	                                                                                     as it is not stored in task stack*/

	Debug_HAL_RegsBuffer[PC_Offset] = *(Debug_SP);                                       /*Get R15=PC */


}

/*
*********************************************************************************************************
*                                                Debug_HAL_Regs_Writeall()
*
* Description : Writes target registers with GDB values in <G> command packet . Registers values
*               are Thread-specific and is stored in Debug_HAL_RegsBuffer[]
*
* Argument(s) : ThreadID : thread id for which this function modify its register context.
*
* Return(s)   : none.
*
* Caller(s)   : Debug_Main_Write_Registers() in Debug_Main Module
*
* Note(s)     : (1)ThreadID could be thread priority , thread ID  depending on the target system(RTOS)
*
*               (2)refer to OSTaskStkInit() in os_cpu_c.c to know how registers are stored in task stack.
*********************************************************************************************************
*/
void Debug_HAL_Regs_Writeall(Debug_TID_t ThreadID)
{
	CPU_INT08U count = 0 ;
	//Debug_TID_t current_thread = current_thread_OF_Focus;

	Debug_TCB *Deubg_TCBCurr = (Debug_TCB *)Debug_RTOS_ThreadListPtr[ThreadID] ; /*get a pointer to the thread stack from the Task Control Block */
	Debug_RegisterWidth *Debug_SP = Deubg_TCBCurr->OSTCBStkPtr;

	/***************Note : Debug_SP is incremented or decremented based on stack-growth setting"OS_STK_GROWTH macro" in os_cpu.h *********************/


	*(Debug_SP++) = Debug_HAL_RegsBuffer[CPSR_Offset];                                 /*set the CPSR*/

	for(count = R0_Offset ; count <= R12_Offset ; count++)                                    /*set R12-R0*/
		*(Debug_SP++) = Debug_HAL_RegsBuffer[count] ;

	Deubg_TCBCurr->OSTCBStkPtr = Debug_HAL_RegsBuffer[SP_Offset];                                 /*set the R13 = SP*/

	*(Debug_SP++) = Debug_HAL_RegsBuffer[LR_Offset] ;                                       /*set R14=LR , R13=SP is skipped
		                                                                                     as it is not stored in task stack*/

	*(Debug_SP++) = Debug_HAL_RegsBuffer[PC_Offset]  ;                                       /*set R15=PC */






}

/*
*********************************************************************************************************
*                                                Debug_HAL_Regs_ReadOne()
*
* Description :Read one register from Thread register context
*
* Argument(s) : ThreadID : thread id for which this function get  register regno.
*               regno    : Register ID as specified by GDB in <p> command packet.
*               RegValue : The returned/read register value.
*
* Return(s)   : DEBUG_SUCCESS : if no error
*               DEBUG_ERR_INVALID_REGID if register ID "regno" is not valid.
*
* Caller(s)   :
*
* Note(s)     : (1)Debug_Main_Read_Register() in Debug_Main module
*
*               (2)
*********************************************************************************************************
*/
CPU_INT08U Debug_HAL_Regs_ReadOne(Debug_TID_t ThreadID,Debug_RegID_t regno ,Debug_RegisterWidth *RegValue )
{

	/***************Note : Debug_SP is incremented or decremented based on stack-growth setting"OS_STK_GROWTH macro" in os_cpu.h *********************/

	/*
	*********************************************************************************************************
	*                                             1st Method to read one register
	*********************************************************************************************************
	*/
	if(regno > Debug_HAL_Regs_IDMAX)
		return DEBUG_ERR_INVALID_REGID;

	Debug_TCB *Deubg_TCBCurr = (Debug_TCB *)Debug_RTOS_ThreadListPtr[ThreadID] ; /*get a pointer to the thread stack from the Task Control Block */
		Debug_RegisterWidth *Debug_SP = Deubg_TCBCurr->OSTCBStkPtr;

	if (regno == CPSR_Offset)   /*CPSR*/
		 *RegValue = *(Debug_SP) ;

	else if (regno >= R0_Offset && regno <= R12_Offset )
		*RegValue = *(Debug_SP +(regno+1));
	else if (regno == SP_Offset)
		*RegValue = Debug_SP;
	else if(regno == LR_Offset || regno == PC_Offset)
		*RegValue = *(Debug_SP + regno);

	/*Floating point register IDs are ignored and they are always 0's*/

	    /*
		*********************************************************************************************************
		*                                            2nd Method to read one register
		*********************************************************************************************************
		*/

	/*just read all registers and return the needed one*/

	/*Debug_HAL_Regs_Readall(ThreadID);
	*RegValue = Debug_HAL_RegsBuffer[regno];
*/
	return DEBUG_SUCCESS;

}

/*
*********************************************************************************************************
*                                                Debug_HAL_Regs_WriteOne()
*
* Description :Write one register to Thread register context
*
* Argument(s) : ThreadID : thread id for which this function set register regno.
*               regno    : Register ID as specified by GDB in <P> command packet.
*               RegValue : The register value to be written to thread register regno.
*
* Return(s)   : DEBUG_SUCCESS : if no error
*               DEBUG_ERR_INVALID_REGID if register ID "regno" is not valid.
*
* Caller(s)   :
*
* Note(s)     : (1)Debug_Main_Write_Register() in Debug_Main module
*
*               (2)
*********************************************************************************************************
*/



CPU_INT08U Debug_HAL_Regs_WriteOne(Debug_TID_t ThreadID, Debug_RegID_t regno, Debug_RegisterWidth RegValue)
{

	if(regno > Debug_HAL_Regs_IDMAX)
			return DEBUG_ERR_INVALID_REGID;

	/***************Note : Debug_SP is incremented or decremented based on stack-growth setting"OS_STK_GROWTH macro" in os_cpu.h *********************/

	/*
	*********************************************************************************************************
	*                                             1st Method to write one register
	*********************************************************************************************************
	*/


	Debug_TCB *Deubg_TCBCurr = (Debug_TCB *)Debug_RTOS_ThreadListPtr[ThreadID] ; /*get a pointer to the thread stack from the Task Control Block */
	Debug_RegisterWidth *Debug_SP = Deubg_TCBCurr->OSTCBStkPtr;
	if (regno == CPSR_Offset)   /*CPSR*/
		*(Debug_SP) = RegValue;

	else if (regno >= R0_Offset && regno <= R12_Offset )
		*(Debug_SP +(regno+1)) = RegValue;
	else if (regno == SP_Offset)
		Deubg_TCBCurr->OSTCBStkPtr = RegValue;
	else if(regno == LR_Offset || regno == PC_Offset)
		*(Debug_SP + regno) = RegValue;

	/*Floating point register IDs are ignored and they are always 0's*/


	/*
	*********************************************************************************************************
	*                                             2nd Method to write one register
	*                                               read-modify-write sequence
	*********************************************************************************************************
	*/


	/*Read all registers*/
/*
	 Debug_HAL_Regs_Readall(ThreadID);
	Modify the required register in register buffer
	 Debug_HAL_RegsBuffer[regno] = RegValue ;
	 Write back the register buffer
	 Debug_HAL_Regs_Writeall(ThreadID) ;
*/

	return DEBUG_SUCCESS;

	}

/*
*********************************************************************************************************
*                                                Debug_HAL_Get_StopSignal()
*
* Description :

*
* Argument(s) : ThreadID :
*               signo    :
*
*
* Return(s)   :
*
* Caller(s)   :  Debug_Main_Report_Halt_Reason() in Debug_Main Module
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/
void Debug_HAL_Get_StopSignal(Debug_TID_t ThreadID,Debug_Signal_t *signo)
{
	 /*For this implementation , we only support one signal which is a breakpoint signal SIGTRAP*/
	*signo = SIGTRAP;

}

/*
*********************************************************************************************************
*                                                Debug_HAL_INST_Is_Condition_True()
*
* Description : This Function returns if the input instruction is executed or not through inspecting
*               condition field in instruction
* Argument(s) : Instruction : the instruction to be checked for execution
*
*
*
* Return(s)   : 1 : if instruction is conditional and executable
* 				0 : if instruction is conditional but not executable
* 				Debug_Instruction_UNCOND : if instruction is unconditional
*
* Caller(s)   :  Debug_Main_Step_machine_instruction() in Debug_Main Module
*
* Note(s)     : (1)this function is very architecture-version specific
*
*               (2)refer to conditional execution section in A8.Instruction Details in ARM Reference Manual
*********************************************************************************************************
*/

CPU_INT08U Debug_HAL_INST_Is_Condition_True(CPU_INT32U Instruction,Debug_TID_t ThreadID)
{

	Debug_RegisterWidth Thread_CPSR;

	/**Condition Field is in bits [31-28]*/
	CPU_INT08U condition = ( (CPU_INT32U)Instruction & Debug_HAL_INST_COND_BM) >> Debug_HAL_INST_COND_BP ;

	/*Get CPSR of ThreadID context*/
		Debug_HAL_Regs_ReadOne(ThreadID,CPSR_Offset,&Thread_CPSR);

	switch (condition )
     {
        case INST_COND_EQ:
        	if (Thread_CPSR & CPSR_Z_BM )  /*Z == 1*/
        		return 1;
        	break;

        case INST_COND_NE:
        	if (!(Thread_CPSR & CPSR_Z_BM )  )   /*Z == 0*/
        		return 1;
        	break;

        case INST_COND_CS:
           if (Thread_CPSR & CPSR_C_BM )   /*C == 1*/
        		return 1;
        	break;

        case INST_COND_CC:
        	if (!(Thread_CPSR & CPSR_C_BM )  )   /*C == 0*/
        		return 1;
        	break;


        case INST_COND_MI:
        	if (Thread_CPSR & CPSR_N_BM )   /*N == 1*/
        		return 1;
        	break;

        case INST_COND_PL:
        	if (!(Thread_CPSR & CPSR_N_BM )  )   /*N == 0*/
        		return 1;
        	break;

        case INST_COND_VS:
        	if (Thread_CPSR & CPSR_V_BM )   /*V == 1*/
        		return 1;
        	break;

        case INST_COND_VC:
        	if (!(Thread_CPSR & CPSR_V_BM )  )   /*V == 0*/
        		return 1;
        	break;

        case INST_COND_HI:
        	if ( (Thread_CPSR & CPSR_C_BM )  && !(Thread_CPSR & CPSR_Z_BM )  )   /*C == 1 andZ=0*/
        		return 1;
        	break;

        case INST_COND_LS:
        	if ( !(Thread_CPSR & CPSR_C_BM )  || (Thread_CPSR & CPSR_Z_BM )  )   /*C==0 or Z == 1*/
        		return 1;
        	break;

        case  INST_COND_GE:
        	if ( ((Thread_CPSR & CPSR_N_BM ) >> CPSR_N_BP )  == ( (Thread_CPSR & CPSR_V_BM ) >> CPSR_V_BP) )   /*N == V*/
        		return 1;
        	break;

        case INST_COND_LT:
        	if ( ((Thread_CPSR & CPSR_N_BM ) >> CPSR_N_BP )  != ( (Thread_CPSR & CPSR_V_BM ) >> CPSR_V_BP) )   /*N != V*/
        		return 1;
        	break;

        case INST_COND_GT:
        	if ( (((Thread_CPSR & CPSR_N_BM ) >> CPSR_N_BP )  == ( (Thread_CPSR & CPSR_V_BM ) >> CPSR_V_BP)  ) && !(Thread_CPSR & CPSR_Z_BM ))   /*Z == 0 and N==V*/
        		return 1;
        	break;

        case INST_COND_LE:
        	if ( ( ((Thread_CPSR & CPSR_N_BM ) >> CPSR_N_BP )  != ( (Thread_CPSR & CPSR_V_BM ) >> CPSR_V_BP)  ) || (Thread_CPSR & CPSR_Z_BM ))   /*Z == 1 or N != V*/
        		return 1;
        	break;

        case INST_COND_AL: /*Any*/
        		return 1;
        	break;

        default :  /* 0x1111 Unconditional_Instruction*/
        	//TODO::
        	return Debug_Instruction_UNCOND; /*to be parsed as unconditional instructions group*/
        	break;
     }//switch
	return 0;  /*instruction is conditional but will not execute*/
}

/*
*********************************************************************************************************
*                                                Debug_HAL_INST_Get_Target_Address()
*
* Description :This Function calculates target Program Counter(PC) register when input Instruction executes.
*			   It decodes instructions according to their wide class
*
* Argument(s) : Instruction : instruction to be decoded
*
*
*
* Return(s)   :  Target Address of new value of PC
*
* Caller(s)   :  Debug_Main_Step_machine_instruction() in Debug_Main Module
*
* Note(s)     : (1)refer to section ARM instruction set encoding in ARM Reference manual to know high-level instruction classes
*
*               (2)
*********************************************************************************************************
*/

CPU_INT32U *Debug_HAL_INST_Get_Target_Address(CPU_INT32U Instruction)
{

	Debug_MemWidth *Target_Address;

	/*get instruction class value bits [27-26]*/
	CPU_INT08U Inst_Class = (Instruction & Debug_HAL_INST_CLASS_BM) >> Debug_HAL_INST_CLASS_BP;

	switch (Inst_Class)
	{
	case INST_CLASS_DataProcess:
		Target_Address = Get_Target_DP_Class(Instruction);
		break;
	case INST_CLASS_LD_STR:
		Target_Address = Get_Target_LDSTR_Class(Instruction);
			break;
	case INST_CLASS_Branch:
		Target_Address = Get_Target_Branch_Class(Instruction);
			break;
	case INST_CLASS_COP_SVC:
		Target_Address = Get_Target_COPSVC_Class(Instruction);
			break;


	}

	return Target_Address;
}

/*
*********************************************************************************************************
*                                                Get_Target_DP_Class()
*
* Description :This function decode instruction of class Data-Processing  conditional instructions
*

*
* Argument(s) : Instruction :"Data-Processing class" instruction to be decoded
*
*
*
* Return(s)   : The Decoded Target Address
*               0 if Program Counter is not affected or instruction is invalid
*
* Caller(s)   :   Debug_HAL_INST_Get_Target_Address() in Debug HAL Module
*
* Note(s)     : (1)refer to "Data-processing and miscellaneous instructions" section in ARM Reference Manual
*
*               (2)
*********************************************************************************************************
*/

static Debug_MemWidth Get_Target_DP_Class(CPU_INT32U Instruction)
{


	/*get Rm[3-0] operand shared between instructions*/

	CPU_INT32U Rm = Debug_HAL_RegsBuffer[Instruction & Instruction_Rm_BM];
	/*TODO:: Do we need to check if Rm is PC ,too.. to make a  PC prefetch*/

	CPU_INT32U shifted;
	Debug_MemWidth Target_address;

	/*Get Rn value TODO::Do we need PC prefetch */
	CPU_INT32U Rn  =  Debug_HAL_RegsBuffer[(Instruction & Instruction_Rn_BM) >>Instruction_Rn_BP];

                 /*Decode different sub-classes inside Data-Processing Class*/
	/*Combine 7 subclasses in this big if-condition*/
	if( ((Instruction & DP_Registers_BM) == DP_Registers) || ((Instruction & DP_RegShiftedReg_BM) == DP_RegShiftedReg) || ((Instruction & DP_Immediate_BM) == DP_Immediate)  )
	{
		 if((Instruction & DP_MISC_BM) == DP_MISC) /*Extract Miscellaneous instructions as it conflicts with register_shift-register instructions bitmask*/
			{
			    if((Instruction & DP_Satu_ADDSUB_BM) == DP_Satu_ADDSUB)/*Extract Saturating addition and subtraction instructions as it conflicts with Miscellaneous bitmask*/
				   {return 0;}

				switch (Instruction & 0x00600070)/*Mask op and op2 in Miscellaneous subclass*/
				{
					case 0x00200010:  //BX
					case 0x00200030: //BLX(Register)
						/*TODO:: should we check if Rm is PC*/
						return Rm;
						break;
					case 0x00600060://ERET
						/*ALthough this instruction should not be in application code but it'll be weakly handled here,*/
						/*target address is in LR register*/
						return Debug_HAL_RegsBuffer[LR_Offset];
						break;
					case 0x00200070: //BKPT
						/*Breakpoint causes a Prefetch Abort Exception */
						return Excep_Vector_Base + Excep_Vector_Prefetch_Offset;
						break;
				}

			}

		 if((Instruction & DP_HW_MUL_ACC_BM) == DP_HW_MUL_ACC)/*Extract Halfwordmultiply and multiply accumulate instructions as it conflicts with (registers) instruction bitmask*/
		 	{return 0;}
		 if((Instruction & DP_MSR_Immediate_BM) == DP_MSR_Immediate)/*Extract MSR_immediate instructions as it conflic with immediate instructions bitmask*/
		 	{return 0;}

			/*continue with register/shift-register/immediate instructions*/

		 /*Continue decoding if target register Rd[12-15] is PC*/
		if( ( (Instruction & Instruction_Rd_BM) >> Instruction_Rd_BP ) == Debug_HAL_INST_PC_ID)
		{
			/*if PC is source register Rn*/
			if ( ((Instruction & Instruction_Rn_BM) >>Instruction_Rn_BP) == Debug_HAL_INST_PC_ID)
				Rn += 8; /*get PC value at prefetch stage*/

			CPU_INT08U imm5Shift = 0;
			CPU_INT08U shift_type =  (Instruction & Instruction_ShiftType_BM) >>Instruction_ShiftType_BP ;

			/*tune imm5Shift to correct values in case of logical/arithmetic shift right*/
			if( (shift_type == Shift_LSR) || ( shift_type == Shift_ASR) )
			{
				if ( (Instruction & Instruction_imm5_BM) == 0 )
					imm5Shift = 32 ;
				else
					imm5Shift = (CPU_INT32U)(Instruction & Instruction_imm5_BM) >> Instruction_imm5_BP;

			}
			shifted  = ( !(Instruction & 0x00000010) ) * ( !(Instruction & 0x02000000) ) *Shift_Shift_C(Rm,shift_type , (CPU_INT32U)imm5Shift) /*DP_Registers*/
					   +( (Instruction & 0x00000010 ) ) * ( !(Instruction & 0x02000000) ) * Shift_Shift_C(Rm,shift_type , (CPU_INT32U)( (CPU_INT08U)Debug_HAL_RegsBuffer[(Instruction & Instruction_Rs_BM) >> Instruction_Rs_BM]) )/*DP_RegShiftedReg : shift count in Rs[0-7]*/
					   +( (Instruction & 0x02000000)  * Shift_Shift_C( (CPU_INT32U)(Instruction & 0x000000FF),Shift_ROR,((Instruction & 0x00000F00) >> 8)<< 1)) ;/*DP_Immediate :Shift count is imm12[0-7] ROR (2*imm12[8-11]*/

			/*Check instruction Groups
			 * groups are constructed according to # operands .there are instructions with 3 operands , 2 operands and one operand
			 * */

			/*Mask Bits[21-24]*/
			/*1)  3 operands instructions(Rn,Rm,optional_Imm5) */
			if( ((Instruction & DP_Registers_XOperands_BM) >> DP_Registers_XOperands_BP) != 0x0D )
			{
				/* Exclude only one instruction  here that take two_operand(Rm,immediate) --> MVN(register)*/
               if((Instruction & DP_Registers_XOperands_BM) == DP_Registers_XOperands_BM )
            	   return ~shifted;

                // instruction group that take 3 operand (Rn,Rm,optional_Imm5)
            	   /*Exception : TST,CMP Instruction not affecting PC*/
            	   if( (Instruction & DP_Registers_TST_CMP_BM) == 0x01100000 )
            		   return 0;

            	   switch((Instruction & 0x01E00000)>> 21)//check "op" field but [21-24]
            	   {
            	      case 0x0://AND
        	    	  return Rn & shifted;
            	    	  break;
            	      case 0x1://EOR
            	    	  return Rn ^ shifted;
            	    	  break;

            	      case 0x2://SUB
            	    	//  if ((Instruction & Instruction_Rn_BM) == Debug_HAL_INST_PC_ID) //ADR-Encoding A2
						  //{
							//  /*TODO::should PC here be prefetch i.e. PC= PC+8*/
							  //return (Debug_HAL_RegsBuffer[PC_Offset] & 0xFFFFFFF6) - shifted;
						  //}
            	    	 //return Rn + (~shifted) + 1;
            	    	  return (CPU_INT32U)Rn - shifted;
            	            break;
            	      case 0x3://RSB
						 // return  (~Rn) + shifted + 1;
            	    	  return  shifted - (CPU_INT32U)Rn;
							break;
            	      case 0x4://ADD
            	    	  //if ((Instruction & Instruction_Rn_BM) == Debug_HAL_INST_PC_ID) //ADR-Encoding A1
							//  {
            	    	//	  /*TODO::should PC here be prefetch i.e. PC= PC+8*/
            	    	  //       return (Debug_HAL_RegsBuffer[PC_Offset] & 0xFFFFFFF6) + shifted /*+ 8*/;
							//  }
							  return  Rn + shifted ;
            	      		break;
            	      case 0x5://ADC
						 return  Rn + (Debug_HAL_RegsBuffer[CPSR_Offset] & 0x20000000)+ shifted ;
							break;
            	      case 0x6://SBC
						 return  Rn + (Debug_HAL_RegsBuffer[CPSR_Offset] & 0x20000000)+ ~shifted ;
							break;
            	      case 0x7://RSC
						 return  ~Rn + (Debug_HAL_RegsBuffer[CPSR_Offset] & 0x20000000)+ shifted ;
							break;
            	      case 0xC://ORR
						 return  Rn | shifted ;
							break;
            	      case 0xE://BIC
						 return Rn & (~shifted);
							break;

            	   }//switch op in 3-operands instructions

			} //instruction group with 3-operands


			/*Handle instructions with op[21-24] = 1101*/

			/*2) 1-operand instruction group (MOV(rgister,ARM) ,RRX */
			else if( ( (Instruction & DP_Registers_1_Operands_BM) == 0x01E00000) ||  ( (Instruction & DP_Registers_1_Operands_BM) == 0x1E00060) )
			{
				/*special case*/
				if ((Instruction & 0x00000060) == 0x00000060) //RRX
				{
					return Shift_Shift_C(Rm,Shift_RRX,1);
				}
                 /*Rm is the target address*/
				return Rm;
			}

			else  /* 3) 2-operands instruction group (Rm,Immediate)*/
			{
				/*LSL,LSR,ASR,ROR*/
				return shifted;
			}
		}//PC is Destination Register
	}// seven Sub Class handling:: DP_Registers,DP_RegShiftedReg, DP_Immediate, DP_COP_SVC, DP_HW_MUL_ACC, DP_MSR_Immediate_BM, DP_MISC

	else if((Instruction & DP_MUL_ACC_BM) == DP_MUL_ACC)
	{}
	else if((Instruction & DP_EXTRA_LDSTR_BM) == DP_EXTRA_LDSTR)
	{}
	else if((Instruction & DP_EXTRA_LDSTR_UN_BM) == DP_EXTRA_LDSTR_UN)
	{
		 if((Instruction & DP_SYNC_BM) == DP_SYNC)
			{return 0;}
	}

	else /*Should it be here undefined instruction*/
		{
		//TODO:: handle odd cases , just return 0 not BAD instruction
			//return DEBUG_Bad_Instruction;
			return 0;
		}
return 0;
}

/*
*********************************************************************************************************
*                                                Get_Target_LDSTR_Class()
*
* Description :This function decode instruction of class Load/Store conditional instructions
*

*
* Argument(s) : Instruction :"Load/Store class" instruction to be decoded
*
*
*
* * Return(s)   : The Decoded Target Address
*               0 if Program Counter is not affected or instruction is invalid
*
*
* Caller(s)   :   Debug_HAL_INST_Get_Target_Address() in Debug HAL Module
*
* Note(s)     : (1)refer to "Load/store word and unsigned byte" section in ARM Reference Manual
*
*               (2)
*********************************************************************************************************
*/
static Debug_MemWidth Get_Target_LDSTR_Class(CPU_INT32U Instruction)
{
	CPU_INT32U Rn = (Instruction & Instruction_Rn_BM) >>Instruction_Rn_BP;
	CPU_INT32U * address =0;
	CPU_INT32U  word =0;
	CPU_INT32U offset;


	/*Continue decoding if  target register Rt[12-15] is the Program counter*/
	if( ( (Instruction & Instruction_Rt_BM) >> Instruction_Rt_BP ) == Debug_HAL_INST_PC_ID)
	{
		if(Rn == Debug_HAL_INST_PC_ID)/*in case of LDR/STR(literal)*/
				Rn = Debug_HAL_RegsBuffer[Rn] + 8;  /*PC at prefetch stage in pipeline*/
		else
			Rn = Debug_HAL_RegsBuffer[Rn];
		/*here , we decode only "Load word and unsigned byte" instructions, Excluding  Media-subclass/store instructions*/
		if ( ((Instruction & LDSTR_WB_BM) != LDSTR_WB_Media) && (((Instruction & 0x00500000) == 0x00100000) || ((Instruction & 0x00500000) == 0x00500000)) )
			{
				switch (Instruction & 0x02000000) /*Check if it is encoding-0 or Encoding-1 --> via A bit [25]*/
				{
						case LDSTR_WB_Encode_A0:/*using immediate,literal*/
							offset = (Instruction & Instruction_imm12_BM);
							break;

						case LDSTR_WB_Encode_A1:/*using shifted-register*/
							offset = (CPU_INT32U)Shift_Shift_C (
									                               (CPU_INT32U)Debug_HAL_RegsBuffer[Instruction & Instruction_Rm_BM] ,
									                              ((Instruction & Instruction_ShiftType_BM)>> Instruction_ShiftType_BP),
									                              ((Instruction & Instruction_imm5_BM)>> Instruction_imm5_BP)
									                              );
							break;

						default:
							break;
				}//switch

				/*
				 * Index Bit - P at 24
				 *               0--> address = Rn
				 *               1 --> address = Rn -/+ immediate
				 * Add bit - U at 23
				 *            if 1 --> Rn + immediate
				 *            else0  --> Rn - immediate*/

				if(Instruction & 0x00800000) /*if U is set*/
					address = Rn + ( (Instruction & 0x01000000)/*if P is set, add offset*/ *offset );
				else
					address = Rn - ( (Instruction & 0x01000000) *offset );

				/*Check if it is byte-access or word access*/

				if ( (Instruction & 0x00500000) == 0x00500000)/*if it is byte access*/
							word = (CPU_INT32U) ( (CPU_INT08U)(*address) );

				else /*if it is word access*/
					word = *address;

				return word;
				}/*if we are not Media instruction and not store instruction */

		else if((Instruction & LDSTR_WB_BM) == LDSTR_WB_Media) /*if it is Media instruction*/
			return Get_Target_LDSTR_Media(Instruction);
	}/*if register is PC*/
return 0;
}
/*
*********************************************************************************************************
*                                                Get_Target_Branch_Class()
*
* Description :This function decode instruction of class Branch/Block-Transfer conditional instructions
*

*
* Argument(s) : Instruction :"Branch/Block-Transfer class" instruction to be decoded
*
*
*
* Return(s)   : The Decoded Target Address
*               0 if Program Counter is not affected or instruction is invalid
*
* Caller(s)   :   Debug_HAL_INST_Get_Target_Address() in Debug HAL Module
*
* Note(s)     : (1)refer to "Branch, branch with link,and block data transfer" section in ARM Reference Manual
*
*               (2)
*********************************************************************************************************
*/
static Debug_MemWidth Get_Target_Branch_Class(CPU_INT32U Instruction)
{
	/*TODO::should we get PC-prefetch*/

	/*Extract Branching instructions via bit[25]
	 *
	 * Is it Branching(B,BLX(immediate) or LD/POP)*/
	if (Instruction & 0x02000000)
	{
		/*B or BLX(immediate)*/
		/*targetAddress = Align(PC,4) + SignExtend(imm24:'00', 32);
		 * SignExtend(x, i) = Replicate(TopBit(x), i-Len(x)) : x
		 * */

		CPU_INT32U offset = ( (CPU_INT32U)(Instruction & Instruction_imm24_BM) << 2);
		if(Instruction & 0x00800000)
			offset |= 0xFC000000; /*replicate Top-Bit of imm24 bit (32total-26shifted imm24)times*/

		return ( Debug_HAL_RegsBuffer[PC_Offset] & 0xFFFFFFF6 ) + 8/*for prefetch stage */ + offset ;
	}
		/*LDMxx/POP/STMxx/PUSH*/

		/*
		 * Extract load/pop instructions via bit [20]
		 * bit[20] = 0 Store/Push
		 * bit[20] = 1  Load/Pop
		 * */
	if( (Instruction & PC_RegList_BM) && (Instruction & 0x00100000))/*Is PC in Reg_List and we are in Load/POP not store/push, this excludes LDM(user register) too*/
		{

			CPU_INT32U Rn = Debug_HAL_RegsBuffer[(Instruction &Instruction_Rn_BM) >> Instruction_Rn_BP];

			if(((Instruction & Instruction_Rn_BM) >>Instruction_Rn_BP) == Debug_HAL_INST_PC_ID)/*in case of LDR(literal)*/
						Rn += 8;  /*PC at prefetch stage in pipeline*/

			/*Calculate how many register are to be loaded including PC*/
			CPU_INT08U regs_Number_loaded = Count_NumRegsLoaded((CPU_INT16U)(Instruction & 0x0000FFFF));

			CPU_INT32U *address ;

			/*Do we increment or decrement*/
			//if((Instruction & 0x00C00000) == 0x00800000)/*increment*/
			if((Instruction & 0x02C00000) == 0x00800000) /*Increment*/
			{
				address = Rn + (4*regs_Number_loaded) - (4*( (Instruction & 0x03400000) == 0x00000000));/*subtract 4 if increment after*/
				/*(!(Instruction & 0x01000000)) subtract 4 if increment after*/
			}
			//if((Instruction & 0x00C00000) == 0x00000000)/*Decrement*/
			else if((Instruction & 0x02C00000) == 0x00000000)/*Decrement*/
			{
				address = Rn - (4* ((Instruction & 0x03400000) == 0x01000000) );/*subtract 4 if decrement before*/
				/*(Instruction & 0x01000000) subtract 4 if decrement before*/
			}
	        else /*LDM(Exception Return)*/
				{
	        	  address = Rn - (4*regs_Number_loaded *(!(Instruction & 0x00800000)) ) /*subtract if U-bit[23] is cleared*/
	        	      + (4*((Instruction & 0x00800000) ==(Instruction & 0x01000000))) ; /*add 4 if P-bit[24]==U-bit*/

				}
			return (*address);

		}//if it is load/pop and PC is in reglist

	return 0;
}

/*
*********************************************************************************************************
*                                                Get_Target_COPSVC_Class()
*
* Description :This function decode instruction of class "Coprocessor instructions, and Supervisor Call" conditional instructions
*

*
* Argument(s) : Instruction :"Coprocessor instructions, and Supervisor Call class" instruction to be decoded
*
*
*
*  Return(s)   : The Decoded Target Address
*               0 if Program Counter is not affected or instruction is invalid
*
* Caller(s)   :   Debug_HAL_INST_Get_Target_Address() in Debug HAL Module
*
* Note(s)     : (1)refer to "Coprocessor instructions, and Supervisor Call" section in ARM Reference Manual
*
*               (2)
*********************************************************************************************************
*/
static Debug_MemWidth Get_Target_COPSVC_Class(CPU_INT32U Instruction)
{
	/*is it SVC call*/
	if ( (Instruction & 0x03000000) == 0x03000000 )
	{
		return Excep_Vector_Base + Excep_Vector_SVC_Offset ;/*SVC entry address in Vector table*/
	}

	else if ((Instruction & Debug_HAL_COP_SVC_UNDEFINED_BM) == Debug_HAL_COP_SVC_UNDEFINED)
	{/*TODO:: Deal with Explicit undefined instruction*/ }
	return 0;
}

/*
*********************************************************************************************************
*                                                Get_Target_LDSTR_Media()
*
* Description :This function decode instruction of class "Media instructions" conditional instructions
*

*
* Argument(s) : Instruction :"Media"  instruction to be decoded
*
*
*
*  Return(s)   : The Decoded Target Address
*               0 if Program Counter is not affected or instruction is invalid
*
* Caller(s)   :   Get_Target_LDSTR_Class() in Debug HAL Module
*
* Note(s)     : (1)refer to "Media instructions" section in ARM Reference Manual
*
*********************************************************************************************************
*/
static Debug_MemWidth Get_Target_LDSTR_Media(CPU_INT32U Instruction)
{

	switch( (Instruction & LDSTR_WB_Media_SubClasses_BM) >> LDSTR_WB_Media_SubClasses_BP)
	{

		case 0x00:/*Parallel addition and subtraction, signed/unsigned*/
			if(Instruction & 0x00400000)/*Parallel addition and subtraction, unsigned*/
			{}
			else/*Parallel addition and subtraction, signed*/
			{}
			break;
		case 0x01:/*Packing, unpacking, saturation, and reversal*/
			break;
		case 0x02:/*Signed multiply, signed and unsigned divide*/
			break;
		default:
			/*TODO:: should it be undefined instruction*/
			break;
	}
return 0;
}
/*
*********************************************************************************************************
*                                                Shift_Shift_C()
*
* Description : This Function apply all shift types to specific register passed as operand input
*
*
* Argument(s) : operand : the register value on which function will apply shift-type
*               shift_t : type of shift operation to be applied, defined in Debug_HAL.h
*                			Shift_LSL   Logical Shift Left
*				 			Shift_LSR   Logical Shift Right
*				 			Shift_ASR   Arithmetic Shift Right
*				 			Shift_ROR   Rotate Right
*				 			Shift_RRX   Rotate Right one bit with eXtend
*               shift_n : Shift count
* Return(s)   : the shifted register value
*
* Caller(s)   : Get_Target_DP_Class() and Get_Target_LDSTR_Class() in Debug_HAL Module
*
* Note(s)     : none
*********************************************************************************************************
*/
static CPU_INT32U Shift_Shift_C(CPU_INT32U operand, CPU_INT08U shift_t, CPU_INT08U shift_n)
{

	CPU_INT08U Shift_n_moduluo =  shift_n%32;
	if (!shift_n)
		return operand;

	switch (shift_t)
	{
	case Shift_LSL:
		return operand << shift_n;
		break;
	case Shift_LSR:
		return operand >> shift_n;
		break;
	case Shift_ASR:
		return (operand >>shift_n) | (operand & 0x80000000);
		break;
	case Shift_ROR:
		return (operand >> Shift_n_moduluo) | (operand << (32 -Shift_n_moduluo ));
		break;
	case Shift_RRX:
		return (operand >> 1) | ( (Debug_HAL_RegsBuffer[CPSR_Offset] & 0x20000000) << 1) ;
		break;


	}
	return 0;
}
/*
*********************************************************************************************************
*                                                Count_NumRegsLoaded()
*
* Description : In general ,  this Function calculates number of bits 1s in input value.
*               Specifically, it is used to calculate number of registers loaded/stored in register_list in Load/Store-multiple instructions.
*
* Argument(s) : CPU_RegList : in general, it is main input
*                             specifically ,it is the Register_list field in Load/Store-multiple instructions
*
*
*
* Return(s)   : number of bits set in passed input
*
* Caller(s)   : Get_Target_Branch_Class() in Debug_HAL Module
*
* Note(s)     : none
*********************************************************************************************************
*/
static CPU_INT08U Count_NumRegsLoaded( CPU_INT16U CPU_RegList)
{
	CPU_INT08U count = 0;
	 while(CPU_RegList)
	  {
		count += CPU_RegList & 1;
		CPU_RegList >>= 1;
	  }
	  return count;

}

/*
*********************************************************************************************************
*                                                Debug_HAL_INST_Get_Target_UNCOND_Class()
*
* Description : This Function calculates target Program Counter(PC) register when input Instruction executes. Instruction is of class "Unconditional Instructions" group
*
* Argument(s) : Instruction : the instruction to be decoded
*
*
*
* Return(s)   : Target PC value
*               0 : if PC will not be affected
*
*
* Caller(s)   :  Debug_Main_Step_machine_instruction() in Debug_Main Module
*
* Note(s)     : (2)refer to "Unconditional instructions" section in A5.ARM Instruction Set Encoding in ARM Reference Manual
*********************************************************************************************************
*/

 Debug_MemWidth Debug_HAL_INST_Get_Target_UNCOND_Class(CPU_INT32U Instruction)
{

	CPU_INT32U Rn = Debug_HAL_RegsBuffer[(Instruction &Instruction_Rn_BM) >> Instruction_Rn_BP];
	CPU_INT32U *address;

	/*check each instruction*/

	if((Instruction & 0x0E500000) == 0x08100000)  /*Is it RFE */
	{

		/**/
		address = Rn
				     -( 8 * (!(Instruction & 0x00800000)) )/*subtract 8 if U bit[23] is cleared*/
				     + (4 * ( (Instruction & 0x00800000) == (Instruction & 0x01000000) ) );/*add 4 if P bit[24] == U bit*/
	return (*address);
	}

	if((Instruction & 0x0E000000) == 0x0A000000)/*Is it BL,BLX */
	{

		/*B or BLX(immediate)*/
				/*targetAddress = Align(PC,4) + SignExtend(imm24:'00', 32);
		 * SignExtend(x, i) = Replicate(TopBit(x), i-Len(x)) : x
		 * */
		CPU_INT32U offset = ( (CPU_INT32U)(Instruction & Instruction_imm24_BM) << 2);
		if(Instruction & 0x00800000)
			offset |= 0xFC000000; /*replicate Top-Bit of imm24 bit (32total-26shifted imm24)times*/

		return ( Debug_HAL_RegsBuffer[PC_Offset] & 0xFFFFFFF6 ) + 8/*for prefetch stage */ + offset ;
	}

	if((Instruction & 0x0FF00000) == 0x0C500000)/*Is it MRRC,MRRC2 */
	{
		/*Is PC is any of destination registers ? */
		if( ((Instruction & Instruction_Rt_BM)>>Instruction_Rt_BP) == Debug_HAL_INST_PC_ID)
		{

		}
		else if ( ((Instruction & Instruction_Rt2_BM) >> Instruction_Rt2_BP) == Debug_HAL_INST_PC_ID)
		{

		}

	}

	if((Instruction & 0x0F100010) == 0x0E100010)/*Is it MRC,MRC2 */
	{
			/*not affecting PC*/
	}
	return 0;
}



 /*Ahmed Functions*/
/*TODO:: Need Commenting , reviewing */

/*
*********************************************************************************************************
*                                               InsertBpInsideBplist()
*
* Description :
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Debug_Main_InsertBreakPoint() and Debug_Main_Step_machine_instruction
*
* Note(s)     : (1)
*
*               (2)
*********************************************************************************************************
*/

 CPU_INT32S InsertBpInsideBplist(Debug_MemWidth *bPAddress,enum BpLifeTime lifetime)
{
	int breakno = -1;
	    int i;

	    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
	        if ((Gdb_BreakList[i].state == BP_SET) &&
	                (Gdb_BreakList[i].bpt_addr == (*bPAddress)))
	            return -1;
	    }
	    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
	        if (Gdb_BreakList[i].state == BP_REMOVED) {
	            breakno = i;
	            break;
	        }
	    }

	    if (breakno == -1) {
	        for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
	            if (Gdb_BreakList[i].state == BP_UNDEFINED) {
	                breakno = i;
	                break;
	            }
	        }
	    }
	    if (breakno == -1)
	           return -1;

	    Gdb_BreakList[breakno].state = BP_SET;
	    Gdb_BreakList[breakno].type = BP_BREAKPOINT;
	    Gdb_BreakList[breakno].bpt_addr = (*bPAddress);
	    Gdb_BreakList[breakno].lifetime = lifetime;
	return DEBUG_SUCCESS;
}

 /*
 *********************************************************************************************************
 *                                               Gdb_Arch_Remove_BreakPoint()
 *
 * Description :
 *
 * Argument(s) : none.
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


CPU_INT32U Gdb_Arch_Remove_BreakPoint(unsigned long bPAddress,char * savedInst)
{
	//BreakPointMemWrite()
	int err;

	    err = BreakPointMemWrite((void *)bPAddress, (void *)savedInst, BREAK_INSTR_SIZE);
		if (err)
			return err;

	    return 0;
}


/*
 *********************************************************************************************************
 *                                               Deactivate_SW_BreakPoints()
 *
 * Description :
 *
 * Argument(s) : none.
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
CPU_INT08U Deactivate_SW_BreakPoints()
{
	unsigned long addr;
	    int error = 0;
	    int i;

	    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
	        if (Gdb_BreakList[i].state != BP_ACTIVE)
	            continue;
	        addr = Gdb_BreakList[i].bpt_addr;
	        error = Gdb_Arch_Remove_BreakPoint(addr,(char *)(Gdb_BreakList[i].saved_instr));
	        if (error)
	            return error;

	        extern void Xil_ICacheInvalidateLine(unsigned int adr);
	       	        Xil_ICacheInvalidateLine(addr);
	        if(Gdb_BreakList[i].lifetime == BP_UserPermenant)
	       	        Gdb_BreakList[i].state = BP_SET;
	        else
       	        Gdb_BreakList[i].state = BP_REMOVED;
	    }
	return 0;
}



/*
 *********************************************************************************************************
 *                                               BreakPointMemWrite()
 *
 * Description :
 *
 * Argument(s) : none.
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

CPU_INT32U BreakPointMemWrite(void * srcAddress,void  * desAddress,unsigned int size)
{
		int i = 0;
	    char *dst_ptr = (char *)desAddress;
	    char *src_ptr = (char *)srcAddress;
	    for (i = 0; i<size; i++) {
	           *(dst_ptr++) = *(src_ptr++);
	       }
	    //TODO: Check Memory Fault handler

	return 0;
}

/*
 *********************************************************************************************************
 *                                               Gdb_Arch_Set_BreakPoint()
 *
 * Description :
 *
 * Argument(s) : none.
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
CPU_INT32U Gdb_Arch_Set_BreakPoint(unsigned long bPAddress,char * savedInst)
{
	//BreakPointMemWrite()
	int err;

	    err = BreakPointMemWrite((void *)bPAddress, (void *)savedInst, BREAK_INSTR_SIZE);
		if (err)
			return err;

	    return BreakPointMemWrite( (void *)arch_gdb_ops.gdb_bpt_instr,(void *)bPAddress,
					  BREAK_INSTR_SIZE);
}
/*
 *********************************************************************************************************
 *                                               Activate_Sw_BreakPoints()
 *
 * Description :
 *
 * Argument(s) : none.
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
CPU_INT32U Activate_Sw_BreakPoints()
{
	 unsigned long addr;
	    int error = 0;
	    int i;

	    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
	        if (Gdb_BreakList[i].state !=BP_SET)
	            continue;

	        addr = Gdb_BreakList[i].bpt_addr;
	        error = Gdb_Arch_Set_BreakPoint(addr,
	                (char *)(Gdb_BreakList[i].saved_instr));
	        if (error)
	            return error;
	        //TODO: invalidateIcash first
	        extern void Xil_ICacheInvalidateLine(unsigned int adr);
	        Xil_ICacheInvalidateLine(addr);
	        Gdb_BreakList[i].state = BP_ACTIVE;
	    }
	    return 0;
	/*Debug_RSP_Info_mM Command_opts_mM;
	Debug_Main_Write_memory()*/
}
