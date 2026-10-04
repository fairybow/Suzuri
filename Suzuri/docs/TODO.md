# TODO

Bugs and code chores. Features that aren't built yet are in [Future.md](Future.md); conventions are in [CodeStyle.md](CodeStyle.md).

## Bugs

- [ ] When a tab drag is dropped, activate and focus the tab's widget
- [ ] VaultSwitcher's menu should open upward
- [ ] Focus the current tab's widget on restore (something probably keeps the focus from working, because it's surely already called)
- [ ] After restore, the first click on a tab doesn't focus its widget. Later clicks do
- [ ] "Go to file" on a new tab in a pop-out used to activate the main window and never return focus to the pop-out. Likely fixed now that the switcher opens over the page's own window: verify
- [ ] Pop-out Z-order isn't restored: a pop-out that was in front of its vault window comes back behind it. Vault windows restore in the right order among themselves, so this may be pop-outs only, and may not be fixable (which would be fine)
- [ ] Opening the file that is already in the active, unpinned tab should do nothing
- [ ] Minor: dragging a file quickly from near its row's bottom edge moves the selection band (grey, blue left edge) down to the next row, though that row isn't selected
- [ ] The "Options" heading in the settings dialog can be hovered or selected and probably shouldn't be

## Rough edges

- [ ] Tabs can be dragged past the left or right end of the tab bar without starting a drag; may want to clamp them
- [ ] No auto-scroll while dragging a tab in an overflowing bar (close to impossible with Qt)

## Before release

- [x] `LogView` opens on every launch, for testing. Put it behind its command-line flag
- [x] `CHANGELOG.md` has only a placeholder release entry. Write the real notes before tagging
- [ ] Test the OS logout save path (`App::onCommitDataRequest_`)

## Code

- [ ] Verify the `PrimeDocument` header, and rename it
- [ ] Rename `BaseWindow`'s protected members that have a trailing underscore
- [ ] Bring `MenuBuilder` over from Hearth (maybe into Coco)?
- [x] Decide on `private slots:`. It isn't used consistently, and using it properly would break up sections that are organized by purpose. Either remove the keyword everywhere or sort slots into it everywhere
- [ ] `AbstractFileModel` has `notifyX()` functions that only emit a signal. Could they just be signals (perhaps named `notify...`)?
- [ ] Decide whether static helpers go before other functions, per access level
- [ ] A shared `TabPage` base for tab pages, holding title and pin state as typed members in place of the window title and a dynamic property. It would trade the leaf's property reads for a cast. Worth doing once a third such value appears

## Audits

Passes over the whole codebase against [CodeStyle.md](CodeStyle.md).

- [ ] Signals passed up parent chains
- [ ] Signals that only ever drive one slot in the same class
- [ ] Consistent terms: what is a pane, a leaf, a view, a page
- [ ] Naming and casing of types, functions, and members, including trailing underscores on private members
- [ ] Parent parameters not named for their parent type
- [ ] Widgets that start without a parent and are inserted into one later: name the parameter to say so
- [ ] Constructor work outside `setup_`
- [ ] `const`, `[[nodiscard]]`, `[[maybe_unused]]`
- [ ] `QDir` where a `Coco::Path` function would do
- [ ] Paths held or passed as `QString`
- [ ] Unneeded `Coco::Path(...)` conversions
- [ ] Includes not using the full path from `src/`
- [ ] Blank lines inside namespace braces
- [ ] Public functions nothing outside the class calls
- [ ] Order of sections within classes
- [ ] Helper classes used in one header that could move to its `Internal` namespace (`ZoomButton` in `ZoomControl.h`, for example)
- [ ] `TODO` comments in the source