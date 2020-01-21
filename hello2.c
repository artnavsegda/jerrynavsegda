#include <string.h>
#include "jerryscript.h"

int main (void)
{
  FILE * jsmain = fopen("./hello.js","r");
  char buf[1000];
  fread(buf,1000,1,jsmain);

  bool ret_value = jerry_run_simple (buf, strlen(buf), JERRY_INIT_EMPTY);

  return (ret_value ? 0 : 1);
}
