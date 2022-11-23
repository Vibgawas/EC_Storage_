#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include "helper.h"


# define default_unique_ID 10000


struct fileinfo st[50];

void insert_data(long int unique_id,char *file_name,int file_size,struct fileinfo st[]){
    
    
    int index = (int)unique_id%default_unique_ID;
    
    st[index].unique_id = unique_id;
    strcpy(st[index].file_name,file_name);
    st[index].file_size = file_size;
   
      
}

void get_data(struct fileinfo st[],int len){

    printf("unique_id      file name    filesize\n\n");
    
    for(int i=0;i<2;i++){
    
       printf("%ld\t\t",st[i].unique_id);
       printf("%s\t\t",st[i].file_name);
       printf("%d\n",st[i].file_size);
       
    } 
  
}

long int findSize(char file_name[])
{
	// opening the file in read mode
       int fp = open(file_name, O_RDONLY);

	// checking if the file exist or not
	if (fp == -1) {
		printf("File Not Found!\n");
		return -1;
	}

        // Calculating size of file
	long int size = lseek(fp, 0, SEEK_END);

	
	// closing the file
	close(fp);

	return size;
}

int get_chunk_size(float num){

       return num <0?num-0.5:num+0.5;
}

char* getFileNameFromPath(char* path, char c)
    {
       for(size_t i = strlen(path) - 1; i; i--)  
       {
            if (path[i] == c)
            {
                return &path[i+1];
            }
        }
        return path;
    }
    
    
void get_folder(int no_of_d_chunk,int no_of_p_chunk){
        
        char fn[50];
        mkdir("EC_Storage",0777);
        
        for(int i=1;i<=no_of_d_chunk;i++){
      	    sprintf(fn, "EC_Storage/data_chunk_%d", i);
            mkdir(fn,0777);
        }
        
        for(int i=0;i<no_of_p_chunk;i++){
      	    sprintf(fn, "EC_Storage/parity_chunk_%d", i);
            mkdir(fn,0777);
        }
        
             
}


void clear_struct(struct fileinfo st[]){
     int len = sizeof(struct fileinfo);
     
     for(int i=0;i<len;i++){
            st[i].unique_id = 0;
	    
     }
}





