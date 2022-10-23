# ScreenshotBOF

An alternative screenshot capability for Cobalt Strike that uses WinAPI and does not perform a fork & run. Screenshot saved to disk as a file.

## Usage
1. import the screenshotBOF.cna script into Cobalt Strike
2. use the command screenshot_bof
3. Download the screenshot from the target e.g.
```
download screenshot.bmp
```

## Notes
- no evasion is performed, which should be fine since the WinAPIs used are not malicious
- in memory downloading of screenshots is planned to be added
- the filename can be changed in the source code.

## Why did I make this?
Cobalt Strike uses a technique known as fork & run for many of its post-ex capabilities, including the screenshot command.  
While this behaviour provides stability, it is now well known and heavily monitored for. This BOF is meant to provide a more  
OPSEC safe version of the screenshot capability.