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

/*  This project was worked on by Katrina Gross and Katie Puhala.
    We only worked on it when we were together, and thus we contributed equal amounts and to every part of the code.
    
    We believe our part one works, and dedicated the majority of our time to it. We had a few bugs for which we met with Professor Stratton to try to fix, but they took longer than expected and therefore we could not spend as much time as we wanted on Part 2. We have added some functions that we believe are needed for setting up the linked list for Part 2, but have commented them out in Main so that our Part 1 runs properly. We will describe what we would have done for the parts of Stage 2 that we did not get to finish in Main.

We are sorry we did not get to fully implement part 2, but we really did try and dedicated a lot of time to this project.
*/

//For stage two we create a struct node so that we can place the directories in a linked list
struct node 
{
   //wanted to pick a big enough array size to avoid seg faults
   char directory[256];
   struct node *next;
} ;

//this function is a modified version of our list_append function from hw03
void read_file(FILE* filename, struct node* head)
{
   struct node *new;
   char line[256];
   
   while(fgets(line,256,filename) != NULL)
   {
      //we want the directories to end with a '/' as they are necessary for the path to be valid (i.e. /bin needs to become /bin/)
      line[strlen(line) - 1] = '/';
      line[strlen(line)] = '\0';
      new = malloc(sizeof(struct node));
      strncpy(head->directory,line,256);
     if(head == NULL){
       new->next = NULL;
       head = new;
       }
     else
     { 
       struct node *temp  = head;
       while(temp->next != NULL)
       {
          temp = temp->next;
       }
    
       new->next = NULL;
       temp->next = new;
       //we know we have to free one of our struct node pointers and we believe that it must be temp because we still have the head pointer we passed it
       free(temp);
     }
   }
   return;   
}

//would call in main to free the nodes of the directories
void free_directory(struct node *list)
{
   while(list != NULL)
   {
      struct node *temp = list;
      list = list->next;
      free(temp);
   }
}

//function to check whether the user input is valid for any of the directories
bool check(struct node *dir, char* input)
{
  struct stat statresult;
  while(dir != NULL)
  {
     //don't want to call strcat on actual entry in linked list of directories and thus create a tempory pointer that will have the same contents as a directory
     char* temp = malloc(sizeof(char) * 256);
     strcpy(temp,dir->directory);
     char* complete = strcat(temp,input);
     int rv = stat(complete, &statresult);
     //rv == 0 means that the pathway is valid
     if(rv == 0)
     {
        return true;
     }
     
     dir = dir->next;
     free(temp);
  }
  
  return false;
}


//Replaces the # or the \n with a '\0' so that the command(s) can be parsed correctly
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

//this is a modified version of tokenify from lab 2
//we changed the parameters so we can use function on either ';' or whitespace
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

//also from lab 2, helps avoid memory leaks
void free_commands(char **commands) {
    int i = 0;
    while (commands[i] != NULL) {
        free(commands[i]); // free each string
        i++;
    }
    free(commands); // then free the array
}

//this function checks if an input is a built in command
int exit_or_mode(char** input){  
    
    //the input that is passed in has already been seperated twice by ';' and by whitespace so can use strcmp just looking at the first index of the input because that is where exit and mode would be called
    if(strcmp(input[0], "exit") == 0)
    {
      return 0; //signals to call exit()
    }

   else if(strcmp(input[0], "mode") == 0)
   {
        //if we find mode we must check the next index to see whether or not the user wants a mode change
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
   
   //these two if statements account for if it is not a valid command (i.e. was whitespace) or if exit was called
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
  //essentially the same as our mode sequential, however we have to take into account the number of child processes we create and then inside the second while loop we must run all the children before returning from the function
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

//Declarations for part 2
/*struct node *head = NULL;
FILE* file = fopen("shell-config","r");
read_file(file,head);
*/


while(run)
{
    //sets up needed variables to get user input and print our prompt
    char** command_set = NULL;
    printf(">>> ");
    fflush(stdout);  
    char buffer[1024];
     
     //strips the user input of # or newline and then seperates commands based on ';'
     if(fgets(buffer, 1024, stdin) != NULL) {
       strip_comment(buffer);
       command_set = get_commands(buffer, ";");
      } 
      
    else
    {
    run = false;
    }
    
    int k;
    //loops through each command that was just seperated by ';' and now seperates them each by whitespace
    for (k = 0; command_set[k] != NULL; k++) {
        //printf("%d %s\n", k, command_set[k]);
        char** args = get_commands(command_set[k], " \t\n\r");
        //this accounts for if a user has entered in whitespace and not an actual command
        if (args[0] != NULL) {
        //calls functions to check if a built in command is called
        int j = exit_or_mode(args);
        
        //function returns 0 if exit has been called
        if(j == 0)
        { 
          //must free before exiting
          free_commands(args);
          free_commands(command_set);
          exit(0);
        }  
        
      //j==3 means just mode has been called and thus we print the current mode
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
      //must free before running through the loop again  
     free_commands(args);   
    }
     
     free_commands(command_set);

}


    /*
         -here we would do an infinite while loop (as to not limit the number of jobs that can be run in the background) that checks the poll from the user
        - have a bool that tracks the state of the parallel func - ie if there is currently a process running (jobs will print these running processes)
        - if there is a user input then pause the program and run the process that was polled in sequential, then once that one is done (check for child carcass), then continue the process that was running in the background in parallel (using kill(pid, sigstop) and kill(pid, sigcont) - have pause PID and resume PID be bools that keep track of whether or not these have been updated )
        - if there was a poll, then execute the process with parallel to be in the backgorund, then prompt again, if not then prompt again
        add a wait command to the parallel process to ensure that the process is not interupted by a user input - put an if statement in parallel that will handle user input, wait for process to finsih, and print an error statement.
        - eventually once all of the processes are done and there are no more user inputs (use code from assignment - check all of the poll if statement possiblities) then exit if entered exit twice (keep track of if exit has been entered)
    */
    
    //free_directory(head);
    return 0;
}

