/*
 * Copyright (C) 2013 Nikolai Kondrashov
 *
 * This file is part of huion-tools.
 *
 * Huion-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Huion-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with huion-tools; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @author Nikolai Kondrashov <spbnick@gmail.com>
 */

#include "config.h"

#include <libusb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <strings.h>

#ifndef HAVE_LIBUSB_STRERROR
const char *
libusb_strerror(enum libusb_error err)
{
    switch (err)
    {
        case LIBUSB_SUCCESS:
            return "Success";
#define MAP(_name, _desc) \
    case LIBUSB_ERROR_##_name:          \
        return _desc " (ERROR_" #_name ")"
	    MAP(IO,
            "Input/output error");
	    MAP(INVALID_PARAM,
            "Invalid parameter");
	    MAP(ACCESS,
            "Access denied (insufficient permissions)");
	    MAP(NO_DEVICE,
            "No such device (it may have been disconnected)");
	    MAP(NOT_FOUND,
            "Entity not found");
	    MAP(BUSY,
            "Resource busy");
	    MAP(TIMEOUT,
            "Operation timed out");
	    MAP(OVERFLOW,
            "Overflow");
	    MAP(PIPE,
            "Pipe error");
	    MAP(INTERRUPTED,
            "System call interrupted (perhaps due to signal)");
	    MAP(NO_MEM,
            "Insufficient memory");
	    MAP(NOT_SUPPORTED,
            "Operation not supported or unimplemented on this platform");
	    MAP(OTHER, "Other error");
#undef MAP
        default:
            return "Unknown error code";
    }
}
#endif

#define GENERIC_ERROR(_fmt, _args...) \
    fprintf(stderr, _fmt "\n", ##_args)

#define GENERIC_FAILURE(_fmt, _args...) \
    GENERIC_ERROR("Failed to " _fmt, ##_args)

#define LIBUSB_FAILURE(_err, _fmt, _args...) \
    GENERIC_FAILURE(_fmt ": %s", ##_args, libusb_strerror(_err))

#define ERROR_CLEANUP(_fmt, _args...) \
    do {                                \
        GENERIC_ERROR(_fmt, ##_args);   \
        goto cleanup;                   \
    } while (0)

#define FAILURE_CLEANUP(_fmt, _args...) \
    do {                                \
        GENERIC_FAILURE(_fmt, ##_args); \
        goto cleanup;                   \
    } while (0)

#define LIBUSB_FAILURE_CLEANUP(_err, _fmt, _args...) \
    do {                                        \
        LIBUSB_FAILURE(_err, _fmt, ##_args);          \
        goto cleanup;                           \
    } while (0)

#define LIBUSB_GUARD(_expr, _fmt, _args...) \
    do {                                                \
        enum libusb_error err = _expr;                  \
        if (err < 0)                                    \
            LIBUSB_FAILURE_CLEANUP(err, _fmt, ##_args); \
    } while (0)

static void
hexdump(const unsigned char *ptr, int len)
{
    static const char   xd[]    = "0123456789ABCDEF";
    static char         buf[]   = " XX\n";
    size_t              pos;
    uint8_t             b;

    for (pos = 1; len > 0; len--, ptr++, pos++)
    {
        b = *ptr;
        buf[1] = xd[b >> 4];
        buf[2] = xd[b & 0xF];

        (void)fwrite(buf, ((pos % 16 == 0) ? 4 : 3), 1, stdout);
    }

    if (pos % 16 != 1)
        putc('\n', stdout);
}

static void
dump(const unsigned char *ptr, int len)
{
    const int   length              = 10;
    const int   max_x_off           = 0;
    const int   max_y_off           = 2;
    const int   max_pressure_off    = 6;
    const int   resolution_off      = 8;
    int         start;

    printf("RAW\n\n");
    hexdump(ptr, len);
    printf("\nDECODED\n\n");

    /* Find the start of the parameters block by first finding its end */
    for (start = len - 1; start >= 0 && ptr[start] == 0; start--);
    if (ptr[start] == 0x08 && start >= length)
        start -= length;
    else
        start = 0;

#define FIELD(_label, _offset) \
    do {                                                                \
        printf("%14s: ", _label);                                       \
        if ((start + _offset) < len - 1) {                              \
            printf("%u\n",                                              \
                   ptr[start + _offset] |                               \
                    ((unsigned int)ptr[start + (_offset) + 1] << 8));   \
        } else {                                                        \
            printf("N/A\n");                                            \
        }                                                               \
    } while (0)
    FIELD("Max X", max_x_off);
    FIELD("Max Y", max_y_off);
    FIELD("Max pressure", max_pressure_off);
    FIELD("Resolution", resolution_off);
#undef FIELD
    fflush(stdout);
}


int
probe(uint8_t bus_num, uint8_t dev_addr)
{
    int                     result      = 1;
    libusb_context         *ctx         = NULL;
    libusb_device         **dev_list    = NULL;
    ssize_t                 dev_num;
    size_t                  i;
    libusb_device          *dev;
    libusb_device_handle   *handle      = NULL;
    unsigned char           buf[32];
    int                     len;

    LIBUSB_GUARD(libusb_init(&ctx), "initialize libusb");

    LIBUSB_GUARD(dev_num = libusb_get_device_list(ctx, &dev_list),
                 "get device list");

    for (i = 0; i < (size_t)dev_num; i++) {
        dev = dev_list[i];
        if (libusb_get_bus_number(dev) == bus_num &&
            libusb_get_device_address(dev) == dev_addr)
            break;
    }
    if (i == (size_t)dev_num)
        ERROR_CLEANUP("Device not found");
    LIBUSB_GUARD(libusb_open(dev, &handle), "open device");
    libusb_free_device_list(dev_list, true);
    dev_list = NULL;
    LIBUSB_GUARD(len = libusb_get_string_descriptor(
                            handle,
                            /* Device info descriptor index */
                            0x64,
                            /* English (United States) */
                            0x0409,
                            buf,
                            sizeof(buf)),
                 "get the string descriptor");
    dump(buf, len);
    result = 0;

cleanup:

    libusb_close(handle);
    libusb_free_device_list(dev_list, true);
    libusb_exit(ctx);
    return result;
}

void
usage(FILE *file, const char *name)
{
    fprintf(file,
            "Usage: %s BUS_NUM DEV_ADDR\n"
            "Probe a Huion tablet.\n"
            "\n"
            "Arguments:\n"
            "    BUS_NUM    Bus number.\n"
            "    DEV_ADDR   Device address.\n"
            "\n",
            name);
}

int
main(int argc, char **argv)
{
    const char *name;

    name = rindex(argv[0], '/');
    if (name == NULL)
        name = argv[0];
    else
        name++;

    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments\n");
        usage(stderr, name);
        exit(1);
    }

    setbuf(stdout, NULL);
    return probe((uint8_t)atoi(argv[1]),
                 (uint8_t)atoi(argv[2]));
}
