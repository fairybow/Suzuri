# Features

Current version: v0.0.0-beta.0

What Suzuri does today. For what is planned, undecided, or deliberately left out, see [Future.md](Future.md). For how it is built, see [Architecture.md](Architecture.md).

## Vaults

A vault is a folder on your computer. Suzuri adds one hidden folder to it, `.suzuri/`, and otherwise leaves it alone: your files stay ordinary files that any other program can open.

- A vault can live anywhere. Opening a folder as a vault is all it takes to make one
- Several vaults can be open at once, each in its own window
- Quitting and relaunching reopens every vault that was open, with its windows, tabs, and layout
- Vaults may be nested inside one another, and may sit inside a Git repository
- One Suzuri process holds every open vault

### The Common Vault

One vault is shared by every project: the Common Vault, for templates, notes, and reference material you want within reach whatever you are working on.

- Lives at `Documents/Suzuri/Common Vault`, and is created for you
- Every vault window shows it in a collapsible drawer under the vault's own file tree
- A Common Vault file open in two windows is one document: an edit in one appears in the other as you type
- Tabs holding a Common Vault file are marked with a box icon
- Can be opened as a window of its own, from the vault switcher
- If its folder is deleted, Suzuri recreates it empty

### Manage Vaults

The vault picker. It opens on first launch, whenever there is no vault to reopen, and from the vault switcher or the File menu.

- Create new vault (a name and a location)
- Open folder as vault
- Recent vaults, most recently used first. A vault whose folder is gone is shown as missing
- Right-click a vault to rename it, reveal it in the file explorer, or remove it from the list. Removing never touches the folder
- A vault that is open can't be renamed or removed until its window is closed. The Common Vault can't be renamed at all

### Vault switcher

At the bottom of the sidebar. Lists your other vaults, opens the Common Vault, and opens Manage Vaults. Choosing a vault that is already open brings its window forward.

## Saving

There is no Save command and no unsaved state.

- Every file exists on disk from the moment it is created. There are no untitled, in-memory documents
- Changes are written about a second after you stop typing, and at least every three seconds while you keep typing
- Everything is also written when Suzuri loses focus, when a window closes, on quit, and on logout
- If a file can't be saved, the window refuses to close and names the files, so nothing is lost by closing
- Text is written back exactly as it was read, apart from your edits: line endings (LF or CRLF), a byte order mark, no-break spaces, and other special characters are preserved. A new file uses LF

## Changes made outside Suzuri

Files are expected to be edited elsewhere too: in another editor, by Git, by a sync client.

- A file changed outside Suzuri reloads in place. The reload is one undo step, so Ctrl+Z brings back what you had
- A file deleted, moved, or renamed outside Suzuri has its tabs closed, in every window
- Files and folders created, renamed, or removed outside Suzuri appear in the file trees on their own
- If an open vault's folder is deleted or moved, Suzuri tells you and closes that vault's window when you return to it. It never recreates the folder
- Many files changing at once (a Git checkout, a sync) are handled as one pass

## Files

### Supported types

| Kind | Extensions |
|---|---|
| Text | `.txt`, `.md`, `.markdown`, `.fountain` |
| PDF | `.pdf` |
| Image | `.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, `.tif`, `.tiff`, `.webp` |

Other files are hidden from the file trees and from Go to File, and can't be opened. So are files and folders whose names begin with a dot.

Text files are read as UTF-8. A text file that isn't valid UTF-8 (an old Windows-1252 manuscript, say) brings up a warning before anything changes, with a preview of the affected lines. Open Anyway converts it to UTF-8 on the spot, with the unreadable characters replaced by �; Cancel leaves the file untouched.

### File tree

- Folders first, then names in natural order (`Chapter 2` before `Chapter 10`)
- File names are shown without their extension. Every type but `.txt` shows its extension at the right edge of the row (`MD`, `PDF`, `FOUNTAIN`)
- Expanded folders are remembered per vault
- Double-click a file to open it
- Right-click a file: Open in new tab, Open to the right, Open in new window, Rename, Delete
- Right-click a folder: New file, New folder, Rename, Delete
- Right-click empty space: New file, New folder (at the vault root)
- Drag a file or folder onto a folder to move it there, or onto empty space to move it to the vault root. Folders expand when hovered and the tree scrolls while dragging
- Drag a file onto an editor pane to open it there
- Delete always asks first, and moves the entry to the system trash. If the trash refuses it (a network share, for instance), nothing is deleted
- Names are checked as you type for characters and names the operating system forbids

### Go to File

Ctrl+O, or "Go to file" on a new tab. A search field over every file in the vault and the Common Vault.

- Type any words, in any order. Each must appear somewhere in the file's path
- Matches in the file name rank above matches in the folder path
- Enter opens the file in the current tab. Ctrl+Enter opens it in a new tab
- Common Vault files are marked with the box icon

## Tabs, panes, and windows

- Opening a file replaces what is in the current tab, unless that tab is pinned
- A new tab opens empty, offering to create a file or go to one
- The same file can be open in any number of tabs, panes, and windows. All of them stay in sync and share one undo history
- Right-click a tab to close, pin, or unpin it. A pinned tab keeps its file when another is opened and shows a pin in place of its close button
- Drag a tab along the bar to reorder it, onto another pane's bar to move it there, onto the edge of a pane to split that pane, or anywhere else to pop it out into its own window
- Panes split side by side and stacked, to any depth. A pane closes when its last tab does
- Pop-out windows have panes and tabs of their own. They belong to their vault's main window and close with it, or when their last tab closes
- Window positions and sizes, the pane layout, tabs, pins, and each tab's cursor, scroll, and zoom are saved per vault and restored on the next launch

## Editor

Plain text. Suzuri does not render Markdown or Fountain; those files are edited as the text they are.

- Undo and redo
- Line numbers (off by default)
- Line wrapping (on by default)
- Left/right margin, as a percentage of the editor's width
- Tab width, in spaces
- Center on scroll: keeps the cursor mid-view and lets the text scroll past its end
- Current line highlight
- Double-click a run of spaces or tabs to select the whole run
- Selection handles: a draggable handle under each end of a selection
- Font family, size, bold, and italic for the text. Courier Prime, Literata (the default), mononoki, and OpenDyslexic are bundled

## PDFs and images

Read-only views.

- Zoom with Ctrl+wheel, Ctrl+= and Ctrl+-, or the control floating over the view. Ctrl+0 resets to fit
- PDFs: all pages in one scrolling view, fitted to width when first opened
- Images: fitted to the pane, and never enlarged past their real size by fitting. When zoomed in, drag to pan. Ctrl+wheel zooms toward the pointer
- Animated GIF and WebP images play
- Photos are shown the right way up (EXIF orientation is applied)
- A file that can't be read shows a message in place of a blank view

## Status bar

Shown in vault windows and pop-outs, for the active text tab. Empty over a PDF, an image, or a new tab.

- Word counter: words and characters, and optionally lines. With text selected it reads "12 of 1,234 words"
- Cursor position: line and column (off by default)
- Words: "don't", "well-known", "e.g.", and "1,000" each count as one. Dashes, slashes, and markup symbols are not words. Chinese and Japanese text counts one word per character
- Characters count what you see: an emoji or an accented letter is one. Spaces are included, line breaks are not
- Lines are paragraphs, not wrapped rows

## Settings

Ctrl+, or the gear beside the vault switcher. Settings belong to the vault and apply as you change them.

- **Editor**: line numbers, wrap lines, left/right margin, highlight current line, selection handles, tab width, center on scroll, double-click selects whitespace
- **Appearance**: text font, bold, italic, font size
- **Status bar**: the word counter and the cursor position, and which parts of each to show

A Common Vault file opened in a project's window uses that project's settings.

## What Suzuri keeps in a vault

| File | Holds | Meant to be committed or synced? |
|---|---|---|
| `.suzuri/settings.json` | Editor and status bar settings | Yes |
| `.suzuri/appearance.json` | Font settings | Yes |
| `.suzuri/workspace.json` | Window, pane, and tab layout | No, it is specific to one machine |
| `.suzuri/.gitignore` | Ignores `workspace.json` | Yes |

Deleting `.suzuri/` while the vault is closed resets its settings and layout and nothing else.

The list of known vaults is kept outside any vault, in Suzuri's application data folder.

## Keyboard shortcuts

| Action | Shortcut |
|---|---|
| New file | Ctrl+N |
| New folder | Ctrl+Shift+N |
| Go to file | Ctrl+O |
| Settings | Ctrl+, |
| Undo / Redo | Ctrl+Z / Ctrl+Y |
| Zoom in / out / reset | Ctrl+= / Ctrl+- / Ctrl+0 |
| Show or hide the menu bar | Ctrl+M |
| Quit | Ctrl+Q |

The menu bar is hidden until asked for. Every shortcut works without it.

## Differences from Obsidian

Suzuri follows Obsidian's behavior wherever it has no reason not to. Where it differs:

- **Plain text first.** `.txt` is the default type (Obsidian doesn't open it). There is no Markdown rendering, and no links, backlinks, tags, or metadata
- **Line endings are preserved.** Obsidian rewrites them to LF
- **A failed save refuses the window close.** Obsidian shows a notice and closes
- **A dropped file opens.** A file dragged onto an editor opens there; in Obsidian it inserts a link
- **Bold and italic font settings**, since there is no Markdown styling to carry emphasis
- **A margin setting** in place of "Readable line length", and a switch to turn wrapping off
- **TIFF images** are supported. SVG and AVIF are not
- **Images zoom**, like PDFs
- **File and folder icons** in the file tree
- **No fallback trash.** If the system trash refuses an entry, Suzuri declines; Obsidian moves it to a `.trash` folder in the vault
- **A vault whose folder vanishes has its window closed**, after a message, where Obsidian leaves an empty window open
- **No in-app vault moving.** Move the folder in your file manager, then open it from its new place

## Known limits

- A file name with a period and no real extension (`Chapter 1. The Start`) is read as having an unsupported extension, so the file is hidden
- On Windows, while Suzuri is open, other programs can't rename or delete a folder if one of its subfolders has been expanded in a file tree or holds an open file. Suzuri's own rename, move, and delete are unaffected
- A moved or renamed vault folder is a new vault to Suzuri: open it again from its new place. Its settings and layout travel with it
- Word counts run low for Thai, Lao, Khmer, and Myanmar text
- Right-to-left layouts are not supported
- macOS and Linux builds exist but are not well tested
