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

#include <QChar>
#include <QEvent>
#include <QFont>
#include <QFontMetricsF>
#include <QList>
#include <QMargins>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QResizeEvent>
#include <QString>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFormat>
#include <QTextLayout>
#include <QTextLine>
#include <QWidget>
#include <QtMath>
#include <QtMinMax>

#include <Coco/Debug.h>

#include "views/SelectionHandles.h"
#include "views/TextSearch.h"
#include "views/ViewConstants.h"

namespace Suzuri {

class TextEditor;

namespace Internal {

// The strip the line numbers are drawn in: a child of the editor, sitting
// in the left viewport margin the editor reserves for it. It only forwards
// its paint event — the editor does the drawing, since the block geometry
// it needs is protected on QPlainTextEdit. Defined below TextEditor, which
// it has to see whole
class LineNumberArea_ : public QWidget
{
public:
    explicit LineNumberArea_(TextEditor* parentTextEditor);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    TextEditor* editor_;
};

} // namespace Internal

// The text editor widget TextFileView holds: a QPlainTextEdit with an optional
// line-number gutter, a left/right text margin, and a tab width counted in
// spaces. A subclass because the gutter needs QPlainTextEdit's protected block
// geometry (firstVisibleBlock, blockBoundingGeometry, contentOffset) and
// setViewportMargins — the shape of Qt's Code Editor example.
//
// Each setting is a setter that returns early when nothing changed, so
// TextFileView::applyConfig can hand over the whole config on any change.
//
// Numbers are hard lines (QTextBlocks), one per paragraph, drawn beside the
// paragraph's first visual row — the same unit as the status bar's line count
// and "Ln".
//
// Where this departs from the example:
// - Block tops are accumulated as qreal, as QPlainTextEdit's own paint does,
//   and rounded nowhere. Rounding each block's height drifts the numbers off
//   their lines down the viewport when a line's height is fractional
// - Each number is drawn on its block's first-line baseline rather than into
//   a box as tall as the font, so it stays aligned if the gutter's font ever
//   differs from the text's
// - Digits are tabular ("tnum"), so the gutter's width is right for a
//   proportional face and the numbers line up in columns
// - The width is recomputed on FontChange explicitly, not left to a
//   full-viewport updateRequest happening to follow one
// - The gutter is never narrower than LINE_NUMBER_MIN_DIGITS: each time it
//   widens, the viewport narrows and the whole document rewraps
// - No band, no separator: muted numbers on the text area's own background
//
// The left/right margin is a percentage of the editor's width kept clear on
// each side of the text. It is viewport margin, like the gutter: the gutter
// stays at the editor's left edge and the margin sits between it and the text.
// The strips are the editor's own surface, not the viewport's, so the editor
// fills itself with the viewport's background; a click in one doesn't reach the
// text.
//
// The tab width is a count of spaces in the editor's font, so it follows the
// font.
//
// The current-line highlight is a band across the row the text cursor is on:
// one visual row of a wrapped paragraph, not the whole paragraph. It is an
// extra selection, as in Qt's example, in a palette role (LINE_HIGHLIGHT_ROLE).
// Shown whether or not the editor has focus.
//
// Double-click whitespace: a double-click on a run of whitespace selects the
// run (Qt's own double-click selects a word, and does nothing useful on
// spaces). Only the clicked paragraph's text is read, and the character tested
// is the one under the mouse, not the one after the nearest cursor position.
//
// Selection handles (views/SelectionHandles.h): two draggable teardrops under
// the ends of a selection. The editor's part is to paint them after the text
// and to offer them each mouse event before handling it itself
class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget* parentTextFileView)
        : QPlainTextEdit(parentTextFileView)
    {
        setup_();
    }

    ~TextEditor() override { TRACER; }

    [[nodiscard]] bool lineNumbers() const noexcept { return lineNumbers_; }

    void setLineNumbers(bool shown)
    {
        if (shown == lineNumbers_) {
            return;
        }

        lineNumbers_ = shown;
        lineNumberArea_->setVisible(shown);
        updateViewportMargins_();
    }

    // On: wrap to the editor's width (at a word boundary, or anywhere in a
    // word wider than the view — QPlainTextEdit's default word wrap mode,
    // left as it is). Off: no wrapping, and a horizontal scrollbar. The base
    // setter returns early when unchanged
    void setWrapLines(bool wrapped)
    {
        setLineWrapMode(
            wrapped ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        updateExtraSelections_();
    }

    // Percent of the editor's width kept clear on each side of the text
    [[nodiscard]] int leftRightMargin() const noexcept
    {
        return leftRightMargin_;
    }

    void setLeftRightMargin(int percent)
    {
        if (percent == leftRightMargin_) {
            return;
        }

        leftRightMargin_ = percent;
        updateViewportMargins_();
    }

    // How wide a tab is, in spaces of the editor's font
    [[nodiscard]] int tabWidth() const noexcept { return tabWidth_; }

    void setTabWidth(int spaces)
    {
        if (spaces == tabWidth_) {
            return;
        }

        tabWidth_ = spaces;
        updateTabStopDistance_();
    }

    [[nodiscard]] bool lineHighlight() const noexcept { return lineHighlight_; }

    void setLineHighlight(bool shown)
    {
        if (shown == lineHighlight_) {
            return;
        }

        lineHighlight_ = shown;
        updateExtraSelections_();
    }

    // What a search found, to tint behind the text. An empty list clears the
    // tint. The list is in document order, as TextSearch::findAll gives it
    void setSearchMatches(const QList<TextSearch::Match>& matches)
    {
        searchMatches_ = matches;
        updateExtraSelections_();
    }

    [[nodiscard]] bool doubleClickWhitespace() const noexcept
    {
        return doubleClickWhitespace_;
    }

    // Read at the next double-click; there is nothing to update now
    void setDoubleClickWhitespace(bool enabled)
    {
        doubleClickWhitespace_ = enabled;
    }

    [[nodiscard]] bool selectionHandles() const noexcept
    {
        return selectionHandles_.isEnabled();
    }

    // Returns early when unchanged, as the others do
    void setSelectionHandles(bool shown)
    {
        selectionHandles_.setEnabled(shown);
    }

protected:
    // The base applies a new font to the document; the gutter's width and the
    // tab width are measured in that font, so they follow, and so do the
    // lines in view. The extra selections' colors are read from the palette
    // when they are built, so a new palette rebuilds them
    void changeEvent(QEvent* event) override
    {
        QPlainTextEdit::changeEvent(event);

        if (event->type() == QEvent::FontChange) {
            updateViewportMargins_();
            updateTabStopDistance_();
            updateExtraSelections_();
        } else if (event->type() == QEvent::PaletteChange) {
            updateExtraSelections_();
        }
    }

    // The selection handles are drawn over the text, so after it. This is
    // the viewport's paint event (QAbstractScrollArea hands those here), and
    // the base's painter is finished by the time it returns
    void paintEvent(QPaintEvent* event) override
    {
        QPlainTextEdit::paintEvent(event);
        selectionHandles_.paint();
    }

    // The selection handles get each mouse event first, and one they take
    // goes no further: a press on a handle must not also place the caret
    void mousePressEvent(QMouseEvent* event) override
    {
        if (selectionHandles_.mousePress(event)) {
            event->accept();
            return;
        }

        QPlainTextEdit::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (selectionHandles_.mouseMove(event)) {
            event->accept();
            return;
        }

        QPlainTextEdit::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (selectionHandles_.mouseRelease(event)) {
            event->accept();
            return;
        }

        QPlainTextEdit::mouseReleaseEvent(event);
    }

    // A double-click on a whitespace run selects it; anything else is Qt's
    // (word selection). The base never sees a click handled here, so it
    // doesn't start its own word-by-word drag or count toward a triple-click
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (doubleClickWhitespace_ && event->button() == Qt::LeftButton &&
            selectWhitespaceRunAt_(event->position().toPoint())) {
            event->accept();
            return;
        }

        QPlainTextEdit::mouseDoubleClickEvent(event);
    }

    // QAbstractScrollArea hands its viewport's resize events here, and the
    // viewport resizes whenever the editor does. The text margin is a share
    // of the editor's width, so it is recomputed first, for the base to lay
    // the document out at the width that results; the gutter is as tall as
    // the viewport, so it follows too. Setting new margins resizes the
    // viewport again and re-enters this once, where the margins are already
    // right and nothing more is set
    void resizeEvent(QResizeEvent* event) override
    {
        updateViewportMargins_();
        QPlainTextEdit::resizeEvent(event);

        // A new size puts different lines in view
        if (!searchMatches_.isEmpty()) {
            updateExtraSelections_();
        }
    }

private:
    friend class Internal::LineNumberArea_;

    Internal::LineNumberArea_* lineNumberArea_ = nullptr;
    bool lineNumbers_ = false;
    int leftRightMargin_ = 0;

    // 0 until first set (TextFileView::applyConfig), which leaves Qt's own
    // tab stop distance in place
    int tabWidth_ = 0;

    bool lineHighlight_ = false;
    QList<TextSearch::Match> searchMatches_{};
    bool doubleClickWhitespace_ = false;
    SelectionHandles selectionHandles_{ this };

    void setup_()
    {
        // The margins around the viewport are this widget's surface. Fill it
        // as the viewport fills itself, so the text margin reads as part of
        // the text area
        setBackgroundRole(viewport()->backgroundRole());
        setAutoFillBackground(true);

        lineNumberArea_ = new Internal::LineNumberArea_(this);
        lineNumberArea_->hide();

        // A tenth, hundredth, thousandth line needs another digit
        connect(
            this,
            &QPlainTextEdit::blockCountChanged,
            this,
            &TextEditor::updateViewportMargins_);

        connect(
            this,
            &QPlainTextEdit::updateRequest,
            this,
            &TextEditor::onUpdateRequest_);

        connect(
            this,
            &QPlainTextEdit::cursorPositionChanged,
            this,
            &TextEditor::onCursorPositionChanged_);
    }

    // The editor's font with tabular numerals. Built on demand rather than
    // set on the gutter: a font set on a widget stops following its parent's,
    // and the gutter should always track the editor's
    [[nodiscard]] QFont lineNumberFont_() const
    {
        auto number_font = font();
        number_font.setFeature(QFont::Tag("tnum"), 1);
        return number_font;
    }

    // Padding, then room for the last line's number (or MIN_DIGITS, if that
    // is wider). 0 while line numbers are off, which removes the margin.
    // Measured as a string, not a QChar: only shaped text has the font's
    // features applied
    [[nodiscard]] int lineNumberAreaWidth_() const
    {
        if (!lineNumbers_) {
            return 0;
        }

        auto digits = 1;

        for (auto count = qMax(1, blockCount()); count >= 10; count /= 10) {
            ++digits;
        }

        digits = qMax(digits, LINE_NUMBER_MIN_DIGITS);

        QFontMetricsF metrics(lineNumberFont_());
        auto numbers_width =
            metrics.horizontalAdvance(QString(digits, QChar(u'9')));

        return LINE_NUMBER_LEFT_PADDING + qCeil(numbers_width) +
               LINE_NUMBER_RIGHT_PADDING;
    }

    // The text margin on each side, in pixels: its share of the editor's
    // whole width (not the viewport's, which the margin itself narrows)
    [[nodiscard]] int textMargin_() const
    {
        return width() * leftRightMargin_ / 100;
    }

    // The one place the viewport's margins are set: the gutter's width plus
    // the text margin on the left, the text margin on the right. Set only
    // when they differ, since setting them lays the viewport out again
    // (see resizeEvent)
    void updateViewportMargins_()
    {
        auto text_margin = textMargin_();
        QMargins margins(
            lineNumberAreaWidth_() + text_margin,
            0,
            text_margin,
            0);

        if (margins != viewportMargins()) {
            setViewportMargins(margins);
        }

        layoutLineNumberArea_();
    }

    // tabWidth_ spaces, measured in the editor's font
    void updateTabStopDistance_()
    {
        if (tabWidth_ <= 0) {
            return;
        }

        QFontMetricsF metrics(font());
        setTabStopDistance(tabWidth_ * metrics.horizontalAdvance(QChar(u' ')));
    }

    // The editor's extra selections: the current-line band, then the tint
    // behind each search match in view, so a match on the current line shows
    // over the band.
    //
    // The band is a selection-less copy of the text cursor flagged full-width,
    // which QPlainTextEdit paints across the whole row that position is on.
    // The copy doesn't follow the text cursor, so this is rebuilt each time
    // the cursor moves (onCursorPositionChanged_).
    //
    // Only the matches in view are given a selection, and this is rebuilt when
    // the view moves. A common word in a long file has thousands of matches,
    // and setting that many selections takes tens of milliseconds each time
    void updateExtraSelections_()
    {
        QList<QTextEdit::ExtraSelection> selections{};

        if (lineHighlight_) {
            QTextEdit::ExtraSelection band{};
            band.format.setBackground(palette().brush(LINE_HIGHLIGHT_ROLE));
            band.format.setProperty(QTextFormat::FullWidthSelection, true);
            band.cursor = textCursor();
            band.cursor.clearSelection();
            selections << band;
        }

        if (!searchMatches_.isEmpty()) {
            appendSearchSelections_(selections);
        }

        setExtraSelections(selections);
    }

    // One selection for each match that starts in a paragraph in view. The
    // matches are in order, so the first is found by bisection and the walk
    // stops at the first one past the last paragraph
    void appendSearchSelections_(QList<QTextEdit::ExtraSelection>& selections)
    {
        auto first_position = firstVisibleBlock().position();

        auto last_block =
            cursorForPosition(viewport()->rect().bottomLeft()).block();
        auto end_position = last_block.position() + last_block.length();

        auto tint = palette().color(SEARCH_MATCH_ROLE);
        tint.setAlpha(SEARCH_MATCH_ALPHA);

        auto it = std::lower_bound(
            searchMatches_.cbegin(),
            searchMatches_.cend(),
            first_position,
            [](const TextSearch::Match& match, int position) {
                return match.position < position;
            });

        for (; it != searchMatches_.cend() && it->position < end_position;
             ++it) {
            QTextEdit::ExtraSelection selection{};
            selection.format.setBackground(tint);
            selection.cursor = QTextCursor(document());
            selection.cursor.setPosition(it->position);
            selection.cursor.setPosition(
                it->position + it->length,
                QTextCursor::KeepAnchor);
            selections << selection;
        }
    }

    // Off, there is nothing to move, and nothing is rebuilt
    void onCursorPositionChanged_()
    {
        if (lineHighlight_) {
            updateExtraSelections_();
        }
    }

    // Select the run of whitespace under a viewport position, and report
    // whether there was one to select. A single ordinary space doesn't count:
    // that is the gap between two words, and is left to Qt.
    //
    // cursorForPosition gives the nearest position BETWEEN characters, so the
    // character under the mouse is the one after it when the mouse is at or
    // right of the caret there, and the one before it otherwise. (Left-to-
    // right text only, like the gutter.) A run never crosses a line break, so
    // the paragraph's own text is all that's read; a click past the end of
    // the text lands on no character
    bool selectWhitespaceRunAt_(const QPoint& viewportPosition)
    {
        auto cursor = cursorForPosition(viewportPosition);
        auto block = cursor.block();
        auto text = block.text();

        auto index = static_cast<qsizetype>(cursor.positionInBlock());
        if (viewportPosition.x() < cursorRect(cursor).left()) {
            --index;
        }

        if (index < 0 || index >= text.size() || !text.at(index).isSpace()) {
            return false;
        }

        auto start = index;
        while (start > 0 && text.at(start - 1).isSpace()) {
            --start;
        }

        auto end = index + 1;
        while (end < text.size() && text.at(end).isSpace()) {
            ++end;
        }

        if (end - start == 1 && text.at(start) == QChar(u' ')) {
            return false;
        }

        cursor.setPosition(block.position() + static_cast<int>(start));
        cursor.setPosition(
            block.position() + static_cast<int>(end),
            QTextCursor::KeepAnchor);
        setTextCursor(cursor);

        return true;
    }

    // At the editor's left edge, in the margin kept for it (the text margin,
    // if any, lies between it and the viewport): the viewport's own top and
    // height, so a y in one is the same y in the other
    void layoutLineNumberArea_()
    {
        auto viewport_rect = viewport()->geometry();

        lineNumberArea_->setGeometry(
            contentsRect().left(),
            viewport_rect.top(),
            lineNumberAreaWidth_(),
            viewport_rect.height());
    }

    // The viewport scrolled by deltaY, or needs rect repainted: the gutter
    // does the same, so the numbers move and refresh with their lines
    void onUpdateRequest_(const QRect& rect, int deltaY)
    {
        // A scroll puts different lines in view
        if (deltaY != 0 && !searchMatches_.isEmpty()) {
            updateExtraSelections_();
        }

        if (!lineNumbers_) {
            return;
        }

        if (deltaY != 0) {
            lineNumberArea_->scroll(0, deltaY);
        } else {
            lineNumberArea_
                ->update(0, rect.y(), lineNumberArea_->width(), rect.height());
        }
    }

    void paintLineNumberArea_(QPaintEvent* event)
    {
        QPainter painter(lineNumberArea_);

        // The text area's background, so the gutter reads as part of it. The
        // viewport fills itself with its background role; do exactly that
        if (viewport()->autoFillBackground()) {
            painter.fillRect(
                event->rect(),
                viewport()->palette().brush(viewport()->backgroundRole()));
        }

        auto number_font = lineNumberFont_();
        QFontMetricsF metrics(number_font);

        painter.setFont(number_font);
        painter.setPen(lineNumberArea_->palette().color(LINE_NUMBER_ROLE));

        qreal right = lineNumberArea_->width() - LINE_NUMBER_RIGHT_PADDING;

        // Walk the visible blocks top-down in viewport coordinates, as
        // QPlainTextEdit's own paint does
        auto block = firstVisibleBlock();
        auto top =
            blockBoundingGeometry(block).translated(contentOffset()).top();

        while (block.isValid() && top <= event->rect().bottom()) {
            auto height = blockBoundingRect(block).height();

            if (block.isVisible() && top + height >= event->rect().top()) {
                auto first_line = block.layout()->lineAt(0);

                if (first_line.isValid()) {
                    auto number = QString::number(block.blockNumber() + 1);
                    auto baseline = top + first_line.y() + first_line.ascent();

                    painter.drawText(
                        QPointF(
                            right - metrics.horizontalAdvance(number),
                            baseline),
                        number);
                }
            }

            top += height;
            block = block.next();
        }
    }
};

namespace Internal {

inline LineNumberArea_::LineNumberArea_(TextEditor* parentTextEditor)
    : QWidget(parentTextEditor)
    , editor_(parentTextEditor)
{
}

inline void LineNumberArea_::paintEvent(QPaintEvent* event)
{
    editor_->paintLineNumberArea_(event);
}

} // namespace Internal

} // namespace Suzuri
