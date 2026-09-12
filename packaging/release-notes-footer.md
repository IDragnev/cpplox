## Requirements

- **Windows x64** - the [Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist), which most machines already have.
- **Linux x64** - @LINUX_REQS@. An older system refuses to start the binary with a loader error naming the version it is missing; building from source is the way round it.
- **macOS arm64** - macOS 11 (Big Sur) or newer, on Apple Silicon.

The binaries are unsigned, so Windows SmartScreen and macOS Gatekeeper ask before the first run. On macOS that means *Right-click → Open*, or clearing the quarantine flag with `xattr -cr cpplox`.
