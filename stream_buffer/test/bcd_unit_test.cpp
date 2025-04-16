#include "utils/debug.h"
#include <iostream>
#include <cstring>
#include <cassert>
#include <limits>
#include <vector>

using namespace stream_buffer::utils;

// Unit test framework structure
struct TestCase
{
    const char *name;
    bool (*test_func)();
};

// Test for normal BCD decoding
bool test_valid_bcd()
{
    uint8_t bcd_data[] = {0x12, 0x34, 0x56, 0x78, 0x90};
    uint64_t result = decode_bcd(bcd_data, sizeof(bcd_data));
    bool passed = (result == 1234567890);

    std::cout << "Test valid BCD: " << (passed ? "PASSED" : "FAILED")
              << " (expected 1234567890, got " << result << ")" << std::endl;
    return passed;
}

// Test for empty data
bool test_empty_data()
{
    uint64_t result = decode_bcd(nullptr, 0);
    bool passed = (result == static_cast<uint64_t>(-1));

    std::cout << "Test empty data: " << (passed ? "PASSED" : "FAILED")
              << " (expected " << static_cast<uint64_t>(-1) << ", got " << result << ")" << std::endl;
    return passed;
}

// Test for invalid BCD digits
bool test_invalid_bcd()
{
    uint8_t invalid_bcd[] = {0x12, 0x3A, 0x45}; // 0xA is not a valid BCD digit
    uint64_t result = decode_bcd(invalid_bcd, sizeof(invalid_bcd));
    bool passed = (result == static_cast<uint64_t>(-1));

    std::cout << "Test invalid BCD: " << (passed ? "PASSED" : "FAILED")
              << " (expected " << static_cast<uint64_t>(-1) << ", got " << result << ")" << std::endl;
    return passed;
}

// Test for overflow handling
bool test_bcd_overflow()
{
    // Create a large BCD array that would overflow uint64_t if not checked
    std::vector<uint8_t> large_bcd(11, 0x99);
    uint64_t result = decode_bcd(large_bcd.data(), large_bcd.size());
    bool passed = (result == static_cast<uint64_t>(-1));

    std::cout << "Test BCD overflow: " << (passed ? "PASSED" : "FAILED")
              << " (expected " << static_cast<uint64_t>(-1) << ", got " << result << ")" << std::endl;
    return passed;
}

// Test for maximum valid value
bool test_bcd_max_value()
{
    // Create a BCD array with max digits that fit in uint64_t (20 digits)
    uint8_t max_bcd[] = {0x18, 0x44, 0x67, 0x44, 0x07, 0x37, 0x09, 0x55, 0x16, 0x15};
    // The value 18446744073709551615 is 2^64-1

    uint64_t result = decode_bcd(max_bcd, sizeof(max_bcd));
    uint64_t expected = std::numeric_limits<uint64_t>::max();
    bool passed = (result == expected);

    std::cout << "Test BCD max value: " << (passed ? "PASSED" : "FAILED")
              << " (expected " << expected << ", got " << result << ")" << std::endl;
    return passed;
}

// Test for a single byte
bool test_single_byte()
{
    uint8_t bcd_data = 0x42;
    uint64_t result = decode_bcd(&bcd_data, 1);
    bool passed = (result == 42);

    std::cout << "Test single byte: " << (passed ? "PASSED" : "FAILED")
              << " (expected 42, got " << result << ")" << std::endl;
    return passed;
}

// Test for hex_dump function (only check that it runs)
bool test_hex_dump()
{
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15};

    std::cout << "\nTesting hex_dump function:\n";
    hex_dump(data, sizeof(data));
    std::cout << "Hex dump completed.\n";

    return true;
}

int main()
{
    std::cout << "==== BCD Unit Tests ====\n"
              << std::endl;

    // Define all test cases
    TestCase test_cases[] = {
        {"Valid BCD", test_valid_bcd},
        {"Empty Data", test_empty_data},
        {"Invalid BCD", test_invalid_bcd},
        {"BCD Overflow", test_bcd_overflow},
        {"BCD Max Value", test_bcd_max_value},
        {"Single Byte", test_single_byte},
        {"Hex Dump", test_hex_dump}};

    // Run all tests and count failures
    size_t num_tests = sizeof(test_cases) / sizeof(TestCase);
    size_t passed_tests = 0;

    for (size_t i = 0; i < num_tests; ++i)
    {
        std::cout << "\nRunning test: " << test_cases[i].name << std::endl;
        if (test_cases[i].test_func())
        {
            passed_tests++;
        }
    }

    // Print summary
    std::cout << "\n==== Test Results ====\n";
    std::cout << "Passed: " << passed_tests << "/" << num_tests
              << " (" << (passed_tests * 100 / num_tests) << "%)" << std::endl;

    // Return 0 if all tests passed, otherwise return the number of failures
    return (passed_tests == num_tests) ? 0 : (num_tests - passed_tests);
}
