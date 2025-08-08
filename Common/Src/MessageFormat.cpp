#include <iostream>
using namespace std;

uint16_t calculateChecksum(const string& data) {
    uint16_t checksum = 0;
    for (char ch : data) {
        checksum += static_cast<uint16_t>(ch);
    }
    return checksum;
}