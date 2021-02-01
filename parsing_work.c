#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512

struct command {
  char *call;
  char *args[];      
};

int main(void)
{
  char cmd[CMDLINE_MAX];
  char original[CMDLINE_MAX];
  char *parsed[16];
  char *token;
  int i = 0; 
  
  fgets(cmd, CMDLINE_MAX, stdin);
  
  /*strcpy(original, cmd);
  
  token = strtok(cmd, " ");
  while(token)
  {
    for(int j = 0; strlen(token); j++)
    {
    |&>
    
    }
    parsed[i] = token;
    token = strtok(NULL, " ");
    i++;
  }*/
  
}
