/*
 * debug.c
 * contains utilitary functions for debugging
 *
 * Copyright (c) 2008 Jonathan Beck All Rights Reserved.
 * Copyright (c) 2010 Martin S. All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdarg.h>
#define _GNU_SOURCE 1
#define __USE_GNU 1
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "src/idevice.h"
#include "debug.h"
#include "libimobiledevice/libimobiledevice.h"

#ifndef STRIP_DEBUG_CODE
#include "asprintf.h"
#endif

static int debug_level;

void internal_set_debug_level(int level)
{
	debug_level = level;
}

#define MAX_PRINT_LEN (16*1024)

#ifndef STRIP_DEBUG_CODE
static void debug_print_line(const char *func, const char *file, int line, const char *buffer)
{
	char str_time[16];
#ifdef _WIN32
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	snprintf(str_time, 13, "%02d:%02d:%02d.%03d", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
#else
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	struct tm tp_;
	struct tm *tp;
	gettimeofday(&tv, NULL);
#ifdef HAVE_LOCALTIME_R
	tp = localtime_r(&tv.tv_sec, &tp_);
#else
	tp = localtime(&tv.tv_sec);
#endif
	strftime(str_time, 9, "%H:%M:%S", tp);
	snprintf(str_time+8, 5, ".%03d", (int)tv.tv_usec/1000);
#else
	time_t the_time;
	time(&the_time);
	strftime(str_time, 15, "%H:%M:%S", localtime (&the_time));
#endif
#endif
	fprintf(stderr, "%s %s:%d %s(): %s\n", str_time, file, line, func, buffer);
}
#endif

void debug_info_real(const char *func, const char *file, int line, const char *format, ...)
{
#ifndef STRIP_DEBUG_CODE
	va_list args;
	char *buffer = NULL;

	if (!debug_level)
		return;

	/* run the real fprintf */
	va_start(args, format);
	if(vasprintf(&buffer, format, args)<0){}
	va_end(args);

	debug_print_line(func, file, line, buffer);

	free(buffer);
#endif
}

void debug_buffer(const char *data, const int length)
{
#ifndef STRIP_DEBUG_CODE
	int i;
	int j;
	unsigned char c;

	if (debug_level) {
		for (i = 0; i < length; i += 16) {
			fprintf(stderr, "%04x: ", i);
			for (j = 0; j < 16; j++) {
				if (i + j >= length) {
					fprintf(stderr, "   ");
					continue;
				}
				fprintf(stderr, "%02x ", *(data + i + j) & 0xff);
			}
			fprintf(stderr, "  | ");
			for (j = 0; j < 16; j++) {
				if (i + j >= length)
					break;
				c = *(data + i + j);
				if ((c < 32) || (c > 127)) {
					fprintf(stderr, ".");
					continue;
				}
				fprintf(stderr, "%c", c);
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "\n");
	}
#endif
}

void debug_buffer_to_file(const char *file, const char *data, const int length)
{
#ifndef STRIP_DEBUG_CODE
	if (debug_level) {
		FILE *f = fopen(file, "wb");
		fwrite(data, 1, length, f);
		fflush(f);
		fclose(f);
	}
#endif
}

void debug_plist_real(const char *func, const char *file, int line, plist_t plist)
{
#ifndef STRIP_DEBUG_CODE
	if (!plist)
		return;

	char *buffer = NULL;
	uint32_t length = 0;
	plist_to_xml(plist, &buffer, &length);

	/* get rid of ending newline as one is already added in the debug line */
	if (buffer[length-1] == '\n')
		buffer[length-1] = '\0';

	if (length <= MAX_PRINT_LEN)
		debug_info_real(func, file, line, "printing %i bytes plist:\n%s", length, buffer);
	else
		debug_info_real(func, file, line, "supress printing %i bytes plist...\n", length);

	free(buffer);
#endif
}

