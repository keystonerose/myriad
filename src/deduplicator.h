#ifndef MYRIAD_DEDUPLICATOR_H
#define MYRIAD_DEDUPLICATOR_H

#include "deduplicatorthread.h"
#include "processor.h"

namespace myriad {
    
    namespace ui {
        class MainWindow;
    }
    
    namespace processing {

        /**
         * Handles those aspects of Myriad's processing that are specific to when it is scanning through a set of input
         * directories to delete duplicates in-place.
         */
        
        class Deduplicator : public Processor {
        
        public:
            
            Deduplicator() = default;
            
            /**
                * Constructs a new Deduplicator, moving across any state from the Processor that existed before it.
                * @param rhs The prior Processor that this Deduplicator should retain settings from.
                */
            
            explicit Deduplicator(Processor&& rhs);
        
        private:
            
            /**
                * @see Processor::createThread()
                */
            
            DeduplicatorThread * createThread(ui::MainWindow * mainWindow) const override final;
            
            /**
                * @see Processor::settingsMode()
                */
            
            int settingsMode() const override final;
        };
    }
}

#endif
