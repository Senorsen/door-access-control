/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014, 2015 Sen "Senorsen" Zhang <sen@senorsen.com>
 *
 * All Rights Reserved. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "macros.h"
#include "config.h"

int get_temp() {
    // There's a bug, blocking us out
    return -1;
    // Only on raspberry pi
    char * filename = "/sys/class/thermal/thermal_zone0/temp";
    if (0 == access(filename, 04)) {
        int temp = 0;
        FILE * fp = fopen(filename, "r");
        if (fp == NULL) {
            return -1;
        } else {
            fscanf(fp, "%d", &temp);
            fclose(fp);
            return temp;
        }
    } else {
        return -1;
    }
}


