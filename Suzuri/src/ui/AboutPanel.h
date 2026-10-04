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

#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <Coco/Debug.h>

#include "core/Publication.h"
#include "core/Version.h"

namespace Suzuri::Ui {

// TODO: Ctor param to show App name or not (for Splash)? Might be something
// that is just handled outside?
class AboutPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AboutPanel(QWidget* parent)
        : QWidget(parent)
    {
        setup_();
    }

    ~AboutPanel() override { TRACER; }

private:
    static constexpr auto ICON_PATH_ = ":/icons/Suzuri-128.png";
    static constexpr auto ICON_SIZE_ = 128;

    void setup_()
    {
        auto root_layout = new QVBoxLayout(this);
        root_layout->setSpacing(12);
        root_layout->addWidget(buildIconLabel_());
        root_layout->addWidget(buildVersionLabel_());
    }

    QLabel* buildIconLabel_()
    {
        auto label = new QLabel(this);
        label->setAlignment(Qt::AlignCenter);
        label->setPixmap(QPixmap(ICON_PATH_)
                             .scaled(
                                 ICON_SIZE_,
                                 ICON_SIZE_,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation));

        return label;
    }

    QLabel* buildVersionLabel_()
    {
        auto text = VERSION_FULL_QSTRING + "\n" + PUB_RELEASE_NAME_QSTRING;

        auto label = new QLabel(text, this);
        label->setAlignment(Qt::AlignCenter);
        label->setEnabled(false);

        return label;
    }
};

} // namespace Suzuri::Ui
