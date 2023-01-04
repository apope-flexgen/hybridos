//============================================================================
// Name        : Valid_URI_Test.cpp
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Test of fims server URI validation.
//============================================================================

#include "fims_test_utilities.h"

TEST(ValidURITest, NormalURI)
{
    EXPECT_TRUE(valid_uri((char*)"/path/to/message"));
}

TEST(ValidURITest, URIWithNoLeadingSlash)
{
    EXPECT_FALSE(valid_uri((char*)"path/to/message"));
}

TEST(ValidURITest, URIWithTrailingSlash)
{
    EXPECT_TRUE(valid_uri((char*)"/path/to/message/"));
}

TEST(ValidURITest, URIWithExtraSlash)
{
    EXPECT_TRUE(valid_uri((char*)"/path//to/message"));
}

TEST(ValidURITest, NullURI)
{
    EXPECT_FALSE(valid_uri(NULL));
}

TEST(ValidURITest, URIWithNonAlphaNumeric)
{
    EXPECT_FALSE(valid_uri((char*)"/?-p+/=**&/"));
}
