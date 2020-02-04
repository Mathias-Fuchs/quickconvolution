#ifndef READNUMERICTABLE

#define READNUMERICTABLE
// nrColumns is input, nrow is output
float* readnumerictable(char * filename, char* tablename, int nrColumns, int* nrow);
long long int* readintegertable(char * filename, char* tablename, int nrColumns, int* nrow);
float* readnumerictables(char * filename, int nrColumns, int* nrow);
char** tableNames(char * filename, int* nrTables);
#endif
