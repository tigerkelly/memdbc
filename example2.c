
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "memdbc.h"
#include "trietree.h"

typedef struct _data_ {
	char name[64];
	int age;
	char address[80];
	long suite;
	char city[32];
	char state[16];
	char zip[16];
} Data_t;

Data_t data[] = {
	{"Kelly Wiles", 64, "4205 Pinwood Drive", 0, "Plano", "TX", "12345"},
	{"John Doe", 6, "9999 Lie Drive", 10002, "Anywere", "TX", "21345"},
	{"Jane Doe", 6, "9999 Lie Drive", 10002, "Anywere", "TX", "21345"},
	{"Larry Doe", 16, "1111 Pickle Drive", 600, "Somewere", "TX", "12345-106"}
};

char *saveCallback(char *key, void *data);
char *walkCallback(char *key, void *data);
void findCallback(char *key, void *data);

int main(int argc, char *argv[]) {

	MemDbc_t *memDbc = memDbcInit(ASCII_DB);

	// Add a few reorcds to DB.
	memDbcAdd(memDbc, data[0].name, &data[0], sizeof(Data_t));
	memDbcAdd(memDbc, data[1].name, &data[1], sizeof(Data_t));
	memDbcAdd(memDbc, data[2].name, &data[2], sizeof(Data_t));
	memDbcAdd(memDbc, data[3].name, &data[3], sizeof(Data_t));

	// Search for a single record.
	Data_t *p = (Data_t *)memDbcFind(memDbc, "John Doe");

	if (p != NULL) {
		printf("Found record: Key=%s, Value=%s,%d,%s,%ld%s,%s\n",
				"John Doe", p->name, p->age, p->address, p->suite, p->city, p->state);
	}

	// Search for a single record with key 'john Doex' which it should NOT find.
	p = (Data_t *)memDbcFind(memDbc, "John Doex");

    if (p == NULL) {
        printf("Record NOT Found: Key=%s\n", "John Doex");
	}

	// Search for a single record with key 'john Do' which it should NOT find.
	p = (Data_t *)memDbcFind(memDbc, "John Do");

    if (p == NULL) {
        printf("Record NOT Found: Key=%s\n", "John Do");
	}

	// Find all records that match the regex.
	memDbcFindAll(memDbc, "[.]*Doe", findCallback);

	// Print number of records in DB.
	printf("Record Count: %lu\n", memDbcNumEntries(memDbc));

	// Print all records in DB.
	memDbcWalk(memDbc, walkCallback);

	// Save all records to an asci text file.
	// If fileName is NULL then caller handles saving data.
	memDbcSave(memDbc, "data2.txt", saveCallback);

	return 0;
}

// You could combine save and walk but I kept them separate for readability.

// Returns a comma separated string of the record.
// The string will be freed by the memDbcSave function.
char *saveCallback(char *key, void *data) {
	char *s = NULL;

	Data_t *d = (Data_t *)data;

	s = calloc(1, sizeof(Data_t) + 2);

	sprintf(s, "%s,%s,%d,%s,%ld,%s,%s,%s",
			key, d->name, d->age, d->address, d->suite, d->city, d->state, d->zip);

	return s;
}

// Returns a comma separated string of the record.
// The string will be freed by the memDbcSave function.
char *walkCallback(char *key, void *data) {
	char *s = NULL;

	Data_t *d = (Data_t *)data;

	s = calloc(1, sizeof(Data_t) + 40);

	sprintf(s, "Key=%s,Name=%s,Age=%d,Address=%s,Suite=%ld,City=%s,State=%s,Zip=%s",
			key, d->name, d->age, d->address, d->suite, d->city, d->state, d->zip);

	return s;
}

void findCallback(char *key, void *data) {

	Data_t *d = (Data_t *)data;
	printf("Regex Found: Key=%s, Value=%s, %d, %s, %lu, %s, %s, %s\n",
			key, d->name, d->age, d->address, d->suite, d->city, d->state, d->zip);
}