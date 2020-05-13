#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

struct my_struct
{
  const char *msg;
} my_struct;

static jerry_value_t readline_handler (const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  char * buffer = readline(">");
  return jerry_create_string (buffer);
}

static jerry_value_t cat_handler (const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  return jerry_create_string ("cat");
}

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
               const jerry_length_t arguments_count)
{
  /* There should be at least one argument */
  if (arguments_count > 0)
  {
    /* Convert the first argument to a string (JS "toString" operation) */
    jerry_value_t string_value = jerry_value_to_string (arguments[0]);

    /* A naive allocation of buffer for the string */
    jerry_char_t buffer[256];

    /* Copy the whole string to the buffer, without a null termination character,
     * Please note that if the string does not fit into the buffer nothing will be copied.
     * More details on the API reference page
     */
    jerry_size_t copied_bytes = jerry_string_to_utf8_char_buffer (string_value, buffer, sizeof (buffer) - 1);
    buffer[copied_bytes] = '\0';

    /* Release the "toString" result */
    jerry_release_value (string_value);

    int filetoread = open(buffer,O_RDONLY);

    struct stat sb;
    fstat(filetoread, &sb);
    char *filecontent = malloc(sb.st_size);

    read(filetoread,filecontent,sb.st_size);
    close(filetoread);

    printf ("Joke handler was called with args %s\n", (const char *)buffer);

    jerry_value_t returnvalue = jerry_create_string (filecontent);

    free(filecontent);

    return returnvalue;
  }
  else
    printf ("Joke handler was called\n");

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined ();
}


int main (void)
{
  int jsmain = open("./cli.js",O_RDONLY);

  struct stat sb;
  fstat(jsmain, &sb);
  char *buf = malloc(sb.st_size);

  read(jsmain,buf,sb.st_size);
  close(jsmain);

  /* Initializing JavaScript environment */
  jerry_init (JERRY_INIT_EMPTY);

  /* Register 'print' function from the extensions */
  jerryx_handler_register_global ((const jerry_char_t *) "print", jerryx_handler_print);

  /* Do something with the native object */
  my_struct.msg = "Hello native object!";

  /* Create an empty JS object */
  jerry_value_t object = jerry_create_object ();

  /* Create a JS function object and wrap into a jerry value */
  jerry_value_t func_obj = jerry_create_external_function (get_msg_handler);

  /* Set the native function as a property of the empty JS object */
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "myFunc");
  jerry_release_value (jerry_set_property (object, prop_name, func_obj));
  jerry_release_value (prop_name);
  jerry_release_value (func_obj);

  /* Wrap the JS object (not empty anymore) into a jerry api value */
  jerry_value_t global_object = jerry_get_global_object ();

  /* Add the JS object to the global context */
  prop_name = jerry_create_string ((const jerry_char_t *) "MyObject");
  jerry_release_value (jerry_set_property (global_object, prop_name, object));
  jerry_release_value (prop_name);
  jerry_release_value (object);
  jerry_release_value (global_object);

  /* Constructing strings */
  prop_name = jerry_create_string ((const jerry_char_t *) "my_var");
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

  jerry_value_t property_name_readline = jerry_create_string ((const jerry_char_t *) "readline");
  jerry_value_t property_value_func_readline = jerry_create_external_function (readline_handler);

  /* Add the "joke" property with the function value to the "global" object */
  set_result = jerry_set_property (global_object, property_name_readline, property_value_func_readline);

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'readline' property\n");
  }

  jerry_release_value (set_result);

  jerry_release_value (property_name_readline);
  jerry_release_value (property_value_func_readline);

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
