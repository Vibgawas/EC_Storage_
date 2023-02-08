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
#include <isa-l.h>
#include "helper.h"
#include "file_operation.h"





extern struct fileinfo* st[HASH_VAL]; 

void insert_data(long int unique_id,char *file_name,int file_size){
     int index = 0;
     
     //printf("%ld\t\t",unique_id);
     //printf("%s\t\t",file_name);
     //printf("%d bytes\n",file_size);
	        
     index = GET_HASH_INDEX(unique_id);
     
     struct fileinfo* node = NULL;
     
     node = (struct fileinfo*) malloc(sizeof(struct fileinfo));
     
     if(node!=NULL){

       node->unique_id = unique_id;
       strncpy(node->file_name,file_name,file_size);
       node->file_size = file_size;
       node->next = NULL;
       //printf("%d, node address : 0x%x node_next : 0x%x\n ",index,node,node->next);
       
       if(st[index]==NULL){
          st[index]=node;
          //printf("reached 1\n");      
       }
       
       else{
          node->next = st[index];
          st[index] = node;
          //printf("reached 2\n"); 
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
        mkdir(DB_,0777);
        
        for(int i=0;i<no_of_d_chunk;i++){
      	    sprintf(fn, "%s/%s_%d",DB_, CHUNK, i+1);
            mkdir(fn,0777);
        }
        
        for(int i=no_of_d_chunk;i<no_of_d_chunk+no_of_p_chunk;i++){
      	    sprintf(fn, "%s/%s_%d",DB_, CHUNK, i+1);
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
    
    int total_chunks = NO_OF_D_CHUNK + NO_OF_P_CHUNK;
    
    for(int i=1;i<=total_chunks;i++){
        
       sprintf(data_folder_path,"%s/%s_%d",path, CHUNK, i);
       clear_file_data(data_folder_path);
       
       
    }
    
    
   
}


int read_soloman(u8 **frag_ptrs,u8 **frag_ptrs1,u8 * frag_err_list,int nerrs,int k,int p,int len){
	
	int i,j,m,ret;
	
	// Fragment buffer pointers	 
	u8 *recover_srcs[KMAX];
	u8 *recover_outp[KMAX];

	
	// Coefficient matrices
	u8 *encode_matrix, *decode_matrix;
	u8 *invert_matrix, *temp_matrix;
	u8 *g_tbls;
	u8 decode_index[MMAX];
	
	m = k+p;
	
	// Allocate coding matrices
    	encode_matrix = malloc(m*k);
    	decode_matrix = malloc(m * k);
	invert_matrix = malloc(m * k);
	temp_matrix = malloc(m * k);
	g_tbls = malloc(k * p * 32);
	
	
	if (encode_matrix == NULL || decode_matrix == NULL
	    || invert_matrix == NULL || temp_matrix == NULL || g_tbls == NULL) {
		printf("Test failure! Error with malloc\n");
		return -1;
	}
	
	// Allocate buffers for recovered data
	for (i = 0; i < p; i++) {
		if (NULL == (recover_outp[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			return -1;
		}
	}
	
	gf_gen_cauchy1_matrix(encode_matrix, m, k);

	// Initialize g_tbls from encode matrix
	ec_init_tables(k, p, &encode_matrix[k * k], g_tbls);
	
	// Find a decode matrix to regenerate all erasures from remaining frags
	ret = gf_gen_decode_matrix_simple(encode_matrix, decode_matrix,
					  invert_matrix, temp_matrix, decode_index,
					  frag_err_list, nerrs, k, m);
				  
	// Pack recovery array pointers as list of valid fragments
	for (i = 0; i < k; i++)
		recover_srcs[i] = frag_ptrs1[decode_index[i]];
		
	// Recover data
	ec_init_tables(k, nerrs, decode_matrix, g_tbls);
	
	ec_encode_data(len, k, nerrs, g_tbls, recover_srcs, recover_outp);
	
	int index= 0;
	for(i=0;i<m;i++){
		if(frag_err_list[index]==i){
			for(int k=0;k<len;k++){
				frag_ptrs1[i][k]=recover_outp[index][k];
		
			}
			index++;
		}	
	}
	
	printf("\n");
	printf("After data recovery : \n");
	printf("**************** frag_ptrs1 ****************\n");	    
	for (i=0; i<m; i++) {		    
		for (j=0; j<len ; j++){
			printf("%u ",frag_ptrs1[i][j]);   
		}		       
		printf("\n");
	}
	
	printf("\ncheck recovery of block {");
	for (i = 0; i < nerrs; i++) {
		printf(" %d", frag_err_list[i]);
		if (memcmp(recover_outp[i], frag_ptrs1[frag_err_list[i]], len)) {
			printf(" Fail erasure recovery %d, frag %d\n", i, frag_err_list[i]);
			return -1;
		}
	}
	printf(" } done all: Pass\n");
	
	return SUCCESS;	
}


static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index, u8 * frag_err_list, int nerrs, int k,
				       int m){
	int i, j, p, r;
	int nsrcerrs = 0;
	u8 s, *b = temp_matrix;
	u8 frag_in_err[MMAX];

	memset(frag_in_err, 0, sizeof(frag_in_err));
	
	// Order the fragments in erasure for easier sorting
	for (i = 0; i < nerrs; i++) {
		if (frag_err_list[i] < k)
			nsrcerrs++;
		frag_in_err[frag_err_list[i]] = 1;
	}
	
	printf("*********frag_err_list********\n");
        for(int iii = 0;iii<nerrs;iii++){
        	printf("%u ",frag_err_list[iii]);
        }
        printf("\n");
	
	// Construct b (matrix that encoded remaining frags) by removing erased rows
	for (i = 0, r = 0; i < k; i++, r++) {
		while (frag_in_err[r])
			r++;
		for (j = 0; j < k; j++)
			b[k * i + j] = encode_matrix[k * r + j];
		decode_index[i] = r;
	}
	
	printf("***************** b_Matrix ***************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",b[ii]);
		}
		printf("\n");
	}
	
	// Invert matrix to get recovery matrix
	if (gf_invert_matrix(b, invert_matrix, k) < 0)
		return -1;
	
	printf("************* invert_matrix ************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",invert_matrix[ii]);
		}
		printf("\n");
	}
	
	// Get decode matrix with only wanted recovery rows
	for (i = 0; i < nerrs; i++) {
		if (frag_err_list[i] < k)	// A src err
			for (j = 0; j < k; j++)
				decode_matrix[k * i + j] =
				    invert_matrix[k * frag_err_list[i] + j];
	}
	
	
	
	// For non-src (parity) erasures need to multiply encode matrix * invert
	for (p = 0; p < nerrs; p++) {
		if (frag_err_list[p] >= k) {	// A parity err
			for (i = 0; i < k; i++) {
				s = 0;
				for (j = 0; j < k; j++)
					s ^= gf_mul(invert_matrix[j * k + i],
						    encode_matrix[k * frag_err_list[p] + j]);
				
				decode_matrix[k * p + i] = s;
			}
		}
	}
	
	printf("*************decode matrix************\n");
	for (int ii=0; ii< nerrs ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",decode_matrix[ii]);
		}
		printf("\n");
	}
	return 0;
}






















