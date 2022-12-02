#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_operation.h"
#include "helper.h"

#define PUT 0
#define GET 1
#define LIST 2
#define QUIT 3


int main(){
     
      // getting unique_ID
      long int unique_ID = 10000;
      long int unique_file_ID;
      char file_path[100];
      char user_path[100];
      
      while (1) {
      
      printf("ObjStorage >");
      
      // to read command line with arguments
      char cmd[20];
      scanf("%s", cmd);
      
      int a = readline(cmd);
      
      switch(a){
         case PUT:
         
              // taking file path from user 
			  scanf("%s",file_path);
			  
			  int n = put(unique_ID,file_path);
			  
			  if(n==0){
				 printf("%ld\n",unique_ID);
				 unique_ID++;
			  }
			  else if(n==4){
				 printf("Nothing to read in file !!");
			  }
			  else if(n==5){
				 printf("Storage is Full !!");
			  }
			  else{
				 printf("File not found !!");
			  }
			  
              break;
              
              
         case GET:
         
               // taking unique_ID of file from user
			   scanf("%ld",&unique_file_ID); 
			   
			   // taking path of file from user where user wants to store file
			   scanf("%s",user_path);
			   
			   int a = get(unique_file_ID, user_path);
				   
			   if (a==0){
				      printf("Object retrieved successfully !!\n");
			   }
			   else if (a==1){
				      printf("Chunk not found !!\n");
			   }
			   else if (a==2){
				      printf("Object with ID %ld does not exist !!\n",unique_file_ID);
			   }
			   
               break;
               
               
         case LIST:
         
              printf("unique_id       file name       filesize\n\n");
              list();
              break;
              
         case QUIT:
             // clear EC_storage
             // clear memory 
             exit(0);
              
         default:
            printf("\nInvalid command !!!\n\nfollow below pattern\n>put file_path\n>get unique_ID file path\n>list\n>quit\n");
            break;
      }
      
      printf("\n");
       
      }     
      
      return 0;
}
