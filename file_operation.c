#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<dirent.h>
#include <getopt.h>
#include <isa-l.h>
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

#define MMAX 255

typedef unsigned char u8;

struct fileinfo* st[HASH_VAL];

int encode(u8 **frag_ptrs, u8 *encode_matrix, u8 *g_tbls, int fp, int m, int k, int p, int file_len){

	int i,j,len,rem_len;
	unsigned char ch;
	
	rem_len = len = get_chunk_size(file_len-1,k);
	printf("%d",len);
    	// Allocate the src & parity buffers
    	for (int i = 0; i < m; i++) {
		if (NULL == (frag_ptrs[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			for(int ii=0; ii<len ;ii++){
	    			frag_ptrs[i][ii] = 0 ;
			}		
        	}
    	}
    	
    	// Fill sources
    	for (i=0; i<k ; i++) {
    	        if(i==k-1){
    			rem_len = (file_len-1) - ((k-1)*len);
    		}
		for (j=0; j<rem_len ; j++){	      
	              	if(read( fp, &ch ,1 ) == 1){
			   	frag_ptrs[i][j] = ch;
			   	//printf("%u ",frag_ptrs[i][j]);
		      	}
		      	else{
		      		break;
		      	}	 
                }
                //printf("\n");       
	}
    	
    	printf("**************** frag_MATRIX ****************\n");	    
	    for (i=0; i<m ; i++) {
		    
		for (j=0; j<len ; j++){
		      printf("%u ",frag_ptrs[i][j]);   
		}
		       
		printf("\n");
	    }
		
	    printf("***************** before (gf_cauchy_1 ) Encode_Matrix ***************\n");
	    for (int ii=0; ii< (m*k) ;){
	       for (int jj=0;jj< k ;jj++, ii++){
		  printf("%u ",encode_matrix[ii]);
	       }
	       printf("\n");
	    }
	    
    	// Pick an encode matrix.
    	gf_gen_cauchy1_matrix(encode_matrix, m, k);
    	
    	// Initialize g_tbls from encode matrix
	ec_init_tables(k, p, &encode_matrix[k * k], g_tbls);

	// Generate EC parity blocks from sources
	ec_encode_data(len, k, p, g_tbls, frag_ptrs, &frag_ptrs[k]);
	
	    	printf("**************** frag_MATRIX ****************\n");	    
	    for (i=0; i<m ; i++) {
		    
		for (j=0; j<len ; j++){
		      printf("%u ",frag_ptrs[i][j]);   
		}
		       
		printf("\n");
	    }
		
	    printf("***************** before (gf_cauchy_1 ) Encode_Matrix ***************\n");
	    for (int ii=0; ii< (m*k) ;){
	       for (int jj=0;jj< k ;jj++, ii++){
		  printf("%u ",encode_matrix[ii]);
	       }
	       printf("\n");
	    }
	return 0;	
 
}
int put(long int file_ID, char* path){

	int i,j,m,fp,len,rem_len;
	int k= NO_OF_D_CHUNK, p=NO_OF_P_CHUNK;
	long int file_len;
	unsigned char ch;
	char *file_name;
	char chunk_name[MMAX];
	
	// Fragment buffer pointers
	u8 *frag_ptrs[MMAX];
	
	// Coefficient matrices
    	u8 *encode_matrix;
    	u8 *g_tbls;
    	
    	m = k+p;
    	
    	// Allocate coding matrices
    	encode_matrix = malloc(m*k);
    	g_tbls = malloc(k * p * 32);
    	
    	
    	if (encode_matrix == NULL || g_tbls == NULL) {
		printf("Test failure! Error with malloc\n");
		return -1;
	}
	
	fp = open(path,O_RDONLY);
	if(!fp){
        	return FILE_NOT_FOUND;
	}
	
	// calculating file size
	file_len = get_file_size(path);
	    	
	printf("file size %ld\n",file_len);
	if(file_len == -1){
		return FILE_IS_EMPTY;
	}
	    	
	// calculating chunk size
	rem_len = len = get_chunk_size(file_len-1,k);
	    	
	// Encoding data to generate parity 
	encode(frag_ptrs, encode_matrix, g_tbls, fp, m, k, p, file_len);
	    		
	// Fill data chunks
    	for (i=0; i<k ; i++){
    	
    		if(i==k-1){
    		
    			rem_len = ((file_len-1)-(len*(k-1)));
    		}
    			
    		sprintf(chunk_name, "EC_Storage/data_chunk_%d/_%ld_",i+1,file_ID);
    		
		// Opening the file in create and write mode
		int fp2 = open(chunk_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		
		for (j=0; j<rem_len ; j++){	      
		    	write(fp2,&frag_ptrs[i][j], 1);
                }
                close(fp2);		       
	}
   		
   	// Fill parity chunks
    	for (i=0; i<p ; i++) {
    	
    		sprintf(chunk_name, "EC_Storage/parity_chunk_%d/_%ld_",i+1,file_ID);
    		
		// Opening the file in create and write mode
		int fp2 = open(chunk_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		for (j=0; j<len ; j++){	      
		    	write(fp2,&frag_ptrs[i+k][j], 1);
                }
                close(fp2);		       
	}
		
	// Closing the file
	close(fp);
	
	// extracting name of file with extension
        file_name = getFileNameFromPath(path,'/');
        	
        // storing file data into structure 
        insert_data(file_ID,file_name,file_len);

        return SUCCESS;
	
}

/*int putt(long int file_ID, char* path){
    
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
	        segments = get_chunk_size(file_size,NO_OF_D_CHUNK);
	    
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
      

}*/


int get(long int unique_f_ID,char* path){

    struct fileinfo* data_node;
    data_node = getnode(unique_f_ID);
    
    if(data_node){
      
      // concatinating user path and file_name extracted from database( structure ) 
      strcat(path,"/");
      strcat(path,data_node->file_name);

      for(int i=1;i<=NO_OF_D_CHUNK;i++){
     
         // file where we store concatinated data
         int op_file = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
         
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

/*int main(){

   put(1001,"abc.txt");
   return 0;
}*/







