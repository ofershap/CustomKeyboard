#include "Forms/configform.h"
#include "Forms/devicefinder.h"

#include <QApplication>
#include <QTranslator>
#include <iostream>
#include <QTimer>
#include <QDir>
#include <QPushButton>
#include <QPixmap>

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // CI/UI review must never probe or configure attached hardware.
    const int previewIndex = a.arguments().indexOf("--preview-shots");
    const bool preview = previewIndex >= 0;
    if (preview && previewIndex + 1 >= a.arguments().size()) return 2;
    if (preview) qputenv("ZDD_OFFLINE_PREVIEW", "1");
    //load window

    DeviceFinder dw;
    dw.smartShow();

    if (preview) {
        const QString outputDir = a.arguments().at(previewIndex + 1);
        if (!QDir().mkpath(outputDir)) return 2;
        QTimer::singleShot(500, [&a, &dw, outputDir]() {
            if (!dw.grab().save(outputDir + "/device-finder.png")) { a.exit(3); return; }
            auto *skip = dw.findChild<QPushButton *>("btn_skip");
            if (!skip) { a.exit(4); return; }
            skip->click();
            QTimer::singleShot(500, [&a, outputDir]() {
                for (QWidget *window : QApplication::topLevelWidgets()) {
                    if (window->isVisible() && window->windowTitle().contains("ZDDKeyboardSetter")) {
                        a.exit(window->grab().save(outputDir + "/configuration.png") ? 0 : 5);
                        return;
                    }
                }
                a.exit(6);
            });
        });
    }
    return a.exec();
}
