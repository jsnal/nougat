#include "jsmn.h"
#include "config.h"

//change up this struct stuff
struct config_options {
  struct category {
    bool windows;
    bool linux;
  };
  
  struct repos {
  
  };
};

jsmn_parser parser;
jsmntok_t tokens[10];

jsmn_init(&parser);

//change up the token variables when editing the structs
jsmn_parse(&parser, config_js, strlen(js), tokens, 10);
