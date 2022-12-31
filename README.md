# poptools
Reverse enginering and notes for the PC version of Prince of Persia: Sands of Time and Warrior Within.


# Launch without the _`PrinceOfPersia.exe`_ launcher

The launcher creates two global named mutexes; `POP5Launcher` and `POP_Watchdog`, and then launches _`POP2.EXE -007`_, which is the actual game binary. _`PrinceOfPersia.exe`_ stays in the background and seems to monitor `POP_Watchdog`, for handling crashes and showing a window.

If the `POP5Launcher` mutex is not found it will error out with a «_You must start the game with PrinceOfPersia.exe_» message box and close. The `-007` parameter doesn't seem to matter.

You can easily fake this by creating your own named mutexes via PowerShell and leaving it open. The game will start normally:
```ps
$mtx = New-Object System.Threading.Mutex($false, "POP5Launcher")
$mtx = New-Object System.Threading.Mutex($false, "POP_Watchdog")
```

You can also avoid all this PowerShell stuff by replacing the `POP5Launcher` string with some other short-enough mutex name that is guaranteed to always exist on running Windows sessions, like `DBWinMutex`.
