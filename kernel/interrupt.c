#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "kernel/io.h"

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define IDT_DESC_CNT 0x30 // the interruption that supported now 

// register eflag  IF = 1
#define EFLAGS_IF 0x00000200 
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" \
        : "=g" (EFLAG_VAR) \
        )

struct gate_desc
{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];

char* intr_name[IDT_DESC_CNT]; // restore the name of interruption
// define the array of interrupt handler, which is defined in kernel.S
// Just the entry of interrupt handler function.
// Will call the processing function in the ide_table finally
intr_handler idt_table[IDT_DESC_CNT];

// declare the entrys array of interrupt handle function defining in kernel.S
extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function)
{
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

static void idt_desc_init(void)
{
    int i;
    for (i = 0; i < IDT_DESC_CNT; ++i) 
    {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    put_str("idt_desc_init done\n");
}

/*
 * init program interrupt chip 8259A
 */
static void pic_init(void)
{
    // initialize main chip
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);

    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    // initialize peripheral chip
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);

    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    // clock interrupt
    // outb(PIC_M_DATA, 0xfe);
    // outb(PIC_S_DATA, 0xff);

    // test keyboard
    outb(PIC_M_DATA, 0xfd);
    outb(PIC_S_DATA, 0xff); 
    
    put_str("   pic_init done\n");
}

/* general interrupt handler function
 * call it usually when the exception occure
 * */
static void general_intr_handler(uint8_t vec_nr)
{
    // The spurious interrupt made by IRQ7 and IRQ15, no need to handle it.
    // 0x2f is the last IRQ pin of 8259A, keep.
    if (vec_nr == 0x27 || vec_nr == 0x2f)
    {
        return;
    }

    set_cursor_(0);
    int cursor_pos = 0;
    while (cursor_pos < 320) {
        put_char(' ');
        cursor_pos++;
    }
    set_cursor_(0);
    put_str("!!!!!!     excetion message begin      !!!!!!\n");
    set_cursor_(88);
    put_str(intr_name[vec_nr]);
    put_str("\n");
    if (vec_nr == 14) {
        int page_fault_vaddr = 0;
        asm volatile ("movl %%cr2, %0" : "=r"(page_fault_vaddr));

        put_str("\npage fault addr is ");
        put_int(page_fault_vaddr);
    }
    put_str("!!!!!!     excetion message end        !!!!!!\n");
    while(1);

}

/*
 * complete registing of general interrupt handler 
 * and the name of exception.
 * */
static void exception_init(void)
{
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++)
    {
        // default, will be registed by resiter_handler later.
        idt_table[i] = general_intr_handler; 
        intr_name[i] = "unknown"; // default name
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR Bound Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    // intr_name[15] = ""; // keep for intel
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}

/*
 * set the interrupt on and return the status before 
 * */
enum intr_status intr_enable()
{
    enum intr_status old_status;
    if (INTR_ON == intr_get_status())
    {
        old_status = INTR_ON;
        return old_status;
    }
    else
    {
        old_status = INTR_OFF;
        asm volatile("sti");
        return old_status;
    }
}

enum intr_status intr_disable()
{
    enum intr_status old_status;
    if (INTR_ON == intr_get_status())
    {
        old_status = INTR_ON;
        asm volatile("cli" : : : "memory");
        return old_status;
    } else {
        old_status = INTR_OFF;
        return old_status;
    }
}

/*
 * set status of interrupt
 * */
enum intr_status intr_set_status(enum intr_status status)
{
    return status & INTR_ON ? intr_enable() : intr_disable();
}

enum intr_status intr_get_status()
{
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

void register_handler(uint8_t vector_no, intr_handler function)
{
    idt_table[vector_no] = function;
}


/*
 * complete all the initializing work before off the interrupt.
 * */
void idt_init()
{
    put_str("   idt_init start\n");
    idt_desc_init(); // init the idt description
    // registing the interrupt handler
    // and init the name of exception
    exception_init(); 
    // init the 8259A
    pic_init();

    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
    asm volatile("lidt %0"::"m"(idt_operand));
    put_str("idt_init done\n");
}
