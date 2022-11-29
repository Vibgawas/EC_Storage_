#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<dirent.h>
#include "helper.h"


# define no_of_d_chunk 4
# define no_of_p_chunk 3
# define storage "EC_Storage"
# define chunk "data_chunk"


struct fileinfo st[50];
int record_count = 0;



void put(long int file_ID){

    int segments, i, len, accum;
    char *file_ex;
    char *file_name;
    
    // taking file path from user 
    char path[100];
    scanf("%s",path);
    
    // calculating file size
    long int file_size = findSize(path);
    

    // checking if the file exist or not
    if (file_size != -1)
    {

	    // getting size of each chunk
	    segments = get_chunk_size(file_size/no_of_d_chunk);
	    //printf("%d",segments);
	    
	    
	    int bytesreader;
	    char smallFileName[segments];
	    unsigned char buffer[segments];

	    // Opening the file in read mode
	    int fp1 = open(path, O_RDONLY);
	    if(fp1)
	    {
	        // checking if EC_Storage directory present or not
	        DIR* dir = opendir("EC_Storage");
    
		if(!dir){
		       get_folder(no_of_d_chunk,no_of_p_chunk);
		}
		
		for(i=1;i<=(no_of_d_chunk);i++)
		{
		    // checking for extra or less no of bytes remaining for last chunk
		    if(i==no_of_d_chunk){
		       segments = (file_size-(segments*(no_of_d_chunk-1)));
		    }
		   
		    bytesreader = read(fp1,buffer,segments);
		    
		    sprintf(smallFileName, "EC_Storage/data_chunk_%d/_%ld_",i,file_ID);
		    
		    // Opening the file in create and write mode
		    int fp2 = open(smallFileName, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		    
		    write(fp2,buffer,bytesreader);
		    
		    // Closing the file
		    close(fp2);
		      
		}
		printf("%ld\n",file_ID);
	    }
	    // Closing the file
	    close(fp1);
      }
      
      // extracting name of file with extension
      file_name = getFileNameFromPath(path,'/');
      
      // storing file data into structure 
      insert_data(file_ID,file_name,file_size,st);
      
      record_count++;
}

char *get_uid_data(long int unique_ID,struct fileinfo st[]){

    for(int i=0;i<record_count;i++){
    
      if(st[i].unique_id==unique_ID){
      
         return st[i].file_name;
      }
   }
   
}

void get(){

   // taking unique_ID of file from user
   long int unique_f_ID;
   scanf("%ld",&unique_f_ID); 
   
   // user input for file_path
   char path[20];
   scanf("%s",path);
   
   // checking for given unique_ID present in database or not
   char *file_name;
   file_name = get_uid_data(unique_f_ID,st);
   
 
   if(file_name){
     
     
     // concatinating user path and file_name extracted from database( structure )  
     strcat(path,"/");
     strcat(path,file_name);
     
     
     
     for(int i=1;i<=no_of_d_chunk;i++){
     
         // file where we store concatinated data
         int op_file = open(path, O_APPEND|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
         
         // getting path of file from unique_ID
         char f_path[80];
         sprintf(f_path, "%s/%s_%d/_%ld_",storage,chunk,i,unique_f_ID);
      
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
    	       printf("Chunk not found !!\n");
    	       break;
    	  }
   
      }
      printf("Object retrieved successfully !!\n");
      
      
      
   }
   else{
      printf("Object with ID %ld does not exist\n",unique_f_ID);
   }   
}



void list(){
 
    if(record_count>0){
	    printf("unique_id       file name       filesize\n\n");
	    
	    for(int i=0;i<record_count;i++){
	    
	       printf("%ld\t\t",st[i].unique_id);
	       printf("%s\t\t",st[i].file_name);
	       printf("%d bytes\n",st[i].file_size);
	       
	    }
   }
   else{
       printf("List is empty !!\n");
   }
}





