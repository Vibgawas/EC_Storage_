#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include "helper.h"


#define DB_DATA_FOLDER "data_chunk"
#define DB_PARITY_FOLDER "parity_chunk"


extern struct fileinfo* st[HASH_VAL]; 

void insert_data(long int unique_id,char *file_name,int file_size){
     int index = 0;
     
     index = GET_HASH_INDEX(unique_id);
     
     struct fileinfo* node = NULL;
     
     node = (struct fileinfo*) malloc(sizeof(struct fileinfo*));
     
     if(node!=NULL){
       
       node->unique_id = unique_id;
       strcpy(node->file_name,file_name);
       node->file_size = file_size;
       
       if(st[index]==NULL){
          st[index]=node;
       }
       else{
          node->next = st[index];
          st[index] = node;
       }
       
     }
     
}

void display(struct fileinfo* node){

	printf("%ld\t\t",node->unique_id);
	printf("%s\t\t",node->file_name);
	printf("%d bytes\n",node->file_size);
	        
}

struct fileinfo* getnode(long int unique_ID){

       struct fileinfo* node=NULL;
       
       node = st[GET_HASH_INDEX(unique_ID)];
       
       while(node!=NULL && node->unique_id != unique_ID){
            
            node = node->next;
       }
       return node;   
}


long int get_file_size(char file_name[])
{
	// opening the file in read mode
       int fp = open(file_name, O_RDONLY);

	// checking if the file exist or not
	if (fp == -1) {
		return -1;
	}

        // Calculating size of file
	long int size = lseek(fp, 0, SEEK_END);

	
	// closing the file
	close(fp);

	return size;
}

int get_chunk_size(int file_size, int no_of_data_chunk){
       
       int chunk_size;
       
       chunk_size = file_size/no_of_data_chunk;
       
       if(file_size % no_of_data_chunk){
           chunk_size = chunk_size+1;
       }
       return chunk_size;
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
    
    
void get_storage(int no_of_d_chunk,int no_of_p_chunk){
        
        char fn[50];
        mkdir("EC_Storage",0777);
        
        for(int i=1;i<=no_of_d_chunk;i++){
      	    sprintf(fn, "EC_Storage/data_chunk_%d", i);
            mkdir(fn,0777);
        }
        
        for(int i=1;i<=no_of_p_chunk;i++){
      	    sprintf(fn, "EC_Storage/parity_chunk_%d", i);
            mkdir(fn,0777);
        }
                   
}


int readline(char* cmd){
   
   if (strcmp(toLower(cmd),"put")==0){
      return 0;
   }
   else if (strcmp(toLower(cmd),"get")==0){
      return 1;
   }
   else if (strcmp(toLower(cmd),"list")==0){
      return 2;
   }
   else if (strcmp(toLower(cmd),"quit")==0){
      return 3;
   }
   else{
      return -1;
   }
  
}


char* toLower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}


void clear_file_data(char* folder_path){

       struct dirent *next_file;

       DIR *theFolder = opendir(folder_path);
       
       if(theFolder){
       char file_path[768];
       
       while ( (next_file = readdir(theFolder)) != NULL)
       {
        
        // build the path for each file in the folder
        sprintf(file_path, "%s/%s",folder_path,next_file->d_name);
        unlink(file_path);
        
       } 
       
       closedir(theFolder);
       }
 
}


void clear_dir(char* path){

    char data_folder_path[512];
    char parity_folder_path[512];
    
    for(int i=1;i<=4;i++){
        
       sprintf(data_folder_path,"%s/%s_%d",path,DB_DATA_FOLDER,i);
       clear_file_data(data_folder_path);
       
       sprintf(parity_folder_path,"%s/%s_%d",path,DB_PARITY_FOLDER,i);
       clear_file_data(parity_folder_path);  
    }
    
   
}


