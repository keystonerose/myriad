#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>

#include "imageinfo.h"
#include "mainwindow.h"
#include "processor.h"
#include "processorthread.h"

namespace myriad {
    namespace processing {
        
        namespace {
            
            /**
             * Uses the MIME type of a file on disk to determine whether it is in a supported format for processing by
             * Myriad.
             * @param path The full filesystem path to the file to check.
             * @return @c true if @p path identifies a supported image file; @c false otherwise.
             */
            
            bool fileIsSupported(const QString& path) {
                
                QMimeDatabase mimeDb;
                const auto mimeName = mimeDb.mimeTypeForFile(path).name();
                return supportedMimeTypes().contains(mimeName.toLatin1());
            }
        }
        
        ProcessorThread::ProcessorThread(MainWindow * const mainWindow)
            : QThread{mainWindow}, m_mainWindow{mainWindow} {
        }
        
        void ProcessorThread::addInput(const QString& inputPath) {
            
            const QFileInfo fileInfo{inputPath};
            if (fileInfo.exists()) {

                if (fileInfo.isFile()) {
                    if (fileIsSupported(inputPath) && !m_images.contains(inputPath)) {
                        
                        m_images.insert(inputPath, ImageInfo{});
                        emitInputCount();
                    }
                }
                else if (fileInfo.isDir()) {
                    
                    QDir dir{inputPath};
                    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
                    
                    const auto dirItems = dir.entryInfoList();
                    ++m_inputFolderCount;
                    emitInputCount();
                    
                    for (const auto& item : dirItems) {
                        addInput(item.absoluteFilePath());
                    }
                }
            }
        }
        
        void ProcessorThread::addInputs(const QStringList& inputPaths) {
            for (const auto& inputPath : inputPaths) {
                addInput(inputPath);
            }
        }
        
        void ProcessorThread::emitInputCount() {
            emit(inputCountChanged(inputFileCount(), inputFolderCount()));
        }
        
        void ProcessorThread::hashImages() {
        }
        
        int ProcessorThread::inputFileCount() const {
            return m_images.size();
        }
        
        int ProcessorThread::inputFolderCount() const {
            return m_inputFolderCount;
        }
        
        void ProcessorThread::run() {
            
            emit(phaseChanged(Phase::SCANNING));
            emitInputCount();
            addInputs(m_mainWindow->inputs());
            
            emit(phaseChanged(Phase::HASHING));
            emit(hashingProgressChanged(0));
            hashImages();
        }
    }
}
