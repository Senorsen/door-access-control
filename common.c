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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include "macros.h"
#include "config.h"
#include "common.h"

int sockfd = -1;
struct sockaddr_in adr_srvr;
struct sockaddr_in adr_inet;
struct sockaddr_in adr_clnt;

void print_function(char * function_name) {
    printf("[FUNCTION] %s\n", function_name);
}

uint16_t calc_checksum(uint8_t * data, uint16_t length) {
    uint16_t checksum = 0;
    int i;
    for (i = 1; i < length - 3; i++) {
        checksum += data[i];
    }
    return checksum;
}

uint16_t write_checksum(uint8_t * data, uint16_t length) {
    uint16_t checksum;
    checksum = calc_checksum(data, length);
    data[length - 3] = checksum % 0x100;
    data[length - 2] = checksum / 0x100;
    return checksum;
}

uint16_t read_checksum(uint8_t * data, uint16_t length) {
    uint16_t checksum = 0;
    checksum = data[length - 3] + data[length - 2] * 0x100;
    return checksum;
}

int socket_init(int prog) {
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if UNLIKELY (sockfd == -1) {
        perror("socket error\n");
        return 1;
    }
    in_addr_t addr = inet_addr(DOOR_IPADDR);
    if UNLIKELY (!prog) printf("Preparing data...\n");
    /* build ip addr */
    memset(&(adr_srvr), 0, sizeof(adr_srvr));
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(DOOR_PORT);
    adr_srvr.sin_addr.s_addr = addr;

    struct timeval tv_out;
    tv_out.tv_sec = 10; // 超时时间
    tv_out.tv_usec = 0;
    printf("[INFO] setsockopt = %d\n", setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out)));
    return 0;
}

void socket_end() {
    close(sockfd);
}

int process_data(int prog, uint8_t control[2], uint8_t data[], uint16_t length, uint8_t * rets, uint16_t * ret_length) {
    int i = 0;
    int z;
    unsigned int len;
    uint8_t buf[4096] = {0x7e};
    // buf ready
    buf[1] = DOOR_SN_1;
    buf[2] = DOOR_SN_2;
    buf[3] = control[0];
    buf[4] = control[1];
    for (i = 0; i < length; i++) {
        buf[i + 5] = data[i];
    }
    int total_length = 5 + length + 2 + 1;
    if LIKELY (total_length < 34) {
        // 小包规格
        total_length = 34;
    }
    buf[total_length - 1] = 0x0d;
    write_checksum(buf, total_length);
    // 0x7e | serial | control | data | checksum | end
    if UNLIKELY (!prog) printf("Sending... ");
    z = sendto(sockfd, buf, total_length, 0, (struct sockaddr *)&adr_srvr, sizeof(adr_srvr));
    if UNLIKELY (z == -1) {
        perror("sendto error\n");
        return 1;
    }
    if UNLIKELY (!prog) printf("Sended & Pending\n");
    len = sizeof(buf);
    z = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &adr_srvr, &len);
    if UNLIKELY (z < 0) {
        perror("recvfrom error, may be network issues?\n");
        return 1;
    }
    if UNLIKELY (!prog) printf("Ok\n");
    total_length = z;
    *ret_length = total_length - 1 - 2 - 2 - 3;
    if UNLIKELY (!prog) printf("Checking... ");
    int succ = 1;
    if UNLIKELY (buf[0] != 0x7e) {
        succ = 0;
        perror("no 7e header\n");
    }
    if UNLIKELY (buf[1] != DOOR_SN_1 || buf[2] != DOOR_SN_2) {
        succ = 0;
        perror("serial error\n");
    }
    if UNLIKELY (buf[3] != control[0] || buf[4] != control[1]) {
        succ = 0;
        perror("control error\n");
    }
    if UNLIKELY (calc_checksum(buf, total_length) != read_checksum(buf, total_length)) {
        succ = 0;
        perror("checksum error\n");
    }
    if UNLIKELY (!succ) {
        perror("buf invalid\n");
        return 1;
    }
    if UNLIKELY (!prog) printf("Ok\n");
    memcpy(rets, buf + 5, *ret_length);
    return 0;
}

int dec_fake_hex(uint16_t n) {
    return n / 10 * 16 + n % 10;
}

void store_int16(uint16_t n, uint8_t * buf) {
    // 大端
    buf[0] = n % 0x100;
    buf[1] = n / 0x100;
}

void store_int32(uint32_t n, uint8_t * buf) {
    buf[0] = n % 0x100;
    buf[1] = n % 0x10000 /  0x100;
    buf[2] = n % 0x1000000 / 0x10000;
    buf[3] = n / 0x1000000;
}

uint16_t convert_int16(uint8_t * buf) {
    return (uint16_t)buf[1] * 0x100 + buf[0];
}

uint32_t convert_int32(uint8_t * buf) {
    return (uint32_t)buf[3] * 0x1000000
         + (uint32_t)buf[2] * 0x10000
         + (uint32_t)buf[1] * 0x100
         + (uint32_t)buf[0] * 0x1;
}

int mask_check(uint8_t num, uint8_t pos) {
    // pos: 0 ~ 7
    return !! (num & (1 << pos));
}

void store_wiegand(uint32_t wiegand, uint8_t * buf) {
    store_int16(wiegand % 100000, buf);
    buf[2] = wiegand / 100000;
}

uint32_t read_card_wiegand(uint8_t * buf) {
    if LIKELY ((buf[0] & buf[1] & buf[2]) == 0xff) {
        return 0;
    }
    uint32_t ret, arg2, arg0;
    arg0 = convert_int16(buf);
    arg2 = buf[2];
    ret = arg2 * 100000 + arg0;
    return ret;
}

uint32_t read_card_org_part(uint8_t * buf) {
    if LIKELY ((buf[0] & buf[1] & buf[2]) == 0xff) {
        return 0;
    }
    uint8_t new_buf[4] = {buf[0], buf[1], buf[2], 0};
    return convert_int32(new_buf);
}

void store_wiegand_date(int type, int year, int month, int day, int hour, int minute, int second, uint8_t * buf) {
    // simulation for msDateToWgDate encode. 
    buf[1] = year % 100 * 2 + month / 8;
    buf[0] = month / 8 * day;
    if UNLIKELY (type) {
        // 4 bytes with h:m:s
        buf[3] = hour * 8 + minute / 8;
        buf[2] = minute / 8 * 32 + second / 2;
    }
}

wgDate read_wiegand_date(uint8_t * buf) {
    // simulation for wgDataToMsDate decode. 
    // bytes data struct: 2 2 2 2
    uint32_t ymdh, ymdl, hmsh = 0, hmsl = 0;
    ymdh = *(buf + 1);
    ymdl = *(buf + 0);
    hmsh = *(buf + 3);
    hmsl = *(buf + 2);
    int num = (ymdh & 127) / 2;
    time_t now;
    struct tm * timenow;
    time(&now);
    timenow = localtime(&now);
    int year, month, day, hour, minute, second;
    wgDate date;
    year = timenow->tm_year + 1900;
    if UNLIKELY (year % 100 > 60 && num < 60) {
        num += 60;
    }
    if UNLIKELY (year < 2000) {
        year = 2000;
    }
    month = (ymdh % 2) * 8 + ymdl / 32;
    day = ymdl % 32;
    hour = hmsh / 8;
    minute = (hmsh % 8) * 8 + hmsl / 32;
    second = (hmsl % 32) * 2 + (ymdh > 128 ? 1 : 0);
    date.year = year;
    date.month = month;
    date.day = day;
    date.hour = hour;
    date.minute = minute;
    date.second = second;
    return date;
}

