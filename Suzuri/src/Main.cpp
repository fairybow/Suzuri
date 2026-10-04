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

#include <Coco/StartCop.h>

#include "App.h"
#include "core/Publication.h"

int main(int argc, char* argv[])
{
    Coco::StartCop cop(PUB_APP_NAME_STRING, argc, argv);

    if (cop.isRunning()) {
        return 0;
    }

    Suzuri::App app(argc, argv);
    app.connect(
        &cop,
        &Coco::StartCop::relaunched,
        &app,
        &Suzuri::App::onRelaunch);
    app.init();

    return app.exec();
}
