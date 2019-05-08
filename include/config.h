/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Config Header
 * ------------------------------------------------------------------ */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef BRFW_CONFIG_H
#define BRFW_CONFIG_H

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#endif
