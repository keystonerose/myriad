#ifndef MYRIAD_PROCESSOR_H
#define MYRIAD_PROCESSOR_H

#include <QString>
#include <QThread>

namespace myriad {
    
    class MainWindow;
    class Settings;
    
    namespace processing {
        
        class ProcessorThread;
        
        /**
         * Codes that identify what phase of execution Myriad is current in. @c IDLE is the state when no worker thread
         * is running; the other three states correspond to various actions performed by the worker thread.
         */
        
        enum class Phase {
           IDLE,
           SCANNING,
           HASHING,
           COMPARING
        };
        
        /**
         * A polymorphic base class used for loading and processing image files for duplicates. Myriad offers two main
         * modes of operation: it may be used to merge new input files into certain target directories if and only if
         * they are not duplicates of existing images, or it may be used to detect duplicates in-place for an existing
         * collection directory, deleting any that it finds. Common aspects of these two behaviours are provided by the 
         * Processor base class; specialisations thereof are provided by its subclasses, Deduplicator and Merger.
         */
        
        class Processor {
            
            public:
                
                virtual ~Processor() = default;
                
                /**
                 * Saves state information about the current processing mode to the application settings so that they
                 * can be restored upon the next run.
                 */
                
                void saveState(Settings * settings) const;
                
                /**
                 * Starts this Processor running in a new thread, and connects the signals of this thread to appropriate
                 * slots on the main window.
                 * @param mainWindow The main window that the Processor will update as it runs.
                 */
                
                void start(MainWindow * mainWindow) const;
                
            private:
                
                /**
                 * Creates a new thread of an appropriate type that will handle the processing that needs to be
                 * performed, and makes any signal-slot connections that are specific to the dynamic type of this
                 * thread. The calling function should take ownership of the returned thread.
                 * @param mainWindow The main window that the thread will update as it runs.
                 * @return The newly created thread, whose ownership passes to the calling function.
                 */
                
                virtual ProcessorThread * createThread(MainWindow * mainWindow) const = 0;
                
                /**
                 * Gets the KConfig XT enum code that is used to identify the processing mode implemented by this
                 * Processor.
                 */
                
                virtual int settingsMode() const = 0;
        };
    }
}

#endif
