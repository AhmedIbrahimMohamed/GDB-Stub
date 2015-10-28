#ifndef CONFIG_H
#define CONFIG_H
////////////////////////////
#define BREAK_INSTR_SIZE	4
#define GDB_BREAKINST		0xe7ffdefe
#ifndef GDB_MAX_BREAKPOINTS
    #define GDB_MAX_BREAKPOINTS 20
#else
    #define GDB_MAX_BREAKPOINTS  GDB_MAX_BREAKPOINTS
#endif
#ifndef GDB_HAVE_SWBP
    #define GDB_HAVE_SWBP 1
#endif
////////////////////////////
#endif
