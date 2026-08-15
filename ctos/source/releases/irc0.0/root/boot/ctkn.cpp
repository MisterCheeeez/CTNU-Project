#include <stdint.h>

// special crap (which is nyi)

//// -~# DRIVERS #~- ////
/// -# asm drivers

    // disables interrupts
    // mildly dangerous if you dont know what you're doing, but not as dangerous as halt() because you can undo it
    static inline void disable_interrupts() {

        __asm__ ("cli" : : : "memory" );

    }


    // enables interrupts
    // probably not dangerous, just know what you're doing
    static inline void enable_interrupts() {

        __asm__ ("sti" : : : "memory" );

    }

    // halts execution, dangerous if you dont know what you're doing (lols)
    static inline void halt() {

        __asm__ ("hlt");

    }

    // in bytes :D
    static inline uint8_t in_byte(uint16_t port) {
        uint8_t result;

        __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));

        return result;
    }

    // bytes out? :d
    static inline void out_byte(uint16_t port, uint8_t value) {

        __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
      
    }

/// -# macro drivers

    // FINALLY uart SINGLE CHARACTER output
    static inline void uart_out_c(char character) {

        uint16_t port = 0x3F8;
        uint16_t status_port = port + 5;

        // wait until its ready then do it
        while ((in_byte(status_port) & 0b00010000) == 0) {}

        out_byte(port, character);
        

    }

    void uart_out_str(const char* str) {

        while (*str) {

            uart_out_c(*str++);

        }

    }


//// -~# KERNEL FUNCTIONS #~- ////


// this handles panics ig
[[noreturn]] static inline void ctkn_panic_handle(uint16_t code) {

    switch (code) {

        case (0x0000):
            // null pointer exception woahh soo scary ahh
            // nyi
            halt();
        
        case (0x1000):
            // random memory exception
            // nyi
            halt();

    }

    halt();

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

        ;

    }



}

// this is the actual kernel strapper be careful careful careful lol
// its supposed to rip limine crap off and then start the kernel
static inline void ctkn_insert_begin() {

    const char* msg = "[stage:insert]:ctkn:message: ctkn_insert_begin() initialized";

    uart_out_str(msg);

    disable_interrupts();

    // tables yay (NO I HATE THIS I HATE TABLES SO MUCH I HATE IBM GOSH WHY THE HECK DOES IBM DO THIS CRAP I HATE CORNER CUTTING I WISH I COULD [[we will remove this from the public source available, --hex]])
    
    typedef struct {
        uint16_t    isr_low;      // The lower 16 bits of the ISR's address
        uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
        uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
        uint8_t     attributes;   // Type and attributes; see the IDT page
        uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
        uint32_t    isr_high;     // The higher 32 bits of the ISR's address
        uint32_t    reserved;     // Set to zero
    } __attribute__((packed)) idt_entry_t;

    __attribute__((aligned(0x10))) 
    static idt_entry_t idt[256]; // Create an array of IDT entries, osdev wiki says alligning it like this is good for performance i think but i also dont think it really matters that much

    typedef struct {
        uint16_t	limit;
        uint64_t	base;
    } __attribute__((packed)) idtr_t;

    static idtr_t idtr;


    enable_interrupts();

    ctkn_kickstart_begin();

}


//// -~# START #~- ////


// this is where limine hooks in, which ive yet to exactly implement support completely for, though irc0.0 isn't done yet so who cares
extern "C" {
    
    // this is SPECIFICALLY where limine hooks in
    void _start() {

        ctkn_insert_begin();

    } 

}
// ALL RIGHTS RESERVED. dont run this on your hardware, there is no warranty provided for any damaged caused by the CTNU Project and anything in it.
