#ifndef OPCODE_H
#define OPCODE_H

enum OpCodes {
    IADD,
    ISUB,
    IMUL,
    IDIV,
    IMOD,
    IEQ,
    INEQ,
    ILT,
    IGT,
    ILTE,
    IGTE,
    IAND,
    IOR,
    ICONST,
    POP,
    SYS_WRITE,
    STORE,
    GSTORE,
    LOAD,
    GLOAD,
    CHARCONST,
    HALT,
    JMP,
    JMPT,
    JMPN,
    CALL,
    RET
};

#endif //!OPCODE_H