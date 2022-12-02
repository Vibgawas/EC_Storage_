# define default_unique_ID 10000
# define HASH_VAL 5 
# define GET_HASH_INDEX(X)	((int)X % HASH_VAL) 


struct fileinfo
{
    long int unique_id;
    char file_name[256];
    int file_size;
    
    struct fileinfo* next;

};

long int findSize(char file_name[]);

int get_chunk_size(float num);

char* getFileNameFromPath(char* path,char c);

void get_storage(int no_of_d_chunk,int no_of_p_chunk);

void insert_data(long int unique_id,char *file_name,int file_size);

int readline(char* cmd);

char* toLower(char* s);

void display(struct fileinfo* node);

struct fileinfo* getnode(long int unique_ID);

