/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014 Senorsen Zhang <sen@senorsen.com>
 *
 * All Rights Reserved. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
int port = 60000;
// 10.76.8.151
#define ADDR 172755095

int main() {
    int sockfd;
    int i = 0;
    int z;
    char buf[80], str1[80];
    struct sockaddr_in adr_srvr;
    FILE * fp;
    printf("Opening file...\n");
    /* read-only method */
    fp = fopen("data", "r");
    if (fp == NULL) {
        perror("Failed to open file.");
        exit(1);
    }
    printf("Connecting to the server...\n");
    /* build ip addr */
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(port);
    adr_srvr.sin_addr.s_addr = htonl(ADDR);
    bzero(&(adr_srvr.sin_zero), 8);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }
    printf("Sending file...");
    while (!feof(fp)) {
        buf[i++] = fgetc(fp);
    }
    z = sendto(sockfd, buf, i - 1, 0, (struct sockaddr *)&adr_srvr, sizeof(adr_srvr));
    if (z < 0) {
        perror("sendto error");
        exit(1);
    }
    fclose(fp);
    close(sockfd);
    return 0;
}

