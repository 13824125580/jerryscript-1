#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>

#include "jerry_util.h"

void js_set_property(const jerry_value_t obj, const char *name,
    const jerry_value_t prop)
{
    jerry_value_t str = jerry_create_string((const jerry_char_t *)name);
    jerry_set_property(obj, str, prop);
    jerry_release_value (str);
}

jerry_value_t js_get_property(const jerry_value_t obj, const char *name)
{
    jerry_value_t ret;

    const jerry_value_t str = jerry_create_string ((const jerry_char_t*)name);
    ret = jerry_get_property(obj, str);
    jerry_release_value (str);

    return ret;
}

void js_add_function(const jerry_value_t obj, const char *name,
    jerry_external_handler_t func)
{
    jerry_value_t str = jerry_create_string((const jerry_char_t *)name);
    jerry_value_t jfunc = jerry_create_external_function(func);

    jerry_set_property(obj, str, jfunc);

    jerry_release_value(str);
    jerry_release_value(jfunc);
}

char *js_value_to_string(const jerry_value_t value)
{
    int len;
    char *str;

    len = jerry_get_string_length(value);

    str = (char*)malloc(len + 1);
    if (str)
    {
        jerry_string_to_char_buffer(value, (jerry_char_t*)str, len);
        str[len] = '\0';
    }

    return str;
}

jerry_value_t js_call_function(const jerry_value_t obj, const char *name,
    const jerry_value_t args[], jerry_size_t args_cnt)
{
    jerry_value_t ret;
    jerry_value_t function = js_get_property(obj, name);

    if (jerry_value_is_function(function))
    {
        ret = jerry_call_function(function, obj, args, args_cnt);
    }
    else
    {
        ret = jerry_create_null();
    }

    jerry_release_value(function);
    return ret;
}

bool object_dump_foreach(const jerry_value_t property_name,
    const jerry_value_t property_value, void *user_data_p)
{
    char *str;
    int str_size;
    int *first_property;

    first_property = (int *)user_data_p;

    // if (*first_property) first_property = 0;
    // else
    {
        printf(",");
    }

    if (jerry_value_is_string(property_name))
    {
        str_size = jerry_get_string_size(property_name);
        str = (char*) malloc (str_size + 1);
        RT_ASSERT(str != NULL);

        jerry_string_to_char_buffer(property_name, (jerry_char_t*)str, str_size);
        str[str_size] = '\0';
        printf("%s : ", str);
        free(str);
    }
    js_value_dump(property_value);

    return true;
}

void js_value_dump(jerry_value_t value)
{
    if (jerry_value_is_undefined(value))
    {
        printf("undefined");
    }
    else if (jerry_value_is_boolean(value))
    {
        printf("%s", jerry_get_boolean_value(value)? "true" : "false");
    }
    else if (jerry_value_is_number(value))
    {
        printf("%f\n", jerry_get_number_value(value));
    }
    else if (jerry_value_is_null(value))
    {
        printf("null");
    }
    else if (jerry_value_is_string(value))
    {
        char *str;
        int str_size;

        str_size = jerry_get_string_size(value);
        str = (char*) malloc (str_size + 1);
        RT_ASSERT(str != NULL);

        jerry_string_to_char_buffer(value, (jerry_char_t*)str, str_size);
        str[str_size] = '\0';
        printf("\"%s\"", str);
        free(str);
    }
    else if (jerry_value_is_promise(value))
    {
        printf("promise??");
    }
    else if (jerry_value_is_function(value))
    {
        printf("function");
    }
    else if (jerry_value_is_constructor(value))
    {
        printf("constructor");
    }
    else if (jerry_value_is_array(value))
    {
        int index;
        printf("[");
        for (index = 0; index < jerry_get_array_length(value); index ++)
        {
            jerry_value_t item = jerry_get_property_by_index(value, index);
            js_value_dump(item);
            jerry_release_value(item);
        }
        printf("]\n");
    }
    else if (jerry_value_is_object(value))
    {
        int first_property = 1;
        printf("{");
        jerry_foreach_object_property(value, object_dump_foreach, &first_property);
        printf("}\n");
    }
    else
    {
        printf("what?");
    }
}
