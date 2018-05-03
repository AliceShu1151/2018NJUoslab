#include "klib.h"
#include "device.h"
#include "common.h"
#include <stdarg.h>


static void print_char(int c) {
    putChar(c);
}

static void print_str(const char *str) {
    while (*str)
        putChar(*str++);
}

static void print_int(int x, int base, int sgn) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i, neg;
    uint32_t ux;

    neg = 0;
    if(sgn && x < 0){
        neg = 1;
        ux = -x;
    } else {
        ux = x;
    }

    i = 0;
    do {
        buf[i++] = digits[ux % base];
    } while((ux /= base) != 0);
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
        print_char(buf[i]);
}

// Only understands %d, %x, %p, %s.
void printf(const char *fmt, ...) {
    int c, i, state;
    va_list ap;
    va_start(ap, fmt);

    state = 0;
    for (i = 0; fmt[i]; i++) {
        c = fmt[i] & 0xff;
        if(state == 0) {
            if(c == '%')
                state = '%';
            else 
                print_char(c);
        } else if (state == '%') {
            switch (c) {
                case 'd': 
                    print_int(va_arg(ap, int), 10, 1); break;
                case 'x': case 'p': 
                    print_int(va_arg(ap, int), 16, 0); break;
                case 's': 
                    print_str(va_arg(ap, const char *)); break;
                case 'c': 
                    print_char(va_arg(ap, int)); break;
                case '%': 
                    print_char('%'); break;
                default: 
                    // Unknown % sequence.  Print it to draw attention.
                    print_char('%');
                    print_char(c);
            }
            state = 0;
        }   
    }
    va_end(ap);
}