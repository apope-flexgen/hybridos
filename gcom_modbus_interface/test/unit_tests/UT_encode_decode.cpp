#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "decode_encode_utils.hpp"
#include "spdlog/fmt/fmt.h"

TEST_SUITE("Encoding and Decoding") 
{
    uint16_t mod_buf[4];
    Jval_buif jval;
    Decode_Info decoder;

    TEST_CASE("Basics")
    {
        // basic setup per SUBCASE:
        decoder.flags.flags = 0;
        decoder.scale = 0.0;
        decoder.shift = 0;
        decoder.invert_mask = 0UL;

        SUBCASE("no_flags/unsigned")
        {
            decoder.flags.set_size(1);
            jval = 2UL;

            SUBCASE("basic")
            {
                encode(mod_buf, decoder, jval);
                jval = 3.14;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 2UL);
            }
        }

        SUBCASE("signed")
        {
            decoder.flags.set_size(1);
            decoder.flags.set_signed(true);
            jval = -2;

            SUBCASE("basic")
            {
                encode(mod_buf, decoder, jval);
                jval = 3.14;
                decode(mod_buf, decoder, jval);
                CHECK(jval == -2);
            }
        }

        SUBCASE("float")
        {
            decoder.flags.set_float(true);
            jval = 3.14;

            SUBCASE("float32")
            {
                decoder.flags.set_size(2);
                encode(mod_buf, decoder, jval);
                jval = 2;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 3.14f); // float32 compare needed here
            }
            SUBCASE("float64")
            {
                decoder.flags.set_size(4);
                encode(mod_buf, decoder, jval);
                jval = 2;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 3.14);
            }
        }

        SUBCASE("scale")
        {
            decoder.flags.set_size(1);
            decoder.scale = 10.0;
            jval = 100UL;

            SUBCASE("basic")
            {
                encode(mod_buf, decoder, jval);
                jval = 3.14;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 100.0); // decoded scaled values come out as float64 to support decimal places
            }
        }

        SUBCASE("shift")
        {
            decoder.flags.set_size(1);
            decoder.shift = -2;
            jval = 100UL;

            SUBCASE("basic")
            {
                encode(mod_buf, decoder, jval);
                jval = 3.14;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 100UL);
            }
        }

        SUBCASE("invert")
        {
            decoder.flags.set_size(1);
            decoder.invert_mask = 1UL;
            jval = 100UL;

            SUBCASE("basic")
            {
                encode(mod_buf, decoder, jval);
                jval = 3.14;
                decode(mod_buf, decoder, jval);
                CHECK(jval == 100UL);
            }
        }
    }
}
