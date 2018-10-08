![Kodi Logo](resources/banner_slim.png)

# Nintendo Switch build guide
This guide describes how to cross-compile Kodi for the Nintendo Switch.


## Table of Contents
1. **[Document conventions](#1-document-conventions)**
2. **[Install the Switch development tools](#2-install-the-switch-development-tools)**
3. **[Get the source code](#3-get-the-source-code)**
4. **[Build tools and dependencies](#4-build-tools-and-dependencies)**
5. **[Build binary add-ons](#5-build-binary-add-ons)**
6. **[Build Kodi](#6-build-kodi)**
7. **[Package](#7-package)**
8. **[Install](#8-install)**
9. **[Debugging Kodi](#9-debugging-kodi)**

## 1. Document conventions
This guide assumes you are using `terminal`, also known as `console`, `command-line` or simply `cli`. Commands need to be run at the terminal, one at a time and in the provided order.

This is a comment that provides context:
```
this is a command
this is another command
and yet another one
```

**Example:** Clone Kodi's current master branch:
```
git clone https://github.com/xbmc/xbmc kodi
```

Commands that contain strings enclosed in angle brackets denote something you need to change to suit your needs.
```
git clone -b <branch-name> https://github.com/xbmc/xbmc kodi
```

**Example:** Clone Kodi's current Krypton branch:
```
git clone -b Krypton https://github.com/xbmc/xbmc kodi
```

Several different strategies are used to draw your attention to certain pieces of information. In order of how critical the information is, these items are marked as a note, tip, or warning. For example:
 
**NOTE:** Linux is user friendly... It's just very particular about who its friends are.  
**TIP:** Algorithm is what developers call code they do not want to explain.  
**WARNING:** Developers don't change light bulbs. It's a hardware problem.

**[back to top](#table-of-contents)** | **[back to section top](#1-document-conventions)**

## 2. Install the Switch development tools
If pacman (the Arch Linux Package Manager) is not installed, you can install the devkitPro fork of pacman available from:

```
https://github.com/devkitPro/pacman/releases
```

This assumes you are using the devkitPro fork of pacman. If you are on Arch or pacman is installed natively, drop the `dkp-` prefixes below.

Install devkitPro tools and libraries for the Switch:

```
sudo dkp-pacman -Sy switch-dev
```

**[back to top](#table-of-contents)**

## 3. Get the source code
Change to your `home` directory:
```
cd $HOME
```

Clone Kodi's current master branch:
```
git clone https://github.com/xbmc/xbmc kodi
```

**[back to top](#table-of-contents)**

## 4. Build tools and dependencies
Prepare to configure build:
```
cd $HOME/kodi/tools/depends
./bootstrap
```

Import the environment variables installed with devkitPro so that Kodi can find the Switch development tools:

```
source /etc/profile.d/devkit-env.sh
```

Configure build for the Switch (WIP):
```
../configure-switch.sh
```

Build tools and dependencies:
```
make -j$(getconf _NPROCESSORS_ONLN)
```

**TIP:** By adding `-j<number>` to the make command, you can choose how many concurrent jobs will be used and expedite the build process. It is recommended to use `-j$(getconf _NPROCESSORS_ONLN)` to compile on all available processor cores. The build machine can also be configured to do this automatically by adding `export MAKEFLAGS="-j$(getconf _NPROCESSORS_ONLN)"` to your shell config (e.g. `~/.bashrc`).

**WARNING:** Look for the `Dependencies built successfully.` success message. If in doubt run a single threaded `make` command until the message appears. If the single make fails, clean the specific library by issuing `make -C target/<name_of_failed_lib> distclean` and run `make`again.


**[back to top](#table-of-contents)** | **[back to section top](#4-build-tools-and-dependencies)**

## 5. Build binary add-ons
You can find a complete list of available binary add-ons **[here](https://github.com/xbmc/repo-binary-addons)**.

Change to Kodi's source code directory:
```
cd $HOME/kodi
```

Build all add-ons:
```
make -j$(getconf _NPROCESSORS_ONLN) -C tools/depends/target/binary-addons
```

Build specific add-ons:
```
make -j$(getconf _NPROCESSORS_ONLN) -C tools/depends/target/binary-addons ADDONS="audioencoder.flac pvr.vdr.vnsi audiodecoder.snesapu"
```

Build a specific group of add-ons:
```
make -j$(getconf _NPROCESSORS_ONLN) -C tools/depends/target/binary-addons ADDONS="pvr.*"
```

**[back to top](#table-of-contents)** | **[back to section top](#5-build-binary-add-ons)**

## 6. Build Kodi
Configure CMake build:
```
cd $HOME/kodi
make -C tools/depends/target/cmakebuildsys
```

Build Kodi:
```
cd $HOME/kodi/build
make -j$(getconf _NPROCESSORS_ONLN)
```

**[back to top](#table-of-contents)**

## 7. Package
TODO

**[back to top](#table-of-contents)**

## 8. Install
TODO

**[back to top](#table-of-contents)**

## 9. Debugging Kodi
TODO

**[back to top](#table-of-contents)**
