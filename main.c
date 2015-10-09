#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

void strip_comment(char* input){

  for(int i = 0; i < strlen(input); i++)
  {
    if(input[i] == '\n')
     {
        input[i] = '\0';
     }
    if(input[i] == '#')
     {
      input[i] = '\0';
     }
  }
  
  return;

}

char** get_commands(char* input, char* splitter){

  char* commands;
  char* copy = strdup(input);
  
  int num = 1;
  
    // find out how many spots we will need in our arr, num starts at 1 to account for null character
    for(commands = strtok(copy,splitter); commands != NULL; commands = strtok(NULL,splitter)){
         num++;
   }
   //free and reset copy
   free(copy);
   copy = strdup(input);
   
   //allocate mem on the heap
   char **arr = malloc(num * sizeof(char*));
 
   char* cmd;

   int i = 0;
   for(cmd = strtok(copy, splitter); cmd != NULL; cmd = strtok(NULL, splitter)) {
        arr[i] = strdup(cmd);
        i++;
   }
  
   arr[i] = NULL;
  
  free(copy);
  return arr;
}

void free_commands(char **commands) {
    int i = 0;
    while (commands[i] != NULL) {
        free(commands[i]); // free each string
        i++;
    }
    free(commands); // then free the array
}


int exit_or_mode(char** input){  

    if(strcmp(input[0], "exit") == 0)
    {
      return 0; //signals to call exit()
    }

   else if(strcmp(input[0], "mode") == 0)
   {
         if(input[1] != NULL)
        {
           if(strcmp(input[1], "s") == 0 || strcmp(input[1],"sequential") == 0)
           {
             return 1;  
           }
           else if(strcmp(input[1],"p") == 0 || strcmp(input[1], "parallel")==0)
           {
             return 2;
           }
        }
       else
       {
         return 3;
       }
   }
   
   return -1; // signals that neither mode or exit were called

}

//code below is based on lab03 - fork03.c
void mode_seq(char** commands){
 
 int i = 0;
 while(commands[i] != NULL)
 {
   
   //int check = exit_or_more(command_line[i
  if(commands[0] == NULL)
   {
     continue;
   }
   
   if(strcmp(commands[0],"exit") == 0)
    {
       exit(0);
    }
    
   pid_t pid = fork();
   
   if(pid < 0)
   {
      printf("Process has failed for command: %s\n",commands[0]);
   } 
   
   else if(pid == 0)
   {
       if(execv(commands[0],commands) < 0)
       {
          fprintf(stderr, "execv failed: %s\n", strerror(errno));
       }
   }
   
   else if (pid > 0){
      int status =0;
      wait(&status);
    }
    
    i++;
 }  
 
  return;
}

void mode_par(char** commands){
  
  int kidz = 0; 
  int i = 0;
  while(commands[i] != NULL)
  {
   
   if(strcmp(commands[0],"exit") == 0)
    {
       exit(0);
    }
    
    kidz++;
    pid_t pid = fork();
    
    
   if(pid < 0)
   {
      printf("Process has failed for command: %s\n",commands[0]);
      
   } 
   
   else if(pid == 0)
   {
       if(execv(commands[0],commands) < 0)
       {
          fprintf(stderr, "execv failed: %s\n", strerror(errno));
       }
   }
 
  i++;
  
  } 
  
  int k = 0;
  
  while(k < kidz)
  {   
      int status =0;
      wait(&status);
      k++;
  }
  return;
  
}

int main(int argc, char **argv) {

//Sequential mode = 1 and Parallel mode = 2
int mode = 1;
bool run = true;

while(run)
{
    char** command_set = NULL;
    printf(">>> ");
    fflush(stdout);  
    char buffer[1024];
    
     if(fgets(buffer, 1024, stdin) != NULL) {
       strip_comment(buffer);
       command_set = get_commands(buffer, ";");
      } 
      
    else
    {
    run = false;
    }
    
    int k;
    for (k = 0; command_set[k] != NULL; k++) {
        //printf("%d %s\n", k, command_set[k]);
        char** args = get_commands(command_set[k], " \t\n\r");
        if (args[0] != NULL) {
           //calls functions to check if a built in command is called
        int j = exit_or_mode(args);
        
        //function returns 0 if exit has been called
        if(j == 0)
        { 
          free_commands(args);
          free_commands(command_set);
          exit(0);
        }  
        
      //j==3 means just mode has been called
      else if(j == 3) 
      {
         if(mode == 1)
         {
            printf("Mode: Sequential\n");
         }
         else if(mode == 2)
         {
            printf("Mode: Parallel\n");
         }
      }
      
      //j == 1 means mode change
      else if(j == 1)
      {
         mode = 1;
      }
      else if(j == 2)
      {
         mode = 2;
      }
      
      //otherwise a built in command was not called
      //execute command
      else if(j == -1)
      {
         //run sequentially
         if(mode == 1)
         {
           mode_seq(args);
         }
         
         //run in parallel mode      
         else if(mode == 2)
         {
           mode_par(args);
         }
       
      }
     
        }
        
     free_commands(args);   
    }
     
     free_commands(command_set);

}
    return 0;
}

