//============================================================================
// Name        : FIMS_Test.cpp
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Runs all tests related to FIMS
//============================================================================
#ifndef FIMS_TEST_H_
#define FIMS_TEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "libfims_test.h"

#include "receive_messages_test.h"
#include "valid_uri_test.h"
#include "insert_subscription_test.h"
#include "get_connections_for_uri_test.h"
#include "remove_subscription_test.h"

int test_main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif /* FIMS_TEST_H_ */
