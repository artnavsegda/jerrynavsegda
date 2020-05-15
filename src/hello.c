#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

static void print_unhandled_exception (jerry_value_t error_value) /**< error value */
{
  assert (!jerry_value_is_error (error_value));

  jerry_char_t err_str_buf[256];

  if (jerry_value_is_object (error_value))
  {
    jerry_value_t stack_str = jerry_create_string ((const jerry_char_t *) "stack");
    jerry_value_t backtrace_val = jerry_get_property (error_value, stack_str);
    jerry_release_value (stack_str);

    if (!jerry_value_is_error (backtrace_val)
        && jerry_value_is_array (backtrace_val))
    {
      printf ("Exception backtrace:\n");

      uint32_t length = jerry_get_array_length (backtrace_val);

      /* This length should be enough. */
      if (length > 32)
      {
        length = 32;
      }

      for (uint32_t i = 0; i < length; i++)
      {
        jerry_value_t item_val = jerry_get_property_by_index (backtrace_val, i);

        if (!jerry_value_is_error (item_val)
            && jerry_value_is_string (item_val))
        {
          jerry_size_t str_size = jerry_get_utf8_string_size (item_val);

          if (str_size >= 256)
          {
            printf ("%3u: [Backtrace string too long]\n", i);
          }
          else
          {
            jerry_size_t string_end = jerry_string_to_utf8_char_buffer (item_val, err_str_buf, str_size);
            assert (string_end == str_size);
            err_str_buf[string_end] = 0;

            printf ("%3u: %s\n", i, err_str_buf);
          }
        }

        jerry_release_value (item_val);
      }
    }
    jerry_release_value (backtrace_val);
  }

  jerry_value_t err_str_val = jerry_value_to_string (error_value);
  jerry_size_t err_str_size = jerry_get_utf8_string_size (err_str_val);

  if (err_str_size >= 256)
  {
    const char msg[] = "[Error message too long]";
    err_str_size = sizeof (msg) / sizeof (char) - 1;
    memcpy (err_str_buf, msg, err_str_size + 1);
  }
  else
  {
    jerry_size_t string_end = jerry_string_to_utf8_char_buffer (err_str_val, err_str_buf, err_str_size);
    assert (string_end == err_str_size);
    err_str_buf[string_end] = 0;

    if (jerry_is_feature_enabled (JERRY_FEATURE_ERROR_MESSAGES)
        && jerry_get_error_type (error_value) == JERRY_ERROR_SYNTAX)
    {
      jerry_char_t *string_end_p = err_str_buf + string_end;
      unsigned int err_line = 0;
      unsigned int err_col = 0;
      char *path_str_p = NULL;
      char *path_str_end_p = NULL;

      /* 1. parse column and line information */
      for (jerry_char_t *current_p = err_str_buf; current_p < string_end_p; current_p++)
      {
        if (*current_p == '[')
        {
          current_p++;

          if (*current_p == '<')
          {
            break;
          }

          path_str_p = (char *) current_p;
          while (current_p < string_end_p && *current_p != ':')
          {
            current_p++;
          }

          path_str_end_p = (char *) current_p++;

          err_line = (unsigned int) strtol ((char *) current_p, (char **) &current_p, 10);

          current_p++;

          err_col = (unsigned int) strtol ((char *) current_p, NULL, 10);
          break;
        }
      } /* for */
    }
  }

  jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Script Error: %s\n", err_str_buf);
  jerry_release_value (err_str_val);
} /* print_unhandled_exception */

int nomain (void)
{
  //const jerry_char_t script[] = "import { exported_value } from \"./module.js\"";
  const jerry_char_t script[] = "print ('Hello, World!');";

  bool ret_value = jerry_run_simple (script, sizeof (script) - 1, JERRY_INIT_EMPTY);

  if (jerry_value_is_error (ret_value))
  {
    puts("eval error");

    ret_value = jerry_get_value_from_error (ret_value, true);
    print_unhandled_exception (ret_value);

  }

  return (ret_value ? 0 : 1);
}

int main (void)
{
  bool run_ok = false;

  //const jerry_char_t script[] = "print ('Hello, World!');";
  const jerry_char_t script[] = "import { exported_value } from \"./module.js\"; print(exported_value)";

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Register 'print' function from the extensions to the global object */
  jerryx_handler_register_global ((const jerry_char_t *) "print", jerryx_handler_print);

  /* Setup Global scope code */
  jerry_value_t ret_val = jerry_parse (NULL, 0, script, sizeof (script) - 1, JERRY_PARSE_NO_OPTS);

  if (!jerry_value_is_error (ret_val))
  {
    jerry_value_t func_val = ret_val;
    ret_val = jerry_run (func_val);
    jerry_release_value (func_val);
  }

  if (!jerry_value_is_error (ret_val))
  {
    /* Print return value */
    const jerry_value_t args[] = { ret_val };
    jerry_value_t ret_val_print = jerryx_handler_print (jerry_create_undefined (),
                                                        jerry_create_undefined (),
                                                        args,
                                                        1);
    jerry_release_value (ret_val_print);
    jerry_release_value (ret_val);
    ret_val = jerry_run_all_enqueued_jobs ();

    if (jerry_value_is_error (ret_val))
    {
      ret_val = jerry_get_value_from_error (ret_val, true);
      print_unhandled_exception (ret_val);
    }
  }
  else
  {
    ret_val = jerry_get_value_from_error (ret_val, true);
    print_unhandled_exception (ret_val);
  }

  jerry_release_value (ret_val);

  /* Cleanup engine */
  jerry_cleanup ();

  return (run_ok ? 0 : 1);
}
