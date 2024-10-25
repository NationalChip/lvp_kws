
/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * libc_port.c:
 *
 */

#include <driver/gx_uart.h>

int fputc(int ch, void *stream)
{
    gx_console_compatible_putc(ch);

    return 0;
}

int fgetc(void *stream)
{
    return 0;
}

void _putchar(char ch)
{
    gx_console_compatible_putc(ch);

}