# Changelog

[Skip to release content](#releases)

---

# Boilerplate

(See [build_release_text.py](https://github.com/fairybow/Suzuri/blob/main/.github/scripts/build_release_text.py))

<!-- release-preamble-start -->

**Suzuri is a plain-text editor for creative writing.**

This is a soft release. For a full feature list, see [Features.md](https://github.com/fairybow/Suzuri/blob/main/Suzuri/docs/Features.md). For past release details, see [CHANGELOG.md](https://github.com/fairybow/Suzuri/blob/main/CHANGELOG.md)

> [!WARNING]
> You should not trust your writing with any version of this software less than 1.0.0! Regardless, always make regular backups of your work.

<!-- release-preamble-end -->

<!-- release-footer-start -->

## Installation

**Windows:** Download and run the installer below.

**macOS:** Download the `.dmg`, open it, and drag Suzuri to Applications.

**Linux:** Download the `.AppImage`, make it executable (`chmod +x`), and run.

> [!NOTE]
> **Windows:** This build is unsigned, so Windows Defender SmartScreen will likely show a warning. Click **More info -> Run anyway** to proceed.
>
> **macOS:** This build is unsigned. You may need to right-click and select **Open** the first time, or allow it in **System Settings -> Privacy & Security**.

## Updating

**Windows:** Download the newest installer, ensure Suzuri is closed, then run the installer and install to the same directory (default `C:/Program Files`), overwriting.

**macOS:** Replace the app in Applications with the new version from the `.dmg`.

**Linux:** Replace the old `.AppImage` with the new one.

## Uninstalling

**Windows:** Run the uninstaller (`unins000.exe`) or remove via **Add or Remove Programs** as usual.

**macOS:** Drag Suzuri from Applications to the Trash.

**Linux:** Delete the `.AppImage`.

Uninstalling leaves these folders in place. The first and last hold only settings and are safe to remove. `Documents/Suzuri/` holds your writing: delete it only if you mean to.

| Folder                 | Location                                                                                                               | Contents                         |
| ---------------------- | ---------------------------------------------------------------------------------------------------------------------- | -------------------------------- |
| App data               | `AppData/Local/Suzuri` (Windows), `~/Library/Application Support/Suzuri` (macOS), `~/.local/share/Suzuri` (Linux) | Known vaults, session, logs, spelling dictionaries |
| Default vault location | `Documents/Suzuri/`                                                                                                    | Your vaults and the Common Vault |
| Per-vault settings     | `.suzuri/` inside each vault                                                                                           | Settings, layout, and the vault's dictionary |

## Platforms

Windows (x64), macOS (ARM), and Linux (x86_64).

> [!NOTE]
> macOS and Linux builds are available but not well-tested. Bug reports are welcome!

:heart:

<!-- release-footer-end -->

---

# Notes

- For release note links: use `blob/main` for evergreen links; use `blob/<tag>` for links to a specific release's snapshot!
- For diffing against previous release: `https://github.com/fairybow/Suzuri/compare/<tag>...main.diff`
- Release commands:

```
git tag v0.0.0-beta.0 [must match tag part of entry title (see below)]
git push origin v0.0.0-beta.0
```

---

# Release Entry Template

```
# 0.0.0-beta.0 (Testing / Soft Release) - tag v0.0.0-beta.0

## What's New?

- ...

## Known Issues

- ...
```

---

<a id="releases"></a>

# 0.2.0-beta.0 (Spellcheck) - tag v0.2.0-beta.0

## What's New?

Suzuri now has spellcheck.

- Misspelled words get a red wavy underline. The word you're typing isn't marked until you move on from it
- **Right-click** a marked word for up to five suggestions. Choosing one replaces the word, and one undo puts it back. Suggestions keep your apostrophe style, straight or curly
- **Add to dictionary** saves the word to the vault's own dictionary, `.suzuri/dictionary.txt`, which travels with the vault. Words added to the Common Vault's dictionary are accepted in every vault
- **Ignore** accepts a word until the vault is closed
- Added words are accepted in their possessive and hyphenated forms too: with "Pangloss" added, so are "Pangloss's" and "Pangloss-like" accepted
- US English is included. For another language, put a Hunspell dictionary (the `.aff` and `.dic` files LibreOffice uses) in Suzuri's dictionaries folder. **Settings → Editor → Spelling** has a button that opens it
- Checking can be turned off, and the language chosen, separately for each vault

Each vault checks one language at a time right now.

Also in this release:

- **Fixed:** the text editor's right-click menu opened to the left of the pointer when line numbers or a margin were on

## Known Issues

- Web addresses and file paths are checked piece by piece, so parts like "github" and "com" are marked
- Suggestions can take a moment to appear for a long or badly garbled word
- Focus doesn't always land in the editor after dragging a tab or restoring a session
- Pop-out windows can reopen behind their vault window
- macOS and Linux builds are not well-tested

---

# 0.1.0-beta.0 (Find and Replace) - tag v0.1.0-beta.0

## What's New?

Suzuri can now find and replace text in a file.

- **Ctrl+F** opens a find bar across the top of the text. Matches are highlighted as you type, and the bar shows which one you're on and how many there are
- **Ctrl+H** opens it with a second row for replacing. The arrow at the left of the bar shows or hides that row
- Enter or F3 goes to the next match, Shift+Enter or Shift+F3 to the previous
- **Match case** and **Whole word** narrow the search. Whole word leaves longer words alone, so replacing "fart" won't touch "farthing"
- **Replace all** changes every match at once, and one undo takes it all back
- Each tab has its own find bar, so two panes on the same file can search for different things

The search is for plain text in one file. Patterns and searching a whole vault aren't built yet.

Also in this release:

- The README now has a screenshot, a feature list, and build instructions

## Known Issues

- Focus doesn't always land in the editor after dragging a tab or restoring a session
- Pop-out windows can reopen behind their vault window
- macOS and Linux builds are not well-tested

---

# 0.0.0-beta.1 (Soft Release) - tag v0.0.0-beta.1

## What's New?

A small release of fixes and safeguards.

- **Fixed:** on Windows, vaults, folders, and files with names outside the system's code page (emoji, and some accented or non-Latin letters) weren't handled correctly. Paths are now Unicode all the way through
- A vault now refuses any path that would step outside its folder, such as one containing `..`. Nothing in the app could produce one, but a hand-edited or damaged `workspace.json` could, and Suzuri would have opened and saved a file outside the vault
- If the views of a file open in more than one tab ever disagree about its text, Suzuri now notices and resets the view to the text that is being saved. No cause for this is known. It's a safety net
- Suzuri now has automated tests for its file handling, run on Linux and macOS for every change

## Known Issues

- Focus doesn't always land in the editor after dragging a tab or restoring a session
- Pop-out windows can reopen behind their vault window
- macOS and Linux builds are not well-tested

---

# 0.0.0-beta.0 (Initial Release) - tag v0.0.0-beta.0

## What's New?

This is Suzuri's first public release.

Suzuri is a plain-text writing app for long-form fiction. Its design follows Obsidian: a vault is just a folder on your computer, and everything you write stays in ordinary files you own.

It's a hobby project, the writing app I've wanted for myself, and I hope to keep working on it for a long time. This release is the starting point: vaults, a file tree, tabs and split panes, pop-out windows, and autosave.

## Known Issues

- Focus doesn't always land in the editor after dragging a tab or restoring a session
- Pop-out windows can reopen behind their vault window
- macOS and Linux builds are not well-tested
