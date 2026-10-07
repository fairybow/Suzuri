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

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QToolButton>
#include <QWidget>

#include <Coco/Debug.h>
#include <Coco/Path.h>

#include "ui/widgets/GlyphButton.h"
#include "views/TextSearch.h"
#include "views/ViewConstants.h"

namespace Suzuri {

using namespace Qt::StringLiterals;

namespace Internal {

// One of the find bar's glyph buttons: previous, next, or close. A GlyphButton
// with a fixed glyph. It takes no focus, so a click leaves the caret in the
// search field
class FindBarButton_ : public Ui::GlyphButton
{
    Q_OBJECT

public:
    FindBarButton_(const Coco::Path& glyphPath, QWidget* parentFindBar)
        : GlyphButton(
              FIND_BAR_BUTTON_EXTENT,
              FIND_BAR_ICON_EXTENT,
              parentFindBar)
        , glyphPath_(glyphPath)
    {
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    [[nodiscard]] Coco::Path glyphPath() const override { return glyphPath_; }

    [[nodiscard]] QPalette::ColorRole glyphRole() const override
    {
        return FIND_BAR_ICON_ROLE;
    }

private:
    Coco::Path glyphPath_;
};

} // namespace Internal

// The row of controls a text view shows across its top while searching: the
// search field, how many matches there are and which one the search is on,
// previous and next, the two options, and close.
//
// It holds what the user typed and chose, and says when that changes or when a
// button is pressed. It searches nothing and knows nothing of the document:
// TextFileView owns the search and connects to these signals directly.
//
// In the field, Enter is next, Shift+Enter previous, and Esc close
class FindBar : public QWidget
{
    Q_OBJECT

public:
    explicit FindBar(QWidget* parentTextFileView)
        : QWidget(parentTextFileView)
    {
        setup_();
    }

    ~FindBar() override { TRACER; }

    [[nodiscard]] QString term() const { return term_->text(); }

    [[nodiscard]] TextSearch::Options options() const
    {
        return { matchCase_->isChecked(), wholeWord_->isChecked() };
    }

    // Announces searchChanged if the text differs from what is there
    void setTerm(const QString& term) { term_->setText(term); }

    // Put the caret in the search field with its text selected, so typing
    // replaces it
    void focusTerm()
    {
        term_->setFocus();
        term_->selectAll();
    }

    // Show where the search stands: "3 of 17" when it is on a match (current
    // counts from 0), "17 matches" when it is on none (current is -1), "No
    // matches" for a term that matched nothing, and nothing for an empty term
    void setCount(int current, int total)
    {
        QLocale locale{};

        if (term_->text().isEmpty()) {
            count_->clear();
        } else if (total == 0) {
            count_->setText(tr("No matches"));
        } else if (current < 0 && total == 1) {
            count_->setText(tr("1 match"));
        } else if (current < 0) {
            count_->setText(tr("%1 matches").arg(locale.toString(total)));
        } else {
            count_->setText(
                tr("%1 of %2")
                    .arg(locale.toString(current + 1), locale.toString(total)));
        }
    }

signals:
    // The term or an option changed
    void searchChanged();

    void nextRequested();
    void previousRequested();
    void closeRequested();

protected:
    // The field's keys. Taken here because QLineEdit reports Enter with no
    // word of Shift, and does nothing with Esc
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == term_ && event->type() == QEvent::KeyPress) {
            auto* key_event = static_cast<QKeyEvent*>(event);

            switch (key_event->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (key_event->modifiers() & Qt::ShiftModifier) {
                    emit previousRequested();
                } else {
                    emit nextRequested();
                }
                return true;

            case Qt::Key_Escape:
                emit closeRequested();
                return true;

            default:
                break;
            }
        }

        return QWidget::eventFilter(watched, event);
    }

private:
    QLineEdit* term_ = new QLineEdit(this);
    QLabel* count_ = new QLabel(this);
    Internal::FindBarButton_* previous_ =
        new Internal::FindBarButton_(u":/lucide/ChevronUp.svg"_s, this);
    Internal::FindBarButton_* next_ =
        new Internal::FindBarButton_(u":/lucide/ChevronDown.svg"_s, this);
    QToolButton* matchCase_ = new QToolButton(this);
    QToolButton* wholeWord_ = new QToolButton(this);
    Internal::FindBarButton_* close_ =
        new Internal::FindBarButton_(u":/lucide/X.svg"_s, this);

    void setup_()
    {
        term_->setPlaceholderText(tr("Find"));
        term_->setMinimumWidth(FIND_BAR_TERM_MIN_WIDTH);
        term_->setMaximumWidth(FIND_BAR_TERM_WIDTH);
        term_->setClearButtonEnabled(true);
        term_->installEventFilter(this);

        count_->setForegroundRole(FIND_BAR_COUNT_ROLE);

        previous_->setToolTip(tr("Previous match"));
        next_->setToolTip(tr("Next match"));
        close_->setToolTip(tr("Close"));

        setupOption_(matchCase_, tr("Match case"));
        setupOption_(wholeWord_, tr("Whole word"));

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(
            FIND_BAR_MARGIN,
            FIND_BAR_MARGIN,
            FIND_BAR_MARGIN,
            FIND_BAR_MARGIN);
        layout->setSpacing(FIND_BAR_SPACING);
        layout->addWidget(term_, 1);
        layout->addWidget(previous_);
        layout->addWidget(next_);
        layout->addWidget(matchCase_);
        layout->addWidget(wholeWord_);
        layout->addWidget(count_);
        layout->addStretch(1);
        layout->addWidget(close_);

        connect(term_, &QLineEdit::textChanged, this, [this] {
            emit searchChanged();
        });

        connect(previous_, &QAbstractButton::clicked, this, [this] {
            emit previousRequested();
        });

        connect(next_, &QAbstractButton::clicked, this, [this] {
            emit nextRequested();
        });

        connect(close_, &QAbstractButton::clicked, this, [this] {
            emit closeRequested();
        });
    }

    // A text button that stays down while its option is on
    void setupOption_(QToolButton* button, const QString& text)
    {
        button->setText(text);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);

        connect(button, &QToolButton::toggled, this, [this] {
            emit searchChanged();
        });
    }
};

} // namespace Suzuri
