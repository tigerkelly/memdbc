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

#ifndef _MEMDBC_
#define _MEMDBC_

#include <stdbool.h>
#include <stdatomic.h>

/* AtomicExchange is used to compare and set p and return 1 on success else 0
 * p pointer to location to test.
 * e pointer to expected value at p.
 * if p equal e then set p to n and return 1 else 0
 * All three arguments are pointers. */
#define AtomicExchange(p, e, n) \
    __atomic_compare_exchange(p, e, n, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)

// These defines return the new value after operation.
#define AtomicAdd(p, n)         __atomic_add_fetch(p, n, __ATOMIC_SEQ_CST)
#define AtomicSub(p, n)         __atomic_sub_fetch(p, n, __ATOMIC_SEQ_CST)

#define AtomicAnd(p, n)         __atomic_and_fetch(p, n, __ATOMIC_SEQ_CST)
#define AtomicXor(p, n)         __atomic_xor_fetch(p, n, __ATOMIC_SEQ_CST)
#define AtomicOr(p, n)          __atomic_or_fetch(p, n, __ATOMIC_SEQ_CST)
#define AtomicNand(p, n)        __atomic_nand_fetch(p, n, __ATOMIC_SEQ_CST)

// These defines return the previous value before the operation.
#define AtomicFetchAdd(p, n)    __atomic_fetch_add(p, n, __ATOMIC_SEQ_CST)
#define AtomicFetchSub(p, n)    __atomic_fetch_sub(p, n, __ATOMIC_SEQ_CST)

#define AtomicFetchAnd(p, n)    __atomic_fetch_and(p, n, __ATOMIC_SEQ_CST)
#define AtomicFetchXor(p, n)    __atomic_fetch_xor(p, n, __ATOMIC_SEQ_CST)
#define AtomicFetchOr(p, n)     __atomic_fetch_or(p, n, __ATOMIC_SEQ_CST)
#define AtomicFetchNand(p, n)   __atomic_fetch_nand(p, n, __ATOMIC_SEQ_CST)

// returns the value at type *v.
#define AtomicGet(p)            __atomic_load_n(p, __ATOMIC_SEQ_CST)
// no return value. type *p, type n
#define AtomicSet(p, n)         __atomic_store_n(p, n, __ATOMIC_SEQ_CST)
//  It writes v into p, and returns the previous contents of p.
#define AtomicFetchSet(p, n)    __atomic_exchange_n(p, n, __ATOMIC_SEQ_CST)

// This performs an atomic test-and-set operation on the byte at *ptr. The byte is
// set to some implementation defined nonzero "set" value and the return value is true if and
// only if the previous contents were "set". It should be only used for operands of type bool or char.
#define AtomicTestSet(p)        __atomic_test_and_set(p, __ATOMIC_SEQ_CST)
// This performs an atomic clear operation on *ptr. After the operation, *ptr contains 0.
// It should be only used for operands of type bool or char and in conjunction with __atomic_test_and_set.
// For other types it may only clear partially. If the type is not bool prefer using __atomic_store.
#define AtomicClear(p)          __atmoic_clear(p, __ATOMIC_SEQ_CST)

#define Err(txt, ...) \
    do { fprintf(stderr, "ERROR: %s(%d): " txt, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#define Info(txt, ...) \
    do { printf("INFO: %s(%d): " txt, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)

typedef enum _dbtypes_ {
	ASCII_DB = 1,
	DIGITAL_DB,
	HEX_DB,
	OCTAL_DB
} DbTypes_t;

typedef enum _memDbcErrors {
	MEMDBC_OK,
	MALLOC_ERR,
	CALLBACK_NULL,
	REGEX_ERR,
	UNKNOWN_TYPE
} MemDbcError_t;

typedef struct _ttkey_ {
    char *key;
    struct _ttkey_ *ptr;
} Key_t;

typedef struct _memdbc_ {
	DbTypes_t dbType;
	Key_t *head;
	unsigned long recCount;
	void *tree;
} MemDbc_t;

extern MemDbcError_t memDbcErrorNum;

MemDbc_t *memDbcInit(DbTypes_t dbType);
int memDbcAdd(MemDbc_t *memDbc, char *key, void *data, int len);
unsigned long memDbcNumEntries(MemDbc_t *memDbc);
void memDbcWalk(MemDbc_t *memDbc, char *(callback)(char *key, void *data));
void *memDbcFind(MemDbc_t *memDbc, char *key);
void memDbcSave(MemDbc_t *memDbc, char *fileName, char *(callback)(char *key, void *data));
void memDbcFindAll(MemDbc_t *memDbc, char *regexStr, void (callback)(char *key, void *data));
int memDbcDelete(MemDbc_t *memDbc, char *key);
MemDbcError_t memDbcError();

#endif
