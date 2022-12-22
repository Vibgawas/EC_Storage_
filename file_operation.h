#define NO_OF_D_CHUNK 4  
#define NO_OF_P_CHUNK 3
#define STORAGE "EC_Storage"
#define CHUNK "data_chunk"
#define MAX_RECORDS 50

// Error code
#define SUCCESS 0
#define CHUNK_NOT_FOUND 1
#define FILE_DOES_NOT_EXIST 2
#define FILE_NOT_FOUND 3
#define FILE_IS_EMPTY 4
#define STORAGE_IS_FULL 5

int put(long int file_ID,char* path);
int get(long int unique_f_ID,char* path);
void list();
