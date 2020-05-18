#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "jerryscript.h"
#include "jerryscript-ext/module.h"
#include "jerryscript-ext/handler.h"

#define MODULE_NAME my_custom_module

static jerry_value_t very_useful_function(const jerry_value_t func_value, const jerry_value_t this_value, const jerry_value_t *args_p, const jerry_length_t args_cnt)
{
  return jerry_create_number (420);
}

static jerry_value_t module_cat_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
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

    char *filecontent = NULL;

    if (filetoread != -1)
    {
      struct stat sb;
      fstat(filetoread, &sb);
      filecontent = malloc(sb.st_size);

      int length = read(filetoread,filecontent,sb.st_size);
      filecontent[length] = '\0';
      close(filetoread);
    }
    else
    {
      filecontent = "empty";
    }

    //printf ("Cat was called with args %s\n", (const char *)buffer);

    //printf("contents: %s",filecontent);

    jerry_value_t returnvalue = jerry_create_string (filecontent);

    if (filetoread != -1)
      free(filecontent);

    return returnvalue;
  }
  else
    printf ("Cat handler was called\n");

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined();
}

static jerry_value_t module_system_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  if (arguments_count > 0)
  {
    jerry_value_t string_value = jerry_value_to_string (arguments[0]);
    jerry_char_t buffer[256];
    jerry_size_t copied_bytes = jerry_string_to_utf8_char_buffer (string_value, buffer, sizeof (buffer) - 1);
    buffer[copied_bytes] = '\0';

    jerry_release_value (string_value);

    printf ("System was called with args %s\n", (const char *)buffer);

    system(buffer);

    //printf("contents: %s",filecontent);

    return jerry_create_undefined();
  }
  else
    printf ("System handler was called\n");

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined();
}

static jerry_value_t module_pipe_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  if (arguments_count > 0)
  {
    jerry_value_t string_value = jerry_value_to_string (arguments[0]);
    jerry_char_t buffer[256];
    jerry_size_t copied_bytes = jerry_string_to_utf8_char_buffer (string_value, buffer, sizeof (buffer) - 1);
    buffer[copied_bytes] = '\0';

    jerry_release_value (string_value);

    printf ("Pipe was called with args %s\n", (const char *)buffer);

    FILE * filetoread = popen(buffer, "r");

    char filecontent[1000] = "empty";

    if (filetoread)
    {
      fread(filecontent,1000,1,filetoread);
      fclose(filetoread);
    }

    //printf("contents: %s",filecontent);

    jerry_value_t returnvalue = jerry_create_string (filecontent);

    return returnvalue;
  }
  else
    printf ("Joke handler was called\n");

  /* Return an "undefined" value to the JavaScript engine */
  return jerry_create_undefined();
}

static void register_module_js_function (jerry_value_t module, const char *name_p, jerry_external_handler_t handler_p)
{
  jerry_value_t func_obj = jerry_create_external_function (handler_p);
  jerry_release_value (jerryx_set_property_str (module, name_p, func_obj));
  jerry_release_value (func_obj);
} /* register_module_js_function */

static jerry_value_t my_custom_module_on_resolve (void)
{
  jerry_value_t object = jerry_create_object ();

  register_module_js_function(object, "myFunc", very_useful_function);
  register_module_js_function(object, "cat", module_cat_handler);
  register_module_js_function(object, "system", module_system_handler);
  register_module_js_function(object, "pipe", module_pipe_handler);

  //jerry_value_t func_obj = jerry_create_external_function (very_useful_function);
  //jerry_release_value (jerryx_set_property_str (object, "myFunc", func_obj));
  //jerry_release_value (func_obj);

  return object;
  //return jerry_create_number (42);
  //return jerry_create_external_function (very_useful_function);
} /* my_custom_module_on_resolve */

JERRYX_NATIVE_MODULE (MODULE_NAME, my_custom_module_on_resolve)
