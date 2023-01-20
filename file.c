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

int encode(u8 **frag_ptrs, u8 *encode_matrix, u8 *g_tbls, int fp, int m, int k, int p, int file_len){

	int i,j,len,rem_len;
	unsigned char ch;
	int nerrs = 0;
	
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
		      	}
		      	else{
		      		break;
		      	}	 
                }      
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
    	
    	printf("***************** After (gf_cauchy_1 ) Encode_Matrix ***************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",encode_matrix[ii]);
	        }
	        printf("\n");
	}
	
    	printf("***************** before (EC_init ) g_tbls ***************\n");
    	for (int ii=0; ii< (k*p) ; ){
       	for (int jj=0; jj< k ; jj++, ii++){
          	printf("%u ",g_tbls[ii]);
       	}
       	printf("\n");
    	}
    	
    	// Initialize g_tbls from encode matrix
	ec_init_tables(k, p, &encode_matrix[k * k], g_tbls);
	
	printf("***************** after (EC_init ) g_tbls ***************\n");
    	for (int ii=0; ii< (k*p) ; ){
       	for (int jj=0; jj< k ; jj++, ii++){
          	printf("%u ",g_tbls[ii]);
       	}
       	printf("\n");
    	}
    
	// Generate EC parity blocks from sources
	ec_encode_data(len, k, p, g_tbls, frag_ptrs, &frag_ptrs[k]);
	
	printf("**************** After Ec_Encode frag_MATRIX ****************\n");	    
	for (i=0; i<m ; i++) {	    
		for (j=0; j<len ; j++){
			printf("%u ",frag_ptrs[i][j]);   
		}       
		printf("\n");
	}
		
	printf("***************** Encode_Matrix ***************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",encode_matrix[ii]);
		}
		printf("\n");
	}
	
	
	return 0;	
 
}


int put(long int file_ID, char* path){

	int i,j,m,fp,len,rem_len,ret;
	int k= NO_OF_D_CHUNK, p=NO_OF_P_CHUNK;
	long int file_len;
	unsigned char ch;
	char *file_name;
	char chunk_name[MMAX];
	
	// Fragment buffer pointers
	u8 *frag_ptrs[MMAX];
	u8 *recover_srcs[KMAX];
	u8 *recover_outp[KMAX];
	u8 frag_err_list[MMAX];
	
	
	frag_err_list[0] = 2;
	frag_err_list[1] = 3;

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
    	
    	
    	if (encode_matrix == NULL || g_tbls == NULL) {
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
        
        
        // Find a decode matrix to regenerate all erasures from remaining frags
	ret = gf_gen_decode_matrix_simple(encode_matrix, decode_matrix,
					  invert_matrix, temp_matrix, decode_index,
					  frag_err_list, 2, k, m);
				  
	// Pack recovery array pointers as list of valid fragments
	for (i = 0; i < k; i++)
		recover_srcs[i] = frag_ptrs[decode_index[i]];
		
	printf("\n");
	printf("\n");
	printf("\n");
	printf("**************** frag_MATRIX ****************\n");	    
	for (i=0; i<m ; i++) {		    
		for (j=0; j<len ; j++){
			printf("%u ",frag_ptrs[i][j]);   
		}		       
		printf("\n");
	}
	printf("**************** recover_SRCS_MATRIX ****************\n");	    
	for (i=0; i<k ; i++) {		    
		for (j=0; j<len ; j++){
			printf("%u ",recover_srcs[i][j]);   
		}		       
		printf("\n");
	}
	
	// Recover data
	ec_init_tables(k, 2, decode_matrix, g_tbls);
	
	printf("***************** Decode_Matrix ***************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",encode_matrix[ii]);
		}
		printf("\n");
	}
	
	ec_encode_data(len, k, 2, g_tbls, recover_srcs, recover_outp);
	
	printf("**************** recover_OUTP_MATRIX ****************\n");	    
	for (i=0; i<k ; i++) {		    
		for (j=0; j<len ; j++){
			printf("%u ",recover_outp[i][j]);   
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
				       int m)
{
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
        
        
        printf("*********frag_in_err********\n");
        for(int iii = 0;iii<8;iii++){
        	printf("%u ",frag_in_err[iii]);
        }
        printf("\n");
        
        printf("***************** b_Matrix ***************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",b[ii]);
		}
		printf("\n");
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
	
	printf("*************decode matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",decode_matrix[ii]);
		}
		printf("\n");
	}
	
	printf("*************gf_invert_matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",invert_matrix[ii]);
		}
		printf("\n");
	}
	
	printf("*************encode matrix************\n");
	for (int ii=0; ii< (m*k) ;){
		for (int jj=0;jj< k ;jj++, ii++){
			printf("%u ",encode_matrix[ii]);
		}
		printf("\n");
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




int get(long int unique_f_ID,char* path){

	int i,j,op_file,fp1;
	char f_path[80];

	struct fileinfo* data_node;
    	data_node = getnode(unique_f_ID);
    
    	if(!data_node){
    		return FILE_DOES_NOT_EXIST;
	}      
      	// concatinating user path and file_name extracted from database( structure ) 
      	strcat(path,"/");
      	strcat(path,data_node->file_name);
	
	// file where we store concatinated data
	op_file = open(path, O_APPEND|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
      	for(int i=1;i<=NO_OF_D_CHUNK;i++){
     

         
         	// getting path of file from unique_ID
         	sprintf(f_path, "%s/%s_%d/_%ld_",STORAGE,CHUNK,i,unique_f_ID);
      
         	// opening the file
         	fp1 = open(f_path,O_RDONLY);
         
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









