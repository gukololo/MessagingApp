#include <iostream>
using namespace std;

uint32_t calculateChecksum(const string& data) {
    uint32_t checksum = 0;
    for (char ch : data) {
        checksum += static_cast<uint32_t>(ch);
    }
    return checksum;
}