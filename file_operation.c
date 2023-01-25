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
#include "file_operation.h"



typedef unsigned char u8;

struct fileinfo* st[HASH_VAL];

int put(long int file_ID, char* path){

	int i,j,m,fp,len,rem_len,ret;
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
		      	}
		      	else{
		      		break;
		      	}	 
                }      
	}
	
	// Pick an encode matrix.
    	gf_gen_cauchy1_matrix(encode_matrix, m, k);
    	
    	// Initialize g_tbls from encode matrix
	ec_init_tables(k, p, &encode_matrix[k * k], g_tbls);
	
	// Generate EC parity blocks from sources
	ec_encode_data(len, k, p, g_tbls, frag_ptrs, &frag_ptrs[k]);
		   	
	rem_len = len;	
	// Fill data chunks
    	for (i=0; i<k ; i++){
    		if(i==k-1){
    			rem_len = ((file_len-1)-(len*(k-1)));
    		}
    		sprintf(chunk_name, "%s/%s_%d/_%ld_",DB_,CHUNK,i+1,file_ID);
    		
		// Opening the file in create and write mode
		int fp2 = open(chunk_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		
		for (j=0; j<rem_len ; j++){	      
		    	write(fp2,&frag_ptrs[i][j], 1);
                }
                close(fp2);		       
	}
   		
   	// Fill parity chunks
    	for (i=k; i<m ; i++) {
    		sprintf(chunk_name, "%s/%s_%d/_%ld_",DB_,CHUNK,i+1,file_ID);
    		
		// Opening the file in create and write mode
		int fp2 = open(chunk_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		for (j=0; j<len ; j++){	      
		    	write(fp2,&frag_ptrs[i][j], 1);
                }
                close(fp2);		       
	}
		
	// Closing the file
	close(fp);
	
	// extracting name of file with extension
        file_name = getFileNameFromPath(path,'/');
        	
        // storing file data into structure 
        insert_data(file_ID,file_name,file_len);
        
        return 0;	
	
}

int get(long int unique_f_ID,char* path){
	
	int i,j,m,ret,op_file,fp1,nerrs,len,rem_len;
	int k= NO_OF_D_CHUNK, p=NO_OF_P_CHUNK;
	long int file_len;
	unsigned char ch;
	char f_path[200];
	
	struct fileinfo* data_node;
    	data_node = getnode(unique_f_ID);
    	
    	//Fragment buffer pointers
	u8 *frag_ptrs[MMAX];
	u8 *frag_ptrs1[MMAX];
	u8 frag_err_list[MMAX];

    	m = k+p;
	nerrs = 0;
    	
    	
    	if(!data_node){
    		printf("fileerr");
    		return FILE_DOES_NOT_EXIST;
	}
	
	// concatinating user path and file_name extracted from database( structure ) 
      	strcat(path,"/");
      	strcat(path,data_node->file_name);
      	
	// opening file to store concatinated data
	op_file = open(path, O_APPEND|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	
	if(op_file==-1){
		printf("Directory does not exist !!");
		return -1;
	}

	// getting total file_size from database
	file_len = data_node->file_size;
	
	// calculating chunk size 
	rem_len = len = get_chunk_size(file_len-1,k);
	
	// Allocate the src & parity buffers to frag_ptrs1
    	for (i = 0; i < m; i++) {
		if (NULL == (frag_ptrs1[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			for(int ii=0; ii<len;ii++){
	    			frag_ptrs1[i][ii] = 0 ;
	    			
			}		
        	}
    	}

	// Allocate the src & parity buffers to frag_ptrs
	for (i = 0; i < m; i++) {
		if (NULL == (frag_ptrs[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			for(int ii=0; ii<len;ii++){
	    			frag_ptrs[i][ii] = 0 ;
	    			
			}		
        	}
    	}

      	for(i=0;i<m;i++){
         	// getting path of file from unique_ID
         	sprintf(f_path, "%s/%s_%d/_%ld_",DB_, CHUNK, i+1, unique_f_ID);
               
         	// opening the file
        	fp1 = open(f_path,O_RDONLY);
        	
        	// calculating size of last chunk for unqeual chunk size 
        	if(i==k-1){
    			rem_len = ((file_len-1)-(len*(k-1)));
    		}
    		
	        // Allocating data to frag_ptrs and frag_ptrs1
         	if(fp1!=-1){
        		for (j=0; j<rem_len ; j++){	      
	              		if(read( fp1, &ch ,1 ) == 1){
			   		frag_ptrs[i-nerrs][j] = ch;
			   		frag_ptrs1[i][j] = ch;
		      		}
		      				 
                	} 
   	  		// closing the file
 	  		close(fp1);
    	  	} 
    	  	else{
    	  		frag_err_list[nerrs] = i;
    	  		nerrs++;
    	  	}
    	  	rem_len = len;
	}
		
	if(nerrs!=0){
		printf("\nRead_soloman\n");
		read_soloman(frag_ptrs,frag_ptrs1,frag_err_list,nerrs,k,p,len);
	}	

	// concatinating data chunks
    	for (i=0; i<k ; i++) {
    		if(i==k-1){
    			rem_len = ((file_len-1)-(len*(k-1)));
    		}
		write(op_file,frag_ptrs1[i], rem_len);  	            			     
	}	
	// closing the file
	close(op_file);
	
	return SUCCESS;
	
}


void list(){
	struct fileinfo* node=NULL;  
	for(int i=0;i<HASH_VAL;i++){
        	node = st[i];
        	while(node!=NULL){
        		//printf("0x%x\n",node);
			display(node);
                	node = node->next;
            	}      
	}
}




