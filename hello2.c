#include <string.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

int main (void)
{
  FILE * jsmain = fopen("./hello.js","r");
  char buf[1000];
  fread(buf,1000,1,jsmain);

  /* Initializing JavaScript environment */
  jerry_init (JERRY_INIT_EMPTY);

  /* Register 'print' function from the extensions */
  jerryx_handler_register_global ((const jerry_char_t *) "print",
                                  jerryx_handler_print);

  /* Getting pointer to the Global object */
  jerry_value_t global_object = jerry_get_global_object ();

  /* Constructing strings */
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "my_var");
  jerry_value_t prop_value = jerry_create_string ((const jerry_char_t *) "Hello from C!");

  /* Setting the string value as a property of the Global object */
  jerry_value_t set_result = jerry_set_property (global_object, prop_name, prop_value);
  /* The 'set_result' should be checked if there was any error */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'my_var' property\n");
  }
  jerry_release_value (set_result);

  /* Releasing string values, as it is no longer necessary outside of engine */
  jerry_release_value (prop_name);
  jerry_release_value (prop_value);

  /* Releasing the Global object */
  jerry_release_value (global_object);

  /* Now starting script that would output value of just initialized field */
  jerry_value_t eval_ret = jerry_eval (buf, strlen(buf), JERRY_PARSE_NO_OPTS);

  /* Free JavaScript value, returned by eval */
  jerry_release_value (eval_ret);

  /* Freeing engine */
  jerry_cleanup ();

  return 0;
}
