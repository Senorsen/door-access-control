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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
#include "temp.h"

char hostname[64];
int temp;
char temp_str[255];

void * ret_vars[100];

int read_warn(uint8_t num, uint8_t *bits) {
    int i, ret = 0;
    for (i = 0; i < 4; i++) {
        ret += bits[i] = mask_check(num, i);
    }
    return ret;
}

uint8_t parse_warn(uint8_t *bits) {
    uint8_t ret = 0;
    ret = bits[0] ^ (bits[1] << 1) ^ (bits[2] << 2) ^ (bits[3] << 3);
    return ret;
}

int disp_warn(uint8_t num, int is_internal) {
    uint8_t bits[4];
    int i, ret = read_warn(num, bits);
    char *prefix_blank, *suffix_lf;
    if (is_internal) {
        prefix_blank = "";
        suffix_lf = "|";
    } else {
        prefix_blank = "    ";
        suffix_lf = "\n";
    }
    if UNLIKELY (ret) {
        if UNLIKELY (bits[0]) {
            printf("%s遭到胁迫%s", prefix_blank, suffix_lf);
        }
        if UNLIKELY (bits[1]) {
            printf("%s门长时间未关闭%s", prefix_blank, suffix_lf);
        }
        if UNLIKELY (bits[2]) {
            printf("%s非法闯入%s", prefix_blank, suffix_lf);
        }
        if UNLIKELY (bits[3]) {
            printf("%s无效刷卡%s", prefix_blank, suffix_lf);
        }
    }
    return (int)ret;
}

int get_logs_count(int prog) {
    uint8_t control[] = {0x8c, 0x10};
    uint8_t data[1024] = {0};
    uint16_t length = 0;
    printf("[FUNCTION] get_logs_count\n");
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret == 0) {
        uint16_t cnt_logs;
        cnt_logs = convert_int16(rets); // 前两位便是
        if UNLIKELY (prog) return cnt_logs;
        else {
            printf("Count of logs: %d\n", cnt_logs);
            printf("[RESULT] result=%d,ret=0\n", cnt_logs);
        }
        return 0;
    } else {
        if UNLIKELY (prog) return 0;
        printf("Failed to fetch count of logs.\n");
        printf("[RESULT] ret=1\n");
        return 1;
    }
}

int read_card(int prog, uint16_t begin) {
    uint8_t control[] = {0x8d, 0x10};
    uint8_t data[1024] = {0};
    store_int16(begin, data);
    uint16_t length = 1;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) printf("[FUNCTION] read_card\n");
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret) {
        if UNLIKELY (!prog) printf("[RESULT] ret=1\n");
        return ret;
    }
    uint32_t rfid_uid = read_card_wiegand(rets);
    uint32_t rfid_uid_org = read_card_org_part(rets);
    char str_date[20];
    wgDate date;
    date = read_wiegand_date(rets + 4);
    sprintf(str_date, "%04d-%02d-%02d %02d:%02d:%02d", date.year, date.month, date.day, date.hour, date.minute, date.second);
    uint8_t status = rets[3];
    if LIKELY (status == 0xff) {
        if UNLIKELY (!prog) printf("[RESULT] ret=255\n");
        return 0xff;
    }
    if (!prog) printf("[RESULT] ret=0,rfid_uid=%d@%s@%d@%06x\n", rfid_uid, str_date, status, rfid_uid_org);
    ret_vars[0] = (uint32_t *) malloc(sizeof(uint32_t));
    ret_vars[1] = (uint8_t *) malloc(256);
    ret_vars[2] = (uint8_t *) malloc(sizeof(uint8_t));
    ret_vars[3] = (uint32_t *) malloc(sizeof(uint32_t));
    *(uint32_t *) ret_vars[0] = rfid_uid;
    strcpy((char *) ret_vars[1], str_date);
    *(uint8_t *) ret_vars[2] = status;
    *(uint32_t *) ret_vars[3] = rfid_uid_org;
    return ret;
}

int read_card_all(int prog, uint16_t begin) {
    if UNLIKELY (!prog) printf("[FUNCTION] read_card_all\n");
    int logs_count = get_logs_count(TRUE);
    if UNLIKELY (begin < 1) begin = 1;
    int read_count = logs_count - begin + 1;
    read_count = read_count > 0 ? read_count : 0;
    printf("[RESULT] logs_count=%d,read_count=%d,begin=%d,cards=", logs_count, read_count, begin);
    int ret = 0;
    for (int i = begin; i <= logs_count; i++) {
        ret = read_card(TRUE, i);
        if UNLIKELY (ret) {
            printf(",ret=%d\n", ret);
            return ret;
        }
        printf("%d@%s@%d@%06x|", *(uint32_t *)ret_vars[0], (char *)ret_vars[1], *(uint8_t *)ret_vars[2], *(uint32_t *)ret_vars[3]);
    }
    printf("|,ret=%d\n", ret);
    return ret;
}

int clear_privilege(int prog) {
    uint8_t control[] = {0x93, 0x10};
    uint8_t data[1024] = {0};
    uint16_t length = 0;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) print_function("clear_privilege");
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if UNLIKELY (ret) {
        if UNLIKELY (!prog) printf("[RESULT] ret=%d\n", ret);
        return ret;
    }
    if (!prog) printf("[RESULT] ret=%d,success=%d\n", ret, rets[0]);
    return ret;
}

int remove_privilege(int prog, int rfid_uid) {
    uint8_t control[] = {0x08, 0x11};
    uint8_t data[1024] = {0};
    uint16_t length = 6;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) print_function("remove_privilege");
    // Data order (fixed to single: 0)
    store_int16(0, data);
    // Wiegand card ID
    store_wiegand(rfid_uid, data + 2);
    // Door No.1 (fixed to 1)
    data[5] = 1;
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if UNLIKELY (ret) {
        if UNLIKELY (!prog) printf("[RESULT] ret=%d\n", ret);
        return ret;
    }
    if UNLIKELY (!prog) printf("[RESULT] ret=%d,count=%d\n", ret, rets[0]);
    else {
        ret_vars[0] = (uint8_t *) malloc(sizeof(uint8_t));
        *(uint8_t *)ret_vars = rets[0];
    }
    return ret;
}

int add_privilege(int prog, int rfid_uid) {
    uint8_t control[] = {0x07, 0x11};
    uint8_t data[1024] = {0};
    uint16_t length = 15;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) print_function("add_privilege");
    // Data order (fixed to single: 1)
    store_int16(1, data);
    // Wiegand card ID
    store_wiegand(rfid_uid, data + 2);
    // Door No.1 (fixed to 1)
    data[5] = 1;
    // Begin YMD (using Wiegand)
    store_wiegand_date(0, 2000, 1, 1, 0, 0, 0, data + 6);
    // End YMD (using Wiegand)
    store_wiegand_date(0, 2020, 12, 31, 23, 59, 59, data + 8);
    // Controller Segment ID (fixed to 1)
    data[10] = 1;
    // User password (fixed to 4xnull)
    // ...
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if UNLIKELY(ret) {
        if UNLIKELY (!prog) printf("[RESULT] ret=%d\n", ret);
        return ret;
    }
    if UNLIKELY (!prog) printf("[RESULT] ret=%d,count=%d\n", ret, rets[0]);
    else {
        ret_vars[0] = (uint8_t *) malloc(sizeof(uint8_t));
        *(uint8_t *)ret_vars = rets[0];
    }
    return ret;
}

int clear_record(int prog) {
    uint8_t control[] = {0x8e, 0x10};
    uint8_t data[1024] = {0};
    uint16_t length = 2;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) print_function("clear_record");
    int rec_count = get_logs_count(TRUE);
    store_int16(rec_count, data);
    ret = process_data(prog, control, data, length, rets, &ret_length);
    if UNLIKELY (ret) {
        if UNLIKELY (!prog) printf("[RESULT] ret=%d\n", ret);
        return ret;
    }
    if UNLIKELY (!prog) printf("[RESULT] ret=%d\n", ret);
    return ret;
}

int ret_check_door() {
    uint8_t control[] = {0x81, 0x10};
    uint8_t data[1024] = {0};
    uint16_t count = 0;
    store_int16(count, data + 0);
    uint16_t length = 2;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    ret = process_data(1, control, data, length, rets, &ret_length);
    return rets[20];
}

int loop_check_door(char * filename) {
    int lastret = 0;
    printf("[FUNCTION] loop_check_door\n");
    while (1) {
        int ret = ret_check_door();
        if (ret == 1 && lastret == 0) {
            printf("Door opened.\n");
            char cmd[255];
            sprintf(cmd, "%s", filename);
            int sys_ret = system(cmd);
        } else if (ret == 0 && lastret == 1) {
            printf("Door closed.\n");
        }
        lastret = ret;
        usleep(100000);
    }
}

int get_detail(int prog, uint16_t count) {
    uint8_t control[] = {0x81, 0x10};
    uint8_t data[1024] = {0};
    store_int16(count, data + 0);
    uint16_t length = 2;
    uint8_t rets[1024];
    uint16_t ret_length;
    int ret;
    if UNLIKELY (!prog) printf("[FUNCTION] get_detail\n");
    ret = process_data(prog, control, data, length, rets, &ret_length);
    char *szweekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    if LIKELY (ret == 0) {
        uint16_t logs_count, permissions_count;
        logs_count = convert_int16(rets + 7);
        permissions_count = convert_int16(rets + 10);
        // belows may be wrong
        uint8_t door_status = rets[20],
                warn_status = rets[22];
        uint32_t rfid_uid = read_card_wiegand(rets + 12);
        printf("Log time: 20%02x-%02x-%02x %s %02x:%02x:%02x\n", rets[0], rets[1], rets[2], szweekday[rets[3]], rets[4], rets[5], rets[6]);
        printf("Count of logs: \t%d\n", logs_count);
        printf("Count of Permissions: \t%d\n", permissions_count);
        printf("Status: \n");
        printf("    Last RFID-UID: %d\n", rfid_uid);
        printf("    Status of door: %d = %s\n", door_status, door_status ? "Opening" : "Not opening (!= closed)");
        printf("    Status of warn: %d\n", warn_status);
        disp_warn(warn_status, 0);
        uint8_t bits[4];
        read_warn(warn_status, bits);
        printf("\n");
        printf("[RESULT] logtime=%02x%02x%02x%02x%02x%02x%02x,", rets[0], rets[1], rets[2], rets[3], rets[4], rets[5], rets[6]);
        printf("logscount=%d,permissionscount=%d,", logs_count, permissions_count);
        printf("doorstatus=%d,warnstatus=%d,warnstatsbits=%d%d%d%d,", door_status, warn_status, bits[0], bits[1], bits[2], bits[3]);
        printf("warnstatus_str=");
        disp_warn(warn_status, 1);
        printf(",");
        printf("rfiduid=%d,", rfid_uid);
        printf("cli_version=%s,cli_copyright=Sen \"Senorsen\" Zhang <sen@senorsen.com>,", VERSION);
        printf("gitrev=%s,", GITCOMMIT);
        printf("build_date=%s,", DATE);
        printf("hostname=%s,", hostname);
        printf("temp=%d,temp_str=%s,", temp, temp_str);
        printf("ret=0\n");
        return 0;
    } else {
        printf("Failed to fetch detail!\n");
        printf("[RESULT] ret=1\n");
        return 1;
    }
}

int sync_system_time(int prog) {
    uint8_t control[] = {0x8b, 0x10};
    uint8_t data[1024];
    uint16_t length = 7; // 固定的data_length
    time_t now;
    struct tm * timenow;
    time(&now);
    timenow = localtime(&now);
    uint16_t year, month, day, weekday, hour, minute, second;
    uint8_t yearh, monthh, dayh, weekdayh, hourh, minuteh, secondh;
    year = timenow->tm_year + 1900;
    month = timenow->tm_mon + 1;
    day = timenow-> tm_mday;
    weekday = timenow->tm_wday;
    hour = timenow->tm_hour;
    minute = timenow->tm_min;
    second = timenow->tm_sec;
    char *szweekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    printf("[FUNCTION] sync_system_time\n");
    printf("System time: %04d-%02d-%02d %s %02d:%02d:%02d\n", year, month, day, szweekday[weekday], hour, minute, second);
    data[0] = yearh = dec_fake_hex(year % 100);
    data[1] = monthh = dec_fake_hex(month);
    data[2] = dayh = dec_fake_hex(day);
    data[3] = weekdayh = dec_fake_hex(weekday);
    data[4] = hourh = dec_fake_hex(hour);
    data[5] = minuteh = dec_fake_hex(minute);
    data[6] = secondh = dec_fake_hex(second);
    uint8_t rets[1024] = {0};
    uint16_t ret_length;
    int ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret == 0) {
        printf("Successfully changed. \nDoor time: ");
        printf("20%02x-%02x-%02x %s %02x:%02x:%02x\n", rets[0], rets[1], rets[2], szweekday[rets[3]], rets[4], rets[5], rets[6]);
        printf("[RESULT] ret=0\n");
        return 0;
    } else {
        printf("Failed to change time.\n");
        printf("[RESULT] ret=1\n");
        return 1;
    }
}

int reset_warn(int prog) {
    // 报警复位
    uint8_t control[] = {0x99, 0x10};
    uint8_t data[1024] = {0};
    uint16_t length = 0, ret_length;
    uint8_t rets[1024];
    if UNLIKELY (!prog) printf("[FUNCTION] reset_warn\n");
    int ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret == 0) {
        int result;
        if LIKELY (rets[0] == 1) {
            if UNLIKELY (!prog) printf("Warn has been reset.\n");
        } else {
            if UNLIKELY (!prog) printf("Warn result unknown: %d\n", result);
        }
        result = rets[0];
        printf("[RESULT] result=%d,ret=0\n", result);
    } else {
        if UNLIKELY (!prog) printf("Failed to reset warn status.\n");
        printf("[RESULT] ret=1\n");
    }
    return ret;
}

int open_door(int prog) {
    // 芝麻开门（短）
    uint8_t control[] = {0x9d, 0x10};
    uint8_t data[1024] = {0x01, 0x01};
    uint16_t length = 2, ret_length;
    uint8_t rets[1024];
    if UNLIKELY (!prog) printf("[FUNCTION] open_door\n");
    int ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret == 0) {
        printf("Door temporarily opened.\n");
        printf("[RESULT] ret=0\n");
    } else {
        printf("Failed to open door.\n");
        printf("[RESULT] ret=1\n");
    }
    return ret;
}

int set_door_status(int prog, int status) {
    // 设置门状态 ( 1 => 常开, 2 => 常闭, 3 => 正常 )
    uint8_t control[] = {0x8f, 0x10};
    uint8_t data[1024] = {0x01, (uint8_t) status, 0x32, 0x00};
    uint16_t length = 4, ret_length;
    uint8_t rets[1024];
    if UNLIKELY (!prog) printf("[FUNCTION] set_door_status\n");
    int ret = process_data(prog, control, data, length, rets, &ret_length);
    if LIKELY (ret == 0 && 
               rets[0] == 0x01 && 
               rets[1] == (uint8_t) status && 
               rets[2] == 0x32 && 
               rets[3] == 0x00) {
        printf("Successfully set door status to %d.\n", status);
        printf("[RESULT] ret=0,current_status=%d\n", rets[1]);
    } else {
        printf("Failed setting door status to %d.\n", status);
        printf("[RESULT] ret=1,current_status=%d\n", rets[1]);
    }
    return ret;
}

int main(int argc, char *argv[]) {
    printf("[START] SENORSEN_ACCESS_CONTROL\n");
    gethostname(hostname, 64);
    socket_init(0);
    temp = get_temp();
    sprintf(temp_str, "%.1f摄氏度", (float)temp / 1000);
    if UNLIKELY (argc < 2) {
print_help:
        printf("This is Senorsen's door access remote control tool.\n");
        printf("    Version:     %s\n", VERSION);
        printf("                 gitrev-%s\n", GITCOMMIT);
        printf("    Build date:  %s\n", DATE);
        printf("    Works for:   %s\n", hostname);
        printf("    Door temp:   %d == %s\n", temp, temp_str);
        printf("Copyright (C) 2014, 2015 Sen \"Senorsen\" Zhang <sen@senorsen.com>\n");
        printf("All rights reserved. \n");
        printf("Contact: sen@senorsen.com https://blog.senorsen.com\n");
        printf("Usage: %s <func>\n", argv[0]);
        printf("function list:\n");
        printf("    time        synchronize door's time with local system\n");
        printf("    logcount    get the count of logs\n");
        printf("    resetwarn   reset the status of door warning\n");
        printf("    opendoor    open the door temporarily\n");
        printf("    setdoor     <open/close/normal> set door perm-status\n");
        printf("    detail      [logscnt] get door detail\n");
        printf("    clearrec    clear all records\n");
        printf("    readcard    [logscnt] get specified card id\n");
        printf("    readall     [begin] get all card id\n");
        printf("    clearpriv   clear all user privilege\n");
        printf("    addpriv     <id> add user privilege\n");
        printf("    rempriv     <id> remove user privilege\n");
        printf("    loopchk     check door in a infinite loop\n");
        printf("    help        display this help\n");
        printf("    version     display version\n");
        printf("\n");
        printf("[END]\n");
        return 0;
    }
    int ret = 0;
    printf("[ACTION] %s\n", argv[1]);
    if UNLIKELY (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        goto print_help;
    } else if UNLIKELY (strcmp(argv[1], "version") == 0) {
        printf("Senorsen's Door Access Remote Control tool.\n");
        printf("    Version:     %s\n", VERSION);
        printf("    Build date:  %s\n", DATE);
        printf("    Works for:   %s\n", hostname);
        printf("    Door temp:   %d == %s\n", temp, temp_str);
        printf("Copyright (C) 2014, 2015 Sen \"Senorsen\" Zhang <sen@senorsen.com>\n");
        printf("All rights reserved. \n");
        printf("[RESULT] ret=0,version=%s,build_date=%s,temp=%d,temp_str=%s\n", VERSION, DATE, temp, temp_str);
        return 0;
    } else if LIKELY (strcmp(argv[1], "readall") == 0) {
        int log_begin = 1;
        if (argc >= 3) log_begin = atoi(argv[2]);
        ret = read_card_all(0, log_begin);
    } else if LIKELY (strcmp(argv[1], "time") == 0) {
        ret = sync_system_time(0);
    } else if (strcmp(argv[1], "logcount") == 0) {
        ret = get_logs_count(0);
    } else if (strcmp(argv[1], "resetwarn") == 0) {
        ret = reset_warn(0);
    } else if (strcmp(argv[1], "opendoor") == 0) {
        ret = open_door(0);
    } else if (strcmp(argv[1], "setdoor") == 0) {
        if (argc < 3) {
            printf("[ERROR] parameter status cannot be null!\n");
            return -1;
        }
        if (strcmp(argv[2], "open") == 0) {
            ret = set_door_status(0, 1);
        } else if (strcmp(argv[2], "close") == 0) {
            ret = set_door_status(0, 2);
        } else if (strcmp(argv[2], "normal") == 0) {
            ret = set_door_status(0, 3);
        } else {
            printf("[ERROR] parameter status can only be open, close or normal!\n");
            return -1;
        }
    } else if (strcmp(argv[1], "detail") == 0) {
        int logs_count = 0;
        if (argc >= 3) logs_count = atoi(argv[2]);
        ret = get_detail(0, logs_count);
    } else if (strcmp(argv[1], "clearrec") == 0) {
        ret = clear_record(0);
    } else if (strcmp(argv[1], "readcard") == 0) {
        int logs_count = 0;
        if (argc >= 3) logs_count = atoi(argv[2]);
        ret = read_card(0, logs_count);
    } else if (strcmp(argv[1], "clearpriv") == 0) {
        ret = clear_privilege(0);
    } else if (strcmp(argv[1], "loopchk") == 0) {
        ret = loop_check_door(argv[2]);
    } else if (strcmp(argv[1], "addpriv") == 0) {
        if (argc < 3) {
            printf("[ERROR] parameter id cannot be null!\n");
            return -1;
        }
        int rfid_uid = atoi(argv[2]);
        ret = add_privilege(0, rfid_uid);
    } else if (strcmp(argv[1], "rempriv") == 0) {
        if (argc < 3) {
            printf("[ERROR] parameter id cannot be null!\n");
            return -1;
        }
        int rfid_uid = atoi(argv[2]);
        ret = remove_privilege(0, rfid_uid);
    } else {
        printf("No such function! Try %s --help\n", argv[0]);
        printf("[RESULT] ret=1,msg=NO_SUCH_FUNCTION\n");
    }
    printf("[END]\n");
    return ret;
}

