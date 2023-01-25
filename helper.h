# define default_unique_ID 10000
# define HASH_VAL 5 
# define GET_HASH_INDEX(X)	((int)X % HASH_VAL) 

#define DB_ "EC_Storage"
#define CHUNK "_chunk"

#define MMAX 255
#define KMAX 255

typedef unsigned char u8;

struct fileinfo
{
    long int unique_id;
    char file_name[256];
    int file_size;
    
    struct fileinfo* next;

};

struct fileinfo* getnode(long int unique_ID);



long int get_file_size(char file_name[]);

int get_chunk_size(int file_size, int no_of_data_chunk);

char* getFileNameFromPath(char* path,char c);

void get_storage(int no_of_d_chunk,int no_of_p_chunk);

void insert_data(long int unique_id,char *file_name,int file_size);

int readline(char* cmd);

char* toLower(char* s);

void display(struct fileinfo* node);

void clear_dir(char* path);

void clear_data(char* folder_path);

int read_soloman(u8 **frag_ptrs,u8 **frag_ptrs1,u8 * frag_err_list,int nerrs,int k,int p,int len);

static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index,
				       u8 * frag_err_list, int nerrs, int k, int m);
				       
				       
