#include <utility>

#include "deduplicator.h"
#include "settings.h"

namespace myriad {
    namespace processing {
        
        Deduplicator::Deduplicator(Processor&& rhs)
            : Processor{std::move(rhs)} {
        }
       
        DeduplicatorThread * Deduplicator::createThread(ui::MainWindow * const mainWindow) const {
            return new DeduplicatorThread{mainWindow};
        }
        
        int Deduplicator::settingsMode() const {
            return Settings::EnumProcessingMode::Deduplicate;
        }
    }
}
