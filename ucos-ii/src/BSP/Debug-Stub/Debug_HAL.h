/*
 * Debug_HAL.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */

#ifndef DEBUG_HAL_H_
#define DEBUG_HAL_H_

/*
 * Debug_RTOS.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include "Debug_Types.h"
#include "Debug_RTOS.h"
//#define Structures_IMPORT
#include "Shared/DataStructures.h"


/*
*********************************************************************************************************
 *                                                 EXTERNS
*********************************************************************************************************
*/
#ifdef Debug_HAL_IMPORT
	#define EXTERN
#else
	#define EXTERN extern
#endif

/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/

#define Debug_HAL_Regs_IDMAX    25                          /*Maximum ID for registers ,  CPSR*/
#define Debug_HAL_MEMWORDMAX   	50  						/*100 * 4 = 400  byte*/

/*instruction-related Decoding Configuration*/
#define Debug_HAL_INST_COND_BM   0xF0000000                 /*BitMask/BitPosition for instruction's condition code*/
#define Debug_HAL_INST_COND_BP   28

/*CPSR Bit Masks and Bit Positions*/
#define CPSR_N_BM  0x80000000
#define CPSR_Z_BM  0x40000000
#define CPSR_C_BM  0x20000000
#define CPSR_V_BM  0x10000000
#define CPSR_N_BP   31
#define CPSR_Z_BP   30
#define CPSR_C_BP   29
#define CPSR_V_BP   28

/*instruction's Bit Masks and Bit Positions*/
#define Debug_HAL_INST_CLASS_BM            0x0C000000
#define Debug_HAL_INST_CLASS_BP              26

/*Instruction Class*/
#define INST_CLASS_DataProcess             0x00
#define INST_CLASS_LD_STR                  0x01
#define INST_CLASS_Branch                  0x02
#define INST_CLASS_COP_SVC                 0x03


/*Instruction SubClasses*/

/*1- Data Processing SubClasses*/
#define DP_Registers_BM                0x0E000010
#define DP_Registers                   0x00000000

#define DP_RegShiftedReg_BM            0x0E000090
#define DP_RegShiftedReg               0x00000010

#define DP_Immediate_BM                0x0E000000
#define DP_Immediate                   0x02000000

#define DP_MUL_ACC_BM                  0x0F0000F0
#define DP_MUL_ACC                     0x00000090

#define DP_Satu_ADDSUB_BM              0x0F9000F0
#define DP_Satu_ADDSUB                 0x01000050



#define DP_HW_MUL_ACC_BM               0x0F900090
#define DP_HW_MUL_ACC                  0x01000080

#define DP_EXTRA_LDSTR_BM               0x0E000090
#define DP_EXTRA_LDSTR                  0x00000090

#define DP_EXTRA_LDSTR_UN_BM            0x0F200090
#define DP_EXTRA_LDSTR_UN               0x00200090

#define DP_SYNC_BM                     0x0F0000F0
#define DP_SYNC                        0x01000090

#define DP_MSR_Immediate_BM            0x0FB00000
#define DP_MSR_Immediate               0x03200000

#define DP_MISC_BM                     0x0F900080
#define DP_MISC                        0x01000000


/*2- Load/Store Subclasees*/

#define LDSTR_WB_BM                   0x0E000010
#define LDSTR_WB_Encode_A0            0x00000000
#define LDSTR_WB_Encode_A1            0x02000000
#define LDSTR_WB_Media                0x06000010

/*Media subclasses*/
//TODO::page : 210 to 213
#define LDSTR_WB_Media_SubClasses_BM                    0x01800000
#define LDSTR_WB_Media_SubClasses_BP					23
#define LDSTR_WB_Media_ParallelAddSub_BM                0x0FC000010
#define LDSTR_WB_Media_ParallelAddSub_Sign              0x060000010
#define LDSTR_WB_Media_ParallelAddSub_Unsign            0x064000010

#define LDSTR_WB_Media_PackUn_Sat_Rev_MUL_DIV_BM        0x0F8000010
#define LDSTR_WB_Media_PackUn_Sat_Rev                   0x068000010
#define LDSTR_WB_Media_MUL_DIV                          0x070000010



/*3- Branch and Block Data Transfer class has no subclass */

/*4- Coprocessor/SVC class has no subclass*/
#define Debug_HAL_COP_SVC_UNDEFINED_BM    0x0FE00000
#define Debug_HAL_COP_SVC_UNDEFINED       0x0C000000

#define Debug_HAL_COP_SVC_SVC_BM          0x03000000
#define Debug_HAL_COP_SVC_SVC             0x03000000
/*Instruction's target address and PC position in instruction */
#define PC_RegList_BM   0x00008000        //[15]
#define PC_RegSpec1_BM  0x00000F00        //[8-11]
#define PC_RegSpec2_BM  0x0000F000        //[12-15]
#define PC_RegSpec2_BP  12
#define PC_RegSpec3_BM  0x000F0000        //[16-19]
#define Debug_HAL_INST_PC_ID   0xF


/*other instruction fields Bitmasks*/
#define Instruction_Rm_BM   0x0000000F
#define Instruction_Rn_BM   0x000F0000
#define Instruction_Rn_BP   16

#define Instruction_Rd_BM   0x0000F000  //[12-15]
#define Instruction_Rd_BP   12

#define Instruction_Rt_BM   0x0000F000
#define Instruction_Rt_BP   12

#define Instruction_Rt2_BM  0x000F0000
#define Instruction_Rt2_BP   16

#define Instruction_Rs_BM  0x00000F00
#define Instruction_ShiftType_BM   0x00000060
#define Instruction_ShiftType_BP   5

//Shift Types
#define Shift_LSL      0x00
#define Shift_LSR      0x01
#define Shift_ASR      0x02
#define Shift_ROR      0x03
#define Shift_RRX      0x04


#define Instruction_imm5_BM   0x00000F80
#define Instruction_imm5_BP   7

#define Instruction_imm12_BM  0x00000FFF

#define Instruction_imm24_BM  0x00FFFFFF


/*Instruction Groups for DP-register Subclass*/
#define DP_Registers_XOperands_BM         0x01E00000
#define DP_Registers_XOperands_BP         21

#define DP_Registers_1_Operands_BM       0x01E00FE0


#define DP_Registers_TST_CMP_BM         0x01900000

/*SVC/BKPT instruction configuration*/
#define Excep_Vector_Base                0x00100000
#define Excep_Vector_UNDEFINED_Offset     0x04
#define Excep_Vector_SVC_Offset           0x08
#define Excep_Vector_Prefetch_Offset      0x0C


		/*
		****************************************************************************************
		*                                       Registers' CONFIGURATION
		*****************************************************************************************
		*/

#define  Debug_HAL_Regs_GPNUM    17  										 /*as in org.gnu.gdb.arm.core feature of GDB standard features*/
#define  Debug_HAL_Regs_FPNUM	 9    										/*as in org.gnu.gdb.arm.fpa feature of GDB standard features*/
#define  Debug_HAL_Regs_MAX     (Debug_HAL_Regs_GPNUM + Debug_HAL_Regs_FPNUM)
#define  Debug_HAL_Regs_BYTES   (Debug_HAL_Regs_MAX <<2 )



/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


EXTERN Debug_RegisterWidth Debug_HAL_RegsBuffer[Debug_HAL_Regs_MAX]
#ifdef Debug_HAL_IMPORT
                        = {0}
#endif
;
EXTERN Debug_MemWidth      Debug_HAL_MemoryBuffer[Debug_HAL_MEMWORDMAX]
#ifdef Debug_HAL_IMPORT
                        = {0}
#endif
;
#if GDB_HAVE_SWBP
EXTERN struct Gdb_Bkpt		Gdb_BreakList[GDB_MAX_BREAKPOINTS]
#ifdef Debug_HAL_IMPORT


         = {
    [0 ... GDB_MAX_BREAKPOINTS-1] = { .state = BP_REMOVED }
}
#endif
;
#endif

/*Signals numbering as defined by GDB source code in <GDB-SourceTree>/include/gdb/gdb.def*/
enum {

	SIGINT  = 2,  /*sent when receiving ctrl+c from gdb host!!*/
	SIGILL  = 4,
	SIGTRAP,/*5*/
	SIGABRT,/*6*/
	SIGBUS = 10

}Debug_HAL_Sig_GDBNumbers;

/*Registers' order in RSP packet as defined by GDB standard feature org.gnu.gdb.arm.core , org.gnu.gdb.arm.fpa */

 enum {
	R0_Offset,
	R1_Offset,
	R2_Offset,
	R3_Offset,
	R4_Offset,
	R5_Offset,
	R6_Offset,
	R7_Offset,
	R8_Offset,
	R9_Offset,
	R10_Offset,
	R11_Offset,
	R12_Offset,
	SP_Offset,
	LR_Offset,
	PC_Offset,   /*15*/
	F0_Offset,
	F1_Offset,
	F2_Offset,
	F3_Offset,
	F4_Offset,
	F5_Offset,
	F6_Offset,
	F7_Offset,
	FPS_Offset,
	CPSR_Offset /*25*/

} Debug_HAL_Regs_GDBOrder;

/*Condition Codes For ARM instructions*/
enum {
	INST_COND_EQ,
	INST_COND_NE,
	INST_COND_CS,
	INST_COND_CC,
	INST_COND_MI,
	INST_COND_PL,
	INST_COND_VS,
	INST_COND_VC,
	INST_COND_HI,
	INST_COND_LS,
	INST_COND_GE,
	INST_COND_LT,
	INST_COND_GT,
	INST_COND_LE,
	INST_COND_AL


}Debug_HAL_INST_CondCodes;




//EXTERN Debug_HAL_Regs_GDBOrder GDB_RegOrder;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
EXTERN void Debug_HAL_init(void);

EXTERN void Debug_HAL_Regs_Readall(Debug_TID_t ThreadID);
EXTERN void Debug_HAL_Regs_Writeall(Debug_TID_t ThreadID);

EXTERN CPU_INT08U Debug_HAL_Regs_ReadOne(Debug_TID_t ThreadID, Debug_RegID_t regno, Debug_RegisterWidth *RegValue );
EXTERN CPU_INT08U Debug_HAL_Regs_WriteOne(Debug_TID_t ThreadID, Debug_RegID_t regno, Debug_RegisterWidth RegValue);
EXTERN void Debug_HAL_Get_StopSignal(Debug_TID_t ThreadID,Debug_Signal_t *signo);

EXTERN CPU_INT08U Debug_HAL_INST_Is_Condition_True(CPU_INT32U Instruction,Debug_TID_t ThreadID);
EXTERN Debug_MemWidth *Debug_HAL_INST_Get_Target_Address(CPU_INT32U Instruction);
EXTERN Debug_MemWidth *Debug_HAL_UNCONDINST_Get_Target_Address(CPU_INT32U Instruction);
EXTERN CPU_INT32S InsertBpInsideBplist(Debug_MemWidth *bPAddress,enum BpLifeTime);
EXTERN CPU_INT32U Activate_Sw_BreakPoints();
EXTERN CPU_INT32U Gdb_Arch_Set_BreakPoint(unsigned long bPAddress,char * savedInst);
EXTERN CPU_INT32U BreakPointMemWrite(void * srcAddress,void  * desAddress,unsigned int size);
EXTERN CPU_INT08U Deactivate_SW_BreakPoints();
EXTERN CPU_INT32U Gdb_Arch_Remove_BreakPoint(unsigned long bPAddress,char * savedInst);
EXTERN Debug_MemWidth Debug_HAL_INST_Get_Target_UNCOND_Class(Debug_MemWidth Instruction);
EXTERN CPU_INT08U Debug_HAL_GetBkPTID_ByAddress(CPU_INT32S BKPT_address);

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


#undef Debug_HAL_IMPORT

#undef EXTERN

#endif /* DEBUG_HAL_H_ */
