/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014, 2015 Sen "Senorsen" Zhang <sen@senorsen.com>
 *
 * All Rights Reserved. 
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

typedef struct nodeWgDate {
    int year, month, day;
    int hour, minute, second;
} wgDate;

void print_function(char * function_name);

uint16_t calc_checksum(uint8_t * data, uint16_t length);

uint16_t write_checksum(uint8_t * data, uint16_t length);

uint16_t read_checksum(uint8_t * data, uint16_t length);

int socket_init(int prog);

int process_data(int prog, uint8_t control[2], uint8_t data[], uint16_t length, uint8_t * rets, uint16_t * ret_length);

int dec_fake_hex(uint16_t n);

int store_int16(uint16_t n, uint8_t * buf);

int store_int32(uint32_t n, uint8_t * buf);

uint16_t convert_int16(uint8_t * buf);

uint32_t convert_int32(uint8_t * buf);

int mask_check(uint8_t num, uint8_t pos);

int store_wiegand(uint32_t wiegand, uint8_t * buf);

uint32_t read_card_wiegand(uint8_t * buf);

uint32_t read_card_org_part(uint8_t * buf);

int store_wiegand_date(int type, int year, int month, int day, int hour, int minute, int second, uint8_t * buf);

wgDate read_wiegand_date(uint8_t * buf);

#endif

