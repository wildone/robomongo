#include <QApplication>
#include <QDesktopWidget>
#include <QDir>

#include "robomongo/core/settings/SettingsManager.h"
#include "robomongo/core/AppRegistry.h"
#include "robomongo/gui/MainWindow.h"

using namespace Robomongo;

int main(int argc, char *argv[])
{
    setlocale(LC_NUMERIC,"C");
    QApplication app(argc, argv);

    AppRegistry::instance().settingsManager()->save();

    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    QSize size(screenGeometry.width() - 450, screenGeometry.height() - 165);

    MainWindow win;
    win.resize(size);
#if defined(Q_OS_MAC)
    win.setUnifiedTitleAndToolBarOnMac(true);
#endif
    int x = (screenGeometry.width() - win.width()) / 2;
    int y = (screenGeometry.height() - win.height()) / 2;
    win.move(x, y);
    win.show();

    return app.exec();
}
