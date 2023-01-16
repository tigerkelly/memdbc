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
#include <regex.h>

#include "memdbc.h"
#include "trietree.h"

// Local variables and functions.
MemDbcError_t memDbcErrorNum = 0;

/* keyListinsert() - Insert record key into sorted linked list.
 * memDbc - returned by memDbcInit()
 * key - Key sting to insert.
 */
void keyListInsert(MemDbc_t *memDbc, char *key) {
	Key_t *temp, *prev, *next;
    temp = (Key_t*)malloc(sizeof(Key_t));
    temp->key = strdup(key);
    temp->ptr = NULL;

    if (memDbc->head == NULL) {
		// printf("Head: %s\n", key);
        memDbc->head = temp;
    } else {
        prev = NULL;
        next = memDbc->head;
        while(next != NULL && strcmp(next->key, key) < 0){
            prev = next;
            next = next->ptr;
        }
        if (next == NULL) {
            prev->ptr = temp;
        } else{
            if (prev != NULL) {
                temp->ptr = prev->ptr;
                prev-> ptr = temp;
            } else {
                temp->ptr = memDbc->head;
                memDbc->head = temp;
            }
        }
    }
}

/* keyListDelete() - delete key from sorted linked list.
 * memDbc - returned by memDbcInit()
 * key - to remove.
 */
void keyListDelete(MemDbc_t *memDbc, char *key) {

	Key_t *next = memDbc->head;
	Key_t *prev = NULL;

	while(next != NULL) {
		if (strcmp(key, next->key) == 0) {
			if (prev == NULL) {
				memDbc->head = next->ptr;
				free(next->key);
				free(next);
			} else {
				prev->ptr = next->ptr;
				free(prev->key);
				free(prev);
			}
			break;
		}
		next = next->ptr;
	}
}

/* keyListWalk() - Walks the sorted linked list and call the callback function.
 * memDbc - returned by memDbcInit()
 * callback - the user supplied callback fucntion.
 */
void keyListWalk(MemDbc_t *memDbc, char *(*callback)(char *key, void *data)) {

	Key_t *next = memDbc->head;
	void *data = NULL;

	printf("Walking sorted list:\n");
	while(next != NULL) {
		switch (memDbc->dbType) {
			case ASCII_DB:
				data = attLookup(memDbc->tree, next->key);
				break;
			case DIGITAL_DB:
				data = dttLookup(memDbc->tree, next->key);
				break;
			case HEX_DB:
				data = httLookup(memDbc->tree, next->key);
				break;
			case OCTAL_DB:
				data = ottLookup(memDbc->tree, next->key);
				break;
			default:
				memDbcErrorNum = UNKNOWN_TYPE;
				break;
		}

		if (callback == NULL)
			printf("Key=%s, Value=%s\n", next->key, (char *)data);
		else {
			char *s = callback(next->key, data);
			if ( s != NULL) {
				printf("%s\n", s);
				free(s);
			}
		}
		next = next->ptr;
	}
}

/* keyListSave() - Walks the sorted linked list and calls the callback function.
 * memDbc - returned by memDbcInit()
 * fileName - file name to save darta to or NULL if user is saving the records.
 * callback - the user supplied callback fucntion.
 */
void keyListSave(MemDbc_t *memDbc, char *fileName, char *(*callback)(char *key, void *data)) {

	Key_t *next = memDbc->head;
	void *data = NULL;
	FILE *out = NULL;

	if (fileName != NULL)
		out = fopen(fileName, "w");

	while(next != NULL) {
		switch (memDbc->dbType) {
			case ASCII_DB:
				data = attLookup(memDbc->tree, next->key);
				break;
			case DIGITAL_DB:
				data = dttLookup(memDbc->tree, next->key);
				break;
			case HEX_DB:
				data = httLookup(memDbc->tree, next->key);
				break;
			case OCTAL_DB:
				data = ottLookup(memDbc->tree, next->key);
				break;
			default:
				memDbcErrorNum = UNKNOWN_TYPE;
				break;
		}

		if (callback == NULL) {
			// Save a record per line.
			fprintf(out, "%s,%s\n", next->key, (char *)data);
		} else {
			if (fileName != NULL) {
				char *s = callback(next->key, data);
				if (s != NULL) {
					fprintf(out, "%s\n", s);
					free(s);
				}
			} else {
				// Caller is saving the data.
				callback(next->key, data);
			}
		}
		next = next->ptr;
	}
	if (fileName != NULL)
		fclose(out);
}

/* allocateRoot() - Allocates the root node of the given type.
 * memDbc - returned by memDbcInit()
 */
void *allocateRoot(MemDbc_t *memDbc) {
	void *p = NULL;

	switch (memDbc->dbType) {
		case ASCII_DB:
			p = (void *)calloc(1, sizeof(AsciiTrieTree));
			break;
		case DIGITAL_DB:
			p = (void *)calloc(1, sizeof(DigitalTrieTree));
			break;
		case HEX_DB:
			p = (void *)calloc(1, sizeof(HexTrieTree));
			break;
		case OCTAL_DB:
			p = (void *)calloc(1, sizeof(OctalTrieTree));
			break;
		default:
			memDbcErrorNum = UNKNOWN_TYPE;
			break;
	}

	return p;
}

/* initTree() - initalize the given type of tree.
 * memDbc - returned by memDbcInit()
 */
void *initTree(MemDbc_t *memDbc) {
	void *p = NULL;

	switch (memDbc->dbType) {
		case ASCII_DB:
			p = (void *)attInit();
			break;
		case DIGITAL_DB:
			p = (void *)dttInit();
			break;
		case HEX_DB:
			p = (void *)httInit();
			break;
		case OCTAL_DB:
			p = (void *)ottInit();
			break;
		default:
			memDbcErrorNum = UNKNOWN_TYPE;
			break;
	}

	return p;
}

// Exported functions.

/* memDbInit() - Initalize the MemDbc_t struture.
 * dbType - The type of database user wants.
 */
MemDbc_t *memDbcInit(DbTypes_t dbType) {

	memDbcErrorNum = MEMDBC_OK;

	MemDbc_t *memDbc = (MemDbc_t *)calloc(sizeof(MemDbc_t), 1);

	if (memDbc == NULL) {
		memDbcErrorNum = MALLOC_ERR;
		return NULL;
	}

	memDbc->dbType = dbType;
	memDbc->tree = allocateRoot(memDbc);

	if (memDbcErrorNum != MEMDBC_OK) {
		if (memDbc != NULL && memDbc->tree != NULL)
			free(memDbc->tree);
		if (memDbc != NULL)
			free(memDbc);
		memDbc = NULL;
	}

	initTree(memDbc);

	return memDbc;
}

/* memDbcAdd() - Add a record to the database.
 * memDbc - returned by memDbcInit()
 * key - the key to store data under.
 * data - the data to store.
 * len - Length of the data.
 */
int memDbcAdd(MemDbc_t *memDbc, char *key, void *data, int len) {

	int r = 0;

	switch (memDbc->dbType) {
		case ASCII_DB:
			r = attInsert(memDbc->tree, key, data, len);
			if (r == 1) {
				// key already exists in trie tree then do NOT add to sorted link list.
				keyListInsert(memDbc, key);
				memDbc->recCount++;
			}
			break;
		case DIGITAL_DB:
			r = dttInsert(memDbc->tree, key, data, len);
			if (r == 1) {
				// key already exists in trie tree then do NOT add to sorted link list.
				keyListInsert(memDbc, key);
				memDbc->recCount++;
			}
			break;
		case HEX_DB:
			r = httInsert(memDbc->tree, key, data, len);
			if (r == 1) {
				// key already exists in trie tree then do NOT add to sorted link list.
				keyListInsert(memDbc, key);
				memDbc->recCount++;
			}
			break;
		case OCTAL_DB:
			r = ottInsert(memDbc->tree, key, data, len);
			if (r == 1) {
				// key already exists in trie tree then do NOT add to sorted link list.
				keyListInsert(memDbc, key);
				memDbc->recCount++;
			}
			break;
		default:
			memDbcErrorNum = UNKNOWN_TYPE;
			return -1;
	}

	return r;
}

/* memDbcFind() - Find a single rcord in database.
 * memDbc - returned by memDbcInit()
 * key - to look for.
 */
void *memDbcFind(MemDbc_t * memDbc, char *key) {
	void *rec = NULL;

	switch (memDbc->dbType) {
		case ASCII_DB:
			rec = attLookup(memDbc->tree, key);
			break;
		case DIGITAL_DB:
			rec = dttLookup(memDbc->tree, key);
			break;
		case HEX_DB:
			rec = httLookup(memDbc->tree, key);
			break;
		case OCTAL_DB:
			rec = ottLookup(memDbc->tree, key);
			break;
		default:
			memDbcErrorNum = UNKNOWN_TYPE;
			break;
	}

	return rec;
}

/* memDbcDelete() - Marks a record as deleted.
 */
int memDbcDelete(MemDbc_t * memDbc, char *key) {
	int r = -1;

	switch (memDbc->dbType) {
		case ASCII_DB:
			r = attDelete(memDbc->tree, key);
			break;
		case DIGITAL_DB:
			r = dttDelete(memDbc->tree, key);
			break;
		case HEX_DB:
			r = httDelete(memDbc->tree, key);
			break;
		case OCTAL_DB:
			r = ottDelete(memDbc->tree, key);
			break;
		default:
			memDbcErrorNum = UNKNOWN_TYPE;
			break;
	}

	if (r == 0) {
		keyListDelete(memDbc, key);
	}

	return r;
}

/* memDbcFindAll() - Find all regex matching records.
 * memDbc - returned by memDbcInit()
 * regexStr - regex pattern to match to.
 * callback - user supplied callback function.
 */
void memDbcFindAll(MemDbc_t *memDbc, char *regexStr, void (*callback)(char *key, void *data)) {
	Key_t *next = memDbc->head;
	void *data = NULL;

	if (callback == NULL) {
		memDbcErrorNum = CALLBACK_NULL;
		return;
	}

	regex_t regex;

	int r = regcomp(&regex, regexStr, 0);
	if (r != 0) {
		memDbcErrorNum = REGEX_ERR;
		return;
	}

	while(next != NULL) {
		if (regexec(&regex, next->key, 0, NULL, 0) == 0) {
			switch (memDbc->dbType) {
				case ASCII_DB:
					data = attLookup(memDbc->tree, next->key);
					break;
				case DIGITAL_DB:
					data = dttLookup(memDbc->tree, next->key);
					break;
				case HEX_DB:
					data = httLookup(memDbc->tree, next->key);
					break;
				case OCTAL_DB:
					data = ottLookup(memDbc->tree, next->key);
					break;
				default:
					memDbcErrorNum = UNKNOWN_TYPE;
					break;
			}

			if (callback != NULL) {
				callback(next->key, data);
			}
		}
		next = next->ptr;
	}
}

/* memDbcNumEntries() - returns the record count.
 * memDbc - returned by memDbcInit()
 */
unsigned long memDbcNumEntries(MemDbc_t *memDbc) {
	return memDbc->recCount;
}

/* memDbcWalk() - Walks the database calling the callback
 * memDbc - returned by memDbcInit()
 * callback - user supplied callback function.
 */
void memDbcWalk(MemDbc_t *memDbc, char *(*callback)(char *key, void *data)) {

	keyListWalk(memDbc, callback);
}

/* memDbcSave() - Saves all records to a file.
 * memDbc - returned by memDbcInit()
 * fileName - File name to save data to or NULL is user's callback is saving the data.
 * callback - user supplied callback function.
 */
void memDbcSave(MemDbc_t *memDbc, char *fileName, char *(*callback)(char *key, void *data)) {

	keyListSave(memDbc, fileName, callback);
}

/* memDbcErro() - returns the MemDbCErrorNum value.
 */
MemDbcError_t memDbcError() {
	return memDbcErrorNum;
}
