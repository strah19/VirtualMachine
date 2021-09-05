  
#ifndef VM_H
#define VM_H

#include <stdint.h>
#include "../include/opcodes.h"

enum ObjTypes {
    F_32BIT, I_32BIT, D_64BIT, V_PTR, I_8BIT, U_32BIT, U_8BIT
};

struct Object {
    uint8_t type;  //The object data type

    union {
        uint32_t u32;
        int32_t i32;
        uint8_t i8;
        uint8_t u8;

        float f32;
        double f64;
        
        void* ptr;
    };
};

struct VMStack {
    int top;
    int size;
    struct Object* stack;
};

struct VM {
    struct VMStack stack;
    struct Object* data;
    int32_t* opcodes;
    uint32_t ip;
    int32_t fp;
    uint32_t data_size;
};

extern struct VMStack vm_create_stack(int size);

extern int vm_push_stack(struct VMStack* stack, struct Object object);

extern struct Object vm_pop_stack(struct VMStack* stack);

extern struct Object vm_peek_stack(struct VMStack* stack);

extern void init_vm();

extern void run_vm(uint32_t data_size, int32_t* opcodes, uint32_t main_ip);

#endif // !VM_H