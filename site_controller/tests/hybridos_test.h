#ifndef SITE_TEST_H_
#define SITE_TEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "site_manager_test.h"
#include "asset_manager_test.h"
#include "fims_object_test.h"
#include "utils_test.h"

int test_main(int argc, char **argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

#endif /* SITE_TEST_H_ */
