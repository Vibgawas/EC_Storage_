#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_operation.h"
#include "helper.h"

int main(){
     
      // getting unique_ID
      long int unique_ID = 10000;
      
      while (1) {
      
      printf("ObjStorage >");
      
      // to read command line with arguments
      char cmd[20];
      scanf("%s", cmd);
      
       
      if (strcmp(cmd,"put")==0){
      
          // taking file path from user 
          char path[100];
          scanf("%s",path);
          
          int n = put(unique_ID,path);
          
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
          
      	  
      }
      
      else if (strcmp(cmd,"get")==0){
      
           // taking unique_ID of file from user
	   long int unique_file_ID;
	   scanf("%ld",&unique_file_ID); 
	   
	   // user input for file_path
	   char path[20];
	   scanf("%s",path);
	   
           int a = get(unique_file_ID, path);
           
           if (a==0){
              printf("Object retrieved successfully !!\n");
           }
           else if (a==1){
              printf("Chunk not found !!\n");
           }
           else if (a==2){
              printf("Object with ID %ld does not exist !!\n",unique_file_ID);
           }
               
      }
      
      else if (strcmp(cmd,"list")==0){
          list();
          
      }
      else if (strcmp(cmd,"quit")==0){
          exit(0);
      }
      else{
          printf("\nInvalid command !!!\n\nfollow below pattern\n>put file_path\n>get unique_ID file path\n>list\n>quit\n");
      }
      printf("\n");
      }     
      
      return 0;
}
