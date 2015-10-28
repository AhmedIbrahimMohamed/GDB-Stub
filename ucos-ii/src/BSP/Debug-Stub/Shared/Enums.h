#ifndef Enums_H
#define Enums_H
enum BpType {
    BP_BREAKPOINT = 0,
    BP_HARDWARE_BREAKPOINT,
    BP_WRITE_WATCHPOINT,
    BP_READ_WATCHPOINT,
    BP_ACCESS_WATCHPOINT,
    BP_POKE_BREAKPOINT,
};

enum BpState {
    BP_UNDEFINED = 0,
    BP_REMOVED,
    BP_SET,
    BP_ACTIVE
};

enum BpLifeTime {
	BP_StubTemp,
	BP_UserPermenant
};
#endif

