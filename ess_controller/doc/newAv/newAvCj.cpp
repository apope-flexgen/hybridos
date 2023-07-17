
#ifndef __NEW_AV2_CPP
#define __NEW_AV2_CPP

#include<iostream>
#include<string>
#include<map>
#include<vector>
#include <cjson/cJSON.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "newAv2.h"

using namespace std;

#define DBL_EPSILON 2.2204460492503131e-16
//int decodeCJ(AssetVar*vmap, cJSON* cji, int level =0);

// setVal will trigger actions
// actAv["shift"] = 2;
// actAv["inValue"] = 0;
// actAv["outValue"] = "This is an out value";
// actAv["uri"] = "/a/b/c:d@foo";
// make up a vector of actAV's
// actVec.push_back (actAv)

// av->addAction("onSet", "enum", actVec)
// we have an actions dict(av)
// add a name into it
//
// we have a function "enum" .. then  that to the vectors
// as an av
// add the actVec to the enum var 

// after this we will have a multi dimensional vmap with params and actions
// all over the place.  

//g++ -std=c++11 -o av doc/newAv.cpp//  Time stuff
//g++ -std=c++11 -o av doc/newAv2.cpp -lcjson
// Dont have time today to work out the chrono stuff.

// // decode options
// // '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in a value
// // '{"a":{"value":c,"p1":d}}' populate params value p1 etc
// //                    
// // we loose the varsmap instead root everything from a base av.

// int decodeCJ(AssetVar*vmap, cJSON* cji, int level =0)
// {
// 	const char* sname;
// 	// we need to make sure we know the type of asset var we are playing with.
// 	cJSON *cj = cji;
// 	string head = "";
// 	for ( int i = 0 ; i < level; i++)
// 	{
// 		head += "  ";
// 	}
// 	while (cj)
// 	{
// 		if(cj->string)
// 		{
// 			sname = cj->string;
// 		}
// 		else
// 		{
// 			sname = NULL;
// 		}

// 		if (cJSON_IsObject(cj))
// 		{
// 			if(!sname)sname = "{";
// 			cout << head+" CJ ["<< sname <<"] IS an Object >>"<< cj<<endl;
// 			AssetVar* av = vmap->setTParam(sname, "Object");
// 			if(cj->child)
// 				decodeCJ(av, cj->child, level+1);
// 		}
// 		else if (cJSON_IsTrue(cj))
// 		{
// 			cout << head+" CJ ["<< sname <<"] IS True >>"<< cj<<endl;
// 			AssetVar* av = vmap->setParam(sname, true);
// 			if(cj->child)
// 				decodeCJ(av, cj->child, level+1);
// 		}
// 		else if (cJSON_IsFalse(cj))
// 		{
// 			cout << head+" CJ ["<< sname <<"] IS False >>"<< cj<<endl;
// 			AssetVar* av = vmap->setParam(sname, false);
// 			if(cj->child)
// 				decodeCJ(av, cj->child, level+1);
// 		}
// 		else if (cJSON_IsString(cj))
// 		{
// 			cout << head+" CJ ["<< sname <<"] IS a String >>"<< cj<<endl;
// 			AssetVar* av = vmap->setParam(sname, "String");
// 			if(cj->child)
// 				decodeCJ(av, cj->child, level+1);
// 		}
// 		else if (cJSON_IsArray(cj))
// 		{
// 			if(!sname)sname = "[";
// 			cout << head+" CJ ["<< sname<<"] IS an Array >>"<< cj<<endl;
// 			AssetVar* av = vmap->setTParam(sname, "Array");
// 			cJSON* cji = cj->child;
// 			decodeCJ(av, cji, level+1);
// 		}
// 		else if (cJSON_IsNumber(cj))
// 		{
// 			// numbers strings etc simply add to the curent object params
// 			cout << head+" CJ ["<< sname<<"] IS a Number >>"<< cj<<endl;
// 			AssetVar * av = vmap->setParam(sname, cj->valuedouble);
// 			cout << head+"   Type "<<cj->type <<"  CJ next["<< cj->next<<"] "<< cj<<endl;
// 			////vmap->getParam(cj->string)->show();
// 			if(cj->child)
// 				decodeCJ(av, cj->child, level + 1);
// 		}
// 		else
// 		{
// 			cout << head+"Unhandled type " << cj->type << endl;
// 		}
// 		cj = cj->next;

// 	}
	
// 	return 0;
// }

// these are all taken from cJSON
// converted a bit to c++
bool print_CJarray(const cJSON * const item, int &depth);
bool print_CJnumber(const cJSON * const item, int &depth);
bool print_CJstring(const cJSON * const item, int &depth);
bool print_CJobject(const cJSON * const item, int &depth);

/* Render a value to text. */
bool print_CJvalue(const cJSON* const item, int &depth)
{

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            cout << "null";
            return true;

        case cJSON_False:
            cout<< "false";
            return true;

        case cJSON_True:
            cout<< "true";
            return true;

        case cJSON_Number:
            return print_CJnumber(item, depth);

        case cJSON_Raw:
        {
            if (item->valuestring == NULL)
            {
                return false;
            }

            cout <<item->valuestring;
            return true;
        }

        case cJSON_String:
            return print_CJstring(item, depth);

        case cJSON_Array:
            return print_CJarray(item, depth);

        case cJSON_Object:
            return print_CJobject(item, ++depth);

        default:
            return false;
    }
	if(depth == 0)
	   cout << endl;
}

/* Render an array to text */
bool print_CJarray(const cJSON * const item, int &depth)
{

    cout << "[";
    depth++;
 	cJSON *current_element = item->child;
    while (current_element != NULL)
    {
        if (!print_CJvalue(current_element, depth))
        {
            return false;
        }
        if (current_element->next)
        {
            cout << ",";
            cout << " ";
        }
        current_element = current_element->next;
    }
	for (int i = 0; i < (depth - 1); i++)
    {
        cout << "\t";
    }
    cout <<  "]";
    depth--;

    return true;
}
/* Render an object to text. */
bool print_CJobject(const cJSON * const item, int &depth)
{
    cJSON *current_item = item->child;
    cout<< "{";
    cout<< endl;
    while (current_item)
    {
        for (int i = 0; i < depth; i++)
        {
            cout << "\t";
        }
        cout << current_item->string;
        cout << ':';
        cout << "\t";
        /* print value */
        if (!print_CJvalue(current_item, depth))
        {
            return false;
        }

        if (current_item->next)
        {
            cout << ",";
        }
        cout << endl;

        current_item = current_item->next;
    }

    for (int i = 0; i < (depth - 1); i++)
    {
        cout << "\t";
    }
    
    cout << "}";
    depth--;

    return true;
}

/* Render the number nicely from the given item into a string. */
bool print_CJnumber(const cJSON * const item, int &depth)
{
    double d = item->valuedouble;
    //size_t i = 0;
	size_t length = 0;
    unsigned char number_buffer[26] = {0}; /* temporary buffer to print the number into */
    //unsigned char decimal_point = '.';
    double test = 0.0;


    /* This checks for NaN and Infinity */
    if (isnan(d) || isinf(d))
    {
        length = sprintf((char*)number_buffer, "null");
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = sprintf((char*)number_buffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || !compare_double((double)test, d))
        {
            /* If not, print with 17 decimal places of precision */
            length = sprintf((char*)number_buffer, "%1.17g", d);
        }
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return false;
    }


    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
	cout << number_buffer;

    return true;
}

/* Render the cstring provided to an escaped version that can be printed. */
bool print_CJstring_ptr(const unsigned char * const input, int &depth)
{
    
    /* empty string */
    if (input == NULL)
    {
		cout << "\"\"";

        return true;
    }
  
    cout << "\"";
    cout << input;
    cout << "\"";

    return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
bool print_CJstring(const cJSON * const item, int &depth)
{
    return print_CJstring_ptr((unsigned char*)item->valuestring, depth);
}


bool parse_CJvalue(cJSON * const item, parse_buffer * const input_buffer);
//bool print_value(const cJSON * const item, printbuffer * const output_buffer);
bool parse_CJarray(cJSON * const item, parse_buffer * const input_buffer);
//static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
bool parse_CJobject(cJSON * const item, parse_buffer * const input_buffer);
bool parse_CJstring(cJSON * const item, parse_buffer * const input_buffer);
bool parse_CJnumber(cJSON * const item, parse_buffer * const input_buffer);

/* Build an object from the text. */
bool parse_CJobject(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* linked list head */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_CreateObject();
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_CJstring(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_CJvalue(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

/* Build an array from input text. */
bool parse_CJarray(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* head of the linked list */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_CreateObject();
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_CJvalue(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

/* Parse the input text into an unescaped cinput, and populate item. */
bool parse_CJstring(cJSON * const item, parse_buffer * const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t) (input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char*)malloc(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = cJSON_String;
    item->valuestring = (char*)output;

    input_buffer->offset = (size_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        free(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}
/* Parse the input text to generate a number, and populate the result into item. */
bool parse_CJnumber(cJSON * const item, parse_buffer * const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_c_string[i] = buffer_at_offset(input_buffer)[i];
                break;

            case '.':
                number_c_string[i] = decimal_point;
                break;

            default:
                goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        return false; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = cJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}

/* Parser core - when encountering text, process appropriately. */
bool parse_CJvalue(cJSON* const item, parse_buffer* const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = cJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    /* false */
    if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = cJSON_False;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = cJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_CJstring(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_CJnumber(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_CJarray(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_CJobject(item, input_buffer);
    }

    return false;
}

cJSON* cJSON_CJParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated);


CJSON_PUBLIC(cJSON *) cJSON_CJParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return cJSON_CJParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
cJSON* cJSON_CJParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated)
{
    //parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    parse_buffer buffer = { 0, 0, 0, 0 };
    cJSON *item = NULL;

    // /* reset error position */
    // global_error.json = NULL;
    // global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length; 
    buffer.offset = 0;
    //buffer.hooks = global_hooks;

    item = cJSON_CreateObject();
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_CJvalue(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        cJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        //global_error = local_error;
    }

    return NULL;
}

/* Default options for cJSON_Parse */
cJSON *cJSON_CJParse(const char *value)
{
    return cJSON_CJParseWithOpts(value, 0, 0);
}


//this is crap
int printCJItem(cJSON *cj, int level = 0, int comma = 0)
{
	string lev = "  ";
	for(int i = 0; i < level;i++ )
	{
		lev += "  ";
	}
	if (cj->string)
	{
		cout << comma << ">"<<lev << "\""<<cj->string<<"\":";
	}
	else
	{
	 	cout << comma << ">"<<lev;
	}
	if (cJSON_IsString(cj))
		cout << "\""<<cj->valuestring<<"\"";
	if (cJSON_IsNumber(cj))
		cout <<cj->valuedouble;
	if (cJSON_IsTrue(cj))
		cout << "true";//<<cj->valuedouble<<"\"";
	if (cJSON_IsFalse(cj))
		cout << "false";//<<cj->valuedouble<<"\"";
	if (cJSON_IsArray(cj))
		cout << cj << "  array type "<< cj->type << " next "<< cj->next << " child " << cj->child << " ";//<<cj->valuedouble<<"\"";
	if (cJSON_IsObject(cj))
		cout << cj << "  object type "<< cj->type << " next "<< cj->next << " child " << cj->child << " ";//<<cj->valuedouble<<"\"";
	// if(comma)
	// 	cout<<",";
	cout << endl;
    return 0;
}

int printCJTree(cJSON *cj, int level = 0, int comma = 0)
{
	string lev = "  ";
	for(int i = 0; i < level;i++ )
	{
		lev += "  ";
	}
    //level += 1;
	printCJItem(cj, level, (cj->next != NULL));

	cJSON* cji = cj->next;
	while (cji)
	{   
		if(cji)
			printCJTree(cji, level, (cji->next != NULL));	
		cji = cji->next;
	}
	if(cj->next)
		cout << lev<<"done with loop cj "<<cj<<" next "<<cj->next <<" child " <<cj->child <<endl;
	if (cj->child)
	{

		cout << lev<<"running child  cj" << cj << " child "<< cj->child<< endl;
		//cout << comma << ">"<<lev << "Child \""<<av->getChild()->getName()<<"\":";
	
		printCJTree(cj->child, level +1, 0);
		cout << lev << "}" << endl;
		cout << lev<<"child done" <<endl;
	}
	cout << lev<<"all done" <<endl;
	return 1;
}
#endif
