#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "stdint.h"

typedef void* intr_handler;

void idt_init();
void register_handler(uint8_t, intr_handler);

/* define the status of interrupt
 * INTR_OFF is 0, interrupt off
 * INTR_ON is 1, interrupt on
 * */
enum intr_status
{
    INTR_OFF, // interrupt off
    INTR_ON // interrupt on
};

enum intr_status intr_get_status(void);
enum intr_status intr_set_status(enum intr_status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);

#endif
