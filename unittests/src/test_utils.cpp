/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2017 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#include "test_utils.h"

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <string.h>
#include <regex>

namespace testing {

const char* CutPath(const char* path)
{
    const char* result = strrchr(path, '/');
    if(result == nullptr) {
        result = path;
    }
    else {
        ++result; // next symbol after /
    }
    return result;
}

std::ostringstream g_failures_stream;

static std::unique_ptr<std::regex> g_test_filter; // unique_ptr as optional

std::vector<TestRegistration>& GetTestsRegistry()
{
    static std::vector<TestRegistration> g_tests_registry;
    return g_tests_registry;
}

void RegisterTest(const TestRegistration& registration)
{
    GetTestsRegistry().push_back(registration);
}

TestRegistration::TestRegistration(const std::string& test_case_name, const std::string& test_name, TestFunction* func):
    test_case_name_(test_case_name), test_name_(test_name), func_(func)
{
    RegisterTest(*this);
}

void InitGoogleTest(int* argc, char** argv)
{
    for(int i = 1; i < *argc; ++i) {
        std::string arg(argv[i]);

        bool ok = false;

        std::string prefix("--gtest_filter=");
        if (!arg.compare(0, prefix.size(), prefix)) {
            try {
                g_test_filter = std::make_unique<std::regex>(arg.substr(prefix.size()));
                ok = true;
            } catch(const std::exception& ex) {
                std::cout << "Regex exception: " << ex.what() << std::endl;
            }
        }

        if(!ok) {
            std::cout << "Invalid argument: " << arg << std::endl;
        }
    }
}

} // namespace testing

using namespace testing;

int RUN_ALL_TESTS()
{
    int failed_tests = 0;
    size_t executing_tests = GetTestsRegistry().size();

    for(const auto& test : GetTestsRegistry()) {

        std::string test_full_name = test.test_case_name_ + ":" + test.test_name_;
        if(nullptr != g_test_filter) {
            if(!regex_match(test_full_name.c_str(), *g_test_filter)) {
                --executing_tests;
                continue;
            }
        }

        std::cout << test_full_name;
        g_failures_stream = std::ostringstream();

        try {
            test.func_();

            if(!g_failures_stream.str().empty()) {
                std::cout << " failed " << g_failures_stream.str() << std::endl;
                ++failed_tests;
            }
            else {
                std::cout << " succeeded" << std::endl;
            }
        }
        catch(const std::ostringstream& str) {
            std::cout << " failed " << str.str() << g_failures_stream.str() << std::endl;
            ++failed_tests;
        }
    }

    std::cout << "\n";

    if(failed_tests != 0) {
        std::cout << failed_tests << " of " << executing_tests << " test(s) FAILED";
    }
    else {
        std::cout << executing_tests << " test(s) SUCCEEDED";
    }

    if(executing_tests != GetTestsRegistry().size()) {
        std::cout << "; " << GetTestsRegistry().size() - executing_tests << " test(s) SKIPPED";
    }

    std::cout << "\n" << std::endl;
    return failed_tests != 0;
}
