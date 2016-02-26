#ifndef MYRIAD_PROCESSOR_H
#define MYRIAD_PROCESSOR_H

#include <functional>
#include <memory>

#include <QMetaObject>
#include <QObject>

class QThread;

namespace myriad {
    
    class Settings;
    namespace ui {
        class MainWindow;
    }
    
    namespace processing {
        
        class ProcessorThread;
        
        /**
         * Codes that identify what phase of execution Myriad is current in. @c Idle is the state when no worker thread
         * is running; the other three states correspond to various actions performed by the worker thread.
         */
        
        enum class Phase {
           Idle,
           Scanning,
           Hashing,
           Comparing
        };
        
        /**
         * A helper class used to connect a certain function object to the QThread::finished signal. To enable this
         * callback to execute in the context of the main thread (not the worker thread), it is necessary for it to be
         * called from a slot on a QObject, rather than (say) from a lambda (which would be executed in the context of
         * the worker thread). The FinishedCallback provides the object to be connected to here: the MainWindow won't
         * do, since it doesn't know about the worker thread directly, and the Processor won't do either, since it's 
         * movable and therefore can't be a QObject. 
         * 
         * In an ideal world, this would be a fully general templated class allowing any kind of lambda to be connected
         * to any kind of signal but run in the context of the owning object. Sadly, though, the Qt MOC doesn't work 
         * with templated classes.
         */
        
        class FinishedCallback : public QObject {
        Q_OBJECT
        
        public:
            
            using RawCallback = std::function<void()>;
            
            /**
             * Sets up the FinishedCallback to execute a specified callback function when a particular thread finishes,
             * and performs the connection that will be responsible for making this happen. This connection is undone as
             * soon as @p callback has been executed, so even if @p thread is run multiple times, this callback function
             * will only be invoked after the first run. 
             * @param thread The thread whose @c finished signal should cause @p callback to be executed.
             * @param callback The callback to execute.
             */
            
            FinishedCallback(QThread * thread, RawCallback callback);
        
        private slots:
            
            /**
             * Executes the callback that this FinishedCallback object was constructed with, and then immediately 
             * disconnects the connection that was made between this slot and the sending thread's @c finished signal so
             * that this callback will not be executed a second time should the thread send its @c finished signal
             * again.
             */
            
            void invoke();
            
        private:
            
            const RawCallback m_callback;
            const QMetaObject::Connection m_connection;
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
            
            /**
             * Constructs a Processor with default state.
             */
            
            Processor() = default;
            
            Processor(const Processor&) = delete;
            Processor& operator=(const Processor&) = delete;
            
            Processor(Processor&&) = default;
            Processor& operator=(Processor&&) = default;
            
            virtual ~Processor() = default;
            
            /**
             * Tests whether the Processor currently has a worker thread running in the background. If not, the
             * application can be terminated safely; if so, a call must be made to stop() beforehard.
             */
            
            bool isBusy() const;
            
            /**
             * Saves state information about the current processing mode to the application settings so that they can be
             * restored upon the next run.
             */
            
            void saveState(Settings * settings) const;
            
            /**
             * Starts this Processor running in a new thread, and connects the signals of this thread to appropriate
             * slots on the main window.
             * @param mainWindow The main window that the Processor will update as it runs.
             */
            
            void start(ui::MainWindow * mainWindow);
            
            /**
             * Asynchronously puts the Processor into a stopped state (by interrupting any running worker thread(s) that
             * it may be using) and executes a provided callback once this has been achieved. Only one callback can be
             * queued in this manner at a time; calling stopAndThen() a second time will replace the callback that was
             * provided to the first call.
             * @param callback The callback to execute once the processor has been stopped.
             * @return @c true if the Processor was already stopped and @p callback was therefore executed already;
             * @c false if the Processor was busy and @p callback was therefore queued to be executed once it stops.
             */
            
            bool stopAndThen(std::function<void()> callback);
            
        private:
            
            /**
             * Creates a new thread of an appropriate type that will handle the processing that needs to be performed,
             * and makes any signal-slot connections that are specific to the dynamic type of this thread. The calling
             * function should take ownership of the returned thread.
             * @param mainWindow The main window that the thread will update as it runs.
             * @return The newly created thread, whose ownership passes to the calling function.
             */
            
            virtual ProcessorThread * createThread(ui::MainWindow * mainWindow) const = 0;
            
            /**
             * Gets the KConfig XT enum code that is used to identify the processing mode implemented by this
             * Processor.
             */
            
            virtual int settingsMode() const = 0;
            
            std::unique_ptr<FinishedCallback> m_finishedCallback = nullptr;
            ProcessorThread * m_thread = nullptr;
        };
    }
}

#endif
