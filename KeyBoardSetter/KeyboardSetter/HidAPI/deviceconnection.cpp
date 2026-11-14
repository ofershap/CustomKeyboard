#include "deviceconnection.h"
#include <cwchar>
#include <string>

namespace {
thread_local std::wstring selection_error;
}

hid_device *zdd_hid_open(unsigned short vid, unsigned short pid,
                         int interface_number, const wchar_t *serial) {
    selection_error.clear();
    if (std::getenv("ZDD_OFFLINE_PREVIEW")) {
        selection_error = L"Offline preview: device access is disabled.";
        return nullptr;
    }
    if (hid_init() != 0) return nullptr;
    hid_device_info *devices = hid_enumerate(vid, pid);
    std::string path;
    unsigned matches = 0;
    for (hid_device_info *item = devices; item; item = item->next) {
        if (item->vendor_id != vid || item->product_id != pid ||
            item->interface_number != interface_number || !item->path) continue;
        if (serial && (!item->serial_number || std::wcscmp(serial, item->serial_number) != 0)) continue;
        ++matches;
        path = item->path;
    }
    hid_free_enumeration(devices);
    if (matches != 1) {
        selection_error = matches == 0
            ? L"No matching USB configuration interface. Check the device model and connection."
            : L"Multiple matching USB configuration interfaces. Unplug extra keypads and try again.";
        return nullptr;
    }
    // No fallback to the first HID device: that can select the keyboard's
    // normal input collection instead of its configuration interface.
    return hid_open_path(path.c_str());
}

const wchar_t *zdd_hid_error(hid_device *device) {
    return selection_error.empty() ? hid_error(device) : selection_error.c_str();
}
