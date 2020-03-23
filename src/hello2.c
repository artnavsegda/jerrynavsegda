#include <string.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

struct my_struct
{
  const char *msg;
} my_struct;

/**
 * Get a string from a native object
 */
static jerry_value_t
get_msg_handler (const jerry_value_t func_value, /**< function object */
                 const jerry_value_t this_value, /**< this arg */
                 const jerry_value_t *args_p, /**< function arguments */
                 const jerry_length_t args_cnt) /**< number of function arguments */
{
  return jerry_create_string ((const jerry_char_t *) my_struct.msg);
} /* get_msg_handler */

static jerry_value_t
joke_handler (const jerry_value_t function_object,
               const jerry_value_t function_this,
               const jerry_value_t arguments[],
               const jerry_length_t argument_count)
{
  /* No arguments are used in this example */
  /* Print out a static string */
  printf ("Joke handler was called\n");

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined ();
}

int main (void)
{
  FILE * jsmain = fopen("./hello.js","r");
  char buf[1000];
  fread(buf,1000,1,jsmain);
  fclose(jsmain);

  /* Initializing JavaScript environment */
  jerry_init (JERRY_INIT_EMPTY);



  /* Register 'print' function from the extensions */
  jerryx_handler_register_global ((const jerry_char_t *) "print",
                                  jerryx_handler_print);

  /* Getting pointer to the Global object */
  jerry_value_t global_object = jerry_get_global_object ();

  /* Create a JS function object and wrap into a jerry value */
  jerry_value_t func_obj = jerry_create_external_function (get_msg_handler);

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

  jerry_value_t property_name_joke = jerry_create_string ((const jerry_char_t *) "joke");
  jerry_value_t property_value_func = jerry_create_external_function (joke_handler);

  /* Add the "joke" property with the function value to the "global" object */
  set_result = jerry_set_property (global_object, property_name_joke, property_value_func);

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'joke' property\n");
  }

  jerry_release_value (set_result);

  jerry_release_value (property_value_func);
  jerry_release_value (property_name_joke);

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
