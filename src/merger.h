#ifndef MYRIAD_MERGER_H
#define MYRIAD_MERGER_H

#include "processor.h"

namespace myriad {
    
    class MainWindow;
    
    namespace processing {
        
        class ProcessorThread;
        
        /**
         * Handles those aspects of Myriad's processing that are specific to when it is merging a set of input files
         * into specified target directories while discarding duplicates.
         */
        
        class Merger : public Processor {
            
            public:
                
                Merger() = default;
            
                /**
                 * Constructs a new Merger, moving across any state from the Processor that existed before it.
                 * @param rhs The prior Processor that this Merger should retain settings from.
                 */
                
                explicit Merger(Processor&& rhs);
            
            private:
                
                /**
                 * @see Processor::createThread()
                 */
                
                ProcessorThread * createThread(MainWindow * mainWindow) const override final;
                
                /**
                 * @see Processor::settingsMode()
                 */
                
                int settingsMode() const override final;
        };
    }
}

#endif
