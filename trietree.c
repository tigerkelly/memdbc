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

#include "trietree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "logutils.h"
#include "strutils.h"
#include "trietree.h"

#define CHAR_BIT	8

int _asciiTrieTreeInit = 0;

static inline int _toAsciiIdx(char ch) __attribute__((always_inline));

AsciiTrieTree *attInit() {

	AsciiTrieTree *attRoot = (AsciiTrieTree *) calloc(1, sizeof(AsciiTrieTree));
	if (attRoot == NULL)
		return NULL;
	attRoot->root = NULL;

	_asciiTrieTreeInit = 1;

	return attRoot;
}

static inline int _toAsciiIdx(char ch) {

	// Only allow printable ASCII characters.
	if (ch > 31 && ch < 127)
		return (int)ch - ' ';
	else {
		pErr("ERROR: Not a printable character.\n");
		return 0;
	}
}

/*
 * Function to find end of tree for a given key.
 *
 *   key = A ascii key
 */
AsciiTrieTreeNode *attFindEnd(AsciiTrieTree *trie, char *key) {
	AsciiTrieTreeNode *node;
	char *p;

	if (_asciiTrieTreeInit == 0) {
		pErr("Must call attInit() first.\n");
		return NULL;
	}

	// Search down the trie until the end of string is reached

	node = trie->root;

	for (p = key; *p != '\0'; ++p) {

		if (node == NULL) {
			// Not found in the tree. Return.
			return NULL;
		}

		// Jump to the next node
		node = node->next[_toAsciiIdx(*p)];
	}

	// This string is present if the value at the last node is not NULL
	if (node == NULL || node->inUse == 0)
		return NULL;

	return node;
}

/*
 * Function _attRollback is private to this file.
 */
static void _attRollback(AsciiTrieTree *trie, char *key) {
	AsciiTrieTreeNode *node;
	AsciiTrieTreeNode **prev_ptr;
	AsciiTrieTreeNode *next_node;
	AsciiTrieTreeNode **next_prev_ptr;
	char *p;

	// Follow the chain along.  We know that we will never reach the
	// end of the string because attInsert never got that far.  As a
	// result, it is not necessary to check for the end of string
	// delimiter (NUL)

	node = trie->root;
	prev_ptr = &trie->root;
	p = key;

	while (node != NULL) {

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[_toAsciiIdx(*p)];
		next_node = *next_prev_ptr;
		++p;

		// Decrease the use count and free the node if it
		// reaches zero.

		AtomicSub(&node->useCount, 1);

		if (node->useCount == 0) {
			free(node);

			if (prev_ptr != NULL) {
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

/*
 * Function attInsert is used to insert data into trie tree.
 */
int attInsert(AsciiTrieTree *trie, char *key, void *value, int valueLen) {
	AsciiTrieTreeNode **rover;
	AsciiTrieTreeNode *node;
	char *p;
	int ret = 0;

	if (_asciiTrieTreeInit == 0) {
		pErr("Must call attInit() first.\n");
		return ret;
	}

	/* Cannot insert NULL values */

	if (value == TRIE_NULL) {
		return ret;
	}

	// Search down the trie until we reach the end of string,
	// creating nodes as necessary

	rover = &trie->root;

	p = key;

	AsciiTrieTreeNode *tmp = NULL;

	for (;;) {

		node = *rover;

		if (tmp == NULL) {
            // tmp will be freed if it is unused at end of loop.
            tmp = (AsciiTrieTreeNode *) calloc(1, sizeof(AsciiTrieTreeNode));
            if (tmp != NULL)
                tmp->inUse = 1;
        }

        if (tmp == NULL) {
            // Allocation failed.  Go back and undo
            // what we have done so far.
            _attRollback(trie, key);

            return ret;
        }

		AsciiTrieTreeNode *expect = NULL;

        // Trying to avoid locks here.
        if (AtomicExchange(&node, &expect, &tmp) == 1) {
            *rover = tmp;
            tmp = NULL;     // Set tmp so another will be allocated.
        } else {
            // Another thread beat us in adding node.
            // Do not free tmp here.
        }

        // Increase the node useCount
        AtomicAdd(&node->useCount, 1);

		// Reached the end of string?  If so, we're finished.
		if (*p == '\0') {
			if (node->data != NULL) {
				// printf(" **** Freeing node data.\n");
				free(node->data);
				node->data = NULL;
				ret = 2;
			} else {
				ret = 1;
			}
			node->data = (void *)calloc(1, valueLen + 1);
			memcpy((char *)node->data, (char *)value, valueLen);
			AtomicAdd(&node->inUse, 1);
			break;
		}

		// Advance to the next node in the chain
		rover = &node->next[_toAsciiIdx(*p)];
		++p;
	}

	if (tmp != NULL)
		free(tmp);

	return ret;
}

int attDelete(AsciiTrieTree *trie, char *key) {
	AsciiTrieTreeNode *node;

	if (_asciiTrieTreeInit == 0) {
		pErr("Must call attInit() first.\n");
		return -1;
	}

	node = attFindEnd(trie, key);

	if (node != NULL) {
		if (node->data != NULL) {
			free(node->data);
			node->data = NULL;
		}
		if (node->useCount > 0)
			node->useCount--;
		if (node->useCount == 0)
			node->inUse = 0;
	} else {
		return -1;		// record not found.
	}

	return 0;
}

void *attLookup(AsciiTrieTree *trie, char *key) {
	AsciiTrieTreeNode *node;

	if (_asciiTrieTreeInit == 0) {
		pErr("Must call attInit() first.\n");
		return NULL;
	}

	node = attFindEnd(trie, key);

	if (node != NULL) {
		return node->data;
	} else {
		return TRIE_NULL;
	}
}

int attNumEntries(AsciiTrieTree *trie) {
	// To find the number of entries, simply look at the use count
	// of the root node.

	if (_asciiTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return 0;
	}

	if (trie->root == NULL) {
		return 0;
	} else {
		return AtomicGet(&trie->root->useCount);
	}
}

#define IDX(c)	((int)c - (int)'0')

int _digitalTrieTreeInit = 0;

DigitalTrieTree *dttInit() {

	DigitalTrieTree *dttRoot = (DigitalTrieTree *) calloc(1, sizeof(DigitalTrieTree));
	if (dttRoot == NULL)
		return NULL;
	dttRoot->root = NULL;

	_digitalTrieTreeInit = 1;

	return dttRoot;
}

/*
 * Function to find end of tree for a given IP address.
 *
 *   ip = A dotted IP address string like, "192.168.0.1"
 */
DigitalTrieTreeNode *dttFindEnd(DigitalTrieTree *trie, char *key) {
	DigitalTrieTreeNode *node;
	char *p;

	if (_digitalTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return NULL;
	}

	// Search down the trie until the end of string is reached

	node = trie->root;

	for (p = key; *p != '\0'; ++p) {

		if (node == NULL) {
			// Not found in the tree. Return.
			return NULL;
		}

		// Jump to the next node
		node = node->next[IDX(*p)];
	}

	if (node == NULL)
		return NULL;

	return node;
}

/*
 * Function _dttRollback is private to this file.
 */
static void _dttRollback(DigitalTrieTree *trie, char *key) {
	DigitalTrieTreeNode *node;
	DigitalTrieTreeNode **prev_ptr;
	DigitalTrieTreeNode *next_node;
	DigitalTrieTreeNode **next_prev_ptr;
	char *p;

	// Follow the chain along.  We know that we will never reach the
	// end of the string because dttInsert never got that far.  As a
	// result, it is not necessary to check for the end of string
	// delimiter (NUL)

	node = trie->root;
	prev_ptr = &trie->root;
	p = key;

	while (node != NULL) {

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[IDX(*p)];
		next_node = *next_prev_ptr;
		++p;

		// Decrease the use count and free the node if it
		// reaches zero.

		AtomicSub(&node->useCount, 1);

		if (node->useCount == 0) {
			free(node);

			if (prev_ptr != NULL) {
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

/*
 * Function dttInsert is used to insert data into trie tree.
 */
int dttInsert(DigitalTrieTree *trie, char *key, void *value, int valueLen) {
	DigitalTrieTreeNode **rover;
	DigitalTrieTreeNode *node;
	char *p;
	int ret = 0;

	if (_digitalTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return ret;
	}

	/* Cannot insert NULL values */

	if (value == TRIE_NULL) {
		return ret;
	}

	// Search down the trie until we reach the end of string,
	// creating nodes as necessary

	rover = &trie->root;

	p = key;

	DigitalTrieTreeNode *tmp = NULL;

	for (;;) {

		node = *rover;

		if (tmp == NULL) {
            // tmp will be freed if it is unused at end of loop.
            tmp = (DigitalTrieTreeNode *) calloc(1, sizeof(DigitalTrieTreeNode));
            if (tmp != NULL)
                tmp->inUse = 1;
        }

        if (tmp == NULL) {
            // Allocation failed.  Go back and undo
            // what we have done so far.
            _dttRollback(trie, key);

            return ret;
        }

        DigitalTrieTreeNode *expect = NULL;

        // Trying to avoid locks here.
        if (AtomicExchange(&node, &expect, &tmp) == 1) {
            *rover = tmp;
            tmp = NULL;     // Set tmp so another will be allocated.
        } else {
            // Another thread beat us in adding node.
            // Do not free tmp here.
        }

        // Increase the node useCount
        AtomicAdd(&node->useCount, 1);

		// Reached the end of string?  If so, we're finished.
		if (*p == '\0') {
			if (node->data != NULL) {
				printf(" **** Freeing node data.\n");
				free(node->data);
				node->data = NULL;
				ret = 2;
			} else {
				ret = 1;
			}
			node->data = (void *)calloc(1, valueLen + 1);
			memcpy((char *)node->data, (char *)value, valueLen);
			AtomicAdd(&node->inUse, 1);
			break;
		}

		// Advance to the next node in the chain
		rover = &node->next[IDX(*p)];
		++p;
	}

	if (tmp != NULL)
		free(tmp);

	return ret;
}

int dttDelete(DigitalTrieTree *trie, char *key) {
	DigitalTrieTreeNode *node;

	if (_digitalTrieTreeInit == 0) {
		pErr("Must call dttInit() first.\n");
		return -1;
	}

	node = dttFindEnd(trie, key);

	if (node != NULL) {
		if (node->data != NULL) {
			free(node->data);
			node->data = NULL;
		}
		if (node->useCount > 0)
			node->useCount--;
		if (node->useCount == 0)
			node->inUse = 0;
	} else {
		return -1;		// record not found.
	}

	return 0;
}

void *dttLookup(DigitalTrieTree *trie, char *key) {
	DigitalTrieTreeNode *node;

	if (_digitalTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return NULL;
	}

	node = dttFindEnd(trie, key);

	if (node != NULL) {
		return node->data;
	} else {
		return TRIE_NULL;
	}
}

int dttNumEntries(DigitalTrieTree *trie) {
	// To find the number of entries, simply look at the use count
	// of the root node.

	if (_digitalTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return 0;
	}

	if (trie->root == NULL) {
		return 0;
	} else {
		return AtomicGet(&trie->root->useCount);
	}
}

int _hexTrieTreeInit = 0;

static inline int _toHexIdx(char ch) __attribute__((always_inline));

HexTrieTree *httInit() {

	HexTrieTree *httRoot = (HexTrieTree *) calloc(1, sizeof(HexTrieTree));
	if (httRoot == NULL)
		return NULL;
	httRoot->root = NULL;

	_hexTrieTreeInit = 1;

	return httRoot;
}

/*
 * Function _toHexIdx is private to this file.
 */
static inline int _toHexIdx(char ch) {
	// Only allow hex characters, both upper and lower case.
	if (ch > 47 && ch < 58)
		return ((int)ch - (int)'0');
	else if ((int)ch > 64 && ch < 71)
		return ((int)ch - (int)'A') + 10;
	else if ((int)ch > 96 && ch < 103)
		return ((int)ch - (int)'a') + 10;
	else {
		pErr("ERROR: Not a Hex character.\n");
		return 0;
	}
}

/*
 * Function to find end of tree for a given IP address.
 */
HexTrieTreeNode *httFindEnd(HexTrieTree *trie, char *key) {
	HexTrieTreeNode *node;
	char *p;

	if (_hexTrieTreeInit == 0) {
		pErr("Must call httInit() first.\n");
		return NULL;
	}

	// Search down the trie until the end of string is reached

	node = trie->root;

	for (p = key; *p != '\0'; ++p) {

		if (node == NULL) {
			// Not found in the tree. Return.
			return NULL;
		}

		// Jump to the next node
		node = node->next[_toHexIdx(*p)];
	}

	if (node == NULL || node->inUse == 0)
		return NULL;

	return node;
}

/*
 * Function _httRollback is private to this file.
 */
static void _httRollback(HexTrieTree *trie, char *key) {
	HexTrieTreeNode *node;
	HexTrieTreeNode **prev_ptr;
	HexTrieTreeNode *next_node;
	HexTrieTreeNode **next_prev_ptr;
	char *p;

	// Follow the chain along.  We know that we will never reach the
	// end of the string because httInsert never got that far.  As a
	// result, it is not necessary to check for the end of string
	// delimiter (NUL)

	node = trie->root;
	prev_ptr = &trie->root;
	p = key;

	while (node != NULL) {

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[_toHexIdx(*p)];
		next_node = *next_prev_ptr;
		++p;

		// Decrease the use count and free the node if it
		// reaches zero.

		AtomicSub(&node->useCount, 1);

		if (node->useCount == 0) {
			free(node);

			if (prev_ptr != NULL) {
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

/*
 * Function httInsert is used to insert data into trie tree.
 */
int httInsert(HexTrieTree *trie, char *key, void *value, int valueLen) {
	HexTrieTreeNode **rover;
	HexTrieTreeNode *node;
	char *p;
	int ret = 0;

	if (_hexTrieTreeInit == 0) {
		pErr("Must call httInit() first.\n");
		return ret;
	}

	/* Cannot insert NULL values */

	if (value == TRIE_NULL) {
		return ret;
	}

	// Search down the trie until we reach the end of string,
	// creating nodes as necessary

	rover = &trie->root;

	p = key;

	HexTrieTreeNode *tmp = NULL;

	for (;;) {

		node = *rover;

		if (tmp == NULL) {
            // tmp will be freed if it is unused at end of loop.
            tmp = (HexTrieTreeNode *) calloc(1, sizeof(HexTrieTreeNode));
            if (tmp != NULL)
                tmp->inUse = 1;
        }

        if (tmp == NULL) {
            // Allocation failed.  Go back and undo
            // what we have done so far.
            _httRollback(trie, key);

            return ret;
        }

        HexTrieTreeNode *expect = NULL;

        // Trying to avoid locks here.
        if (AtomicExchange(&node, &expect, &tmp) == 1) {
            *rover = tmp;
            tmp = NULL;     // Set tmp so another will be allocated.
        } else {
            // Another thread beat us in adding node.
            // Do not free tmp here.
        }

        // Increase the node useCount
        AtomicAdd(&node->useCount, 1);

		// Reached the end of string?  If so, we're finished.
		if (*p == '\0') {
			if (node->data != NULL) {
				free(node->data);
				node->data = NULL;
				ret = 2;
			} else {
				ret = 1;
			}
			node->data = (void *)calloc(1, valueLen + 1);
			memcpy((char *)node->data, (char *)value, valueLen);
			AtomicAdd(&node->inUse, 1);
			break;
		}

		// Advance to the next node in the chain
		rover = &node->next[_toHexIdx(*p)];
		++p;
	}

	if (tmp != NULL)
		free(tmp);

	return ret;
}

int httDelete(HexTrieTree *trie, char *key) {
	HexTrieTreeNode *node;

	if (_hexTrieTreeInit == 0) {
		pErr("Must call httInit() first.\n");
		return -1;
	}

	node = httFindEnd(trie, key);

	if (node != NULL) {
		if (node->data != NULL) {
			free(node->data);
			node->data = NULL;
		}
		if (node->useCount > 0)
			node->useCount--;
		if (node->useCount == 0)
			node->inUse = 0;
	} else {
		return -1;		// record not found.
	}

	return 0;
}

void *httLookup(HexTrieTree *trie, char *key) {
	HexTrieTreeNode *node;

	if (_hexTrieTreeInit == 0) {
		pErr("Must call httInit() first.\n");
		return NULL;
	}

	node = httFindEnd(trie, key);

	if (node != NULL) {
		return node->data;
	} else {
		return TRIE_NULL;
	}
}

int httNumEntries(HexTrieTree *trie) {
	// To find the number of entries, simply look at the use count
	// of the root node.

	if (_hexTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return 0;
	}

	if (trie->root == NULL) {
		return 0;
	} else {
		return AtomicGet(&trie->root->useCount);
	}
}

int _octalTrieTreeInit = 0;

static inline int _toOctalIdx(char ch) __attribute__((always_inline));

OctalTrieTree *ottInit() {

	OctalTrieTree *ottRoot = (OctalTrieTree *) calloc(1, sizeof(OctalTrieTree));
	if (ottRoot == NULL)
		return NULL;
	ottRoot->root = NULL;

	_octalTrieTreeInit = 1;

	return ottRoot;
}

/*
 * Function _toOctalIdx is private to this file.
 */
static inline int _toOctalIdx(char ch) {
	// Only allow octal characters, both upper and lower case.
	if (ch > 47 && ch < 56) {
		return ((int)ch - (int)'0');
	} else {
		pErr("ERROR: Not a Octal character. (%c)(%d)\n", ch, ch);
		return 0;
	}
}

/*
 * Function to find end of tree for a octal value.
 */
OctalTrieTreeNode *ottFindEnd(OctalTrieTree *trie, char *key) {
	OctalTrieTreeNode *node;
	char *p = NULL;

	if (_octalTrieTreeInit == 0) {
		pErr("Must call ottInit() first.\n");
		return NULL;
	}

	// Search down the trie until the end of string is reached

	node = trie->root;
	for (p = key; *p != '\0'; ++p) {

		if (node == NULL) {
			// Not found in the tree. Return.
			return NULL;
		}

		// Jump to the next node
		node = node->next[_toOctalIdx(*p)];
	}

	if (node == NULL || node->inUse == 0)
		return NULL;

	return node;
}

/*
 * Function _ottRollback is private to this file.
 */
static void _ottRollback(OctalTrieTree *trie, char *key) {
	OctalTrieTreeNode *node;
	OctalTrieTreeNode **prev_ptr;
	OctalTrieTreeNode *next_node;
	OctalTrieTreeNode **next_prev_ptr;
	char *p = NULL;

	// Follow the chain along.  We know that we will never reach the
	// end of the string because ottInsert never got that far.  As a
	// result, it is not necessary to check for the end of string
	// delimiter (NUL)

	node = trie->root;
	prev_ptr = &trie->root;
	p = key;

	while (node != NULL) {

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[_toOctalIdx(*p)];
		next_node = *next_prev_ptr;
		++p;

		// Decrease the use count and free the node if it
		// reaches zero.

		AtomicSub(&node->useCount, 1);

		if (node->useCount == 0) {
			free(node);

			if (prev_ptr != NULL) {
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

/*
 * Function ottInsert is used to insert data into trie tree.
 */
int ottInsert(OctalTrieTree *trie, char *key, void *value, int valueLen) {
	OctalTrieTreeNode **rover;
	OctalTrieTreeNode *node;
	char *p = key;
	int ret = 0;

	if (_octalTrieTreeInit == 0) {
		pErr("Must call ottInit() first.\n");
		return ret;
	}

	/* Cannot insert NULL values */

	if (value == TRIE_NULL) {
		return ret;
	}

	// Search down the trie until we reach the end of unsigned int,
	// creating nodes as necessary

	rover = &trie->root;

	OctalTrieTreeNode *tmp = NULL;

	for (;;) {

		node = *rover;

		if (tmp == NULL) {
            // tmp will be freed if it is unused at end of loop.
            tmp = (OctalTrieTreeNode *) calloc(1, sizeof(OctalTrieTreeNode));
            if (tmp != NULL)
                tmp->inUse = 1;
        }

        if (tmp == NULL) {
            // Allocation failed.  Go back and undo
            // what we have done so far.
            _ottRollback(trie, key);

            return ret;
        }

        OctalTrieTreeNode *expect = NULL;

        // Trying to avoid locks here.
        if (AtomicExchange(&node, &expect, &tmp) == 1) {
            *rover = tmp;
            tmp = NULL;     // Set tmp so another will be allocated.
        } else {
            // Another thread beat us in adding node.
            // Do not free tmp here.
        }

        // Increase the node useCount
        AtomicAdd(&node->useCount, 1);

		// Reached the end of string?  If so, we're finished.
		if (*p == '\0') {
			if (node->data != NULL) {
				free(node->data);
				node->data = NULL;
				ret = 2;
			} else {
				ret = 1;
			}
			node->data = (void *)calloc(1, valueLen + 1);
			memcpy((char *)node->data, (char *)value, valueLen);
			AtomicAdd(&node->inUse, 1);
			break;
		}

		// Advance to the next node in the chain
		rover = &node->next[_toOctalIdx(*p)];
		++p;
	}

	if (tmp != NULL)
		free(tmp);

	return ret;
}

int ottDelete(OctalTrieTree *trie, char *key) {
	OctalTrieTreeNode *node;

	if (_octalTrieTreeInit == 0) {
		pErr("Must call ottInit() first.\n");
		return -1;
	}

	node = ottFindEnd(trie, key);

	if (node != NULL) {
		if (node->data != NULL) {
			free(node->data);
			node->data = NULL;
		}
		if (node->useCount > 0)
			node->useCount--;
		if (node->useCount == 0)
			node->inUse = 0;
	} else {
		return -1;		// record not found.
	}

	return 0;
}

void *ottLookup(OctalTrieTree *trie, char *key) {
	OctalTrieTreeNode *node;

	if (_octalTrieTreeInit == 0) {
		pErr("Must call ottInit() first.\n");
		return NULL;
	}

	node = ottFindEnd(trie, key);

	if (node != NULL) {
		return node->data;
	} else {
		return TRIE_NULL;
	}
}

int ottNumEntries(OctalTrieTree *trie) {
	// To find the number of entries, simply look at the use count
	// of the root node.

	if (_octalTrieTreeInit == 0) {
		pErr("Must call ttInit() first.\n");
		return 0;
	}

	if (trie->root == NULL) {
		return 0;
	} else {
		return AtomicGet(&trie->root->useCount);
	}
}
