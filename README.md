# memdbc
In memory database in C

This library uses both a Trie tree and a sorted link list to store data.  The data is stored in the Trie tree and the keys are stored in a sorted linked list.

The database size is limited on the amount of free memory in system.

Thier are two example programs, example1.c is a simple string data and example2.c is a C structure data.

The data you can store in the database can be anything, structures, strings or integers.

If you use a structure, use arrays instead of pointers to your data. This is because the library does not
know anything about your data. (see example2.c)

The Makefile builds both example executables and libmemdbc.a

The lbrary calls are:

	MemDbc_t *memDbcInit(DbTypes_t dbType);
		Call this function first with one of the database type to use.
		ASCI_DB - the key is a string of printable characters 95 in all but avoid using commas.  
		DIGITAL_DB - the key is all digits characters 0-9
		HEX_DB - the key is hexadecimal characters 0-9 and a-z or A-Z
		OCTAL_DB - the key is octal characters 0-8

	int memDbcAdd(MemDbc_t *memDbc, char *key, void *data, int len);
		This adds a record to the database.

	unsigned long memDbcNumEntries(MemDbc_t *memDbc);
		Returns the number of records in database.

	void memDbcWalk(MemDbc_t *memDbc, char *(callback)(char *key, void *data));
		Prints stdout all records in database in a sorted order.

	void *memDbcFind(MemDbc_t *memDbc, char *key);
		Find the record based on the key given.

	void memDbcSave(MemDbc_t *memDbc, char *fileName, char *(callback)(char *key, void *data));
		Saves the database to an ascii text file if the fileName is not NULL.
		The callback mainly formats the data into an string so the memDbcSave function write it
		to the file.
		If the fileName is NULL then file is not creaeted and no data is saved by the memDbcSave
		function, this allows the callback to process the record as they want.

	void memDbcFindAll(MemDbc_t *memDbc, char *regexStr, void (callback)(char *key, void *data));
		This uses a regex to find all reocrds that match pattern and calls the users callback
		function for each record found.

	int memDbcDelete(MemDbc_t *memDbc, char *key);
		Deletes a record from the database based on key given.

	MemDbcError_t memDbcError();
		Returns the error code.
