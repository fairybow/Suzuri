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

#include <algorithm>

#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPalette>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include <Coco/Debug.h>
#include <Coco/Path.h>

#include "core/FileRef.h"
#include "core/Vault.h"
#include "ui/OpenMode.h"
#include "ui/UiConstants.h"
#include "ui/widgets/Glyph.h"

namespace Suzuri::Ui {

// Go to File: an Obsidian-style quick switcher — a filter field over a flat
// list of every file the window's two vaults show. The listing is
// Vault::visibleFiles, so it agrees with the sidebar trees by construction (no
// dot-entries, nothing unsupported), and every row carries the FileRef of the
// vault that listed it: nothing outside a vault can be picked, so no caller
// ever resolves an absolute path.
//
// The list is a snapshot taken at construction. A file deleted before the pick
// is refused by Vault::openModel, which is the authoritative gate either way.
//
// Matching: case-insensitive; the query splits on spaces and every term must
// appear somewhere in the vault-relative path. Rows whose FILE NAME holds every
// term rank above path-only matches; within a tier, project files come before
// Common Vault files, then alphabetical by path. An empty query therefore lists
// everything in that presorted order. Divergences from Obsidian
// (docs/Future.md, "Go to File"): no fuzzy scoring, no recents for the empty
// query, and Enter with no match does nothing rather than creating a note.
//
// Keys: focus never leaves the field. Up/Down/PageUp/PageDown are forwarded to
// the list; Enter opens the current row as ReplaceActive, Ctrl+Enter as NewTab
// (Cmd+Enter on macOS — Qt maps Cmd to ControlModifier); Esc cancels (the line
// edit ignores it, so QDialog rejects). A click opens the same way, honoring a
// held Ctrl
class FileSwitcher : public QDialog
{
    Q_OBJECT

public:
    struct Pick
    {
        FileRef fileRef{};
        OpenMode mode{};
    };

    // parentWindow is the window the switcher opens over — the one hosting the
    // request, which may be a pop-out. commonVault may be null, and is skipped
    // if it's the same vault
    FileSwitcher(QWidget* parentWindow, Vault* vault, Vault* commonVault)
        : QDialog(parentWindow)
    {
        setup_(vault, commonVault);
    }

    ~FileSwitcher() override { TRACER; }

    // Only meaningful after exec() returns Accepted
    [[nodiscard]] Pick pick() const { return pick_; }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched != queryEdit_ || event->type() != QEvent::KeyPress) {
            return QDialog::eventFilter(watched, event);
        }

        auto key_event = static_cast<QKeyEvent*>(event);

        switch (key_event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            // The list moves its current row on these whether or not it has
            // focus — it only needs the event
            QCoreApplication::sendEvent(list_, event);
            return true;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            acceptRow_(list_->currentRow(), modeFor_(key_event->modifiers()));
            return true;

        default:
            return QDialog::eventFilter(watched, event);
        }
    }

private:
    // One listable file. path and name are precomputed display/match strings,
    // so a keystroke's refilter does no path work
    struct Entry_
    {
        FileRef fileRef{};
        QString path{}; // vault-relative, forward slashes
        QString name{};
        bool isCommon = false;
    };

    QList<Entry_> entries_{};  // presorted: project first, then by path
    QList<qsizetype> shown_{}; // entries_ indices, in list-row order
    QLineEdit* queryEdit_ = nullptr;
    QListWidget* list_ = nullptr;
    QIcon commonIcon_{};
    QIcon blankIcon_{};
    Pick pick_{};

    void setup_(Vault* vault, Vault* commonVault)
    {
        setWindowTitle(tr("Go to File"));
        setWindowModality(Qt::ApplicationModal);

        collect_(vault, false);
        if (commonVault && commonVault != vault) {
            collect_(commonVault, true);
        }

        presort_();
        setupUi_();
        placeOverParent_();
        refilter_();
    }

    void collect_(Vault* vault, bool isCommon)
    {
        for (const auto& relative : vault->visibleFiles()) {
            entries_ << Entry_{
                .fileRef = { vault, relative },
                .path = relative.prettyQString(),
                .name = relative.nameQString(),
                .isCommon = isCommon,
            };
        }
    }

    // Once, at construction. refilter_ then only partitions by tier, and a
    // stable partition keeps this order inside each tier
    void presort_()
    {
        std::sort(
            entries_.begin(),
            entries_.end(),
            [](const Entry_& a, const Entry_& b) {
                if (a.isCommon != b.isCommon) {
                    return !a.isCommon;
                }

                return QString::compare(a.path, b.path, Qt::CaseInsensitive) <
                       0;
            });
    }

    void setupUi_()
    {
        setupIcons_();

        queryEdit_ = new QLineEdit(this);
        queryEdit_->setPlaceholderText(tr("Type to find a file..."));
        queryEdit_->installEventFilter(this);

        connect(
            queryEdit_,
            &QLineEdit::textChanged,
            this,
            &FileSwitcher::refilter_);

        list_ = new QListWidget(this);
        list_->setFocusPolicy(Qt::NoFocus); // typing always lands in the field
        list_->setUniformItemSizes(true);   // cheap layout for long listings
        list_->setIconSize(
            { FILE_SWITCHER_ICON_EXTENT, FILE_SWITCHER_ICON_EXTENT });

        connect(
            list_,
            &QListWidget::itemClicked,
            this,
            [this](QListWidgetItem* item) {
                acceptRow_(
                    list_->row(item),
                    modeFor_(QGuiApplication::keyboardModifiers()));
            });

        auto root_layout = new QVBoxLayout(this);
        root_layout->addWidget(queryEdit_);
        root_layout->addWidget(list_);

        queryEdit_->setFocus();
    }

    // Common rows carry the tinted glyph. Project rows get a transparent icon
    // of the same size: the item delegate only reserves decoration space for a
    // row that HAS an icon, so without it project paths would sit flush left
    // and common paths indented
    void setupIcons_()
    {
        auto extent = FILE_SWITCHER_ICON_EXTENT;

        commonIcon_ = QIcon(
            Glyph::render(
                QString::fromLatin1(COMMON_VAULT_ICON_PATH),
                extent,
                palette().color(FILE_SWITCHER_COMMON_VAULT_ICON_ROLE),
                devicePixelRatioF()));

        QPixmap blank(extent, extent);
        blank.fill(Qt::transparent);
        blankIcon_ = QIcon(blank);
    }

    // Top-center over the host window, Obsidian-style, instead of QDialog's
    // default centering. Moving before show sets WA_Moved, which is what stops
    // QDialog::setVisible from re-centering it
    void placeOverParent_()
    {
        auto* host = parentWidget();
        if (!host) {
            return;
        }

        auto geometry = host->window()->geometry();
        auto width = qMin(FILE_SWITCHER_WIDTH, geometry.width());

        resize(width, FILE_SWITCHER_HEIGHT);
        move(
            geometry.center().x() - width / 2,
            geometry.top() + FILE_SWITCHER_TOP_OFFSET);
    }

    void refilter_()
    {
        auto terms = queryEdit_->text().split(u' ', Qt::SkipEmptyParts);

        shown_.clear();

        for (qsizetype i = 0; i < entries_.size(); ++i) {
            if (containsAll_(entries_[i].path, terms)) {
                shown_ << i;
            }
        }

        // Name matches ahead of path-only matches. Stable, so the presort
        // (project before common, then path) holds inside each tier. With no
        // terms every entry is a name match and the order is the presort
        std::stable_partition(shown_.begin(), shown_.end(), [&](qsizetype i) {
            return containsAll_(entries_[i].name, terms);
        });

        rebuildList_();
    }

    void rebuildList_()
    {
        list_->setUpdatesEnabled(false);
        list_->clear();

        for (auto i : shown_) {
            const auto& entry = entries_[i];

            auto* item = new QListWidgetItem(
                entry.isCommon ? commonIcon_ : blankIcon_,
                entry.path,
                list_);

            if (entry.isCommon) {
                item->setToolTip(tr("Common Vault"));
            }
        }

        list_->setCurrentRow(shown_.isEmpty() ? -1 : 0);
        list_->setUpdatesEnabled(true);
    }

    // No row (an empty listing, or no match) does nothing and keeps the
    // switcher open. Creating a file from here isn't built
    void acceptRow_(int row, OpenMode mode)
    {
        if (row < 0 || row >= shown_.size()) {
            return;
        }

        pick_ = { entries_[shown_[row]].fileRef, mode };
        accept();
    }

    [[nodiscard]] static bool
    containsAll_(const QString& haystack, const QStringList& terms)
    {
        return std::all_of(
            terms.begin(),
            terms.end(),
            [&](const QString& term) {
                return haystack.contains(term, Qt::CaseInsensitive);
            });
    }

    [[nodiscard]] static OpenMode modeFor_(Qt::KeyboardModifiers modifiers)
    {
        return (modifiers & Qt::ControlModifier) ? OpenMode::NewTab
                                                 : OpenMode::ReplaceActive;
    }
};

} // namespace Suzuri::Ui
