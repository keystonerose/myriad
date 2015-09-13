#ifndef MYRIAD_MAINWINDOW_H
#define MYRIAD_MAINWINDOW_H

#include <memory>
#include <KXmlGuiWindow>

class QStringList;

namespace myriad {
    
    /**
     * Manages Myriad's primary window and the actions that can be performed through interaction 
     * with it.
     */
    
    class MainWindow : public KXmlGuiWindow {
        Q_OBJECT
        
        public:
            
            /**
             * Initialises the main Myriad window and performs the steps needed to set up its GUI
             * and binds its methods to particular actions.
             */
        
            explicit MainWindow(const QString& caption, QWidget * parent = nullptr);
            
            /**
             * Destroys the main window, freeing any resources that it currently has in acquisition.
             */
            
            ~MainWindow();
            
            /**
             * Gets a list of all the targets that the user has loaded into the MainWindow's input
             * list, each of which will be a full filesystem path to an image file or a directory.
             * @return A list of the inputs currently specified by the user.
             */
            
            QStringList inputs() const;
            
            /**
            * Called before the main window is closed, information about its current state to be 
            * saved so that it can be restored the next time Myriad is run.
            * @return Whether to proceed with the closing the window.
            */
            
            bool queryClose() override final;
            
        private:
        
            class Private;
            std::unique_ptr<Private> d;
    };
}

#endif
