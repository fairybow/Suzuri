/*
 * Suzuri — A plain-text editor for creative writing
 * Copyright (C) 2026 fairybow
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#pragma once

#include <QWidget>

#include <Coco/Debug.h>

#include "core/Vault.h"
#include "core/VaultConfig.h"
#include "ui/settings/SettingsPage.h"
#include "ui/widgets/DisplaySlider.h"

namespace Suzuri::Ui {

// The settings dialog's Editor page, Obsidian's Editor tab: how the text
// editor behaves and what it shows besides the text. The column, and how a
// page talks to its Vault, are SettingsPage's.
//
// The two sections are Obsidian's. Display is what the editor shows: line
// numbers sit in it there too ("Show line numbers"), and the left/right margin
// stands where Obsidian has "Readable line length". Behavior is how it
// responds: the tab width is Obsidian's "Indent visual width", which it keeps
// under Behavior as well. The rest — wrapping, the current-line highlight,
// selection handles, center on scroll, double-click whitespace — are Suzuri's
// own, which Obsidian doesn't have.
class SettingsEditorPage : public SettingsPage
{
    Q_OBJECT

public:
    SettingsEditorPage(Vault* vault, QWidget* parentSettingsDialog)
        : SettingsPage(vault, parentSettingsDialog)
    {
        setup_();
    }

    ~SettingsEditorPage() override { TRACER; }

private:
    void setup_()
    {
        setupDisplaySection_();
        setupBehaviorSection_();
    }

    void setupDisplaySection_()
    {
        const auto& config = vault()->config();

        addHeading(tr("Display"));

        addSwitch(
            tr("Show line numbers"),
            tr("Show line numbers in the gutter."),
            config.lineNumbers(),
            &Vault::setLineNumbers);

        addSwitch(
            tr("Wrap lines"),
            tr("Wrap long lines to the editor's width. Turn this off to "
               "scroll sideways instead."),
            config.wrapLines(),
            &Vault::setWrapLines);

        // Applies live while dragging; the Vault writes once it settles
        auto* margin = new DisplaySlider(
            VaultConfig::MIN_LEFT_RIGHT_MARGIN,
            VaultConfig::MAX_LEFT_RIGHT_MARGIN,
            config.leftRightMargin(),
            content());
        connect(margin, &DisplaySlider::valueChanged, this, [this](int value) {
            vault()->setLeftRightMargin(value);
        });
        addRow(
            tr("Left/right margin"),
            tr("Space kept clear on each side of the text, as a percentage "
               "of the editor's width."),
            margin);

        addSwitch(
            tr("Highlight current line"),
            tr("Show a band behind the line the text cursor is on."),
            config.lineHighlight(),
            &Vault::setLineHighlight);

        addSwitch(
            tr("Selection handles"),
            tr("Show a handle under each end of a selection. Drag one to "
               "move that end."),
            config.selectionHandles(),
            &Vault::setSelectionHandles);
    }

    void setupBehaviorSection_()
    {
        const auto& config = vault()->config();

        addHeading(tr("Behavior"));

        auto* tab_width = new DisplaySlider(
            VaultConfig::MIN_TAB_WIDTH,
            VaultConfig::MAX_TAB_WIDTH,
            config.tabWidth(),
            content());
        connect(
            tab_width,
            &DisplaySlider::valueChanged,
            this,
            [this](int value) { vault()->setTabWidth(value); });
        addRow(
            tr("Tab width"),
            tr("Number of spaces a tab character will render as."),
            tab_width);

        addSwitch(
            tr("Center on scroll"),
            tr("Keep the text cursor in the middle of the editor when it "
               "would scroll out of view, and let the text scroll past its "
               "end."),
            config.centerOnScroll(),
            &Vault::setCenterOnScroll);

        addSwitch(
            tr("Double-click selects whitespace"),
            tr("Double-click a run of spaces or tabs to select all of it."),
            config.doubleClickWhitespace(),
            &Vault::setDoubleClickWhitespace);
    }
};

} // namespace Suzuri::Ui
