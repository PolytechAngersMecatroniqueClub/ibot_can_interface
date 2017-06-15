#ifndef LIB_H
#define LIB_H

/*
 * lib.h - library include for command line tools
 *
 * Copyright (c) 2002-2007 Volkswagen Group Electronic Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Volkswagen nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * The provided data structures and external interfaces from this code
 * are not restricted to be used by modules with a GPL compatible license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Send feedback to <linux-can@vger.kernel.org>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <sys/socket.h> /* for sa_family_t */
#include <linux/can.h>
#include <linux/can/error.h>


#define CANID_DELIM '#'
#define DATA_SEPERATOR '.'

#define MAX_CANFRAME      "12345678#01.23.45.67.89.AB.CD.EF"
#define MAX_LONG_CANFRAME_SIZE 256

can_frame generateRemoteFrame(const uint16_t id);
can_frame generateDataFrame(const uint16_t id, const size_t len, const uint8_t* data);

// convert a quartet into a char (ASCII value)
char quartet2char(uint8_t quartet);

unsigned char asc2nibble(char c);
/*
 * Returns the decimal value of a given ASCII hex character.
 *
 * While 0..9, a..f, A..F are valid ASCII hex characters.
 * On invalid characters the value 16 is returned for error handling.
 */

int hexstring2candata(char *arg, can_frame *cf);
/*
 * Converts a given ASCII hex string to values in the can_frame data[].
 *
 * A valid ASCII hex string consists of an even number of up to 16 chars.
 * Leading zeros '00' in the ASCII hex string are interpreted.
 *
 * Examples:
 *
 * "1234"   => data[0] = 0x12, data[1] = 0x34
 * "001234" => data[0] = 0x00, data[1] = 0x12, data[2] = 0x34
 *
 * Return values:
 * 0 = success
 * 1 = error (in length or the given characters are no ASCII hex characters)
 *
 * Remark: The not written data[] elements remain unchanged.
 *
 */

int parse_canframe(char *cs, can_frame *cf);
/*
 * Transfers a valid ASCII string decribing a CAN frame into struct can_frame.
 *
 * <can_id>#{R|data}
 *
 * can_id can have 3 (standard frame format) or 8 (extended frame format)
 *        hexadecimal chars
 *
 * data has 0 to 8 hex-values that can (optionally) be seperated by '.'
 *
 * Examples:
 *
 * 123# -> standard CAN-Id = 0x123, dlc = 0
 * 12345678# -> extended CAN-Id = 0x12345678, dlc = 0
 * 123#R -> standard CAN-Id = 0x123, dlc = 0, RTR-frame
 * 7A1#r -> standard CAN-Id = 0x7A1, dlc = 0, RTR-frame
 *
 * 123#00 -> standard CAN-Id = 0x123, dlc = 1, data[0] = 0x00
 * 123#1122334455667788 -> standard CAN-Id = 0x123, dlc = 8
 * 123#11.22.33.44.55.66.77.88 -> standard CAN-Id = 0x123, dlc = 8
 * 123#11.2233.44556677.88 -> standard CAN-Id = 0x123, dlc = 8
 * 32345678#112233 -> error frame with CAN_ERR_FLAG (0x2000000) set
 *
 * Simple facts on this compact ASCII CAN frame representation:
 *
 * - 3 digits: standard frame format
 * - 8 digits: extendend frame format OR error frame
 * - 8 digits with CAN_ERR_FLAG (0x2000000) set: error frame
 * - an error frame is never a RTR frame
 * 
 */

void fprint_canframe(FILE *stream , can_frame *cf, char *eol, int sep);
void sprint_canframe(char *buf , can_frame *cf, int sep);
/*
 * Creates a CAN frame hexadecimal output in compact format.
 * The CAN data[] is seperated by '.' when sep != 0.
 *
 * 12345678#112233 -> exended CAN-Id = 0x12345678, dlc = 3, data, sep = 0
 * 12345678#R -> exended CAN-Id = 0x12345678, RTR
 * 123#11.22.33.44.55.66.77.88 -> standard CAN-Id = 0x123, dlc = 8, sep = 1
 * 32345678#112233 -> error frame with CAN_ERR_FLAG (0x2000000) set
 *
 * Examples:
 *
 * fprint_canframe(stdout, &frame, "\n", 0); // with eol to STDOUT
 * fprint_canframe(stderr, &frame, NULL, 0); // no eol to STDERR
 *
 */

#define CANLIB_VIEW_ASCII	0x1
#define CANLIB_VIEW_BINARY	0x2
#define CANLIB_VIEW_SWAP	0x4
#define CANLIB_VIEW_ERROR	0x8

#define SWAP_DELIMITER '`'

void fprint_long_canframe(FILE *stream , can_frame *cf, char *eol, int view);
void sprint_long_canframe(char *buf , can_frame *cf, int view);
/*
 * Creates a CAN frame hexadecimal output in user readable format.
 *
 * 12345678  [3] 11 22 33 -> exended CAN-Id = 0x12345678, dlc = 3, data
 * 12345678  [0] remote request -> exended CAN-Id = 0x12345678, RTR
 * 14B0DC51  [8] 4A 94 E8 2A EC 58 55 62   'J..*.XUb' -> (with ASCII output)
 * 20001111  [7] C6 23 7B 32 69 98 3C      ERRORFRAME -> (CAN_ERR_FLAG set)
 *
 * Examples:
 *
 * fprint_long_canframe(stdout, &frame, "\n", 0); // with eol to STDOUT
 * fprint_long_canframe(stderr, &frame, NULL, 0); // no eol to STDERR
 *
 */

void snprintf_can_error_frame(char *buf, size_t len, can_frame *cf,
			      char *sep);
/*
 * Creates a CAN error frame output in user readable format.
 */

#endif
