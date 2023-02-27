/*
 * Copyright (c) 2023 Richard Kelly Wiles (rkwiles@twc.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  Created on: Jan 16, 2023
 *      Author: Kelly Wiles
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "memdbc.h"
#include "trietree.h"

char *saveCallback(char *key, void *data);
char *walkCallback(char *key, void *data);
void findCallback(char *key, void *data);

void testAscii();
void testDigital();
void testHex();
void testOctal();

int main(int argc, char *argv[]) {

	if (argc == 1) {
		printf("Usage: example1 [-all] [-ascii] [-digital] [-hex] [-octal]\n");
		printf("    -all, run all tests.\n");
		printf("    -ascii, run ascii test.\n");
		printf("    -digital, run digital test.\n");
		printf("    -hex, run hex test.\n");
		printf("    -octal, run octal test.\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-ascii") == 0) {
			printf("**** Testing Ascii database. ****\n");
			testAscii();
		} else if (strcmp(argv[i], "-digital") == 0) {
			printf("\n**** Testing Digital database. ****\n");
			testDigital();
		} else if (strcmp(argv[i], "-hex") == 0) {
			printf("\n**** Testing Hex database. ****\n");
			testHex();
		} else if (strcmp(argv[i], "-octal") == 0) {
			printf("\n**** Testing Octal database. ****\n");
			testOctal();
		} else if (strcmp(argv[i], "-all") == 0) {
			printf("**** Testing Ascii database. ****\n");
			testAscii();
			printf("\n**** Testing Digital database. ****\n");
			testDigital();
			printf("\n**** Testing Hex database. ****\n");
			testHex();
			printf("\n**** Testing Octal database. ****\n");
			testOctal();
		}
	} 

	return 0;
}

void testAscii() {
	char *v1 = "Wiles was here.";
	char *v2 = "Kelly was here.";
	char *v3 = "Never was here.";
	char *v4 = "Maybe was here.";

	MemDbc_t *memDbc = memDbcInit(ASCII_DB);


	// Add a few records and they are plain strings.
	memDbcAdd(memDbc, "kelly", v1, strlen(v1));
	memDbcAdd(memDbc, "richard", v2, strlen(v2));
	memDbcAdd(memDbc, "wiles", v3, strlen(v3));
	memDbcAdd(memDbc, "kellywiles", v4, strlen(v4));

	// Find a single record given the key.
	char *p = (char *)memDbcFind(memDbc, "richard");

	if (p != NULL)
		printf("Found record: Key=%s, Value=%s\n", "richard", p);

	// Test that is does NOT find 'richardx' key.
	p = (char *)memDbcFind(memDbc, "richardx");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "richardx");
	}

	// Test that is does NOT find 'richar' key.
	p = (char *)memDbcFind(memDbc, "richar");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "richar");
	}

	// Find all recorda that match regex.
	memDbcFindAll(memDbc, "kelly[.]*", findCallback);

	// Print the number of records in DB.
	printf("Record Count: %lu\n", memDbcNumEntries(memDbc));

	// Print all records in the DB.
	memDbcWalk(memDbc, walkCallback);

	// Delete a record.
	memDbcDelete(memDbc, "wiles");

	memDbcWalk(memDbc, walkCallback);

	// update record.
	printf("Update record 'kellywiles'\n");
	memDbcAdd(memDbc, "kellywiles", "hello world.", 12);

	// Save all records to an ascii text file.
	// If fileName is NULL then caller handles saving data.
	memDbcSave(memDbc, "ascii1.txt", saveCallback);
}

void testDigital() {
	char *v1 = "012345 was here.";
	char *v2 = "678 was here.";
	char *v3 = "123 was here.";
	char *v4 = "01234 was here.";

	MemDbc_t *memDbc = memDbcInit(DIGITAL_DB);


	// Add a few records and they are plain strings.
	memDbcAdd(memDbc, "012345", v1, strlen(v1));
	memDbcAdd(memDbc, "678", v2, strlen(v2));
	memDbcAdd(memDbc, "123", v3, strlen(v3));
	memDbcAdd(memDbc, "01234", v4, strlen(v4));

	// Find a single record given the key.
	char *p = (char *)memDbcFind(memDbc, "012345");

	if (p != NULL)
		printf("Found record: Key=%s, Value=%s\n", "012345", p);

	// Test that is does NOT find '0123456' key.
	p = (char *)memDbcFind(memDbc, "0123456");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "0123456");
	}

	// Test that is does NOT find '0123' key.
	p = (char *)memDbcFind(memDbc, "0123");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "0123");
	}

	// Find all recorda that match regex.
	memDbcFindAll(memDbc, "01[.]*", findCallback);

	// Print the number of records in DB.
	printf("Record Count: %lu\n", memDbcNumEntries(memDbc));

	// Print all records in the DB.
	memDbcWalk(memDbc, walkCallback);

	// Delete a record.
	memDbcDelete(memDbc, "123");

	memDbcWalk(memDbc, walkCallback);

	// update record.
	printf("Update record '123'\n");
	memDbcAdd(memDbc, "123", "updated, hello world.", 21);

	memDbcWalk(memDbc, walkCallback);

	// Save all records to an ascii text file.
	// If fileName is NULL then caller handles saving data.
	memDbcSave(memDbc, "digital1.txt", saveCallback);
}

void testHex() {
	char *v1 = "A33F was here.";
	char *v2 = "E678 was here.";
	char *v3 = "123B was here.";
	char *v4 = "12C34 was here.";

	MemDbc_t *memDbc = memDbcInit(HEX_DB);


	// Add a few records and they are plain strings.
	memDbcAdd(memDbc, "A34F", v1, strlen(v1));
	memDbcAdd(memDbc, "E678", v2, strlen(v2));
	memDbcAdd(memDbc, "123B", v3, strlen(v3));
	memDbcAdd(memDbc, "12C34", v4, strlen(v4));

	// Find a single record given the key.
	char *p = (char *)memDbcFind(memDbc, "12C34");

	if (p != NULL)
		printf("Found record: Key=%s, Value=%s\n", "12C34", p);

	// Test that is does NOT find '0123456' key.
	p = (char *)memDbcFind(memDbc, "12C34A");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "12C34A");
	}

	// Test that is does NOT find '0123' key.
	p = (char *)memDbcFind(memDbc, "123B");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "123B");
	}

	// Find all recorda that match regex.
	memDbcFindAll(memDbc, "1[.]*", findCallback);

	// Print the number of records in DB.
	printf("Record Count: %lu\n", memDbcNumEntries(memDbc));

	// Print all records in the DB.
	memDbcWalk(memDbc, walkCallback);

	// Delete a record.
	memDbcDelete(memDbc, "E678");

	memDbcWalk(memDbc, walkCallback);

	// update record.
	printf("Update record '123B'\n");
	memDbcAdd(memDbc, "123B", "was updated.", 12);

	// Save all records to an ascii text file.
	// If fileName is NULL then caller handles saving data.
	memDbcSave(memDbc, "hex1.txt", saveCallback);
}

void testOctal() {
	char *v1 = "012345 was here.";
	char *v2 = "067 was here.";
	char *v3 = "0123 was here.";
	char *v4 = "01234 was here.";

	MemDbc_t *memDbc = memDbcInit(OCTAL_DB);


	// Add a few records and they are plain strings.
	memDbcAdd(memDbc, "012345", v1, strlen(v1));
	memDbcAdd(memDbc, "067", v2, strlen(v2));
	memDbcAdd(memDbc, "0123", v3, strlen(v3));
	memDbcAdd(memDbc, "01234", v4, strlen(v4));

	// Find a single record given the key.
	char *p = (char *)memDbcFind(memDbc, "012345");

	if (p != NULL)
		printf("Found record: Key=%s, Value=%s\n", "012345", p);

	// Test that is does NOT find '0123456' key.
	p = (char *)memDbcFind(memDbc, "0123456");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "0123456");
	}

	// Test that is does NOT find '01' key.
	p = (char *)memDbcFind(memDbc, "01");

	if (p == NULL) {
		printf("Record NOT Found: Key=%s\n", "01");
	}

	// Find all recorda that match regex.
	memDbcFindAll(memDbc, "01[.]*", findCallback);

	// Print the number of records in DB.
	printf("Record Count: %lu\n", memDbcNumEntries(memDbc));

	// Print all records in the DB.
	memDbcWalk(memDbc, walkCallback);

	// Delete a record.
	memDbcDelete(memDbc, "067");

	memDbcWalk(memDbc, walkCallback);

	// update record.
	printf("Update record '067'\n");
	memDbcAdd(memDbc, "067", "was updated.", 12);

	memDbcWalk(memDbc, walkCallback);

	// Save all records to an ascii text file.
	// If fileName is NULL then caller handles saving data.
	printf("Save octal db\n");
	memDbcSave(memDbc, "octal1.txt", saveCallback);
}

// Returns a comma separated string of the record.
// The string will be freed by the memDbcSave function.
char *saveCallback(char *key, void *data) {
	char *s = NULL;

	// printf("key=%p (%s), data=%p (%s)\n", key, key, data, (char *)data);
	if (key == NULL || data == NULL)
		return s;

	int len = strlen(key) + strlen((char *)data) + 2;
	if (len > 2)
		s = (char *)calloc(1, len);

	sprintf(s, "%s,%s", key, (char *)data);

	return s;
}

// Returns a comma separated string of the record.
// The string will be freed by the memDbcSave function.
char *walkCallback(char *key, void *data) {
	char *s = NULL;

	if (key == NULL || data == NULL)
		return s;

	int len = strlen(key) + strlen((char *)data) + 2;
	if (len > 2)
		s = (char *)calloc(1, len);

	sprintf(s, "%s,%s", key, (char *)data);

	return s;
}

void findCallback(char *key, void *data) {

	printf("Regex Found: Key=%s, Value=%s\n", key, (char *)data);
}
