#ifndef __NEW_UTILS_H
#define __NEW_UTILS_H
#include <map>

#include "newAv2.h"
#include "parse_buffer.h"

using namespace std;

class AssetVar;

class NewUtils
{
public:
    NewUtils() { cout << " Hello NewUtils" << endl; };
    void addDelMap(AssetVar* av);
    ~NewUtils();

    std::map<AssetVar*, void*> delavMap;
};
vector<string> splitString(string input, vector<string> delimeters);
// now import the real cJSON Parser
// first into a cJSON object
// then into out AssetVar
typedef struct
{
    const unsigned char* json;
    size_t position;
} error;
// static error global_error = { NULL, 0 };
/* get the decimal point character of the current locale */
// static
unsigned char get_decimal_point(void);

// typedef struct
// {
//     const unsigned char *content;
//     size_t length;
//     size_t offset;
//     size_t depth; /* How deeply nested (in arrays/objects) is the input at
//     the current offset. */
//     //internal_hooks hooks;
// } parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting
 * with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)
/* parse 4 digit hexadecimal number */
// static
unsigned parse_hex4(const unsigned char* const input);
/* converts a UTF-16 literal to UTF-8
/ * A literal can be one or two sequences of the form \uXXXX */
// static
unsigned char utf16_literal_to_utf8(const unsigned char* const input_pointer, const unsigned char* const input_end,
                                    unsigned char** output_pointer);
/* Utility to jump whitespace and cr/lf */
// static
parse_buffer* buffer_skip_whitespace(parse_buffer* const buffer);
/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
// static
parse_buffer* skip_utf8_bom(parse_buffer* const buffer);
#endif
