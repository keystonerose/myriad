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
        
        ProcessorThread::ProcessorThread(ui::MainWindow * const mainWindow)
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
                    
                    auto iter = dirItems.constBegin();
                    const auto end = dirItems.constEnd();
                    
                    for (; iter != end && !isInterruptionRequested(); ++iter) {
                        addInput(iter->absoluteFilePath());
                    }
                }
            }
        }
        
        void ProcessorThread::addInputs(const QStringList& inputPaths) {
            for (const auto& inputPath : inputPaths) {
                addInput(inputPath);
            }
        }
        
        void ProcessorThread::emitInputCount(const bool force) {
            
            // Since the input count changes very quickly, we wind up with a huge queue of backed-up signals if we emit
            // an inputCountChanged() every time we scan a new file or folder. To mitigate this, we enforce a delay 
            // between emissions unless the force parameter is set (e.g. to ensure that a count is emitted once scanning
            // completes).
            
            constexpr qint64 EMISSION_PERIOD = 20;
            if (force || !m_countEmissionTimer.isValid() || m_countEmissionTimer.elapsed() >= EMISSION_PERIOD) {
            
                emit(inputCountChanged(inputFileCount(), inputFolderCount()));
                m_countEmissionTimer.start();
            }
        }
        
        void ProcessorThread::hashImages() {
            
            auto numImagesHashed = 0;
            
            auto iter = m_images.begin();
            const auto end = m_images.end();
            
            for (; iter != end && !isInterruptionRequested(); ++iter) {
            
                const auto& path = iter.key();
                auto& imageInfo = iter.value();
                
                imageInfo.read(path);
                ++numImagesHashed;
                
                auto progress = static_cast<int>(100.0f * static_cast<float>(numImagesHashed) / static_cast<float>(inputFileCount()) + 0.5f);
                if (progress > m_lastHashingProgress) {
                    emit(hashingProgressChanged(progress));
                    m_lastHashingProgress = progress;
                }
            }
        }
        
        int ProcessorThread::inputFileCount() const {
            return m_images.size();
        }
        
        int ProcessorThread::inputFolderCount() const {
            return m_inputFolderCount;
        }
        
        void ProcessorThread::run() {
            
            emit(phaseChanged(Phase::SCANNING));
            emitInputCount(true);
            addInputs(m_mainWindow->inputs());
            emitInputCount(true);
            
            emit(phaseChanged(Phase::HASHING));
            emit(hashingProgressChanged(0));
            hashImages();
        }
    }
}
