/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jerryscript.h"
#include "jerryscript-ext/module.h"
#include "jerryscript-ext/handler.h"

#define MODULE_NAME my_custom_module

static jerry_value_t very_useful_function(const jerry_value_t func_value, const jerry_value_t this_value, const jerry_value_t *args_p, const jerry_length_t args_cnt)
{
  return jerry_create_number (420);
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

  //jerry_value_t func_obj = jerry_create_external_function (very_useful_function);
  //jerry_release_value (jerryx_set_property_str (object, "myFunc", func_obj));
  //jerry_release_value (func_obj);

  return object;
  //return jerry_create_number (42);
  //return jerry_create_external_function (very_useful_function);
} /* my_custom_module_on_resolve */

JERRYX_NATIVE_MODULE (MODULE_NAME, my_custom_module_on_resolve)
