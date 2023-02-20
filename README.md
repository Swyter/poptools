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

The three games use your `Profile.DAT` to store the video settings, resolution and fullscreen mode. You can use the *010 Editor* template and script in this repository to edit the file yourself, change the field and recompute the checksum... or just use these handy premade profiles:

* *Sands of Time*, drop it in your `<game-dir>/Profiles/<name>/` folder:
  *  https://cdn.discordapp.com/attachments/760854217509830676/1060513753734598686/Profile.DAT
* *Warrior Within*, drop it in your `<game-dir>/POPWWProfiles/<name>/` folder:
  * https://cdn.discordapp.com/attachments/760854217509830676/1060513840418263120/Profile.DAT
* *The Two Thrones* drop it in your `<game-dir>/POP3Profiles/<name>/` folder:
  * https://cdn.discordapp.com/attachments/760854217509830676/1060524464015675402/Profile.DAT

Make a copy of the original one, you will lose your existing options (remapped keys included) and the progress for your unlocked extras.

It will take another restart for the game to start windowed from the beginning; as on start-up the game loads the video and sound settings from `DefaultProfile.DAT`. When you exit the game, the settings for that last profile overwrite the default one.

## Show the close, minimize and maximize buttons in windowed mode

Open your `POP.exe`, `POP2.exe` or `POP3.exe` in some hex editor, depending on the game. Search for the `05 00 00 c0 00` byte pattern, replace that `c0` with `cb`, and save.

This changes the original window style that gets supplied to `CreateWindowEx()` to include the `WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX` bits/window-styles.

*i.e.* Goes from `0xc00000` to `0xcb0000`, the constant appears reversed due to the x86 CPU architecture being little-endian.

# Unlocking the level-selector menu
The easiest way is to replace the `P_StartNewGame` string in `POP2.EXE` with `P_SpecialLoad`. Make sure to keep the zero-padding and add a `NULL` terminator at the end. _i.e._ Don't mess with the alignment.

# Unlocking the hidden game options
Swapping the names of the `P_Options` and `P_GameOptions` pages in `POP5Common.MGM` makes the game skip the blacklist that hides the secret `Setup menu ON/OFF`, `Cheats ON/OFF`, `Interface ON/OFF`, `Show motion camera ON/OFF`, `Interface ON/OFF` and `Work in progress ON/OFF` entries under _Options > Game_.

One of the (still unidentified) options causes the _Tab_ and _C_ keys to pause the game, another one takes raw .BMP screenshots. These debug options seem to be stored as part of the profile data.

# Unlocking the hidden advanced video options
Replace `P_SoundsMenu` with `P_VideoAdvanced` by hex-editing the string in the game's binary without affecting the other string's positions and ensuring it ends with `00`; a `NULL` terminator.

It needs to be called from the _Options_ menu itself, does not seem to work from the main menu. _Options > Sounds_ will lead to it.


# Internal names
Here is a list of the project names that seem to be used internally for each of these games:
| Year | Game         | Codename | Game engine | Frameworks + Middleware | Audio engine | Team |
|------|--------------|----------|-------------|-------------------------|--------------|------|
| 2003 | Prince of Persia: The Sands of Time                   | POP4        | Jade, BIG file `v37` |                                | DARE | Montréal |
| 2004 | Prince of Persia: Warrior Within                      | POP5        | Jade, BIG file `v38` |                                | DARE | Montréal |
| 2005 | Prince of Persia: The Two Thrones                     | POP3        | Jade, BIG file `v38` | GEAR                           | DARE | Montréal |
| 2008 | Prince of Persia (*2008*)                             | POP0, POP09 | Scimitar             | Havok, GEAR                    | DARE | Montréal |
| 2010 | Prince of Persia: The Forgotten Sands                 | POPTM       | Scimitar             | Havok, GEAR                    | DARE | Montréal |
| 2010 | Prince of Persia: The Forgotten Sands (*Wii variant*) | Djinn       | Jade, BIG file `v42` | Scaleform GFx, GEAR, zlib, lzo | DARE |   Québec |

## Libraries internal to Ubisoft's Technology Group:
 * `GEAR`: Generic engine C++ abstraction layer/framework; various special allocators, async, data containers, intrinsics, byte-swapping, platform-specific, input, logging, math, memory, C++ STL, I/O, stream, string, plus threading primitives like rwlock, atomics, and so on.
 * `DARE`: Sound engine; seems to be split into `dare`, `dare_ext`, and `dare_plus`. Streaming, playback, 3D spatialization, signal filtering and audio effects. The system seems to be built around 3D objects in a map, with callbacks for interacting with the main game.

## Glossary
* `BIG`: Big file subsystem; files with the `.bf` extension that store most game assets.
* `LOA`: Loading subsystem, to retrieve BF archive data from the FAT descriptor.
* `GAO`: Game object, they retain their names once exported.
* `MDF`: Modified vertex data at runtime, an effect like the wavy water displacement, wind, look-at rotator or folliage-moved-by-player that can be applied each frame to a mesh instance to transform it.
# Fun facts

* The PS2 version of *Warrior Within* includes a full standalone demo ISO disguised as padding files. On the NTSC version `PRINCE01.BF`, `PRINCE02.BF`, `PRINCE03.BF`, `PRINCE04.BF`, `PRINCE05.BF` under the `DATA` folder all hold multiple copies of the same PS2 disc image, on the PAL version the file is called both `DUMMY.PAD` and `DUMMY2.PAD`. It works out of the box with PCSX2. The SHA1 is `dbdec007953d67f480a6f5d2c91df01dd233cad9`.
