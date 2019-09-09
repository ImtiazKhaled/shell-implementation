/*

  Name: Imtiaz Mujtaba Khaled
  ID: 1001551928

*/
// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell now supports ten arguments

#define HISTORY_NUM 15          // The number of commands to store

#define SHELL_NAME "msh>"

// Structure to store the previous 15 commands in
typedef struct hist{
 
  char command[MAX_COMMAND_SIZE];
  pid_t command_pid;

}hist;


// This function handels the two specified singals it receives from main
static void handle_signal (int sig ){
  switch( sig )
  {
    case SIGINT: 
    printf("\n%s ", SHELL_NAME);
    break;

    case SIGTSTP: 
      printf("Caught cntrl Z\n");
    break;

    default: 
      printf("Unable to determine the signal\n");
    break;

  }
}

// This function handels all the shell operations 
void shell_operations(char * cmd_str, int (*history_index), hist * history){
  
  int i;
  
  // Ignores user input if it is a blank line
  if(strcmp(cmd_str, "\n") == 0)  return;
  
  /* Parse input */
  char *token[MAX_NUM_ARGUMENTS];
  int   token_count = 0;                                 
                                                         
  // Pointer to point to the token
  // parsed by strsep
  char *arg_ptr;                                         
                                                         
  char *working_str  = strdup( cmd_str );

  // we are going to move the working_str pointer so
  // keep track of its original value so we can deallocate
  // the correct amount at the end
  char *working_root = working_str;
  
  // Stores the pid for the current process
  pid_t curr_pid;
  char *curr_command = strdup(working_root);
  curr_command = strtok(curr_command, "\n");  // Removes trailing new line from input
  
  // Tokenize the input stringswith whitespace used as the delimiter
  while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
            (token_count<MAX_NUM_ARGUMENTS)){
    token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
    if( strlen( token[token_count] ) == 0 ){

      token[token_count] = NULL;
    }
      token_count++;

  }
  // Now print the tokenized input as a debug check
  if(strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0){

  // Exit when user input is exit or quit        
    exit(0);

  }else if(strcmp(token[0],"cd") == 0){
  
    if(chdir(token[1]) != 0){ // Check if directory entered exits
      printf("that directory does not exist.\n");
    }else{
    // Directory changed, if it exits
    }

  }else if((strcmp(token[0],"listpids") == 0)|| (strcmp(token[0],"showpids") == 0)){

    // If input is listpids or showpids, then program iterates through the array
    // of hist structures to print out the command_pids
    i = 0;
    for(i = 0; i < (*history_index); ++i){
        printf("%d. %d\n", i, (int)history[i].command_pid);
    }

  }else if(strcmp(token[0],"history") == 0){

    // If the input is history, then the program iterates throught the array of
    // stucture hist, to print out the previous commands
    i = 0;
    for(i = 0; i < (*history_index); ++i){
        printf("%d. %s\n", i, history[i].command);
    }

  }else if(token[0][0] == '!'){
    // Handles the case where a previous command is repeated
    char* commandNumber = strtok_r(token[0], "!", &token[0]);
    
    if(commandNumber[0] >= '0' && commandNumber[0] <= '9') {  // Checks to see if the first chracter is an integer

      int cmdNum = atoi(commandNumber); // Converts the string input to an int
      if(cmdNum > (*history_index)){
    
        // If the command does not exist the next line of input is prompted
        printf("Sorry that command is not available.");

      }else{
      
        //Execute the command
        shell_operations(history[cmdNum].command, history_index, history);
      
      }

    }else{

      printf("%s: Command not found.\n", curr_command);


    }
  }else{
    
    printf("something else");

  }
  i = 1;
  if( (*history_index) == HISTORY_NUM ){

    // If the array is filled, then the program removes the first index, and 
    // replaces it next one, and iterates through the arary to do it through
    // the whole array, and inserts the current command to the end of the array
    for(i = 1; i < HISTORY_NUM; ++i){

        history[i - 1] = history[i];

    }
    strcpy(history[(*history_index) - 1].command, curr_command);
    history[(*history_index) - 1].command_pid = curr_pid;

  }else{

    // Program copies the current command and the current pid to the next free
    // array index of the hist structure 
    strcpy(history[(*history_index)].command, curr_command);
    history[(*history_index)].command_pid = curr_pid;
    ++(*history_index);

  }

  // int token_index  = 0;
  // for( token_index = 0; token_index < token_count; token_index ++ ){
  //   printf("token[%d] = %s\n", token_index, token[token_index] );  
  // }

  free( working_root );
  free( curr_command );

}


int main(){
  
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  
  // History and Showpids command helpers
  hist *history = (hist*)malloc(HISTORY_NUM*sizeof(hist));
  int history_index = 0;

  struct sigaction act;

  // Set all sigaction struct to 0
  memset(&act, '\0', sizeof(act));
    
  // Set handler to use the signal handle function
  act.sa_handler = &handle_signal;

  while( 1 ){
    // Print out the msh prompt
    printf("%s ", SHELL_NAME);

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    
    // Signal handlers
    if (sigaction(SIGINT , &act, NULL) < 0){
      perror("cntrl C error");
      return 0;
    }

    if (sigaction(SIGTSTP , &act, NULL) < 0){
      perror ("cntrl Z error");
      return 0;
    }    

    shell_operations(cmd_str, &history_index, history);

    // Call function here    
  }

  free( history );
  return 0;
}