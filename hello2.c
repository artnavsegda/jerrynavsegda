#include <string.h>
#include <stdio.h>
#include "jerryscript.h"

static jerry_value_t
print_handler (const jerry_value_t function_object,
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

    printf ("%s\n", (const char *)buffer);
  }

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined ();
}

int
main (void)
{
  FILE * jsmain = fopen("./hello.js","r");
  char buf[1000];
  fread(buf,1000,1,jsmain);

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Add the "print" method for the JavaScript global object */
  {
    /* Get the "global" object */
    jerry_value_t global_object = jerry_get_global_object ();
    /* Create a "print" JS string */
    jerry_value_t property_name_print = jerry_create_string ((const jerry_char_t *) "print");
    /* Create a function from a native C method (this function will be called from JS) */
    jerry_value_t property_value_func = jerry_create_external_function (print_handler);
    /* Add the "print" property with the function value to the "global" object */
    jerry_value_t set_result = jerry_set_property (global_object, property_name_print, property_value_func);

    /* Check if there was no error when adding the property (in this case it should never happen) */
    if (jerry_value_is_error (set_result)) {
      printf ("Failed to add the 'print' property\n");
    }

    /* Release all jerry_value_t-s */
    jerry_release_value (set_result);
    jerry_release_value (property_value_func);
    jerry_release_value (property_name_print);
    jerry_release_value (global_object);
  }

  /* Setup Global scope code */
  jerry_value_t parsed_code = jerry_parse (NULL, 0, buf, strlen(buf), JERRY_PARSE_NO_OPTS);

  if (!jerry_value_is_error (parsed_code))
  {
    /* Execute the parsed source code in the Global scope */
    jerry_value_t ret_value = jerry_run (parsed_code);

    /* Returned value must be freed */
    jerry_release_value (ret_value);
  }

  /* Parsed source code must be freed */
  jerry_release_value (parsed_code);

  /* Cleanup engine */
  jerry_cleanup ();

  return 0;
}
