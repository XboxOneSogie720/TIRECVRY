# TIRECVRY
irecovery but on a calculator.

## Features
- Intuitive GUI
- Automatic device detection and management
- Polling device info

## Per-Mode Features
| Feature  | Description                                  | Recovery | DFU | WTF |
|----------|----------------------------------------------|----------|-----|-----|
| Reset    | Resets the USB Connection                    | ✅        | ✅   | ✅   |
| Send var | Sends an AppVar to the device                | ✅        | ✅   | ✅   |
| Command  | Runs the inputted command on the device      | ✅        | ❌   | ❌   |
| Reboot   | Reboots the device                           | ✅        | ❌   | ❌   |
| Normal   | Reboots the device into normal mode          | ✅        | ❌   | ❌   |
| Get ENV  | Gets an environment variable off the device. | ✅        | ❌   | ❌   |
| Set ENV  | Sets an environment variable on the device.  | ✅        | ❌   | ❌   |
| Save ENV | Saves environment variables on the device.   | ✅        | ❌   | ❌   |

## Quirks/Known Issues
- When connecting a device, have the cable connected to the device and adapter first, then connect the adapter side to the calculator. Otherwise, the usbdrvce library won't be able to reset the connection.

## Successful Tests
- Putting an `iPhone 3G` into `DFU mode from WTF mode` by sending a `full` WTF file from an `iOS 4.2.1 IPSW`.
- Putting an `iPhone 3G` into `DFU mode from WTF mode` by sending `split up` WTF files (`split -n 2 wtf.bin` and `convbin...`) from an `iOS 4.2.1 IPSW`.
- Sending an `iBSS`, `iBEC`, and `Recovery Mode picture` to an `iPhone 4` in `PWNDFU mode` (pwned on a mac), then sending the `setpicture 0x1` and `bgcolor` commands.

## Credits 
- [Apple Inc.](https://www.apple.com/) for their amazing devices
- [libimobiledevice](https://libimobiledevice.org/) for the implementation.
- [TI-84 Plus CE Toolchain Team](https://ce-programming.github.io/toolchain/)
- [Embedded sscanf()](https://41j.com/blog/2015/01/standalone-sscanf-implementation-esp8266/)
