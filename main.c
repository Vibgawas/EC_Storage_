#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_operation.h"
#include "helper.h"

int main(){
     
      // getting unique_ID
      int unique_ID = 10000;
      
      while (1) {
      
      printf("ObjStorage >");
      
      // to read command line with arguments
      char cmd[20];
      scanf("%s", cmd);
      
       
      if (strcmp(cmd,"put")==0){
          put(unique_ID);
          unique_ID++;
      	  
      }
      else if (strcmp(cmd,"get")==0){
          get();
          
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
