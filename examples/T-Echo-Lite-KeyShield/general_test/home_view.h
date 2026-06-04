/**
 * @file home_view.h
 * @brief general_test home page data provider.
 */
#ifndef T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_HOME_VIEW_H_
#define T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_HOME_VIEW_H_

#include <stddef.h>

#include <Arduino.h>

#include <string>
#include <vector>

namespace home_view {

const char* GetSoftwareName();
const char* GetBoardVersion();
String GetBuildTime();
std::vector<std::string> CreateLines();
size_t GetMaxScrollIndex();

}  // namespace home_view

#endif  // T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_HOME_VIEW_H_
