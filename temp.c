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


