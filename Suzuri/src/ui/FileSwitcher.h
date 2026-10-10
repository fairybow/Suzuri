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

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QObject>
#include <QPainter>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QtTypes>

#include <Coco/Debug.h>
#include <Coco/Path.h>

#include "core/FileRef.h"
#include "core/Vault.h"
#include "ui/OpenMode.h"
#include "ui/UiConstants.h"
#include "ui/widgets/RowBadges.h"

namespace Suzuri::Ui {

namespace Internal {

// Paints a Go to File row with its labels ("RECENT", "COMMON") at the right
// end, each with a border so two side by side read as two. The labels ride on
// the item as a QStringList under BADGES_ROLE, in the order they're drawn left
// to right; a row with none is the style's alone
class FileSwitcherDelegate_ : public QStyledItemDelegate
{
public:
    static constexpr auto BADGES_ROLE = Qt::UserRole;

    explicit FileSwitcherDelegate_(QObject* parentList)
        : QStyledItemDelegate(parentList)
    {
    }

    void paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        auto badges = index.data(BADGES_ROLE).toStringList();

        if (badges.isEmpty()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        auto style = opt.widget ? opt.widget->style() : QApplication::style();
        auto badge_font = RowBadges::font(opt.font);
        auto badge_rects = RowBadges::rects(opt.rect, badges, badge_font);

        opt.text =
            RowBadges::nameElidedBefore(opt, style, badge_rects[0].left());
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        for (qsizetype i = 0; i < badges.size(); ++i) {
            RowBadges::paint(
                painter,
                opt,
                badges[i],
                badge_font,
                badge_rects[i],
                true);
        }
    }
};

} // namespace Internal

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
// An empty query lists the window's recent files first (RecentFiles), newest
// first, then every other file in the presorted order below. When the newest
// is the file already in the active tab, the second row starts selected, so
// Enter goes back to the file before it. Obsidian's quick switcher also opens
// on the recent files.
//
// Matching: case-insensitive; the query splits on spaces and every term must
// appear somewhere in the vault-relative path. Rows whose FILE NAME holds every
// term rank above path-only matches; within a tier, project files come before
// Common Vault files, then alphabetical by path. Recent files get no extra
// weight. Divergences from Obsidian (docs/Future.md, "Go to File"): no fuzzy
// scoring, and Enter with no match does nothing rather than creating a note.
//
// A recent file's row says RECENT at its right end, wherever it appears, and a
// Common Vault file's says COMMON (Internal::FileSwitcherDelegate_). A recent
// Common Vault file has both, RECENT against the edge.
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
    // if it's the same vault. recentFiles are the window's, newest first, and
    // current the file in its active tab (none for a new tab)
    FileSwitcher(
        QWidget* parentWindow,
        Vault* vault,
        Vault* commonVault,
        const QList<FileRef>& recentFiles,
        const FileRef& current)
        : QDialog(parentWindow)
    {
        setup_(vault, commonVault, recentFiles, current);
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

    // The recent files that are listed, as entries_ indices, newest first;
    // the same as a set, for the RECENT label; and the row an empty query
    // starts on (see class note)
    QList<qsizetype> recent_{};
    QSet<qsizetype> recentSet_{};
    int emptyQueryRow_ = 0;

    QLineEdit* queryEdit_ = nullptr;
    QListWidget* list_ = nullptr;
    Pick pick_{};

    void setup_(
        Vault* vault,
        Vault* commonVault,
        const QList<FileRef>& recentFiles,
        const FileRef& current)
    {
        setWindowTitle(tr("Go to File"));
        setWindowModality(Qt::ApplicationModal);

        collect_(vault, false);
        if (commonVault && commonVault != vault) {
            collect_(commonVault, true);
        }

        presort_();
        findRecent_(recentFiles, current);
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

    // Each recent file that is in the listing, in the recent order. One that
    // isn't (deleted or moved since) is skipped
    void findRecent_(const QList<FileRef>& recentFiles, const FileRef& current)
    {
        for (const auto& ref : recentFiles) {
            for (qsizetype i = 0; i < entries_.size(); ++i) {
                if (entries_[i].fileRef == ref) {
                    recent_ << i;
                    recentSet_ << i;
                    break;
                }
            }
        }

        if (recent_.size() > 1 &&
            entries_[recent_.first()].fileRef == current) {
            emptyQueryRow_ = 1;
        }
    }

    void setupUi_()
    {
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
        list_->setItemDelegate(new Internal::FileSwitcherDelegate_(list_));

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

        if (terms.isEmpty()) {
            shown_ = recent_;

            for (qsizetype i = 0; i < entries_.size(); ++i) {
                if (!recentSet_.contains(i)) {
                    shown_ << i;
                }
            }

            rebuildList_(emptyQueryRow_);
            return;
        }

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

        rebuildList_(0);
    }

    // currentRow is the row to select, when there is one
    void rebuildList_(int currentRow)
    {
        list_->setUpdatesEnabled(false);
        list_->clear();

        for (auto i : shown_) {
            const auto& entry = entries_[i];

            auto* item = new QListWidgetItem(entry.path, list_);

            QStringList badges{};

            if (entry.isCommon) {
                badges << tr("COMMON");
                item->setToolTip(tr("Common Vault"));
            }

            if (recentSet_.contains(i)) {
                badges << tr("RECENT");
            }

            item->setData(Internal::FileSwitcherDelegate_::BADGES_ROLE, badges);
        }

        list_->setCurrentRow(shown_.isEmpty() ? -1 : currentRow);
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
