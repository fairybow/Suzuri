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

#include <optional>
#include <string>

#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QString>
#include <QStringConverter>
#include <QStringList>
#include <QStringView>

#include <hunspell.hxx>

#include <Coco/Debug.h>
#include <Coco/Path.h>

namespace Suzuri {

using namespace Qt::StringLiterals;

// One language's spelling dictionary, over Hunspell: is this word spelled
// correctly, and what might have been meant. GUI-free.
//
// Built from a Hunspell dictionary's two files, the affix rules (.aff) and the
// word list (.dic). Nothing is added to it after that, so an answer for a word
// never changes and each is kept. A vault's own words are held elsewhere and
// asked first.
//
// Without a usable dictionary (a missing file, or an encoding Qt can't
// convert) the checker is not valid, and calls every word correct: no
// dictionary means no complaints, not a page of them.
//
// What it adds to the dictionary's own rules:
// - A typographic apostrophe is read as a plain one, since dictionaries list
//   "don't" with the plain one and prose is set with the other
// - A word with a digit in it is not checked
//
// A hyphenated word is left to Hunspell, which, unless the dictionary says
// otherwise, takes one as correct when each of its parts is
class SpellChecker
{
public:
    SpellChecker(const Coco::Path& affixFile, const Coco::Path& wordFile)
    {
        setup_(affixFile, wordFile);
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return hunspell_.has_value();
    }

    // One word, with no space in it. An empty word is correct
    [[nodiscard]] bool isCorrect(const QString& word)
    {
        if (!isValid() || word.isEmpty()) {
            return true;
        }

        if (auto it = known_.constFind(word); it != known_.constEnd()) {
            return it.value();
        }

        auto correct = check_(plain_(word));
        known_.insert(word, correct);

        return correct;
    }

    // Up to max words the dictionary offers in place of this one, best first.
    // Slow next to isCorrect: for a word the user has asked about, not for
    // every word on screen
    [[nodiscard]] QStringList suggestions(const QString& word, int max)
    {
        QStringList result{};

        if (!isValid() || word.isEmpty() || max <= 0) {
            return result;
        }

        const auto found = hunspell_->suggest(encode_(plain_(word)));

        for (const auto& suggestion : found) {
            if (result.size() >= max) {
                break;
            }

            result << decoder_->decode(QByteArrayView(suggestion));
        }

        return result;
    }

private:
    // Empty until a dictionary has loaded. The converters go between QString
    // and the encoding the dictionary's files declare, which is the one
    // Hunspell takes and gives words in
    std::optional<Hunspell> hunspell_{};
    std::optional<QStringEncoder> encoder_{};
    std::optional<QStringDecoder> decoder_{};

    // Every answer given, by the word as it was asked
    QHash<QString, bool> known_{};

    void setup_(const Coco::Path& affixFile, const Coco::Path& wordFile)
    {
        // Hunspell reports nothing when a file is missing: it becomes a
        // dictionary with no words, to which every word is wrong
        if (!affixFile.isFile() || !wordFile.isFile()) {
            WARN(
                "No spelling dictionary at {} and {}",
                affixFile.prettyQString(),
                wordFile.prettyQString());
            return;
        }

        hunspell_.emplace(
            hunspellPath_(affixFile).c_str(),
            hunspellPath_(wordFile).c_str());

        const auto& encoding = hunspell_->get_dict_encoding();

        if (auto known = knownEncoding_(encoding)) {
            encoder_.emplace(*known);
            decoder_.emplace(*known);
        } else {
            encoder_.emplace(encoding.c_str());
            decoder_.emplace(encoding.c_str());
        }

        if (!encoder_->isValid() || !decoder_->isValid()) {
            WARN(
                "Can't use the spelling dictionary at {}: its encoding, {}, "
                "isn't one Qt converts here",
                affixFile.prettyQString(),
                QString::fromLatin1(encoding));

            hunspell_.reset();
        }
    }

    // Most dictionaries are UTF-8, and some older ones Latin-1. Qt converts
    // those two in every build, and they are picked out here by name so that
    // doesn't rest on how Qt matches Hunspell's spelling of them. Any other
    // encoding is given to Qt by name, which works only where Qt was built
    // with ICU
    [[nodiscard]] static std::optional<QStringConverter::Encoding>
    knownEncoding_(const std::string& name)
    {
        auto bare = QString::fromLatin1(name).toUpper();
        bare.remove(QChar(u'-'));
        bare.remove(QChar(u'_'));

        if (bare == u"UTF8"_s) {
            return QStringConverter::Utf8;
        }

        if (bare == u"ISO88591"_s) {
            return QStringConverter::Latin1;
        }

        return std::nullopt;
    }

    // A file's path as Hunspell opens it. It hands a path to the C++ runtime
    // as it stands, which on Windows reads it in the system's code page, not
    // as UTF-8: a path outside that code page wouldn't open. Given the long-
    // path prefix, Hunspell converts the path from UTF-8 itself. The prefix
    // wants an absolute path with backslashes
    [[nodiscard]] static std::string hunspellPath_(const Coco::Path& file)
    {
#if defined(Q_OS_WIN)

        auto absolute = QFileInfo(file.toQString()).absoluteFilePath();
        auto native = QDir::toNativeSeparators(absolute);

        return (u"\\\\?\\"_s + native).toStdString();

#else

        return file.toQString().toStdString();

#endif
    }

    // The word with each typographic apostrophe made a plain one
    [[nodiscard]] static QString plain_(const QString& word)
    {
        auto plain = word;
        plain.replace(QChar(0x2019), QChar(u'\''));

        return plain;
    }

    [[nodiscard]] std::string encode_(QStringView word)
    {
        QByteArray bytes = encoder_->encode(word);
        return bytes.toStdString();
    }

    [[nodiscard]] bool check_(const QString& word)
    {
        for (auto character : word) {
            if (character.isDigit()) {
                return true;
            }
        }

        return hunspell_->spell(encode_(word));
    }
};

} // namespace Suzuri
