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

#ifndef _TRIETREE_H_
#define _TRIETREE_H_

#include <sys/types.h>
#include "memdbc.h"

#ifndef TRIE_NULL
#define TRIE_NULL ((void *) 0)
#endif

// The *next array on a 64bit system is 760 bytes in size,
// cause on 64bit systems pointers are 8 bytes long.
typedef struct _asciiTrieTreeNode {
	void *data;
	unsigned int useCount;
	unsigned short inUse;
	struct _asciiTrieTreeNode *next[95];
} AsciiTrieTreeNode;

typedef struct _asciiTrieTree {
	AsciiTrieTreeNode *root;
} AsciiTrieTree;

AsciiTrieTree *attInit();
AsciiTrieTreeNode *attFindEnd(AsciiTrieTree *trie, char *key);
int attInsert(AsciiTrieTree *trie, char *key, void *value, int valueLen);
int attDelete(AsciiTrieTree *trie, char *key);
void *attLookup(AsciiTrieTree *trie, char *key);
int attNumEntries(AsciiTrieTree *trie);

// The *next array on a 64bit system is 80 bytes in size,
// cause on 64bit systems pointers are 8 bytes long.
typedef struct _digitalTrieTreeNode {
	void *data;
	unsigned int useCount;
	unsigned short inUse;
	struct _digitalTrieTreeNode *next[10];
} DigitalTrieTreeNode;

typedef struct _digitalTrieTree {
	DigitalTrieTreeNode *root;
} DigitalTrieTree;

DigitalTrieTree *dttInit();
DigitalTrieTreeNode *dttFindEnd(DigitalTrieTree *trie, char *key);
int dttInsert(DigitalTrieTree *trie, char *key, void *value, int valueLen);
int dttDelete(DigitalTrieTree *trie, char *key);
void *dttLookup(DigitalTrieTree *trie, char *key);
int dttNumEntries(DigitalTrieTree *trie);

// The *next array on a 64bit system is 128 bytes in size,
// cause on 64bit systems pointers are 8 bytes long.
typedef struct _hexTrieTreeNode {
	void *data;
	unsigned int useCount;
	unsigned short inUse;
	struct _hexTrieTreeNode *next[16];
} HexTrieTreeNode;

typedef struct _hexTrieTree {
	HexTrieTreeNode *root;
} HexTrieTree;

HexTrieTree *httInit();
HexTrieTreeNode *httFindEnd(HexTrieTree *trie, char *key);
int httInsert(HexTrieTree *trie, char *key, void *value, int valueLen);
int httDelete(HexTrieTree *trie, char *key);
void *httLookup(HexTrieTree *trie, char *key);
int httNumEntries(HexTrieTree *trie);

// The *next array on a 64bit system is 64 bytes in size,
// cause on 64bit systems pointers are 8 bytes long.
typedef struct _octalTrieTreeNode {
	void *data;
	unsigned int useCount;
	unsigned short inUse;
	struct _octalTrieTreeNode *next[8];
} OctalTrieTreeNode;

typedef struct _octalTrieTree {
	OctalTrieTreeNode *root;
} OctalTrieTree;

OctalTrieTree *ottInit();
OctalTrieTreeNode *ottFindEnd(OctalTrieTree *trie, char *key);
int ottInsert(OctalTrieTree *trie, char *key, void *value, int valueLen);
int ottDelete(OctalTrieTree *trie, char *key);
void *ottLookup(OctalTrieTree *trie, char *key);
int ottNumEntries(OctalTrieTree *trie);



#endif /* _TRIETREE_H_ */
