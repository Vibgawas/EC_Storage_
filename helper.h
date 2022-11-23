long int findSize(char file_name[]);

int get_chunk_size(float num);

char* getFileNameFromPath(char* path,char c);

void get_folder(int no_of_d_chunk,int no_of_p_chunk);

struct fileinfo
{
    long int unique_id;
    char file_name[20];
    int file_size;
    

};

void insert_data(long int unique_id,char *file_name,int file_size,struct fileinfo st[]);

void get_data(struct fileinfo st[],int len);


