#include <stdint.h>

#include "asm_basics.hpp"
#include "uart_x_out.hpp"

#include "limine.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexcessive-regsave"

// special crap

// limine use rev 2
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(2);

uint8_t stack[1024];

// -
struct InterruptFrame {
    uint64_t ip;     // instruction ptr
    uint64_t cs;     // code segment
    uint64_t flags;  // cpu state
    uint64_t sp;     // stack ptr
    uint64_t ss;     // stack seg
};

typedef struct {
    uint16_t limit; // size of idt -1
    uint64_t base;  // raw idt
} __attribute__((packed)) idtr_t;


typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isr_high;     // The higher 32 bits of the ISR's address
    uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

/////



//// -~# DRIVERS #~- ////



/// -# macro drivers

    // idt insertion thing
    static void idt_append_function(uint8_t entry, void* function, uint8_t attributes, uint8_t ist_index, idt_entry_t idt[]) {

        uint64_t pointer = (uint64_t)function;

        idt[entry].isr_high = ((pointer >> 32) & 0xffffffff);
        idt[entry].isr_mid = ((pointer >> 16) & 0xffff);
        idt[entry].isr_low = (pointer & 0xffff);

        idt[entry].ist = ist_index;

        idt[entry].attributes = attributes;

        idt[entry].kernel_cs = 0x08;

        idt[entry].reserved = 0;

    }

//// -~# KERNEL FUNCTIONS #~- ////

/// -# Interrupts and whatnot #- ///

    // this handles panics ig
    [[noreturn]] static void ctkn_panic_handle(uint16_t code) {

        switch (code) {

            // null pointer exception (scary)
            case (0x0000):
                
                uart_out_str("[stage:unknown]:ctkn:error: CTKN_NULL_PTR_EXCEPTION, FATAL");

                halt();
            
            // general protection fault (scarier)
            case (0x0100): {

                uart_out_str("[stage:unknown]:ctkn:error: CTKN_GENERAL_PROTECTION_FAULT, FATAL");

                halt();

            }

            // fancy one incase my code didn't end up working because im a crackhead when it comes to writing this kernel lels \(-.-)/
            case (0x0101): {

                uart_out_str("[stage:unknown]:ctkn:error: CTKN_CRITICAL_PROTECTION_FAULT, CRITICAL FATAL");

                halt();

            }

            // page fault (memory's mad, i couldn't possibly code something that could page fault im such a "good" dev ;D (trust))
            case (0x0200): {

                uart_out_str("[stage:unknown]:ctkn:error: CTKN_PAGE_FAULT, FATAL");

                halt();

            }

            // testing one for good measure
            case (0xFF00): {

                uart_out_str("[stage:unknown]:ctkn:error: CTKN_FUNNY_TEST_PANIC_EXCEPTION, FATAL");

                halt();

            }

        }

        halt();

    }

    extern "C" {
        
        [[gnu::interrupt]] void ctkn_handle_null_expt(InterruptFrame* ptr) {

            disable_interrupts();

            ctkn_panic_handle(0x0000);
            
        }

        [[gnu::interrupt]] void ctkn_handle_gp_flt(InterruptFrame* ptr, uint64_t nyi) {

            disable_interrupts();

            ctkn_panic_handle(0x0100);

        }

        [[gnu::interrupt]] void ctkn_handle_dp_flt(InterruptFrame* ptr, uint64_t nyi) {

            disable_interrupts();

            ctkn_panic_handle(0x101);

        }

        [[gnu::interrupt]] void ctkn_handle_page_flt(InterruptFrame* ptr, uint64_t nyi) {

            disable_interrupts();

            ctkn_panic_handle(0x0200);

        }

        [[gnu::interrupt]] void ctkn_handle_timer(InterruptFrame* ptr) {

            disable_interrupts();

            // no

        }

    }

// this is the very evil very dangerous CTNU kernel starter wooah scary
// inside lies the CTNU kernel which is very scary haha scary woooah ahhh
static inline void ctkn_kickstart_begin() {

    disable_interrupts();

    // im gonna do stuff here, probably?

    ///

    const char* msg = "[stage:kickstart]:ctkn:message: ctkn_kickstart_begin() initialized";

    uart_out_str(msg);

    // kernel loop finally after all that init
    while (1) {

        // nyi

        halt();

    }



}

// this is the actual kernel strapper be careful careful careful lol
// its supposed to rip limine crap off and then start the kernel
static inline void ctkn_insert_begin() {

    const char* msg = "[stage:insert]:ctkn:message: ctkn_insert_begin() initialized";

    uart_out_str(msg);

    disable_interrupts();

    //

    __attribute__((aligned(0x10))) 
    idt_entry_t idt[256]; // Create an array of IDT entries, osdev wiki says alligning it like this is good for performance i think

    for (uint16_t i = 256; i > 0; idt_append_function(--i, (void*)ctkn_handle_gp_flt, 0b10001110, 1, idt))

    idt_append_function(0, (void*)ctkn_handle_null_expt, 0b10001110, 1, idt);
    idt_append_function(8, (void*)ctkn_handle_dp_flt, 0b10001110, 1, idt);
    idt_append_function(14, (void*)ctkn_handle_page_flt, 0b10001110, 1, idt);

    



    //

    uint64_t GDT[7] = {0};

    GDT[0] = 0x00; // null desc
    GDT[1] = 0x00AF9A000000FFFF; // kernel code
    GDT[2] = 0x00CF92000000FFFF; // kernel data
    GDT[3] = 0x00CFF2000000FFFF; // user code
    GDT[4] = 0x00AFA8000000FFFF; // user data
    GDT[5] = (); // tss low
    GDT[6] = (); // tss high

    //

    // enable_interrupts();

    ctkn_kickstart_begin();

}


//// -~# START #~- ////


// this is where limine hooks in
extern "C" {
    
    // this is SPECIFICALLY where limine hooks in
    void _start() {

        ctkn_insert_begin();

    } 

}
