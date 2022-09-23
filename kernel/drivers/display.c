/*
    Copyright (c) 2022 thatOneArchUser
    All rights reserved
*/

#include "mem.h"
#include "types.h"
#include "ports.h"
#include "display.h"
#include "stdlib.h"

#define TAB_LENGTH 4

void set_cursor(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (u8) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (u8) (offset & 0xff));
}

int get_cursor() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2;
}

static int get_offset(int col, int row) {
    return 2 * (row * MAX_COLS + col);
}

static int get_row_from_offset(int offset) {
    return offset / (2 * MAX_COLS);
}

static int move_offset_to_new_line(int offset) {
    return get_offset(0, get_row_from_offset(offset) + 1);
}

static void set_char_at_video_memory(char character, int offset) {
    u8 *vidmem = (u8*) VIDEO_ADDRESS;
    vidmem[offset] = character;
    vidmem[offset + 1] = WHITE_ON_BLACK;
}

static int scroll_ln(int offset) {
    memcpy(
            (u8*)(get_offset(0, 1) + VIDEO_ADDRESS),
            (u8*)(get_offset(0, 0) + VIDEO_ADDRESS),
            MAX_COLS * (MAX_ROWS - 1) * 2
    );

    for (int col = 0; col < MAX_COLS; col++) {
        set_char_at_video_memory(' ', get_offset(col, MAX_ROWS - 1));
    }

    return offset - 2 * MAX_COLS;
}

void print_char(char c) {
    int offset = get_cursor();
    if (offset >= MAX_ROWS * MAX_COLS * 2) {
        offset = scroll_ln(offset);
    }
    if (c == '\n') {
        print_nl();
    } else if (c == '\t') {
        for (char i = 0; i < TAB_LENGTH; i++) {
            print_char(' ');
        }
    } else {
        set_char_at_video_memory(c, offset);
        offset += 2;
        set_cursor(offset);
    }
}

void print_string(char *string) {
    int offset = get_cursor();
    int i = 0;
    while (string[i] != 0) {
        print_char(string[i]);
        i++;
    }
    set_cursor(offset);
}

void print_nl() {
    int newOffset = move_offset_to_new_line(get_cursor());
    if (newOffset >= MAX_ROWS * MAX_COLS * 2) {
        newOffset = scroll_ln(newOffset);
    }
    set_cursor(newOffset);
}

void clear_screen() {
    int screen_size = MAX_COLS * MAX_ROWS;
    for (int i = 0; i < screen_size; ++i) {
        set_char_at_video_memory(' ', i * 2);
    }
    set_cursor(get_offset(0, 0));
}

void print_backspace() {
    int newCursor = get_cursor() - 2;
    set_char_at_video_memory(' ', newCursor);
    set_cursor(newCursor);
}