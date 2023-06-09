#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

void panic_spin(char* filename, int line, const char* ssunc, const char* condition);

/************************************************
 * __VA_ARGS__ is the special symbol supporting by preprocessor
 * mean the conterpart parameters of ...
 * ... means the parameter of macro is variable
 * */
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION)   \
    if(CONDITION){}else{    \
        /* # let compiler convert the parameter of macro to string */ \
        PANIC(#CONDITION);  \
    }
#endif /*__NDEBUG*/

#endif /*__KERNEL_DEBUG_H*/
