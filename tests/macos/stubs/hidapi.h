#ifndef TEST_HIDAPI_H
#define TEST_HIDAPI_H
#include <cstddef>
struct hid_device_ {};
typedef hid_device_ hid_device;
struct hid_device_info {
    char *path;
    unsigned short vendor_id, product_id;
    wchar_t *serial_number;
    int interface_number;
    hid_device_info *next;
};
int hid_init();
hid_device_info *hid_enumerate(unsigned short, unsigned short);
void hid_free_enumeration(hid_device_info *);
hid_device *hid_open_path(const char *);
const wchar_t *hid_error(hid_device *);
#endif
