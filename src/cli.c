#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include "jerryscript-config.h"
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "jerryscript-ext/module.h"
#include "jerryscript-ext/debugger.h"
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"

extern void my_custom_module_register (void);

static int fileselect(const struct dirent *entry)
{
  char *ptr = rindex((char *)entry->d_name, '.');
  if ((ptr!=NULL) && ((strcmp(ptr,".js")==0)))
    return 1;
  else
    return 0;
}

struct my_struct
{
  const char *msg;
} my_struct;

static jerry_value_t readline_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  char * buffer = readline(">");
  return jerry_create_string (buffer);
}

static jerry_value_t get_msg_handler(const jerry_value_t func_value, const jerry_value_t this_value, const jerry_value_t *args_p, const jerry_length_t args_cnt)
{
  return jerry_create_string ((const jerry_char_t *) my_struct.msg);
}

static jerry_value_t cat_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
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

static jerry_value_t system_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
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

static jerry_value_t pipe_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
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

static jerry_value_t komplete_handler(const jerry_value_t function_object, const jerry_value_t function_this, const jerry_value_t arguments[], const jerry_length_t arguments_count)
{
  /* The the 'this_val' is the 'MyObject' from the JS code below */
  /* Note: that the argument count check is ignored for the example's case */

  /* Get 'this.x' */
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "x");
  jerry_value_t x_val = jerry_get_property (function_this, prop_name);

  if (!jerry_value_is_error (x_val))
  {
    /* Convert Jerry API values to double */
    double x = jerry_get_number_value (x_val);
    double d = jerry_get_number_value (arguments[0]);

    /* Add the parameter to 'x' */
    jerry_value_t res_val = jerry_create_number (x + d);

    /* Set the new value of 'this.x' */
    jerry_release_value (jerry_set_property (function_this, prop_name, res_val));
    jerry_release_value (res_val);
  }

  jerry_release_value (x_val);
  jerry_release_value (prop_name);

  return jerry_create_undefined ();
}

int zc_completion2(int count, int key)
{
  // const jerry_char_t script[] = "MyObject.komplete(14);";
  // jerry_value_t eval_ret = jerry_eval (script, sizeof (script) - 1, JERRY_PARSE_NO_OPTS);
  // jerry_release_value (eval_ret);

  char parseline[1000] = "";

  sprintf(parseline,"MyObject.komplete(\"%s\");",rl_line_buffer);
  jerry_value_t eval_ret = jerry_eval (parseline, strlen (parseline), JERRY_PARSE_NO_OPTS);

  if (jerry_value_is_undefined (eval_ret))
  {
    rl_on_new_line();
  }
  else
  {
    // Read the string into a byte buffer.
    jerry_size_t string_size = jerry_get_string_size (eval_ret);
    jerry_char_t *string_buffer_p = (jerry_char_t *) malloc (sizeof (jerry_char_t) * (string_size + 1));

    jerry_size_t copied_bytes = jerry_string_to_char_buffer (eval_ret, string_buffer_p, string_size);
    string_buffer_p[copied_bytes] = '\0';

    jerry_release_value (eval_ret);

    //printf ("Test string: %s\n", string_buffer_p);

    rl_insert_text(&string_buffer_p[rl_point]);

    free (string_buffer_p);
  }

}

static const jerryx_module_resolver_t *resolvers[1] =
{
  &jerryx_module_native_resolver
};

static jerry_value_t handle_require (const jerry_value_t js_function, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_count)
{
  (void) js_function;
  (void) this_val;
  (void) args_count;
  jerry_value_t return_value = 0;
  return_value = jerryx_module_resolve (args_p[0], resolvers, 1);
  return return_value;
}

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

jerry_value_t execute(char *buf)
{
  //jerry_value_t eval_ret = jerry_parse (NULL, 0, buf, sizeof (buf) - 1, JERRY_PARSE_NO_OPTS);

  //jerry_value_t eval_ret = jerry_eval (buf, strlen(buf), JERRY_PARSE_NO_OPTS);

  jerry_value_t eval_ret = jerry_parse (NULL, 0, buf, strlen (buf), JERRY_PARSE_NO_OPTS);

  if (!jerry_value_is_error (eval_ret))
  {
    jerry_value_t func_val = eval_ret;
    eval_ret = jerry_run (func_val);
    jerry_release_value (func_val);
  }

  // /* Check if there is any JS code parse error */
  // if (!jerry_value_is_error (eval_ret))
  // {
  //   /* Execute the parsed source code in the Global scope */
  //   jerry_value_t ret_value = jerry_run (eval_ret);
  //
  //   /* Returned value must be freed */
  //   jerry_release_value (ret_value);
  // }
  // else
  // {
  //   puts("error");
  // }

  if (!jerry_value_is_error (eval_ret))
  {
    /* Print return value */
    const jerry_value_t args[] = { eval_ret };
    jerry_value_t ret_val_print = jerryx_handler_print (jerry_create_undefined (), jerry_create_undefined (), args, 1);
    jerry_release_value (ret_val_print);
    jerry_release_value (eval_ret);
    eval_ret = jerry_run_all_enqueued_jobs ();

    if (jerry_value_is_error (eval_ret))
    {
      eval_ret = jerry_get_value_from_error (eval_ret, true);
      print_unhandled_exception (eval_ret);
    }
  }
  else
  {
    eval_ret = jerry_get_value_from_error (eval_ret, true);
    print_unhandled_exception (eval_ret);
  }


  /* Parsed source code must be freed */
  // jerry_release_value (eval_ret);
  return eval_ret;
}

int main (void)
{
  printf("ES6 status is %d\n", JERRY_ES2015);
  printf("module status is %d\n", JERRY_ES2015_MODULE_SYSTEM);

  jerry_port_default_set_log_level(JERRY_LOG_LEVEL_TRACE);
  //jerry_port_log(JERRY_LOG_LEVEL_ERROR, "shit");

  // multi load
  // char path[PATH_MAX];
  // getcwd(path,PATH_MAX);
  // struct dirent **dirs;
  //
  // char *buf = NULL;
  // int bufsize = 0;
  //
  // int n = scandir(path,&dirs,fileselect,alphasort);
  // if (n >= 0)
  // {
  //   for (int cnt = 0;cnt < n;++cnt)
  //   {
  //     int jsmain = open(dirs[cnt]->d_name,O_RDONLY);
  //     struct stat sb;
  //     fstat(jsmain, &sb);
  //
  //     buf = realloc(buf, bufsize+sb.st_size+2);
  //     read(jsmain,&buf[bufsize],sb.st_size+1);
  //     bufsize = bufsize+sb.st_size;
  //     buf[bufsize] = '\0';
  //     close(jsmain);
  //
  //   }
  // }
  // else
  // {
  //   printf("Cannot find files in %s\n", path);
  // }

  // single load

  int jsmain = open("./cli.js",O_RDONLY);

  struct stat sb;
  fstat(jsmain, &sb);
  char *buf = malloc(sb.st_size+2);

  read(jsmain,buf,sb.st_size+1);
  close(jsmain);
  buf[sb.st_size] = '\0';

  /* Initializing JavaScript environment */
  my_custom_module_register();
  jerry_init (JERRY_INIT_EMPTY);
  // jerryx_debugger_after_connect (jerryx_debugger_tcp_create (5001)& jerryx_debugger_ws_create ());

  // if (jerry_debugger_is_connected ())
  // {
  //   printf ("A remote debugger client is connected.\n");
  // }
  // else
  // {
  //   printf ("A remote debugger client is not connected.\n");
  // }


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

  func_obj = jerry_create_external_function (handle_require);
  prop_name = jerry_create_string ((const jerry_char_t *) "require");
  jerry_release_value (jerry_set_property (global_object, prop_name, func_obj));
  jerry_release_value (prop_name);
  jerry_release_value (func_obj);

  /* Add the JS object to the global context */
  prop_name = jerry_create_string ((const jerry_char_t *) "MyObject");
  jerry_release_value (jerry_set_property (global_object, prop_name, object));
  jerry_release_value (prop_name);
  jerry_release_value (object);
  jerry_release_value (global_object);

  /* Constructing strings */
  prop_name = jerry_create_string ((const jerry_char_t *) "my_var");
  jerry_value_t prop_value = jerry_create_string ((const jerry_char_t *) "cli>");

  /* Setting the string value as a property of the Global object */
  jerry_value_t set_result = jerry_set_property (global_object, prop_name, prop_value);
  /* The 'set_result' should be checked if there was any error */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'my_var' property\n");
  }
  jerry_release_value (set_result);

  // ======= cat =======

  set_result = jerry_set_property (global_object, jerry_create_string ((const jerry_char_t *) "cat"), jerry_create_external_function (cat_handler));

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'cat' property\n");
  }

  jerry_release_value (set_result);

  // ======= system =======

  set_result = jerry_set_property (global_object, jerry_create_string ((const jerry_char_t *) "system"), jerry_create_external_function (system_handler));

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'system' property\n");
  }

  jerry_release_value (set_result);

  // ======= pipe =======

  set_result = jerry_set_property (global_object, jerry_create_string ((const jerry_char_t *) "pipe"), jerry_create_external_function (pipe_handler));

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'pipe' property\n");
  }

  jerry_release_value (set_result);

  // ======= readline =======

  /* Add the "joke" property with the function value to the "global" object */
  set_result = jerry_set_property (global_object, jerry_create_string ((const jerry_char_t *) "readline"), jerry_create_external_function (readline_handler));

  /* Check if there was no error when adding the property (in this case it should never happen) */
  if (jerry_value_is_error (set_result)) {
    printf ("Failed to add the 'readline' property\n");
  }

  jerry_release_value (set_result);

  // OVER

  /* Releasing string values, as it is no longer necessary outside of engine */
  jerry_release_value (prop_name);
  jerry_release_value (prop_value);

  /* Releasing the Global object */
  jerry_release_value (global_object);

  rl_bind_key('\t', zc_completion2);

  jerry_release_value(execute(buf));

  //tut

  // jerry_value_t komplete_func_obj = jerry_create_external_function(komplete_handler);
  // prop_name = jerry_create_string ((const jerry_char_t *) "komplete123");
  // jerry_release_value (jerry_set_property (eval_ret, prop_name, komplete_func_obj));
  // jerry_release_value (komplete_func_obj);
  // jerry_release_value (prop_name);
  // jerry_release_value (eval_ret);

  jerry_release_value(execute("MyObject.startup();"));

  //jerry_value_t eval_ret = jerry_eval ("MyObject.startup();", strlen("MyObject.startup();"), JERRY_PARSE_NO_OPTS);

  // if (jerry_value_is_error (eval_ret))
  // {
  //   puts("eval error");
  //
  //   eval_ret = jerry_get_value_from_error (eval_ret, true);
  //   print_unhandled_exception (eval_ret);
  //
  //
  // }
  //
  // jerry_release_value(eval_ret);

  char * inputline = NULL;
  char parseline[1000] = "";

  while(1)
  {
    global_object = jerry_get_global_object();
    prop_name = jerry_create_string ((const jerry_char_t *) "my_var");
    prop_value = jerry_get_property (global_object, prop_name);

    jerry_size_t string_size = jerry_get_string_size (prop_value);
    jerry_char_t *string_buffer_p = (jerry_char_t *) malloc (sizeof (jerry_char_t) * (string_size + 1));
    jerry_size_t copied_bytes = jerry_string_to_char_buffer (prop_value, string_buffer_p, string_size);

    string_buffer_p[copied_bytes] = '\0';

    //char prompt[] = ">";
    //inputline = readline(prompt);
    inputline = readline(string_buffer_p);
    if (!inputline)
      break;

    sprintf(parseline,"MyObject.compute(\"%s\");",inputline);
    // jerry_value_t eval_ret = jerry_eval (parseline, strlen (parseline), JERRY_PARSE_NO_OPTS);
    jerry_value_t eval_ret = execute(parseline);

    if (jerry_get_number_value (eval_ret) == 300.0)
    {
      exit(0);
    }

    //printf ("number: %lf\n", jerry_get_number_value (eval_ret));
    jerry_release_value (eval_ret);

    jerry_release_value (prop_value);
    jerry_release_value (prop_name);
    jerry_release_value (global_object);
  }

  /* Freeing engine */
  jerry_cleanup ();

  return 0;
}
