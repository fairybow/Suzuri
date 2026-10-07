<p align="center">
    <img src="Suzuri/resources/icons/Suzuri-128.png" alt="Suzuri icon">
    <br>
    <b>A plain-text editor for creative writing</b>
</p>
<p align="center">
    <a href="LICENSE"><img src="https://img.shields.io/badge/GPL%203-red.svg?style=flat-square" alt="GPL 3 license"></a>
    <a href="https://qt.io/"><img src="https://img.shields.io/badge/Qt%206.11-brightgreen?style=flat-square" alt="Qt-6.11"></a>
    <a href="https://github.com/fairybow/Suzuri/actions/workflows/ci.yml"><img src="https://img.shields.io/github/actions/workflow/status/fairybow/Suzuri/ci.yml?branch=main&style=flat-square&label=CI" alt="CI status"></a>
    <br>
    <a href="https://github.com/fairybow/Suzuri/releases"><img src="https://img.shields.io/badge/Windows%20|%20macOS%20|%20Linux-grey.svg?style=flat-square" alt="Platforms: Windows, macOS, Linux"></a>
    <br>
    <a href="https://github.com/fairybow/Suzuri/releases"><b>Releases</b></a> •
    <a href="Suzuri/docs"><b>Documentation</b></a>
</p>
<p align="center">
    &nbsp
    <br>
    <img src="Suzuri/resources/social/SuzuriWindow.png" alt="Screenshot" width="640">
    <br>
    &nbsp
</p>

## About

I'm a writer, and Suzuri is a personal project: the writing app I've wanted for myself.

Its design is based on [Obsidian](https://obsidian.md/), where a vault is just a folder on your computer. But Suzuri is meant for novel writing, and everything you write stays in plain-text files you own and can open in any editor.

Suzuri is early in development, and more features are planned. I expect to keep working on it for a long time.

> [!WARNING]
> Suzuri hasn't reached 1.0 yet. Keep regular backups of your work.

*Suzuri is an independent project and isn't affiliated with or endorsed by Obsidian.*

## Features

- **A vault is a folder.** Open any folder as a vault. Suzuri adds one hidden folder for its settings and leaves the rest alone
- **No Save command.** Every file exists on disk from the moment it's created, and changes are written about a second after you stop typing
- **Your text is written back as it was read.** Line endings, byte order marks, no-break spaces, and other special characters are preserved
- **Tabs, split panes, and pop-out windows.** The same file can be open in any number of them, in sync, with one undo history
- **A Common Vault** shared by every project, for templates, notes, and reference material
- **A file tree and Go to File** (Ctrl+O), with drag-and-drop moves and delete-to-trash
- **Changes made elsewhere are picked up.** A file edited in another program, by Git, or by a sync client reloads in place, as one undo step
- **PDFs and images** open read-only beside your text
- **A word counter**, bundled fonts for prose, and settings kept per vault

Suzuri edits plain text. Markdown and Fountain files open as the text they are, with no rendering for now. There are no links, tags, or metadata, and find and replace and spellcheck aren't built yet.

The full list is in [Features.md](Suzuri/docs/Features.md), and what's planned is in [Future.md](Suzuri/docs/Future.md).

## Installing

Download the latest build for Windows, macOS, or Linux from [Releases](https://github.com/fairybow/Suzuri/releases). Each release's notes cover installing, updating, and uninstalling.

The builds are unsigned, so Windows and macOS will warn you the first time you run one. The macOS and Linux builds aren't well tested yet.

## Building

You'll need:

- CMake 3.21 or later
- A C++20 compiler: MSVC 2022, GCC 13, or Apple Clang
- Qt 6.7 or later, with the Qt PDF and Qt Image Formats modules. Suzuri is developed on Qt 6.11

Clone with submodules, since Suzuri's support library, [Coco](https://github.com/fairybow/Coco), is one:

```
git clone --recurse-submodules https://github.com/fairybow/Suzuri
cd Suzuri
```

Then configure, build, and run the tests. Point `CMAKE_PREFIX_PATH` at your Qt kit:

```
cmake -B build -S Suzuri -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.2/gcc_64
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

On Windows, with Visual Studio's generator, the configuration is chosen at build time:

```
cmake -B build -S Suzuri -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/msvc2022_64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

On Linux you'll also need the usual Qt build dependencies. On Ubuntu:

```
sudo apt-get install libxcb1-dev libx11-xcb-dev libxkbcommon-dev libgl1-mesa-dev
```

Pass `-DAPP_BUILD_TESTS=OFF` to skip building the tests.

## Support

I enjoy working on Suzuri, and I'm genuinely grateful for any support. You can do that [here](https://ko-fi.com/fairybow).

But if you're going to give anywhere, I'd rather you please give to [United24 🇺🇦](https://u24.gov.ua/) and help defend Ukraine. Civilians there are under attack every day, and your money will do far more good that way.
