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

#define MMAX 255
#define KMAX 255

typedef unsigned char u8;

struct fileinfo* st[HASH_VAL];

static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index,
				       u8 * frag_err_list, int nerrs, int k, int m);
				       
static int pre_encoding(u8 *encode_matrix, u8 *g_tbls, int m, int k, int p);

int read_soloman(u8 **frag_ptrs,u8 **frag_ptrs1,u8 * frag_err_list,int nerrs,int k,int p,int len);

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
	
	// Allocate the src & parity buffers
    	for (int i = 0; i < m; i++) {
		if (NULL == (frag_ptrs[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			for(int ii=0; ii<len ;ii++){
	    			frag_ptrs[i][ii] = 0 ;
			}		
        	}
    	}
    	
	// calculating file size
	file_len = get_file_size(path);
	    	
	printf("file size %ld\n",file_len);
	
	if(file_len == -1){
		return FILE_IS_EMPTY;
	}
	    	
	// calculating chunk size 
	rem_len = len = get_chunk_size(file_len-1,k);
	    	
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
		   		
	// Fill data chunks
    	for (i=0; i<k ; i++){
    		if(i==k-1){
    			rem_len = ((file_len-1)-(len*(k-1)));
    		}
    		sprintf(chunk_name, "EC_Storage/_chunk_%d/_%ld_",i+1,file_ID);
    		
		// Opening the file in create and write mode
		int fp2 = open(chunk_name, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		
		for (j=0; j<rem_len ; j++){	      
		    	write(fp2,&frag_ptrs[i][j], 1);
                }
                close(fp2);		       
	}
   		
   	// Fill parity chunks
    	for (i=k; i<m ; i++) {
    		sprintf(chunk_name, "EC_Storage/_chunk_%d/_%ld_",i+1,file_ID);
    		
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
	
	int i,j,m,ret,op_file,fp1,nerrs;
	int k= NO_OF_D_CHUNK, p=NO_OF_P_CHUNK;
	long int file_len,len=3*1024;
	unsigned char ch;
	char f_path[80];
	
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
	
	// Allocate the src & parity buffers
    	for (int i = 0; i < m; i++) {
		if (NULL == (frag_ptrs1[i] = malloc(len))) {
			printf("alloc error: Fail\n");
			for(int ii=0; ii<len ;ii++){
	    			frag_ptrs1[i][ii] = 0 ;
			}		
        	}
    	}

      	for(int i=0;i<7;i++){
         	// getting path of file from unique_ID
         	sprintf(f_path, "%s/_%s_%d/_%ld_",STORAGE,CHUNK,i+1,unique_f_ID);
               
         	// opening the file
        	fp1 = open(f_path,O_RDONLY);
        	
        	
        		
         	//char ch;
         	
         	if(fp1!=-1){
         	
         		// calculating file size
			len = get_file_size(f_path);
			
			if (NULL == (frag_ptrs[i] = malloc(len))) {
				printf("alloc error: Fail\n");
				for(int ii=0; ii<len ;ii++){
	    				frag_ptrs[i-nerrs][ii] = 0 ;
				}		
        		}
        		for (j=0; j<len ; j++){	      
	              		if(read( fp1, &ch ,1 ) == 1){
			   		frag_ptrs[i-nerrs][j] = ch;
			   		frag_ptrs1[i][j] = ch;
		      		}
		      		else{
		      			break;
		      		}		 
                	} 
                	
                	
        		
        		
        		
   	  		// closing the file
 	  		close(fp1);
    	  	} 
    	  	else{
    	  		if (NULL == (frag_ptrs1[i] = malloc(len))) {
				printf("alloc error: Fail\n");
				for(int ii=0; ii<len ;ii++){
	    				frag_ptrs1[i][ii] = 0 ;
				}		
        		}
        		for (j=0; j<len ; j++){	      
			   	frag_ptrs1[i][j] = 0;			 
                	} 
                	
    	  		if (NULL == (frag_ptrs[i] = malloc(len))) {
				printf("alloc error: Fail\n");
				for(int ii=0; ii<len ;ii++){
	    				frag_ptrs[i][ii] = 0 ;
				}		
        		}
        		
    	  		frag_err_list[nerrs] = i;
    	  		nerrs++;
    	  		
    	     		//return CHUNK_NOT_FOUND;
    	     		//break;
    	  	}
	}
	
	printf("*********frag_err_list********\n");
        for(int iii = 0;iii<8;iii++){
        	printf("%u ",frag_err_list[iii]);
        }
        printf("\n");
	printf("**************** frag_MATRIX ****************\n");	    
	for (i=0; i<7; i++) {		    
		for (j=0; j<4 ; j++){
			printf("%u ",frag_ptrs[i][j]);   
		}		       
		printf("\n");
	}
	
	// concatinating user path and file_name extracted from database( structure ) 
      	strcat(path,"/");
      	strcat(path,data_node->file_name);
      	
	// file where we store concatinated data
	op_file = open(path, O_APPEND|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	
	if(nerrs==0){		
		// Fill parity chunks
    		for (i=0; i<4 ; i++) {
			for (j=0; j<4 ; j++){	      
		    		write(op_file,&frag_ptrs[i][j], 1);
                	}               			       
		}
		
		// closing the file
		close(op_file);	
	}
	else{
		printf("read_soloman\n");
		
		read_soloman(frag_ptrs,frag_ptrs1,frag_err_list,nerrs,4,3,len);
	}
	printf("hiii");
	return SUCCESS;
}


int read_soloman(u8 **frag_ptrs,u8 **frag_ptrs1,u8 * frag_err_list,int nerrs,int k,int p,int len){
	
	int i,j,m,ret;
	//int len = 8 * 1024;
	
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
					  
	
	printf("*********decode_index********\n");
        for(int iii = 0;iii<8;iii++){
        	printf("%u ",decode_index[iii]);
        }
					  
	// Pack recovery array pointers as list of valid fragments
	for (i = 0; i < k; i++)
		recover_srcs[i] = frag_ptrs1[decode_index[i]];
	
	printf("**************** recover_SRCS_MATRIX ****************\n");	    
	for (i=0; i<k ; i++) {		    
		for (j=0; j<len ; j++){
			printf("%u ",recover_srcs[i][j]);   
		}		       
		printf("\n");
	}
		
	// Recover data
	ec_init_tables(k, nerrs, decode_matrix, g_tbls);
	
	ec_encode_data(len, k, nerrs, g_tbls, recover_srcs, recover_outp);
	
	printf("**************** recover_OUTP_MATRIX ****************\n");	    
	for (i=0; i<p ; i++) {		    
		for (j=0; j<4 ; j++){
			printf("%u ",recover_outp[i][j]);   
		}		       
		printf("\n");
	}
	
	printf("*********frag_err_list********\n");
        for(int iii = 0;iii<8;iii++){
        	printf("%u ",frag_err_list[iii]);
        }
        printf("\n");
	
	int index= 0;
	for(i=0;i<m;i++){
		if(frag_err_list[index]==i){
			printf("in");
			for(int k=0;k<len;k++){
				frag_ptrs1[i][k]=recover_outp[index][k];
		
			}
			index++;
		}
		
	}
	printf("**************** frag_MATRIX ****************\n");	    
	for (i=0; i<7; i++) {		    
		for (j=0; j<4 ; j++){
			printf("%u ",frag_ptrs1[i][j]);   
		}		       
		printf("\n");
	}
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
	
	printf("*************gf_invert_matrix************\n");
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
	
	printf("*************encode matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",encode_matrix[ii]);
		}
		printf("\n");
	}
	printf("*************decode matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",decode_matrix[ii]);
		}
		printf("\n");
	}
	
	printf("*************b matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",b[ii]);
		}
		printf("\n");
	}
	return 0;
}

void list(){

	struct fileinfo* node=NULL;
       
	for(int i=0;i<HASH_VAL;i++){
        	node = st[i];
        	while(node!=NULL){
        		printf("0x%x\n",node);
			display(node);
                	node = node->next;
            	}      
	}
}


/*int main(){
	char path[100] = "/home/vaibhavi/Desktop/EC_store";
	//put1(10000,"/home/vaibhavi/Desktop/EC_store/EC__/xyz.txt");
	//printf("before");
	get1(10000,path);
	//printf("after");
	//list1();
	return 0;
	
}*/


