#ifndef ZDD_DEVICECONNECTION_H
#define ZDD_DEVICECONNECTION_H
#include <cstdlib>

#if defined(__APPLE__)
#include <hidapi.h>
// The vendor backend takes an extra USB interface argument. Standard HIDAPI
// does not, so macOS must enumerate and select the configuration interface.
hid_device *zdd_hid_open(unsigned short vid, unsigned short pid,
                         int interface_number, const wchar_t *serial);
const wchar_t *zdd_hid_error(hid_device *device);
#else
#include "hidapi.h"
inline hid_device *zdd_hid_open(unsigned short vid, unsigned short pid,
                                int interface_number, const wchar_t *serial) {
    if (std::getenv("ZDD_OFFLINE_PREVIEW")) return nullptr;
    return hid_open(vid, pid, interface_number, serial);
}
inline const wchar_t *zdd_hid_error(hid_device *device) { return hid_error(device); }
#endif
#endif
