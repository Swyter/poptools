# poptools
Reverse enginering and notes for the PC version of «_Prince of Persia: The Sands of Time_» and «_Prince of Persia: Warrior Within_».

# Launch without the _`PrinceOfPersia.exe`_ launcher

The _Warrior Within_ launcher creates two global named mutexes; `POP5Launcher` and `POP_Watchdog`, and then launches _`POP2.EXE -007`_, which is the actual game binary. _`PrinceOfPersia.exe`_ stays in the background and seems to monitor `POP_Watchdog`, for handling crashes and showing a window.

If the `POP5Launcher` mutex is not found it will error out with a «_You must start the game with PrinceOfPersia.exe_» message box and close. The `-007` parameter doesn't seem to matter.

You can easily fake this by creating your own named mutexes via PowerShell and leaving it open. The game will start normally:
```ps
# PrinceOfPersia.EXE -> POP2.EXE -007
New-Object System.Threading.Mutex($false, "POP5Launcher")
New-Object System.Threading.Mutex($false, "POP_Watchdog")
./POP2.EXE
```

You can also avoid all this PowerShell stuff by replacing/hex-editing the `POP5Launcher` string in the game binary with some other short-enough mutex name that is guaranteed to always exist, like the `POP_Game` one the game creates just before to check that only a single instance is running at a time.


For the original _Sands of Time_ the launcher is called with the `-uplay_steam_mode` parameter, and the mutex name is `POP_Launcher`:
```ps
# PrinceOfPersia.EXE -uplay_steam_mode -> POP.EXE -007
New-Object System.Threading.Mutex($false, "POP_Launcher")
./POP.EXE
```

For _The Two Thrones_ the mutex is called `POP3Launcher`, back from the `5` of _Warrior Within_, funny:
```ps
# PrinceOfPersia.EXE -uplay_steam_mode -> POP3.EXE -007
New-Object System.Threading.Mutex($false, "POP3Launcher")
./POP3.EXE
```

# Unlocking the level-selector menu
The easiest way is to replace the `P_StartNewGame` string in `POP2.EXE` with `P_SpecialLoad`. Make sure to keep the zero-padding and add a `NULL` terminator at the end. _i.e._ Don't mess with the alignment.

# Unlocking the hidden game options
Swapping the names of the `P_Options` and `P_GameOptions` pages in `POP5Common.MGM` makes the game skip the blacklist that hides the secret `Setup menu ON/OFF`, `Cheats ON/OFF`, `Interface ON/OFF`, `Show motion camera ON/OFF`, `Interface ON/OFF` and `Work in progress ON/OFF` entries under _Options > Game_.

One of the (still unidentified) options causes the _Tab_ and _C_ keys to pause the game. These debug options seem to be stored as part of the profile data.
