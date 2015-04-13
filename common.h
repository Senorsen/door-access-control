/*
 * Copyright (C) 2014, 2015 Sen "Senorsen" Zhang <sen@senorsen.com>
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
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

void socket_end();

int process_data(int prog, uint8_t control[2], uint8_t data[], uint16_t length, uint8_t * rets, uint16_t * ret_length);

int dec_fake_hex(uint16_t n);

void store_int16(uint16_t n, uint8_t * buf);

void store_int32(uint32_t n, uint8_t * buf);

uint16_t convert_int16(uint8_t * buf);

uint32_t convert_int32(uint8_t * buf);

int mask_check(uint8_t num, uint8_t pos);

void store_wiegand(uint32_t wiegand, uint8_t * buf);

uint32_t read_card_wiegand(uint8_t * buf);

uint32_t read_card_org_part(uint8_t * buf);

void store_wiegand_date(int type, int year, int month, int day, int hour, int minute, int second, uint8_t * buf);

wgDate read_wiegand_date(uint8_t * buf);

#endif

