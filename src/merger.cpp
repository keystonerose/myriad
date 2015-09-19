#include "merger.h"
#include "settings.h"

namespace myriad {
    namespace processing {

        Merger::Merger(const Processor& rhs)
            : Processor{rhs} {
        }

        ProcessorThread * Merger::createThread(MainWindow * const) const {

            // TODO: Not yet implemented!
            return nullptr;
        }
        
        int Merger::settingsMode() const {
            return Settings::EnumProcessingMode::Merge;
        }
    }
}
