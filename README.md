# ScreenshotBOF

An alternative screenshot capability for Cobalt Strike that uses WinAPI and does not perform a fork & run. Screenshot downloaded in memory.

Includes the original compiled BOF and [get_screenshot.c](get_screenshot.c), a Beacon Interpreter script that captures and encodes screenshots without compiling a BOF.

# Features
- JPEG compression
- In memory download as screenshot or as file
- Supports capturing of minimized windows
- Grayscale, JPEG quality and Downscaling to reduce file size

## Self Compilation
These steps build the original BOF from `entry.cpp`; `get_screenshot.c` runs through Beacon Interpreter instead.

1. git clone the repo
2. run `make`

## Save methods:  
0. drop file to disk
1. download file over beacon (Cobalt Strike only)
2. download file over beacon as a screenshot (Cobalt Strike only)

## PID
0: capture full screen (PID = 0)
specific PID: capture specific PID (works even when minimized!)

## Usage
### Compiled BOF

1. import the `ScreenshotBOF/screenshotBOF.cna` script into Cobalt Strike
2. use the command screenshot_bof {local filename} {save method 0/1/2} {pid/0} {grayscale 0/1} {quality 0-100} {scale 0-100}
   - running `screenshot_bof` with no arguments will pop a GUI dialog
  
```
beacon> screenshot_bof file.jpg 2 21964 0 90 100
[*] Running screenshot BOF by (@codex_tf2)
[+] host called home, sent: 12421 bytes
[+] received output:
Downloading JPEG over beacon as a screenshot with filename file.jpg
[*] received screenshot of Screenshot from Admin (26kb)
[+] received output:
Screenshot saved/downloaded successfully
```

### Beacon Interpreter (`get_screenshot.c`)

Load [get_screenshot.c](get_screenshot.c) in [Beacon Interpreter's Script Editor](https://hstechdocs.helpsystems.com/manuals/cobaltstrike/current/userguide/content/topics/post-exploitation_beacon-interpreter.htm); no BOF compilation is needed. The script targets the default 64-bit Interpreter headers. For 32-bit targets, define `_WIN32` before the includes and validate separately.

Use the same arguments and order as the BOF above (`str` filename followed by five `int` values). All arguments are optional; omitted trailing values default to `screenshot.jpg 2 0 0 85 100`. Supply all preceding arguments when overriding a later value.

For example, run `get_screenshot.c` from the Script Editor with these typed argument rows:

|Type|Value|
| --- | --- |
|str|file.jpg|
|int|2|
|int|21964|
|int|0|
|int|90|
|int|100|

Example output (username and image size vary):

```text
[*] received screenshot of Screenshot from Admin (26kb)
[+] received output:
Returned JPEG through the screenshot callback
```

## Notes
- no evasion is performed, which should be fine since the WinAPIs used are not malicious

## Why did I make this?
Cobalt Strike uses a technique known as fork & run for many of its post-ex capabilities, including the screenshot command. While this behaviour provides stability, it is now well known and heavily monitored for. This BOF is meant to provide a more OPSEC safe version of the screenshot capability.

## Credits
- Save BMP to file from https://stackoverflow.com/a/60667564
- in memory download from https://github.com/anthemtotheego/CredBandit
- @BinaryFaultline for (deprecated) BMP rendering in aggressorscript, and screenshot callback function
- bitmap to jpeg from https://github.com/WKL-Sec/HiddenDesktop

## Disclaimer
usual disclaimer here, I am not responsible for any crimes against humanity you may commit or nuclear war you may cause using this piece of poorly written code.
