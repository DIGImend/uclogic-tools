/*
 * Copyright (C) 2013 Nikolai Kondrashov
 *
 * This file is part of uclogic-tools.
 *
 * Uclogic-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Uclogic-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uclogic-tools; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @author Nikolai Kondrashov <spbnick@gmail.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#define GENERIC_ERROR(_fmt, _args...) \
    fprintf(stderr, _fmt "\n", ##_args)

#define GENERIC_FAILURE(_fmt, _args...) \
    GENERIC_ERROR("Failed to " _fmt, ##_args)

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

/** Format string for a field header */
#define FIELD_HEAD_FMT "    | %22s: "

/** Format string for a field header with hex index */
#define IDX_FIELD_HEAD_FMT " %02x | %22s: "

/** Format string for a sub-field header */
#define SUB_FIELD_HEAD_FMT "    | %30s: "

/** Blank output line string */
#define BLANK_LINE_STR "    |"

struct decoder {
    /** Decoder ID */
    uint8_t     id;
    /**
     * Decoder function.
     *
     * @param idx   Index of the decoded data (e.g. descriptor number).
     * @param ptr   Pointer to the decoded data.
     * @param len   Length of the decoded data.
     *
     * @return Zero if decoded successfully, non-zero otherwise.
     */
    int       (*decode)(uint8_t idx, const uint8_t *ptr, int len);
};

/**
 * Print a 16-bit unsigned sub-field.
 * Assumes "ptr" and "len" variables are in scope.
 *
 * @param _offset   Field offset from ptr, bytes.
 * @param _label    Field label string literal.
 */
#define PRINT_SUB_FIELD_U16(_offset, _label) \
    do {                                                        \
        printf(SUB_FIELD_HEAD_FMT, _label);                     \
        if ((_offset) + 1 < len) {                              \
            printf("%u\n",                                      \
                   ptr[_offset] |                               \
                    ((unsigned int)ptr[(_offset) + 1] << 8));   \
        } else {                                                \
            printf("N/A\n");                                    \
        }                                                       \
    } while (0)

/**
 * Print a 24-bit unsigned sub-field.
 * Assumes "ptr" and "len" variables are in scope.
 *
 * @param _offset   Field offset from ptr, bytes.
 * @param _label    Field label string literal.
 */
#define PRINT_SUB_FIELD_U24(_offset, _label) \
    do {                                                        \
        printf(SUB_FIELD_HEAD_FMT, _label);                     \
        if ((_offset) + 2 < len) {                              \
            printf("%u\n",                                      \
                   ptr[_offset] |                               \
                    ((unsigned int)ptr[(_offset) + 1] << 8) |   \
                    ((unsigned int)ptr[(_offset) + 2] << 16));  \
        } else {                                                \
            printf("N/A\n");                                    \
        }                                                       \
    } while (0)

static void
print_unicode(const uint8_t *ptr, int len)
{
    uint16_t c;
    for (; len > 1; ptr += 2, len -= 2) {
        c = ptr[0] | (ptr[1] << 8);
        putchar((c >= 0x20 && c <= 0x7E) ? c : '?');
    }
}

static void
print_field_unicode(const char *name, const uint8_t *ptr, int len)
{
    printf(FIELD_HEAD_FMT, name);
    print_unicode(ptr, len);
    putchar('\n');
}

static void
print_idx_field_unicode(uint8_t idx, const char *name,
                        const uint8_t *ptr, int len)
{
    printf(IDX_FIELD_HEAD_FMT, idx, name);
    print_unicode(ptr, len);
    putchar('\n');
}

static int
decode_params1(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Params block #1", ptr + 2, len - 2);
    PRINT_SUB_FIELD_U16(2, "Max X");
    PRINT_SUB_FIELD_U16(4, "Max Y");
    PRINT_SUB_FIELD_U16(8, "Max pressure");
    PRINT_SUB_FIELD_U16(10, "Resolution");
    puts(BLANK_LINE_STR);
    return 0;
}

static int
decode_internal_model(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Internal model", ptr + 2, len - 2);
    return 0;
}

static int
decode_buttons_status(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Buttons status", ptr + 2, len - 2);
    return 0;
}

static int
decode_params2(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Params block #2", ptr + 2, len - 2);
    PRINT_SUB_FIELD_U24(2, "Max X");
    PRINT_SUB_FIELD_U24(5, "Max Y");
    PRINT_SUB_FIELD_U16(8, "Max pressure");
    PRINT_SUB_FIELD_U16(10, "Resolution");
    puts(BLANK_LINE_STR);
    return 0;
}

static int
decode_unknown_string1(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Unknown string #1", ptr + 2, len - 2);
    return 0;
}

static int
decode_internal_manufacturer(uint8_t idx, const uint8_t *ptr, int len)
{
    print_idx_field_unicode(idx, "Internal manufacturer", ptr + 2, len - 2);
    return 0;
}

/* List of string descriptor decoders */
static const struct decoder desc_list[] = {
    {0x64, decode_params1},
    {0x79, decode_internal_model},
    {0x7b, decode_buttons_status},
    {0xc8, decode_params2},
    {0xc9, decode_unknown_string1},
    {0xca, decode_internal_manufacturer},
    {0x00, NULL}
};

static int
decode_desc(uint8_t idx, const uint8_t *buf, int len)
{
    const struct decoder   *d;

    (void)idx;

    if (len == 0) {
        GENERIC_ERROR("String descriptor without index");
        return 1;
    }

    for (d = desc_list; d->decode != NULL; d++) {
        if (d->id == *buf)
            return d->decode(d->id, buf + 1, len - 1);
    }

    return 0;
}

static int
decode_manufacturer(uint8_t idx, const uint8_t *buf, int len)
{
    (void)idx;
    print_field_unicode("Manufacturer", buf, len);
    return 0;
}

static int
decode_product(uint8_t idx, const uint8_t *buf, int len)
{
    (void)idx;
    print_field_unicode("Product", buf, len);
    return 0;
}

/* List of chunk decoders */
static const struct decoder chunk_list[] = {
    {'M',   decode_manufacturer},
    {'P',   decode_product},
    {'S',   decode_desc},
    {'\0',  NULL}
};

static int
decode_chunk(const uint8_t *buf, int len)
{
    const struct decoder   *d;

    assert(len > 0);

    for (d = chunk_list; d->decode != NULL; d++) {
        if (d->id == *buf)
            return d->decode(d->id, buf + 1, len - 1);
    }

    return 0;
}

static int
decode(FILE *input)
{
    int             result      = 1;
    char           *word        = NULL;
    char           *end;
    /*
     * Chunk type byte +
     * string descriptor index byte +
     * maximum descriptor length
     */
    uint8_t         buf[258];
    uint8_t        *p           = buf;
    unsigned long   n;

    assert(sizeof(buf) > 0);

    /* For each "word" (a run of non-whitespace characters) on stdin */
    while (fscanf(input, "%ms", &word) == 1) {
        /* If it's a single character chunk type */
        if (word[0] != '\0' && !isxdigit(word[0]) && word[1] == '\0') {
            /* If we read a chunk before and it is failing decoding */
            if (p != buf && decode_chunk(buf, p - buf) != 0)
                FAILURE_CLEANUP("decode chunk");
            /* Start a new chunk */
            p = buf;
            n = word[0];
        /* Otherwise it is some other word */
        } else {
            /* If there was nothing else for this chunk (i.e. no type) */
            if (p == buf)
                ERROR_CLEANUP("Expecting chunk type indicator");
            /* Try to decode the word as a hex byte */
            errno = 0;
            n = strtoul(word, &end, 16);
            if (*end != '\0' || errno != 0 || n > UINT8_MAX)
                ERROR_CLEANUP("Invalid byte \"%s\"", word);
            /* If there's no space in the buffer */
            if (p >= buf + sizeof(buf))
                ERROR_CLEANUP("Descriptor too long");
        }
        /* Write the byte into the buffer */
        *p++ = n;
        /* Discard the read word */
        free(word);
        word = NULL;
    };

    if (p != buf && decode_chunk(buf, p - buf) != 0)
        FAILURE_CLEANUP("decode chunk");

    result = 0;

cleanup:
    free(word);
    return result;
}

static void
usage(FILE *file, const char *name)
{
    fprintf(file,
            "Usage: %s [PROBE_OUTPUT]\n"
            "Decode a UC-Logic tablet probe dump.\n"
            "\n",
            name);
}

int
main(int argc, char **argv)
{
    const char *name;
    FILE *input;

    name = rindex(argv[0], '/');
    if (name == NULL)
        name = argv[0];
    else
        name++;

    if (argc > 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        usage(stderr, name);
        exit(1);
    } else if (argc > 1) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Failed opening %s: %s\n", argv[1],
                    strerror(errno));
        }
    } else {
        input = stdin;
    }

    setlinebuf(stdout);
    return decode(input);
}
