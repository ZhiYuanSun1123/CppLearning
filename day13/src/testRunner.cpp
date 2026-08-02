#include"testRunner.hpp"
#include<iostream>
void TestRunner::expect_true(
    bool condition,
    const std::string& test_name
){
    if(condition){
        ++passed_;
        std::cout
            << "[PASS] "
            << test_name
            << std::endl;
    } else{
        ++failed_;
        std::cerr
            << "[FAIL] "
            << test_name
            << std::endl;
    }
}
void TestRunner::expect_equal(
    const std::string& actual,
    const std::string& expected,
    const std::string& test_name
) {
    if(actual == expected){
        ++passed_;
        std::cout
            << "[PASS] "
            << test_name
            << std::endl;
    } else {
        ++failed_;
        std::cerr
            << "[FAIL] "
            << test_name
            << std::endl
            << " expected "
            << expected
            << std::endl
            << " actual "
            << actual
            << std::endl;
    }
}
void TestRunner::expect_equal(
    int actual,
    int expected,
    const std::string& test_name
) {
    if(actual == expected){
        ++passed_;
        std::cout
            << "[PASS] "
            << test_name
            << std::endl;
    } else{
        ++failed_;
        std::cout
            << "[FAIL] "
            << test_name
            << std::endl
            << " expected "
            << expected
            << std::endl
            << " actual "
            << actual
            << std::endl;
    }
}
int TestRunner::final() const{
    std::cout
        << "\nPassed: "
        << passed_
        << "\nFailed: "
        << failed_
        << std::endl;
    return failed_ == 0 ? 0 : 1;
}