#include <utility>

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
        
        FinishedCallback::FinishedCallback(QThread * const thread, RawCallback callback)
            : m_callback(std::move(callback)),
              m_connection(connect(thread, &QThread::finished, this, &FinishedCallback::invoke)) {            
        }
        
        void FinishedCallback::invoke() {
            disconnect(m_connection);
            m_callback();
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
            QObject::connect(m_thread, &ProcessorThread::hashingProgressChanged, mainWindow, &MainWindow::setHashingProgress);
            
            m_thread->start();
        }
        
        bool Processor::stopAndThen(std::function<void()> callback) {
            
            if (isBusy()) {
                
                // The FinishedCallback will ensure that callback is only executed once (the next time that m_thread
                // finishes). This isn't strictly necessary -- since the thread is recreated anew each time start() is
                // called, its finished signal will only be fired once -- but it's functionality we get essentially for
                // free when implementing FinishedCallback, and make things more robust should the handling of m_thread
                // change in the future.
                
                m_finishedCallback = std::make_unique<FinishedCallback>(m_thread, callback);
                m_thread->requestInterruption();
                return false;
            }
            else {
                
                callback();
                return true;
            }
        }
    }
}
