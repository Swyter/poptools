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

# Natively launch the games in windowed mode

The three games use your Profile.DAT to store the video settings, resolution and fullscreen mode. You can use the *010 Editor* template and script in this repository to edit the file yourself, change the field and recompute the checksum or just use these handy premade profiles.

* *Sands of Time*, drop it in your `Profiles/Name` folder: https://cdn.discordapp.com/attachments/760854217509830676/1060513753734598686/Profile.DAT
* *Warrior Within*, drop it in your *POPWWProfiles/Name* folder: https://cdn.discordapp.com/attachments/760854217509830676/1060513840418263120/Profile.DAT
* *The Two Thrones* drop it in your *POP3Profiles/Name* folder: https://cdn.discordapp.com/attachments/760854217509830676/1060524464015675402/Profile.DAT

## Show the close, minimize and maximize buttons in windowed mode

Open your `POP.exe`, `POP2.exe` or `POP3.exe` in some hex editor, depending on the game. Search for the `05 00 00 c0 00` byte pattern, replace that `c0` with `cb`, and save.

This changes the original window style that gets supplied to `CreateWindowEx()` to include the `WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX` bits/window-styles.

*i.e.* Goes from `0xc00000` to `0xcb0000`, the constant appears reversed due to little-endian.

# Unlocking the level-selector menu
The easiest way is to replace the `P_StartNewGame` string in `POP2.EXE` with `P_SpecialLoad`. Make sure to keep the zero-padding and add a `NULL` terminator at the end. _i.e._ Don't mess with the alignment.

# Unlocking the hidden game options
Swapping the names of the `P_Options` and `P_GameOptions` pages in `POP5Common.MGM` makes the game skip the blacklist that hides the secret `Setup menu ON/OFF`, `Cheats ON/OFF`, `Interface ON/OFF`, `Show motion camera ON/OFF`, `Interface ON/OFF` and `Work in progress ON/OFF` entries under _Options > Game_.

One of the (still unidentified) options causes the _Tab_ and _C_ keys to pause the game, another one takes raw .BMP screenshots. These debug options seem to be stored as part of the profile data.

# Unlocking the hidden advanced video options
Replace `P_SoundsMenu` with `P_VideoAdvanced` by hex-editing the string in the game's binary without affecting the other string's positions and ensuring it ends with `00`; a `NULL` terminator.

It needs to be called from the _Options_ menu itself, does not seem to work from the main menu. _Options > Sounds_ will lead to it.
