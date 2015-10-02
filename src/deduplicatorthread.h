#ifndef MYRIAD_DEDUPLICATORTHREAD_H
#define MYRIAD_DEDUPLICATORTHREAD_H

#include "processorthread.h"

namespace myriad {
    namespace processing {
        
        //TODO:DOC
        
        class DeduplicatorThread : public ProcessorThread {
            Q_OBJECT
            
            public:
                
                /**
                 * @see ProcessorThread::ProcessorThread()
                 */
            
                explicit DeduplicatorThread(ui::MainWindow * parent);
        };
    }
}

#endif
