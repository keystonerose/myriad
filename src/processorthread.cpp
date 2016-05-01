#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMutexLocker>

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
            
            /**
             * Calculates @p numerator / @p denominator as a percentage, rounded to the nearest 1%.
             */
            
            int intPercentage(const int numerator, const int denominator) {
                return static_cast<int>(100.0f * static_cast<float>(numerator) / static_cast<float>(denominator) + 0.5f);
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
        
        void ProcessorThread::compareImages() {
            
            auto comparisonsMade = 0;
            const auto totalComparisonCount = comparisonCount();
            auto lastComparisonProgress = 0;
            
            auto iter1 = m_images.constBegin();
            const auto end = m_images.constEnd();
            
            for (; iter1 != end && !isInterruptionRequested(); ++iter1) {
                
                const auto& imageInfo1 = iter1.value();
                if (imageInfo1.isNull()) {
                    continue;
                }
                
                auto iter2 = iter1;
                ++iter2;
                
                for (; iter2 != end && !isInterruptionRequested(); ++iter2) {
                    
                    const auto& imageInfo2 = iter2.value();
                    if (imageInfo2.isNull()) {
                        continue;
                    }
                    
                    if (ImageInfo::difference(imageInfo1, imageInfo2) < 0.1) {
                        QMutexLocker locker(&m_mutex);
                        m_waitCond.wait(&m_mutex);
                    }
                    
                    ++comparisonsMade;
                    const auto progress = intPercentage(comparisonsMade, totalComparisonCount);
                    if (progress > lastComparisonProgress) {
                        emit(comparisonProgressChanged(progress));
                        lastComparisonProgress = progress;
                    }
                    
                    // It is possible for the image represented by imageInfo (and pointed to by iter1) to have been
                    // deleted (and the ImageInfo object invalidated) while the thread was paused, so we check for this
                    // case and move on to the next iteration of the outer loop if so.
                    
                    if (imageInfo1.isNull()) {
                        break;
                    }
                }
            }
        }
        
        int ProcessorThread::comparisonCount() {
            const auto imageCount = m_images.size();
            return imageCount * (imageCount - 1) / 2;
        }
        
        void ProcessorThread::emitInputCount(const bool force) {
            
            // Since the input count changes very quickly, we wind up with a huge queue of backed-up signals if we emit
            // an inputCountChanged() every time we scan a new file or folder. To mitigate this, we enforce a delay 
            // between emissions unless the force parameter is set (e.g. to ensure that a count is emitted once scanning
            // completes).
            
            constexpr qint64 EmissionPeriod = 20;
            if (force || !m_countEmissionTimer.isValid() || m_countEmissionTimer.elapsed() >= EmissionPeriod) {
            
                emit(inputCountChanged(inputFileCount(), inputFolderCount()));
                m_countEmissionTimer.start();
            }
        }
        
        void ProcessorThread::hashImages() {
            
            auto numImagesHashed = 0;
            auto lastHashingProgress = 0;
            
            auto iter = m_images.begin();
            const auto end = m_images.end();
            
            for (; iter != end && !isInterruptionRequested(); ++iter) {
            
                const auto& path = iter.key();
                auto& imageInfo = iter.value();
                
                imageInfo.read(path);
                ++numImagesHashed;
                
                const auto progress = intPercentage(numImagesHashed, inputFileCount());
                if (progress > lastHashingProgress) {
                    emit(hashingProgressChanged(progress));
                    lastHashingProgress = progress;
                }
            }
        }
        
        int ProcessorThread::inputFileCount() const {
            return m_images.size();
        }
        
        int ProcessorThread::inputFolderCount() const {
            return m_inputFolderCount;
        }
        
        void ProcessorThread::resume() {
            m_waitCond.wakeAll();
        }
        
        void ProcessorThread::run() {
            
            emit(phaseChanged(Phase::Scanning));
            emitInputCount(true);
            addInputs(m_mainWindow->inputs());
            emitInputCount(true);
            
            emit(phaseChanged(Phase::Hashing));
            emit(hashingProgressChanged(0));
            hashImages();
            
            emit(phaseChanged(Phase::Comparing));
            emit(comparisonProgressChanged(0));
            compareImages();
        }
    }
}
