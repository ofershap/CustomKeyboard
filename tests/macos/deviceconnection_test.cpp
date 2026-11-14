#include "deviceconnection.h"
#include <cassert>
#include <cstring>
#include <cwchar>
#include <iostream>

static hid_device_info *enumeration = nullptr;
static int init_result = 0, opens = 0, frees = 0;
static hid_device device;
static const char *expected_path = "config";
int hid_init() { return init_result; }
hid_device_info *hid_enumerate(unsigned short, unsigned short) { return enumeration; }
void hid_free_enumeration(hid_device_info *) { ++frees; }
hid_device *hid_open_path(const char *path) {
    assert(std::strcmp(path, expected_path) == 0); ++opens; return &device;
}
const wchar_t *hid_error(hid_device *) { return L"backend error"; }
int main() {
    hid_device_info input{const_cast<char *>("input"),0x5131,0x2019,nullptr,0,nullptr};
    hid_device_info config{const_cast<char *>("config"),0x5131,0x2019,nullptr,1,nullptr};
    enumeration = &input;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr));
    assert(opens == 0 && std::wcsstr(zdd_hid_error(nullptr),L"No matching"));
    input.next = &config;
    assert(zdd_hid_open(0x5131,0x2019,1,nullptr) == &device);
    assert(opens == 1 && std::wcscmp(zdd_hid_error(nullptr),L"backend error") == 0);
    hid_device_info duplicate = config;
    config.next = &duplicate;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr));
    assert(opens == 1 && std::wcsstr(zdd_hid_error(nullptr),L"Multiple"));
    config.next = nullptr;
    config.interface_number = -1;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr)); // fail closed, no interface metadata
    config.interface_number = 1;
    config.vendor_id = 0x1234;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr));
    config.vendor_id = 0x5131;
    assert(!zdd_hid_open(0x5131,0x2019,1,L"expected"));
    config.serial_number = const_cast<wchar_t *>(L"expected");
    assert(zdd_hid_open(0x5131,0x2019,1,L"expected") == &device);
    enumeration = nullptr;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr));
    init_result = -1;
    assert(!zdd_hid_open(0x5131,0x2019,1,nullptr));
    assert(opens == 2 && frees == 8);
    std::cout << "PASS: 9 interface-selection scenarios; no USB I/O performed\n";
}
