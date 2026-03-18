#include "mainwindow.h"
#include "ResourceRegistry.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(AppSettings::RobotoRegular);

    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont roboto(family, 12);
        roboto.setStyleStrategy(QFont::PreferAntialias);
        roboto.setHintingPreference(QFont::PreferNoHinting);
        a.setFont(roboto);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
