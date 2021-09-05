/**
 * @file vm.c
 * @author strah19
 * @date June 16 2021
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * This file contains code for running our Pine bytecode from a file.
 */

#include "../include/vm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define DUMP_BYTECODE

struct OpcodeInfo {
    const char* name;
    uint32_t num_args;
};

struct OpcodeInfo opcode_debug_info[256];

struct OutputInfo {
    struct Object o;
    int loc;
};

#define MAX_OUTPUT 1000

struct OutputInfo output[MAX_OUTPUT];
int output_index = 0;

struct OpcodeInfo create_opcode_info(const char* name, uint32_t num_args) {
    struct OpcodeInfo info;
    info.name = name;
    info.num_args = num_args;

    return info;
}

void fatal_runtime_error(const char* error_msg) {
    fprintf(stderr, "\nRuntime Error: %s.\n", error_msg);
    exit(EXIT_FAILURE);
}

struct VMStack vm_create_stack(int size) {
    struct VMStack stack;

    stack.top = 0;
    stack.size = size;
    stack.stack = (struct Object*)malloc(sizeof(struct Object) * size);

    return stack;
}

struct VM create_vm(uint32_t data_size, uint32_t main) {
    struct VM vm;
    vm.stack = vm_create_stack(1028);
    vm.ip = main;
    vm.data = malloc(sizeof(struct Object) * data_size);
    memset(vm.data, 0, sizeof(struct Object) * data_size);
    vm.data_size = data_size;
    vm.fp = 0;

    return vm;
}

struct Object obj_push(int32_t val) {
    struct Object o;
    o.i32 = val;
    return o;
} 

int vm_push_stack(struct VMStack* stack, struct Object object) {
    stack->stack[stack->top++] = object;
    return stack->top;
}

struct Object vm_pop_stack(struct VMStack* stack) {
    if (stack->top > 0)
        return stack->stack[--(stack->top)];
    else
        fatal_runtime_error("Cannot pop stack that is already empty");
}

struct Object vm_peek_stack(struct VMStack* stack) {
    return stack->stack[stack->top];
}

typedef void (*instruction)(struct VM* vm);

void op_nop(struct VM* vm) {
    vm->ip++;
}

void op_charconst(struct VM* vm) {
    struct Object o;
    o.type = CHARCONST;

    o.u8 = (char) vm->opcodes[vm->ip + 1];
    vm_push_stack(&vm->stack, o);
    vm->ip += 2;
}

void op_iconst(struct VM* vm) {
    struct Object o;

    o.type = ICONST;
    o.i32 = vm->opcodes[vm->ip + 1];
    vm_push_stack(&vm->stack, o);

    vm->ip += 2; 
}

void operate_on_operands(struct VM* vm, char operator) {
    struct Object o2 = vm_pop_stack(&vm->stack);
    struct Object o1 = vm_pop_stack(&vm->stack);

    struct Object result;
    result.type = o2.type;  

    switch (operator) {
        case '+': result.i32 = o1.i32 +  o2.i32; break;
        case '-': result.i32 = o1.i32 -  o2.i32; break;
        case '*': result.i32 = o1.i32 *  o2.i32; break;
        case '/': result.i32 = o1.i32 /  o2.i32; break;
        case '%': result.i32 = o1.i32 %  o2.i32; break;
        case '=': result.i32 = (o1.i32 == o2.i32) ? 1 : 0; break;
        case '!': result.i32 = (o1.i32 != o2.i32) ? 1 : 0; break;
        case '<': result.i32 = (o1.i32 <  o2.i32) ? 1 : 0; break;
        case '>': result.i32 = (o1.i32 >  o2.i32) ? 1 : 0; break;
        case 'l': result.i32 = (o1.i32 <= o2.i32) ? 1 : 0; break;
        case 'g': result.i32 = (o1.i32 >= o2.i32) ? 1 : 0; break; 
        case 'a' : result.i32 = (o1.i32 && o2.i32) ? 1 : 0; break;
        case 'o' : result.i32 = (o1.i32 || o2.i32) ? 1 : 0; break;
    default:
        break;
    }
    
    vm_push_stack(&vm->stack, result);
    vm->ip++;
}

void op_iadd(struct VM* vm) {
    operate_on_operands(vm, '+');
}

void op_isub(struct VM* vm) {
    operate_on_operands(vm, '-');
}

void op_imul(struct VM* vm) {
    operate_on_operands(vm, '*');
}

void op_idiv(struct VM* vm) {
    operate_on_operands(vm, '/');
}

void op_imod(struct VM* vm) {
    operate_on_operands(vm, '%');
}

void op_ieq(struct VM* vm) {
    // '=' is actually perform a compariative equal on the two operands
    operate_on_operands(vm, '=');
}

void op_ineq(struct VM* vm) {
    operate_on_operands(vm, '!');
}

void op_ilt(struct VM* vm) {
    operate_on_operands(vm, '<');
}

void op_igt(struct VM* vm) {
    operate_on_operands(vm, '>');
}

void op_ilte(struct VM* vm) {
    //Use a l because <= is 2 chars but don't want to do a str compare, just a simple switch
    operate_on_operands(vm, 'l');
}

void op_igte(struct VM* vm) {
    //Use a g because >= is 2 chars but don't want to do a str compare, just a simple switch
    operate_on_operands(vm, 'g');
}

void op_iand(struct VM* vm) {
    operate_on_operands(vm, 'a');
}

void op_ior(struct VM* vm) {
    operate_on_operands(vm, 'o');
}

void print_data(struct Object* o) {
    switch (o->type) {
    case ICONST:
        printf("%d", o->i32);
        break;
    case CHARCONST:
        printf("%c", (char) o->u8);
        break;
    }
}

void op_syswrite(struct VM* vm) {
    struct Object o = vm_pop_stack(&vm->stack);
    //print_data(&o);
    
    if (output_index == MAX_OUTPUT)
        output_index = 0;

    output[output_index].loc = vm->ip;
    output[output_index++].o = o;
    vm->ip++;
}

//g_load will push whatever variable data onto the stack
void op_gload(struct VM* vm) {
    if (vm->data_size <= vm->opcodes[vm->ip + 1]) {
        printf("Data retrieval of %d out of bounds: cannot get data that does not exist.\n", vm->opcodes[vm->ip + 1]);
        exit(EXIT_FAILURE);
    }

    uint32_t addr = vm->opcodes[vm->ip + 1];
    struct Object o = vm->data[addr];

    vm_push_stack(&vm->stack, o);

    vm->ip += 2;
}

//g_store will pop whats on stack and put in variable
void op_gstore(struct VM* vm) {
    if (vm->data_size <= vm->opcodes[vm->ip + 1]) {
        printf("\nData retrieval of %d out of bounds: cannot get data that does not exist.\n", vm->opcodes[vm->ip + 1]);
        exit(EXIT_FAILURE);
    }

    struct Object o = vm_pop_stack(&vm->stack);
    uint32_t addr = vm->opcodes[vm->ip + 1];
    vm->data[addr] = o;
    vm->ip += 2;
}

void op_load(struct VM* vm) {
    vm->ip++;
    int32_t offset = vm->opcodes[vm->ip++];
    vm_push_stack(&vm->stack, vm->stack.stack[(vm->fp - 1) + offset]);
}

void op_store(struct VM* vm) {
    vm->ip++;
    int32_t offset = vm->opcodes[vm->ip++];
    vm->stack.stack[(vm->fp - 1) + offset] = vm_pop_stack(&vm->stack);
}

void op_jmp(struct VM* vm) {
    vm->ip = vm->opcodes[vm->ip + 1];
}

void op_jmpt(struct VM* vm) {
    struct Object o = vm_pop_stack(&vm->stack);
    if (o.i32) vm->ip = vm->opcodes[vm->ip + 1];
    else vm->ip += 2;
}

void op_jmpn(struct VM* vm) {
    struct Object o = vm_pop_stack(&vm->stack);
    if (!o.i32) vm->ip = vm->opcodes[vm->ip + 1];
    else vm->ip += 2;
}

void op_pop(struct VM* vm) {
    vm_pop_stack(&vm->stack);
    vm->ip++;
}

void op_call(struct VM* vm) {
    struct Object address;
    struct Object num_args;

    vm->ip++;
    address.i32 = vm->opcodes[vm->ip];
    vm->ip++;
    num_args.i32 = vm->opcodes[vm->ip];

    vm_push_stack(&vm->stack, num_args);

    vm_push_stack(&vm->stack, obj_push(vm->fp));
    vm_push_stack(&vm->stack, obj_push(vm->ip));

    vm->fp = vm->stack.top;
    vm->ip = address.i32;
}

void op_ret(struct VM* vm) {
    struct Object ret = vm_pop_stack(&vm->stack);

    vm->stack.top = vm->fp;
    vm->ip = vm_pop_stack(&vm->stack).i32;
    vm->fp = vm_pop_stack(&vm->stack).i32;
    int32_t args = vm_pop_stack(&vm->stack).i32;
    vm->stack.top -= args;
    vm_push_stack(&vm->stack, ret);

    vm->ip++;
}

static instruction ops[256];
static struct VM vm;

void init_vm() {
    for(int i = 0; i < 256; i++) 
        ops[i] = op_nop;

    ops[CHARCONST] = op_charconst;
    opcode_debug_info[CHARCONST] = create_opcode_info("CHARCONST", 1);

    ops[SYS_WRITE] = op_syswrite;
    opcode_debug_info[SYS_WRITE] = create_opcode_info("SYS_WRITE", 0);

    ops[ICONST] = op_iconst;
    opcode_debug_info[ICONST] = create_opcode_info("ICONST", 1);

    ops[POP] = op_pop;
    opcode_debug_info[POP] = create_opcode_info("POP", 0);

    ops[IADD] = op_iadd;
    opcode_debug_info[IADD] = create_opcode_info("IADD", 0);

    ops[ISUB] = op_isub;
    opcode_debug_info[ISUB] = create_opcode_info("ISUB", 0);
    
    ops[IMUL] = op_imul;
    opcode_debug_info[IMUL] = create_opcode_info("IMUL", 0);
    
    ops[IDIV] = op_idiv;
    opcode_debug_info[IDIV] = create_opcode_info("IDIV", 0);
    
    ops[IMOD] = op_imod;
    opcode_debug_info[IMOD] = create_opcode_info("IMOD", 0);
    
    ops[IEQ] = op_ieq;
    opcode_debug_info[IEQ] = create_opcode_info("IEQ", 0);
    
    ops[INEQ] = op_ineq;
    opcode_debug_info[INEQ] = create_opcode_info("INEQ", 0);

    ops[ILT] = op_ilt;
    opcode_debug_info[ILT] = create_opcode_info("ILT", 0);

    ops[IGT] = op_igt;
    opcode_debug_info[IGT] = create_opcode_info("IGT", 0);

    ops[IGTE] = op_igte;
    opcode_debug_info[IGTE] = create_opcode_info("IGTE", 0);

    ops[ILTE] = op_ilte;
    opcode_debug_info[ILTE] = create_opcode_info("ILTE", 0);

    ops[IAND] = op_iand;
    opcode_debug_info[IAND] = create_opcode_info("IAND", 0);

    ops[IOR] = op_ior;
    opcode_debug_info[IOR] = create_opcode_info("IOR", 0);
    
    ops[GLOAD] = op_gload;
    opcode_debug_info[GLOAD] = create_opcode_info("GLOAD", 1);
    
    ops[GSTORE] = op_gstore;
    opcode_debug_info[GSTORE] = create_opcode_info("GSTORE", 1);
    
    ops[JMP] = op_jmp;
    opcode_debug_info[JMP] = create_opcode_info("JMP", 1);
    
    ops[JMPT] = op_jmpt;
    opcode_debug_info[JMPT] = create_opcode_info("JMPT", 1);
    
    ops[JMPN] = op_jmpn;
    opcode_debug_info[JMPN] = create_opcode_info("JMPN", 1);
    
    ops[CALL] = op_call;
    opcode_debug_info[CALL] = create_opcode_info("CALL", 2);
    
    ops[RET] = op_ret;
    opcode_debug_info[RET] = create_opcode_info("RET", 0);
    
    ops[LOAD] = op_load;
    opcode_debug_info[LOAD] = create_opcode_info("LOAD", 1);

    ops[STORE] = op_store;
    opcode_debug_info[STORE] = create_opcode_info("STORE", 1);
}

void color_red() {
    printf("\033[1;31m");
}

void color_green() {
    printf("\033[0;32m");
}

void color_reset() {
    printf("\033[0m");
}

void run_vm(uint32_t data_size, int32_t* opcodes, uint32_t main_ip) {
    vm = create_vm(data_size, main_ip);
    vm.opcodes = opcodes;

    printf("Data Allocated: %d\tMain IP: %d\n", data_size, main_ip);
    while(vm.opcodes[vm.ip] != HALT) {
        #ifdef DUMP_BYTECODE
            color_red();
            printf("%04x:\t%s\t", vm.ip, opcode_debug_info[vm.opcodes[vm.ip]].name);
            color_reset();
            uint32_t backtrack = vm.ip;
            uint32_t beg_opcode_args = opcode_debug_info[vm.opcodes[vm.ip]].num_args;
            for (int i = 0; i < beg_opcode_args; i++) {
                vm.ip++;
                printf("%d\t", vm.opcodes[vm.ip]);
            }
            vm.ip = backtrack;
        #endif

        ops[vm.opcodes[vm.ip]](&vm);

        #ifdef DUMP_BYTECODE
            color_green();
            printf("Stack: [ ");
            for (int i = 0; i < vm.stack.top; i++)
                printf("%d ", vm.stack.stack[i].i32);
            printf("]\t");

            printf("Data: [ ");
            for (int i = 0; i < data_size; i++) 
                printf("%d ", vm.data[i].i32);
            printf("]\n");
            color_reset();
        #endif
    } 

    printf("VM Output: \n");
    for (int i = 0; i < output_index; i++) {
        print_data(&output[i].o);
        printf("\n");
    }

    free(vm.stack.stack);
    free(vm.data);
}