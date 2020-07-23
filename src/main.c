#include "utils.h"
#include "config.h"

static void usage(char *binary);

void usage(char *binary)
{
  /* TODO: Write a proper usage output */
  fprintf(stdout, "%s: Usage\n", binary);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{

  int c;
  
  while (1) {
  
    static struct option long_options[] = 
      {
        {"config", required_argument, 0, 'c'}
      };
      
    int option_index = 0;
    
    c = getopt_long (argc, argv, "c:", long_options, &option_index);
    
    if(c == -1)
      break;
      
    switch (c) {
      case 'c':
        //set filepath to config_path
        
        fp = fopen(config_path, "r");
        
        if(fp == NULL) {
          perror("Couldn't open config file");
          exit(EXIT_FAILURE);
        }
        
        fgets(config_js, strlen(config_js), fp); 
        break;
      
    }
  
	
  D fprintf(stderr, __PG_NAME__": Warning DEBUG is on\n");

  if (argc <= 1) usage(argv[0]);

  create_header();
  
  

  return 0;
}
