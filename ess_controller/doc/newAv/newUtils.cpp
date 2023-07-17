#include <string>
#include <vector>

#include "parse_buffer.h"

#include "newUtils.h"

using namespace std;
void NewUtils::addDelMap(AssetVar*av)
{
        delavMap.insert(std::pair<AssetVar *, void *>(av, (void*)av));
}
NewUtils::~NewUtils()
{
    for (auto x: delavMap)
    {
        cout << x.first;
        if(x.second)
        {
            cout << " id : " << x.first->id<<" type : "<<x.first->type;
            if(x.first->cstring)
            {
                cout << " cstring [" << x.first->cstring <<"]";
            }
            else
            {
                cout << " name [" << x.first->name <<"]";
            }

            }
        else
        {
            cout << " ... deleted";
        }
        cout << endl;
    }
    cout << " Bye NewUtils" << endl;
};
// UTILS here 
double base_time = 0.0;
long int get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
    return ltime_us;
}

double get_time_dbl()
{
    if(base_time == 0.0)
    {
        base_time = get_time_us()/ 1000000.0;
    }

    return  (double)get_time_us() / 1000000.0 - base_time;
}

int split(vector<string> &output, const string& s, char seperator)
{
    int i = 0;
    string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != string::npos)
    {
        string substring( s.substr(prev_pos, pos-prev_pos) );
		if(substring.size()> 0)
		{
        	output.push_back(substring);
			i++;
		}
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return i;
}
// decode options
// '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in a value
// '{"a":{"value":c,"p1":d}}' populate params value p1 etc
//                    
// we loose the varsmap instead root everything from a base av.


/* securely comparison of floating-point variables */
bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char) lconv->decimal_point[0];
#else
    return '.';
#endif
}

// /* check if the given size is left to read in a given parse buffer (starting with 1) */
// #define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
// /* check if the buffer can be accessed at the given index (starting with 0) */
// #define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
// #define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
// /* get a pointer to the buffer at the position */
/* parse 4 digit hexadecimal number */
//static 
unsigned parse_hex4(const unsigned char * const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}
/* converts a UTF-16 literal to UTF-8
/ * A literal can be one or two sequences of the form \uXXXX */
unsigned char utf16_literal_to_utf8(const unsigned char* const input_pointer, const unsigned char* const input_end, unsigned char** output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }


        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}
/* Utility to jump whitespace and cr/lf */
parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
       buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
parse_buffer *skip_utf8_bom(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

// recovers /builds an AssetVar
// "TestValues:val1@myParam45":    45
//find the first delimeter in the string
string findFirstOf(string input, vector<string> del)
{

    //get a map of delimeter and position of delimeter
    size_t pos;
    std::map<std::string, size_t> m;

    for (int i = 0; i < (int)del.size(); i++)
    {
        pos = input.find(del[i]);
        if (pos != std::string::npos)
            m[del[i]] = pos;
    }

    //find the smallest position of all delimeters i.e, find the smallest value in the map

    if (m.size() == 0)
        return "";

    size_t v = m.begin()->second;
    string k = m.begin()->first;

    for (auto it = m.begin(); it != m.end(); it++)
    {
        if (it->second < v)
        {
            v = it->second;
            k = it->first;
        }
    }
    return k;
}

vector<string> splitString(string input, vector<string> delimeters)
{
    vector<string> result;
    size_t pos = 0;
    string token;
    string delimeter = findFirstOf(input, delimeters);

    while(delimeter != "")
    {
        if ((pos = input.find(delimeter)) != string::npos)
        {
            token = input.substr(0, pos);
            result.push_back(token);
            result.push_back(delimeter);
            input.erase(0, pos + delimeter.length());
        }
        delimeter = findFirstOf(input, delimeters);
    }
    result.push_back(input);
    return result;
}
