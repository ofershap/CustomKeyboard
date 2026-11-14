# macOS development build

This change ports the existing Qt setter to macOS. It keeps the original
configuration frames and key layout. It does not flash device firmware.
The existing Windows backend remains selected on Windows.

## Build

Use a Mac with Xcode command-line tools and Homebrew:

```sh
brew install qt@5 hidapi pkg-config
scripts/build-macos.sh
```

The script builds `build-macos/KeyboardSetter.app`, bundles its dependencies,
checks an ad-hoc signature, captures two offline UI previews, and creates an
architecture-specific zip. Qt 5 is deprecated in Homebrew; it is retained
here to keep this portability change separate from a Qt 6 migration.
The dependency's minimum OS requirements still apply. A build on a modern
runner does not establish compatibility with older macOS versions.

## Safety and compatibility

The Mac backend uses standard HIDAPI 0.14.0 or later. It enumerates the vendor
VID/PID (0x5131:0x2019) and opens only USB configuration interface 1, matching
the original backend. If it cannot identify exactly one matching interface,
it refuses to open a device. Unplug extra matching keypads before configuring.
There is no fallback to the first keyboard endpoint.

A shared VID/PID is not proof that a particular keypad uses this firmware.
Confirm your model and the vendor key-mapping image before choosing a key.
Do not assume every single-button model uses the same software slot.

`--preview-shots DIRECTORY` disables all device access before creating the
windows and saves device-finder/configuration images. This tests UI launch,
not real device discovery, ACKs, persistence, or emitted keyboard events.

The generated development app is ad-hoc signed, not Developer ID signed or
notarized. No Apple signing identity or credentials are used. Do not disable
Gatekeeper or other security protections to run an untrusted build.

## Verification status

- Interface selection: automated mock tests cover unique match, wrong
  interface, missing interface metadata, ambiguous matches, wrong VID,
  serial filtering, empty enumeration, and initialization failure.
- Original Qt UI: offline Linux regression build and pixel inspection done.
- Native macOS 15 arm64 and x86_64: compile, bundle, ad-hoc signature
  verification, offline launch, and native UI image inspection passed in
  [run 43616165462](https://github.com/ofershap/CustomKeyboard/actions/runs/43616165462).
- Actual keypad discovery/configuration ACKs, reconnect persistence, and emitted
  keyboard events through this port: untested. No physical keypad is attached
  to CI; do not describe this port's hardware support as tested.
- Earlier ARM launch failed because macdeployqt missed a transitive Brotli
  library. The build script now bundles the matching Homebrew Brotli libraries
  before signing. Both native architectures passed after that change.

For a real device test, verify the selected physical slot, record the ACK
result, unplug/reconnect, and check the emitted modifier/key event. A chord
such as Shift + backtick is a modifier and a normal key, not two sequential
macro steps. Programming can replace existing device settings; preserve the
intended configuration before pushing.

## Provenance

This adapts this repository's Qt setter. It does not copy the separate
`19JVJeffery/CustomKeyboard-macOS` Swift rewrite. HIDAPI is a separate dependency
with its own licenses: https://github.com/libusb/hidapi . Preserve all original
copyright notices. No repository-wide license is asserted or added by this
change; maintainers should clarify redistribution terms before a release.
