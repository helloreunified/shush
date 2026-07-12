# Welcome!
`shush` is a terminal shell primarily oriented to portability and simple customization.<br>
Instead of having a shell with too much customizations you may not need, or maybe heavy, or it has questionable design,<br>
`shush` is the shell that is both minimal, portable and also lightweight, perfect for use across filesystems and systems!<br>

# Installation
The current version of `shush` sits at snapshot 6, get it in Releases section!<br>
If you wanted to build this program yourself, please do the following:<br>
> Dependencies needed: g++ and/or ar
1. Clone the repository or download the source in .zip
2. (Extract if needed) Go to the root level of the repository and run `./buildseq`
3. A binary will pop up/be overwritten in the source folder under the name "shush", that is the result!

If you wanted to download a specific commit of this program/repository, please do the following:<br>
1. Go to commits in its main branch.
2. Select your desired commit.
3. Click on "Browse source"
4. Download the source file in .zip and extract it.

If you wanted to download a specific snapshot of this program, please do the following:<br>
1. Go to the releases tab;
2. Download the attached source code under each snapshot.

If you wanted to download the precompiled binary at any snapshot, please do the following:<br>
1. Get that specific snapshot of this program using instructions above,
2. and then you now get the binary named "shush" directly, that is the result!

# Features
`shush` currently supports many features, including but not limited to:<br>
- Self-brought configuration (not here yet)<br>
- Syntax highlighting (not here yet)<br>
- Prompt customization (too basic, not fully here yet)<br>
- File redirection<br>
- And more...! (kind of, will update this list later)<br>

# NOT features
`shush` does NOT support the following functions:<br>
- scripts<br>

# Milestones;
- snapshot 1, 2, 3: basic prototype<br>
- snapshot 4: line editing update<br>
> 4.0: change line exiting engine to `https://github.com/antirez/linenoise`<br>
> 4.1: support for running external programs without dependency and command history<br>
> 4.2: partial prompt customization<br>
- snapshot 5: file descriptor update<br>
- snapshot 6: line editing update <br>
> 6.0: change line editing engine to `https://github.com/AmokHuginnsson/replxx`<br>
> 6.1: syntax helper<br>
> 6.2: tab completion<br>
> 6.3: history functions<br>
- snapshot 7: customization plus interactive outputs<br>
> 7.0: universal variables and globbing<br>
> 7.1: prompting and prompt generator<br>
> 7.2: settings changer built-in<br>
> 7.3: abrupt abbreviations and character width<br>
> 7.4: keybinds
> 7.5: pager 
> 7.6: partial developer integeration (e.g. git and venv)
> 7.7. jobs control
> 7.8: more interactive and useful `help` function
- snapshot 8: finalizing
> 8.0: package its config into its own binary, added some toggles
> 8.1: add support for self-brought string configs
> 8.11: code refactoring and quick bug fixes

# Notes
Potentially cancerous code.<br>

# Licenses
The core engine and code of this software are licensed under the MIT License. It is provided completely "AS IS", without warranty of any kind. The author assumes zero liability for its use or any operational damages. Any superseded versions of this shell are also licensed under the MIT license.
This software is strictly prohibited for corporate or enterprise use by default for any versions of this software that are "snapshot 6.14 and up", due to the inclusion of third-party, non-commercial fan references. However, permission for enterprise use is granted if and only if all third-party referenced assets, names, configurations, and media not originally created by the primary maintainer are completely stripped out and removed from the codebase.<br>
Built-in configuration terms, theme names, or references (specifically; "mizuki" as in "Akiyama Mizuki", "miku" as in "Hatsune Miku") are inspired by properties owned by *Colorful Palette / SEGA* and *Crypton Future Media*, respectively. These are included purely for non-commercial, fan-made hobby customization under standard fair-use and derivative work guidelines. Any commercial exploitation of these terms or assets by an enterprise is a violation of the respective rights holders' terms and is entirely the user's legal responsibility.<br>
Forks of this software are permitted, provided they retain the original copyright notice and this limitation of liability disclaimer.<br>
