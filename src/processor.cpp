#include <QFileDialog>
#include <QMetaType>
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
        
        namespace {
            
            /**
             * Some of the types passed between the worker thread and the main window (GUI) thread via the signal-slot
             * mechanism are of types not known by Qt. In order for these connections to work, we have to register these
             * types with Qt before the connections are made. This function performs this registration if it hasn't been
             * done already, and therefore must be called before signals on any ProcessorThread are connected to
             * corresponding slots.
             */
            
            void registerMetaTypes() {
                
                static auto registered = false;
                if (!registered) {
                
                    qRegisterMetaType<myriad::processing::Phase>("myriad::processing::Phase");
                    registered = true;
                }
            }
        }
        
        bool Processor::isBusy() const {
            return m_thread->isRunning();
        }

        void Processor::saveState(Settings * const settings) const {
            settings->setProcessingMode(settingsMode());
        }
        
        void Processor::start(MainWindow * const mainWindow) {
            
            registerMetaTypes();
            
            m_thread = createThread(mainWindow);
            QObject::connect(m_thread, &ProcessorThread::phaseChanged, mainWindow, &MainWindow::setPhase);
            QObject::connect(m_thread, &ProcessorThread::inputCountChanged, mainWindow, &MainWindow::setInputCount);
            
            m_thread->start();
        }
        
        void Processor::stopAndThen(std::function<void()> callback) {
            
            if (isBusy()) {

                //QObject::connect(m_thread, &QThread::finished, this, &ProcessorThread::stopCallback);
                m_thread->requestInterruption();
            }
            else {
                callback();
            }
        }
    }
}
