#ifndef MYRIAD_MAINWINDOW_H
#define MYRIAD_MAINWINDOW_H

#include <memory>
#include <KXmlGuiWindow>

class QStringList;

namespace myriad {
    
    namespace processing {
        enum class Phase;
    }
    
    namespace ui {
        
        /**
        * Manages Myriad's primary window and the actions that can be performed through interaction with it.
        */
        
        class MainWindow : public KXmlGuiWindow {
        Q_OBJECT
        
        public:
            
            /**
             * Initialises the main Myriad window and performs the steps needed to set up its GUI and binds its methods
             * to particular actions.
             */
        
            explicit MainWindow(const QString& caption, QWidget * parent = nullptr);
            
            /**
             * Destroys the main window, freeing any resources that it currently has in acquisition.
             */
            
            ~MainWindow();
            
            /**
             * Gets a list of all the targets that the user has loaded into the MainWindow's input list, each of which
             * will be a full filesystem path to an image file or a directory.
             * @return A list of the inputs currently specified by the user.
             */
            
            QStringList inputs() const;
            
            /**
             * Called before the main window is closed, information about its current state to be saved so that it can
             * be restored the next time Myriad is run.
             * @return Whether to proceed with the closing the window.
             */
            
            bool queryClose() override final;
            
        public slots:
            
            /**
             * Displays the completion progress of Myriad's image comparison phase.
             * @param progress The percentage completion to display.
             */
            
            void setComparisonProgress(int progress);
            
            /**
             * Displays the completion progress of Myriad's image hashing processing phase.
             * @param progress The percentage completion to display.
             */
            
            void setHashingProgress(int progress);
            
            /**
             * Sets the number of files and folders that Myriad has currently scanned for processing, and displays this
             * information in the main UI.
             * @param fileCount The new file count to display.
             * @param folderCount The new folder count to display.
             */
            
            void setInputCount(int fileCount, int folderCount);
            
            /**
             * Sets the current processing phase that Myriad is executing, and displays this information in the main UI.
             * @param phase The processing phase to indicate.
             */
            
            void setPhase(myriad::processing::Phase phase);
            
        private:
        
            struct Private;
            std::unique_ptr<Private> d;
        };
    }
}

#endif
