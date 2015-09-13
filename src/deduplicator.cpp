#include "deduplicator.h"
#include "settings.h"

namespace myriad {
    namespace processing {
        
        Deduplicator::Deduplicator(const Processor& rhs)
            : Processor(rhs) {
        }
       
        DeduplicatorThread * Deduplicator::createThread(MainWindow * const mainWindow) const {
            return new DeduplicatorThread{mainWindow};
        }
        
        int Deduplicator::settingsMode() const {
            return Settings::EnumProcessingMode::Deduplicate;
        }
    }
}
