// #include<iostream>
// #include<sstream>
// #include<string>
// #include<map>
// #include<vector>
// #include <cjson/cJSON.h>
// #include <cstring>
// #include <stdlib.h>
// #include <math.h>
// #include <limits.h>


using namespace std;

#define DBL_EPSILON 2.2204460492503131e-16

#include "parse_buffer.h"
#include "newAv2.h"
#include "newUtils.h"
//#include "newAvCj.cpp"

extern NewUtils* vmp; 
#include <time.h>

void AssetVar::show(int level) 
{ 
	if((level >=0) && parent)
	{
		parent->show(level+1);
	}
	if(level >= 0)
		cout << " ["<<level<<"]";
	cout << "name :[" << name 
	     << "] type :" << atype  << " val :"<< valuedouble; 
	if ((level <= 0))
	{
		cout << endl;
	}
	
}

void AssetVar::showKids(int level) 
{
  	int i = level;
 	while (i> 0)
 	{
		cout << "   ";
	 	i--;
 	}
 	cout << name << endl; 
 	// if (child)
 	// 	child->showKids(level+1);
}

bool stream_Avnumber(ostringstream &sout, const AssetVar* const item, int &depth)
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
	sout << number_buffer;

    return true;
}
/* Render the cstring provided to an escaped version that can be printed. */
bool stream_Avstring_ptr(ostringstream &sout, const unsigned char* const input, int &depth)
{
    
    /* empty string */
    if (input == NULL)
    {
		sout << "\"\"";

        return true;
    }
  
    sout << "\"";
    sout << input;
    sout << "\"";

    return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
bool stream_Avstring(ostringstream &sout, const AssetVar* const item, int &depth)
{
    return stream_Avstring_ptr(sout, (unsigned char*)item->valuestring, depth);
}

/* Render a value to text. */
bool stream_Avvalue(ostringstream &sout, const AssetVar* const item, int &depth)
{

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            sout << "null";
            return true;

        case cJSON_False:
            sout<< "false";
            return true;

        case cJSON_True:
            sout<< "true";
            return true;

        case cJSON_Number:
            return stream_Avnumber(sout, item, depth);

        case cJSON_Raw:
        {
            if (item->valuestring == NULL)
            {
                return false;
            }

            sout <<item->valuestring;
            return true;
        }

        case cJSON_String:
            return stream_Avstring(sout, item, depth);

        case cJSON_Array:
            return stream_Avarray(sout, item, depth);

        case cJSON_Object:
            return stream_Avobject(sout, item, ++depth);

        default:
            return false;
    }
	if(depth == 0)
	   sout << endl;
}

/* Render an array to text */
bool stream_Avarray(ostringstream &sout, const AssetVar* const item, int &depth)
{

    sout << "[";
    depth++;
    AssetVar *av = NULL;
    int ix = 0;
    int imax = item->aList.size();
    for (auto x: item->aList)
    {
        av = x.first;
        if (!stream_Avvalue(sout, av, depth))
        {
            return false;
        }
        if (++ix< imax)
        {
            sout << ",";
            sout << " ";
        }
    }

 	
	for (int i = 0; i < (depth - 1); i++)
    {
        sout << "\t";
    }
    sout <<  "]";
    depth--;

    return true;
}
/* Render an object to text. */
bool stream_Avobject(ostringstream &sout, const AssetVar* const item, int &depth)
{
    AssetVar *av;
    sout<< "{";
    sout<< endl;
    int endix = item->aList.size();
    int thisix = 0;
    for (auto x: item->aList)
    {
        av = x.first;
        for (int i = 0; i < depth; i++)
        {
            sout << "\t";
        } 
        //if(av->cstring)
        if(x.second)
        {
            sout << "\""<<x.second<<"\"";
            sout << ':';
        }
        sout << "\t";
        /* print value */
        if (!stream_Avvalue(sout, av, depth))
        {
            return false;
        }

        if (++thisix < endix)
        {
           sout << ",";
        }
        sout << endl;

        //current_item = current_item->next;
    }

    for (int i = 0; i < (depth - 1); i++)
    {
        sout << "\t";
    }
    
    sout << "}";
    depth--;

    return true;
}

///////////////////////////////bool parse_CJvalue(cJSON * const item, parse_buffer * const input_buffer);
// bool parse_Avarray(AssetVar * const item, parse_buffer * const input_buffer);
// //static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
// bool parse_Avobject(AssetVar * const item, parse_buffer * const input_buffer);
// bool parse_Avstring(AssetVar * const item, parse_buffer * const input_buffer);
// bool parse_Avnumber(AssetVar * const item, parse_buffer * const input_buffer);
// bool parse_Avvalue(AssetVar* const item, parse_buffer* const input_buffer);
/* Build an object from the text. */
bool parse_Avobject(AssetVar* const item, parse_buffer* const input_buffer)
{
    bool defer = false;
    const char* cs = item->cstring;
    if(!cs) cs = "NoName";
    // cout << __func__<< " item id: "<< item->av_id<<" item name  ["<< cs<<"]" << endl; 
    // cout << __func__<<" item num alist items :" << item->aList.size() << endl;
    AssetVar *av =  NULL;//head = NULL; //item->head; /* linked list head */
    
    if(item->aList.size()> 0)
    {
        av = item->aList.at(item->aList.size()-1).first;
        // cout << __func__ << " current item cstring :["<<av->cstring<<"]"<<endl;
    }

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
        // cout <<__func__<< " new assetvar created Item alist size 1 " 
        //<< item->aList.size() << endl;
        AssetVar*new_item = new AssetVar(__func__, item);
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }
        vmp->addDelMap(new_item);
        /* attach next item to list */
        //int numP =item->numParams();
        //cout << ">>>>>>>>>>>item id:"<<item->id<<" num params "<<numP<<endl;
        cout << __func__<<">>>>>>before parse string >>>>>new item id:"<<new_item->id
        << " type : " << new_item->type
        <<endl;
        int old_type = new_item->type;
             //<<" num params "<<new_item->numParams()<<endl;
        if (av == NULL)
        {
            defer = false;
        }
        else
        {
            defer = true;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_Avstring(new_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        new_item->type = old_type;
        cout << __func__<<">>> after parse string >>>>>new item id:"<<new_item->id
        << " type : " << new_item->type
        <<endl;
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        new_item->cstring = strdup(new_item->valuestring);
        // TOSO this may leak
        new_item->valuestring = NULL;
        // do we already have a param
        // TODO perhaps check aList instead
        if (item->gotParam(new_item->cstring))
        {
            defer = false;
            // cout << " Already found ["<<new_item->cstring<<"] in Params"<<endl; 
            av = item->getParam(new_item->cstring);
            // cout << " current_item id ["<<av->id<<"] item->id "<< item->id<<endl; 
            // cout << " delete new_item id ["<<new_item->id<<"] item->id "<< item->id<<endl; 
            // delete new_item;
            cout << ">>>>>>>>>>>decremented av_id :"<<AssetVar::av_id<<endl;
            AssetVar::av_id--;
            new_item = NULL;

        }
        else
        {
            item->Params[new_item->cstring] = new_item;
            {
                 new_item->alidx = item->aList.size();
                 item->aList.push_back(make_pair(new_item,strdup(new_item->cstring)));
            }
        }

        if(new_item)
        {
            // const char* sp = new_item->cstring;
            // int idx = new_item->alidx;
            // //cout << " setting aList idx ["<< idx << " ] name as ["<< sp<<"] item aList size "<< item->aList.size() << endl;
            // pair<AssetVar*,char *> *avp = &item->aList.at(idx);
            // //cout << " avp first " << avp<< " defer :" << defer <<endl;

            av =  new_item;
        }
        if(defer)
        {
            
            av = new_item;
        }
        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!av) {
            cout << __func__ << " No av , cannot proceed defer :"<<defer<< endl;
            cout <<__func__<< " current item ["<<item->cstring<<"]"<<endl; 
            goto fail;
        }
        if (!parse_Avvalue(av, input_buffer))
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
    item->type = cJSON_Object;
    input_buffer->offset++;
    return true;

fail:
    return false;
}

/* Build an array from input text. */
bool parse_Avarray(AssetVar* const item, parse_buffer * const input_buffer)
{
    //AssetVar *av = NULL; /* head of the linked list */
    // if(item->aList.size()> 0)
    // {
    //     av = item->aList.at(item->aList.size()-1).first;
    //     //cout << __func__ << " current item cstring :["<<av->cstring<<"]"<<endl;
    // }
//    AssetVar* current_item = NULL;

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
        AssetVar* new_item = new AssetVar(__func__, item);
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }
        vmp->addDelMap(new_item);

        new_item->alidx = item->aList.size();
        char *sp = NULL;
        item->aList.push_back(make_pair(new_item,sp));

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_Avvalue(new_item, input_buffer))
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
    item->type = cJSON_Array;
    input_buffer->offset++;

    return true;

fail:
    // if (head != NULL)
    // {
    //     delete head;
    // }

    return false;
}

/* Parse the input text into an unescaped cinput, and populate item. */
bool parse_Avstring(AssetVar* const item, parse_buffer* const input_buffer)
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
    //if item->type was a number we have to add a "value" param
    // if(item->type == cJSON_Number)
    // {
    //     cout<< __func__ << " converting number to value param" << endl;
    //     item->addCJParam("value",item->valuedouble);
    // }  
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
bool parse_Avnumber(AssetVar* const item, parse_buffer* const input_buffer)
{
//return true;
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }
    //cout << __func__ << " stage 1.1"<<endl;

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        //cout << __func__ << " stage 1.2 -> i:"<<i<<endl;

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
    // if (number >= INT_MAX)
    // {
    //     item->valueint = INT_MAX;
    // }
    // else if (number <= (double)INT_MIN)
    // {
    //     item->valueint = INT_MIN;
    // }
    // else
    // {
    //     item->valueint = (int)number;
    // }
    //run action but we need the base object.
    item->type = cJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}
//parse_Avobject
/* Parser core - when encountering text, process appropriately. */
bool parse_Avvalue(AssetVar* const item, parse_buffer* const input_buffer)
{
   // cout << __func__ << " stage 1  item ->"<<item<<endl;
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }
    const char * sp = item->cstring?item->cstring:"noName";
    cout << __func__ << " item name : [" << sp <<"] id: [" 
            << item->id <<"] incoming type:"<<item->type 
            << endl;

//return false;
    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) 
         && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = cJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    //return true;
    /* false */
    if (can_read(input_buffer, 5) 
         && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = cJSON_False;
        item->valuebool = false;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (can_read(input_buffer, 4) 
          && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = cJSON_True;
        item->valuebool = true;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) 
             && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        // if item->type == 8 we must promote it to an object
        // todo what about things that were trings noe been given parameters.
        item->type = cJSON_String;
        return parse_Avstring(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') 
            || ((buffer_at_offset(input_buffer)[0] >= '0') 
                && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        item->type = cJSON_Number;
        //cout << __func__ << " stage 1 number"<<endl;
        return parse_Avnumber(item, input_buffer);
    }
    //cout << __func__ << " stage 2"<<endl;
    /* array */
    if (can_access_at_index(input_buffer, 0) 
             && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_Avarray(item, input_buffer);
    }
    /* object */
    //item->type =cJSON_Object;
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        if(item->type == cJSON_Number)
        {
            cout<< __func__ << " converting number to value param" << endl;
            AssetVar* item2 = item->addCJParam("value",cJSON_Number);
            item2->valuedouble = item->valuedouble;
        }  
        if(item->type == cJSON_String)
        {
            cout<< __func__ << " converting string to value param" << endl;
            AssetVar* item2 = item->addCJParam("value",cJSON_String);
            item2->valuestring = item->valuestring;
            item->valuestring = NULL;
        }  
        cout << "["<< item->cstring<<"] handing over to parse_Avobject type :"
            <<  item->type<< " "
            << endl;
        return parse_Avobject(item, input_buffer);
    }

    return false;
}


//AssetVar* cJSON_AvParseWithOpts(const char *value, const char **return_parse_end, bool require_null_terminated);

/* Default options for cJSON_Parse */
AssetVar* cJSON_AvParse(const char *value)
{
    return cJSON_AvParseWithOpts(value, 0, 0);
}

//AssetVar* cJSON_AvParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated);


AssetVar* cJSON_AvParseWithOpts(const char *value, const char **return_parse_end, bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return cJSON_AvParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
AssetVar* cJSON_AvParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated)
{
    //parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    parse_buffer buffer = { 0, 0, 0, 0 };
    AssetVar *item = NULL;

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

    item = new AssetVar();
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }
    vmp->addDelMap(item);
    if (!parse_Avvalue(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
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
        delete item;
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
AssetVar* cJSON_AVParse(const char *value)
{
    return cJSON_AvParseWithOpts(value, 0, 0);
}


AssetVar* cJSON_AvParseWithLengthOpts2(AssetVar* item,const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated);


AssetVar* cJSON_AvParseWithOpts2(AssetVar* item, const char *value, const char **return_parse_end, bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");
    //return NULL;
    return cJSON_AvParseWithLengthOpts2(item, value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
AssetVar* cJSON_AvParseWithLengthOpts2(AssetVar* item, const char *value, size_t buffer_length, const char **return_parse_end, bool require_null_terminated)
{
    //goto fail;
    //parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    parse_buffer buffer = { 0, 0, 0, 0 };
   //
    int created  = 0;
    // /* reset error position */
    // global_error.json = NULL;
    // global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }
    //return NULL;
    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length; 
    buffer.offset = 0;
    //buffer.hooks = global_hooks;
    if(item == NULL)
    {
        item = new AssetVar(__func__, NULL);
        //cout << " >>>>>> created  item id:" << item->id <<endl;
        created = 1;

    }
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }
    //return NULL;
    //goto fail;
    if(item->id > 0 && created)
    {
        vmp->addDelMap(item);
        cout << " yes adding item id "<< item->id <<" to del map "<< endl;
    }
    else
    {
        cout << " not adding item id "<< item->id <<" to del map "<< endl;
    }
    //return NULL;
    if (!parse_Avvalue(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }
    //return NULL;

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
    // cout << "looks like the parse worked" <<endl;
    return item;

fail:
    cout  << __func__ << " Parsing failed ..." <<endl;
    if (item != NULL)
    {
        if(created)
        {
            cout << " delete item too " << endl;
            delete item;
        }
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
AssetVar* cJSON_AVParse2(AssetVar*item, const char *value)
{
    //return NULL;
    return cJSON_AvParseWithOpts2(item, value, 0, 0);
}

AssetVar &AssetVar::operator << (const char* sp) {
    return *cJSON_AVParse2(this, sp);
}

ostringstream &operator << (ostringstream &out, const AssetVar &av)
{
    //out << __func__<<" Id :" << av.id << " Name ["<<av.cstring<<"]";
    int depth = 0;
    stream_Avvalue(out, &av, depth);
    return out;
}

// decode options
// '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in a value
// '{"a":{"value":c,"p1":d}}' populate params value p1 etc
//                    
// we loose the varsmap instead root everything from a base av.
AssetVar* AssetVar::addCJParam(const char* name, int cjtype)
{
    if(Params.find(name) != Params.end())
    {
        return Params[name];
    }
    AssetVar* av = new AssetVar(__func__, this); 
    //cout << __func__ << " creating av called :["<<name<<"] av_id:["<<av_id<<"]"<<endl;
    vmp->addDelMap(av);
    Params[name] = av;
    av->name = name;
    av->cstring = strdup(name);
    av->alidx = aList.size();
	aList.push_back(make_pair(av, strdup(name)));
    cout << __func__ << " creating av "<< av<<" called :["<<name
        <<"] av_id:["<<av_id<<"] idx ["<<av->alidx<< "]" <<endl;
	//aVec.push_back(Params[name]);
    av->type = cjtype;
    av->parent = this;

    return av;
}

// this blindly appends actions
AssetVar* AssetVar::addAction(const char* name, const char* when,const char* act, const char* args)
{
    AssetVar *av1 = getAv(name);
    AssetVar *av2 = av1->addCJParam("actions");
    // if (av2->type != cJSON_Array)
    // {
    //    cout <<__func__ << " setting type for  id  :"<<av2->id << endl;
    //    av2->type = cJSON_Array;
    // }
    // AssetVar *av21 = new AssetVar();
    //char * sp = NULL;
    // av2->aList.push_back(make_pair(av21,sp));

    AssetVar *av3 = av2->addCJParam(when);
    //AssetVar *av4 = av3->addCJParam(when);
    if (av3->type != cJSON_Array)
    {
        cout <<__func__ << " setting type for  id  :"<<av3->id << endl;
        av3->type = cJSON_Array;
    }
    AssetVar *av41 = av3->addCJParam(act);
    AssetVar *av42 = 
    av41->addCJParam(act);
    if (av42->type != cJSON_Array)
    {
        cout <<__func__ << " setting type for  id  :"<<av42->id << endl;
        av42->type = cJSON_Array;
    }
    // //now we create an object out of args and append it to av4->aList
    AssetVar *av5 = new AssetVar();
    cJSON_AVParse2(av5, args);
    char* sp = NULL;
    av42->aList.push_back(make_pair(av5,sp));

    cout <<__func__ << " actions id is :"<<av2->id << " act_id "<< av3->id<<endl;
    return av1;

}
AssetVar* AssetVar::getActions(const char* actname)
{
    AssetVar* actVar = Params["actions"];
    if (actVar->gotParam(actname))
    {
        return actVar->Params[actname];
    }
    return nullptr;
}

AssetVar* AssetVar::setVal(const char* var, const char*val)
{
    AssetVar* av1 = getAv(var);
    *av1 = val;
    if (av1->parent)
    {
        if (av1->parent->gotParam("actions"))
        {
            cout << __func__ << "HEY  actions found for [" << av1->parent->cstring<<"]" << endl; 
            AssetVar* actList = av1->parent->getActions("onSet");
            if(actList)
            {
                for ( auto aa: actList->aList)
                {
                    cout <<__func__<< " Action [" <<aa.second <<"]" << endl; 
                }
            }
            else
            {
                cout << __func__ << " >>>>>>no Actlist found " << endl;
            }
        }
        else
        {
            cout << __func__ << " no actions found for [" << av1->parent->cstring<<"]" << endl; 
        }
    }
    return av1;
}

AssetVar* AssetVar::setVal(const char* var, int val)
{
    AssetVar *av1 = getAv(var);
    *av1 = val;
    if (av1->parent)
    {
        if (av1->parent->gotParam("actions"))
        {
            cout << __func__ << "HEY  actions found for [" << av1->parent->cstring<<"]" << endl; 

        }
        else
        {
            cout << __func__ << " no actions found for [" << av1->parent->cstring<<"]" << endl; 
        }
    }
    return av1;
}

AssetVar* AssetVar::setVal(const char* var, double val)
{
    AssetVar *av1 = getAv(var);
    *av1 = val;
    if (av1->parent)
    {
        if (av1->parent->gotParam("actions"))
        {
            cout << __func__ << "HEY  actions found for [" << av1->parent->cstring<<"]" << endl; 

        }
        else
        {
            cout << __func__ << " no actions found for [" << av1->parent->cstring<<"]" << endl; 
        }
    }
    return av1;
}

AssetVar* AssetVar::setVal(const char* var, bool val)
{
    AssetVar *av1 = getAv(var);
    *av1 = val;
    if (av1->parent)
    {
        if (av1->parent->gotParam("actions"))
        {
            cout << __func__ << "HEY  actions found for [" << av1->parent->cstring<<"]" << endl; 

        }
        else
        {
            cout << __func__ << " no actions found for [" << av1->parent->cstring<<"]" << endl; 
        }
    }
    return av1;
}


AssetVar* AssetVar::getAv(const char* var, int options)
{
    AssetVar* res = this;
    vector<string> svec;
    
    svec = splitString(var, {"/",":","@"});
    for ( auto x: svec)
    {
        if (x != "/" && x != ":" && x != "@")
        {
            cout << "[ item x["<< x <<"]" << endl;
            res=res->addCJParam(x.c_str());
        }
    }
    return res;
}

AssetVar* AssetVar::getAv2(const char* var)
{
    AssetVar* res = this;
    vector<string> svec;
    vector<string> svec2;
    vector<string> svec3;
    split(svec, var, '/');
    int idxmax = (int)svec.size();
    int idx = 0;
    for ( auto x: svec)
    {
        cout << "["<< idx <<"]  item x["<< x <<"]" << endl;
        if(idx<idxmax-1) 
        {
            res=res->addCJParam(x.c_str());
        }
        else
        {
            split(svec2, x.c_str(),':');
            idxmax = (int)svec2.size();
            idx = 0;
            for (auto y: svec2)
            {
                cout << "["<< idx <<"]  item xy ["<< x <<"] [" << y <<"]" <<endl;
                //res=res->addCJParam(y.c_str());
                if(idx<idxmax-1) 
                {
                    res=res->addCJParam(y.c_str());
                }
                else
                {
                    split(svec3, y.c_str(),'@');
                    idxmax = (int)svec3.size();
                    idx = 0;
                    for (auto z: svec3)
                    {
                        res=res->addCJParam(z.c_str());
                    }
                }
                idx++;
            }
        }
        idx++;
    }
    if(res == this)
        return NULL;
    return res;
}

void AssetVar::makeLink(const char* var1, const char* var2)
{
    cout << __func__<<" making a link between ["<<var1<<"] and ["<<var2<<"]"<< endl;
    AssetVar* av1 = getAv(var1);
    AssetVar* av2 = getAv(var2);
    if(av1 && av2)
        *av1 = *av2;    
}

AssetVar* AssetVar::makeLink(const char* var1, const char* name, AssetVar* av)
{
    cout << __func__<<" making a link between ["<<var1
            << "] called [" << name <<"] and ["<<av->cstring<<"]"<< endl;
    AssetVar* av1 = getAv(var1);
    AssetVar* av2 = av1->addCJParam(name, cJSON_Object);
    if(av2)
        *av2 = *av;
    return av2;    
}

AssetVar* findAv(AssetVar *av, const char* comp, const char*name , int type)
{
    AssetVar* avn = NULL;
    AssetVar* avc = av->gotParam(comp);
    if(!avc)
    {    
        avc = av->addCJParam(comp, type);        
    }
    avn = avc;
    if(avc&&name)
    {
        avn = avc->gotParam(name);
        if(!avn)
        {
            avn = avc->addCJParam(name, type);
        }
    }
    return avn;
}

// darn it acts need a parent
// vars with acts wont relocate if we have to do this....
// unless we tuck the relocaing acts under the var.
// we can relocate the parent...

AssetVar* getActs(AssetVar *av, const char *act)
{
    //AssetVar*avact = NULL;
    //return avact;
    cout << " av->parent " << av->parent << endl;
    if (av->parent && av->parent->cstring)
    {
        //cout << " av->parent name " << av->parent->cstring << endl;
        AssetVar *avacts = av->parent->gotParam("actions");
        if(avacts)
        {
            //cout << __func__ <<" >>>>>found avacts " << avacts << endl;
            AssetVar* actset = avacts->gotParam(act); 
            if(actset)
            {
                // // an actset needs to be an object
                // cout << endl
                //      << __func__
                //      << " actset " << actset << " type " << actset->type << " list size " << actset->aList.size()<< endl;
                if(actset->aList.size()> 0)
                {
                    // return actset;
                    // cout <<__func__
                    //     << " actset  " << actset 
                    //     << " type " << actset->type
                    //     << " name " << actset->cstring
                    //     << endl; 
                    AssetVar* ava = actset->aList[0].first;

                    // cout <<__func__
                    //     << " ava  " << ava 
                    //     << " type " << ava->type;
                    //     if (ava->cstring)

                    //         cout << " name " << ava->cstring;

                    //     cout << endl; 
                    return ava; // we may be need to return the array
                }
            }
        }
    }
    return NULL;
}

AssetVar* runEnum(AssetVar* av, AssetVar*avn, AssetVar*act, int idx, AssetVar*aparms)
{
    cout << __func__<< " $$$$$$$$$$ enum running  "<< endl;

    for ( auto ay: aparms->aList)
    {
        // list of param items for the action
        cout << "  act params ..." << ay.first << " type "<<ay.first->type;
        if (ay.first->cstring)
            cout << " param name ["<<ay.first->cstring<<"]";
        cout << endl;
    }
    return av;
}

// run through the enums 
AssetVar* runEnums(AssetVar* av, AssetVar*avn, AssetVar*act, int idx) 
{
    auto aa = act->aList.at(idx);
    if(aa.first->type == cJSON_Array) 
    {
        AssetVar* aname = aa.first;
        for ( auto ax: aname->aList)
        {
            // list of param sets for the action
            cout << "  act items ..." << ax.first << " type "<<ax.first->type;
            if (ax.first->cstring)
                cout << " param name ["<<ax.first->cstring<<"]";
            cout << endl;
            AssetVar* aparms = ax.first;
            //runEnums(av, avn, aa.first,)
            runEnum(av,avn, act, idx, aparms) ;
        }
    }
    return av;
}

AssetVar* runAction(AssetVar* av, AssetVar* avn, const char* when)
{
    if(!avn->cstring || (strcmp(avn->cstring, "value")!= 0))
    {
        cout <<__func__<<" Skipped Action";
        if(avn->cstring)
            cout  <<" for name " << avn->cstring;
        cout << endl;
        return NULL;
    }
    AssetVar* act = getActs(avn, when);

    if(act)
    {

        for ( auto aa: act->aList)
        {
            
            if(strcmp(aa.first->cstring, "enum")==0)
            {
                return runEnums(av, avn, act, aa.first->alidx);
            }
            else
            {
                cout << " @@@ unknown acton ["<<aa.first->cstring<<"]"<<endl;
                return NULL;
            }

        }
    }
    return av;
}

AssetVar* setValue(AssetVar *av, const char* comp, const char*name, const char* val)
{
    
    AssetVar* avn = findAv(av, comp, name);
    if (avn)
    {
        avn->type = cJSON_String;
        if(avn->valuestring)
        {
            free(avn->valuestring);
        }
        avn->valuestring = strdup(val);
        runAction(av, avn, "onSet");

    }
    else
    {
        cout << __func__ << " Unable to find Av  ["<<comp<<":"<<name<<"]"<<endl;
    }

    return avn;
}

AssetVar* setValue(AssetVar *av, const char* comp, const char*name, double val)
{   
    AssetVar* avn = findAv(av, comp, name, cJSON_Number);
    if (avn)
    {
        avn->valuedouble = val;
        //avn->valueint = (int)val;
        runAction(av, avn, "onSet");
    }

    return avn;
}

AssetVar* setValue(AssetVar *av, const char* comp, const char*name, bool val)
{
    //cout << " running setvalue "<<endl;
    AssetVar* avn = findAv(av, comp, name, cJSON_True);
    if(!avn)
        avn = findAv(av, comp, name, cJSON_False);

    if (avn)
    {
        avn->valuebool = val;
        //avn->valueint = (int)val;
        if(val)
            avn->type = cJSON_True;
        else
            avn->type = cJSON_False;
        runAction(av, avn, "onSet");
    }

    return avn;
}

AssetVar* setValue(AssetVar *av, const char* comp, const char*name, int val)
{
    //cout << " running setvalue "<<endl;
    AssetVar* avn = findAv(av, comp, name, cJSON_Number);
    if (avn)
    {
        avn->valuedouble = val;
        //avn->valueint = (int)val;
        runAction(av, avn, "onSet");
    }

    return avn;
}

void* getFunc(AssetVar* av, const char* aname, const char* fname)
{
    void* res = nullptr;
    string fun = "/functions/";
    fun += aname;

    AssetVar* av1 = av->getAv(fun.c_str());
    if(av1->gotParam(fname))
        res =  av1->Params[fname]->func;
    return res;
}

AssetVar* setFunc(AssetVar* av, const char* aname, const char* fname, void* func)
{
    string fun = "/functions/";
    fun += aname;

    AssetVar* av1 = av->getAv(fun.c_str());
    AssetVar* av2 = av1->addCJParam(fname, cJSON_String);
    if(av2->valuestring)
        free(av2->valuestring);
    av2->valuestring = strdup(fname);
    av2->func = func; 
    return av2;   
}