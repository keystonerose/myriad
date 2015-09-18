#ifndef MYRIAD_PROCESSORTHREAD_H
#define MYRIAD_PROCESSORTHREAD_H

#include <QAtomicInt>
#include <QHash>
#include <QSet>
#include <QString>
#include <QThread>

#include "imageinfo.h"

class QStringList;

namespace myriad {
    
    class MainWindow;
    
    namespace processing {
        
        enum class Phase;
        
        /**
         * A base class for threads that provides functionality for processing a collection of input images for
         * duplicates. The basic usage pattern of ProcessorThread instances is that they should be constructed, then
         * have targets added via calls to addInputs(), then be launched.
         */
        
        class ProcessorThread : public QThread {
            Q_OBJECT
            
            public:
            
                /**
                 * Constructs the thread.
                 * @param mainWindow The main window that provides input data for this thread and displays information
                 * about it as it executes. This will take ownership of the thread.
                 */
            
                explicit ProcessorThread(MainWindow * parent);
            
                /**
                 * Gets the number of files that have been added as inputs to this thread so far (either by being
                 * directly passed to addInputs() or by virtue of belonging to folders passed to the same method).
                 */
                
                int inputFileCount() const;
                
                /**
                 * Gets the number of folders that have been scanned for inputs by this thread so far.
                 */
                
                int inputFolderCount() const;
                
                /**
                 * Executes the thread by adding the targets specified in the main window and performing whatever
                 * specific logic is specified by its dynamic type.
                 */
                
                void run() override final;
                
            signals:
                
                /**
                 * Emitted when the percentage progress of the thread's image hash generation changes.
                 * @param progress The completion percentage of the hashing process.
                 */
                
                void hashingProgressChanged(int progress);
                
                /**
                 * Emitted when the number of files or folders scanned by the thread as inputs changes.
                 * @param fileCount The number of files that have been scanned by the thread so far.
                 * @param folderCount The number of folders that have been scanned by the thread so far.
                 */
                
                void inputCountChanged(int fileCount, int folderCount);
                
                /**
                 * Emitted when the type of processing being done by the thread changes (including once when the thread
                 * is initially started).
                 * @param phase A code identifying the phase of execution that the thread is currently in.
                 */
                
                void phaseChanged(myriad::processing::Phase phase);
                
            private:
                
                /**
                 * Adds a single target to the thread, which will be processed for duplicates when it is launched.
                 * @p inputPath should be a filesystem path to either an image file (which will be added directly) or a
                 * directory (which will be recursively scanned for supported image files to be added).
                 */
                
                void addInput(const QString& inputPath);
            
                /**
                 * Repeatedly calls addInput() to add a list of targets to the thread.
                 */
                
                void addInputs(const QStringList& inputPaths);
                
                /**
                 * Emits the inputCountChanged() signal with appropriate values for the number of files and folders
                 * scanned so far.
                 */
                
                void emitInputCount();
                
                /**
                 * Scans through all image files previously passed to addInput() and generates a perceptual hash for
                 * each so that they may subsequently be compared efficiently.
                 */
                
                void hashImages();
                
                QAtomicInt m_inputFileCount{0};
                QAtomicInt m_inputFolderCount{0};
                
                QHash<QString, ImageInfo> m_images;
                const MainWindow * const m_mainWindow;
        };
    }
}

#endif
