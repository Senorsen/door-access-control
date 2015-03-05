/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014, 2015 Sen "Senorsen" Zhang <sen@senorsen.com>
 *
 * All Rights Reserved. 
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define VERSION     "0.4.3"
#define PROGNAME    "doorsend"
#define AUTHOR      "Senorsen"

#ifndef DATE
#define DATE "unknown"
#endif

#ifndef GITCOMMIT
#define GITCOMMIT "unknown"
#endif

#define DOOR_IPADDR "xxx.xxx.xxx.xxx"
#define DOOR_SN_1    0xab           // DOOR-SN: 0xabcd (four hexadecimal num)
#define DOOR_SN_2    0xcd
#define DOOR_PORT    60000          // UDP Port: 60000

#endif

