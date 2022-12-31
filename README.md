# poptools
Reverse enginering and notes for the PC version of «_Prince of Persia: The Sands of Time_» and «_Prince of Persia: Warrior Within_».

# Launch without the _`PrinceOfPersia.exe`_ launcher

The _Warrior Within_ launcher creates two global named mutexes; `POP5Launcher` and `POP_Watchdog`, and then launches _`POP2.EXE -007`_, which is the actual game binary. _`PrinceOfPersia.exe`_ stays in the background and seems to monitor `POP_Watchdog`, for handling crashes and showing a window.

If the `POP5Launcher` mutex is not found it will error out with a «_You must start the game with PrinceOfPersia.exe_» message box and close. The `-007` parameter doesn't seem to matter.

You can easily fake this by creating your own named mutexes via PowerShell and leaving it open. The game will start normally:
```ps
New-Object System.Threading.Mutex($false, "POP5Launcher")
New-Object System.Threading.Mutex($false, "POP_Watchdog")
```

You can also avoid all this PowerShell stuff by replacing the `POP5Launcher` string with some other short-enough mutex name that is guaranteed to always exist on running Windows sessions, like `DBWinMutex`.


For the original _Sands of Time_ the Mutex name is `POP_Launcher`:
```ps
New-Object System.Threading.Mutex($false, "POP_Launcher")
```

# Unlocking the level-selector menu
The easiest way is to replace the `P_StartNewGame` string in `POP2.EXE` with `P_SpecialLoad`. Make sure to keep the zero-padding and add a `NULL` terminator at the end. _i.e._ Don't mess with the alignment.
