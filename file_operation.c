#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<dirent.h>
#include "helper.h"


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


struct fileinfo* st[HASH_VAL];


int put(long int file_ID, char* path){
    
    int segments, i, len, accum;
    char *file_ex;
    char *file_name;
    
    
    // Opening the file in read mode
    int fp1 = open(path, O_RDONLY);
    
    // checking if the file exist or not
    if (fp1)
    {

            // calculating file size
            long int file_size = findSize(path);
            
            if(file_size != -1)
	    {
	        // getting size of each chunk
	        segments = get_chunk_size(file_size/NO_OF_D_CHUNK);
	    
	        //printf("%d",segments);
	    
	    
	        int bytesreader;
	        char smallFileName[segments];
	        unsigned char buffer[segments];

	    
	    
	        // checking if EC_Storage directory present or not
	        DIR* storage = opendir("EC_Storage");
    
		if(!storage){
		       get_storage(NO_OF_D_CHUNK,NO_OF_P_CHUNK);
		}
		
		for(i=1;i<=(NO_OF_D_CHUNK);i++)
		{
		    // checking for extra or less no of bytes remaining for last chunk
		    if(i==NO_OF_D_CHUNK){
		       segments = (file_size-(segments*(NO_OF_D_CHUNK-1)));
		    }
		   
		    bytesreader = read(fp1,buffer,segments);
		    
		    sprintf(smallFileName, "EC_Storage/data_chunk_%d/_%ld_",i,file_ID);
		    
		    // Opening the file in create and write mode
		    int fp2 = open(smallFileName, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		    
		    write(fp2,buffer,bytesreader);
		    
		    // Closing the file
		    close(fp2);
		      
		}
		
		// Closing the file
	        close(fp1);
	    
               // extracting name of file with extension
               file_name = getFileNameFromPath(path,'/');
      
               // storing file data into structure 
               insert_data(file_ID,file_name,file_size);
     
            
               return SUCCESS;
	    }
	    else{
	       return FILE_IS_EMPTY;
	    }
	       	    
      }
      else{
          
          return FILE_NOT_FOUND;
      }
      

}


int get(long int unique_f_ID,char* path){

    struct fileinfo* data_node;
    data_node = getnode(unique_f_ID);
    
    if(data_node){
      
      // concatinating user path and file_name extracted from database( structure ) 
      strcat(path,"/");
      strcat(path,data_node->file_name);

      for(int i=1;i<=NO_OF_D_CHUNK;i++){
     
         // file where we store concatinated data
         int op_file = open(path, O_APPEND|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
         
         // getting path of file from unique_ID
         char f_path[80];
         sprintf(f_path, "%s/%s_%d/_%ld_",STORAGE,CHUNK,i,unique_f_ID);
      
         // opening the file
         int fp1 = open(f_path,O_RDONLY);
         
         char c;
         if(fp1){
               
   		while (read(fp1, &c, sizeof(c)) == sizeof(c)) {  
        		if (write(op_file, &c, sizeof(c)) != sizeof(c))
            		    break;  
    		} 
    		
    	  // closing the file
    	  close(fp1);
    	  close(op_file);
    	  } 
    	  
    	  else{
    	     return CHUNK_NOT_FOUND;
    	     break;
    	  }
   
       }
      
      return SUCCESS;      
      
    }
    else{
      return FILE_DOES_NOT_EXIST;
   }
}

void list(){

    struct fileinfo* node=NULL;
       
    for(int i=0;i<HASH_VAL;i++){
            node = st[i];
            while(node!=NULL){
            
                display(node);
                node = node->next;
            }      
	}
}







