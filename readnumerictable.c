#include "readnumerictable.h"

#ifndef __EMSCRIPTEN__
#include <sqlite3.h>
#else
#include "sqlite3.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int callback(void *count, int argc, char **argv, char **azColName) {
	int *c = count;
	*c = atoi(argv[0]);
	return 0;
}

float* readnumerictables(char * filename, int nrColumns, int* nrow) {
	int nrTables;
	char** tNames = tableNames(filename, &nrTables);
	*nrow = 0;
	for (int i = 0; i < nrTables; i++) {
		int nn;
		float* xx = readnumerictable(filename, tNames[i], nrColumns, &nn);
		free(xx);
		(*nrow) += nn;
	}
	// now we know the total number;

	float* self = malloc(nrColumns * *nrow * sizeof(float));
	int k = 0;
	for (int i = 0; i < nrTables; i++) {
		int nn;
		float* xx = readnumerictable(filename, tNames[i], nrColumns, &nn);
		for (int j = 0; j < nrColumns * nn; j++) {
			self[k++] = xx[j];
		}
		free(xx);
		free(tNames[i]);
	}
	free(tNames);
	return self;
}


float* readnumerictable(char * filename, char* tablename, int nrColumns, int* nrow) {
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int kk = sqlite3_open(filename, &db);
	if (db == NULL || kk != SQLITE_OK)
	{
		printf("Failed to open DB ");
		printf(filename);
		printf(" with error code %i \n", kk);
		exit(1);
	}
	sqlite3_extended_result_codes(db, 1);

	char m[500];
	strcpy(m, "select count(*) from '");
	strcat(m, tablename);
	strcat(m, "';");
	char* zErrMsg;
	int rc = sqlite3_exec(db, m, callback, nrow, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	fprintf(stdout, m);
	fprintf(stdout, "%i rows \n", *nrow);
	// now, we have nrow
	float* res = malloc(nrColumns * *nrow * sizeof(float));


	strcpy(m, "select * from '");
	strcat(m, tablename);
	strcat(m, "';");

	int k = sqlite3_prepare_v2(db, m, -1, &stmt, NULL);
	if (k != SQLITE_OK) {
		printf("Failed to open DB ");
		printf("error code %i\n", k);
		printf(filename);
		printf("\n");
		exit(1);
	}

	int i = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		for (int j = 0; j < nrColumns; j++) {
			float f = (float)sqlite3_column_double(stmt, j);
			// i is row index, j is column
			res[j + nrColumns * i] = f;
		}
		i++;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	fprintf(stdout, "I pass the value %i along.\n", *nrow);
	return res;
}

/*
int* readintegertable(char * filename, char* tablename, int nrColumns, int* nrow) {
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int kk = sqlite3_open(filename, &db);
	if (db == NULL || kk != SQLITE_OK)
	{
		printf("Failed to open DB ");
		printf(filename);
		printf(" with error code %i \n", kk);
		exit(1);
	}
	sqlite3_extended_result_codes(db, 1);

	char m[500];
	strcpy(m, "select count(*) from '");
	strcat(m, tablename);
	strcat(m, "';");
	char* zErrMsg;
	int rc = sqlite3_exec(db, m, callback, nrow, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	// now, we have nrow
	long long int* res = malloc(nrColumns * *nrow * sizeof(long long int));


	strcpy(m, "select * from '");
	strcat(m, tablename);
	strcat(m, "';");

	int k = sqlite3_prepare_v2(db, m, -1, &stmt, NULL);
	if (k != SQLITE_OK) {
		printf("Failed to open DB ");
		printf("error code %i\n", k);
		printf(filename);
		printf("\n");
		exit(1);
	}

	int i = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		for (int j = 0; j < nrColumns; j++) {
			int f;
			if (j == 0) {
				int long long ff = sqlite3_column_integer(stmt, j);
				f = (int) (ff / 1000);
			} else {
				f = sqlite3_column_integer(stmt, j);
			}


			// i is row index, j is column
			res[j + nrColumns * i] = f;
		}
		i++;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return res;
}

*/


char** tableNames(char * filename, int* nrTables) {
	sqlite3 *db;

	int kk = sqlite3_open(filename, &db);
	if (db == NULL || kk != SQLITE_OK)
	{
		printf("Failed to open DB ");
		printf(filename);
		printf(" with error code %i \n", kk);
		exit(1);
	}
	sqlite3_extended_result_codes(db, 1);
	char* m = "SELECT count(tbl_name) FROM sqlite_master WHERE type='table' AND tbl_name != 'legend';";
	char* zErrMsg;
	int rc = sqlite3_exec(db, m, callback, nrTables, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}


	// now, we have nrTables
	char** res = malloc(*nrTables * sizeof(char*));

	char* n = "SELECT tbl_name FROM sqlite_master WHERE type='table' AND tbl_name != 'legend';";
	sqlite3_stmt *stmt;
	int k = sqlite3_prepare_v2(db, n, -1, &stmt, NULL);
	if (k != SQLITE_OK) {
		printf("Failed to open DB ");
		printf("error code %i\n", k);
		printf(filename);
		printf("\n");
		exit(1);
	}

	int i = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		char* tname = (char*)sqlite3_column_text(stmt, 0);
		res[i++] = strdup(tname);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return res;
}

