#include <QFileDialog>
#include <QObject>
#include <QStatusBar>
#include <QStringList>

#include <KLocalizedString>

#include "mainwindow.h"
#include "processor.h"
#include "processorthread.h"
#include "settings.h"

namespace myriad {
    namespace processing {

        void Processor::saveState(Settings * const settings) const {
            settings->setProcessingMode(settingsMode());
        }
        
        void Processor::start(MainWindow * const mainWindow) const {
            
            ProcessorThread * thread = createThread(mainWindow);
            QObject::connect(thread, &ProcessorThread::inputCountChanged, [mainWindow] (const int fileCount, const int folderCount) {
                
                const auto statusMessage = QString{"Target count: %L1 files in %L2 folders"}.arg(fileCount).arg(folderCount);
                mainWindow->statusBar()->showMessage(statusMessage);
            });
            
            thread->start();
        }
    }
}
