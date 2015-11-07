#ifndef DataStructure_H
#define DataStructure_H

#include"Enums.h"
#include"config.h"




struct Gdb_Bkpt {
    unsigned long		bpt_addr;
    unsigned char		saved_instr[BREAK_INSTR_SIZE];
    enum BpType	type;
    enum BpState	state;
    enum BpLifeTime lifetime;
};
/**
 * struct gdb_arch - Describe architecture specific values.
 * @gdb_bpt_instr: The instruction to trigger a breakpoint.
 * @flags: Flags for the breakpoint, currently just %GDB_HW_BREAKPOINT.
 * @set_hw_breakpoint: Allow an architecture to specify how to set a hardware
 * breakpoint.
 * @remove_hw_breakpoint: Allow an architecture to specify how to remove a
 * hardware breakpoint.
 * @remove_all_hw_break: Allow an architecture to specify how to remove all
 * hardware breakpoints.
 */
typedef struct gdb_arch {
	unsigned char		gdb_bpt_instr[BREAK_INSTR_SIZE];
	unsigned long		flags;

	int	(*set_hw_breakpoint)(unsigned long, int, enum BpType);
	int	(*remove_hw_breakpoint)(unsigned long, int, enum BpType);
	void	(*remove_all_hw_break)(void);
}_gdb_arch;
extern  _gdb_arch arch_gdb_ops;
/*AhmedIbrahim:2015-09-14
 * This struct is specified based on the HW architecture on how to set and remove HW break point and also
 * break point instruction.
 * */

#endif
