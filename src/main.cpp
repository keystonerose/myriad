#include <QApplication>
#include <QString>

#include <KAboutData>
#include <KLocalizedString>

#include "mainwindow.h"
 
int main(int argc, char ** argv) {
    
    const auto displayName = i18n("Myriad");

    KAboutData aboutData{
        QStringLiteral("myriad"),
        displayName,
        QStringLiteral("0.1"),
        i18n("A tool for managing similar images."),
        KAboutLicense::GPL_V2,
        i18n("(c) 2015")
    };
    
    aboutData.addAuthor(
        i18n("James Dent"),
        i18n("Developer"),
        QStringLiteral("semajdent@gmail.com"),
        QStringLiteral("http://www.keystonerose.net/")
    );
    
    KAboutData::setApplicationData(aboutData);
    
    QApplication app{argc, argv};
    QApplication::setApplicationDisplayName(displayName);
    
    auto * const mainWindow = new myriad::MainWindow{displayName};
    mainWindow->show();
    
    return app.exec();
}
