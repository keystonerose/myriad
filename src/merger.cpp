#include <utility>

#include "merger.h"
#include "settings.h"

namespace myriad {
    namespace processing {

        Merger::Merger(Processor&& rhs)
            : Processor{std::move(rhs)} {
        }

        ProcessorThread * Merger::createThread(ui::MainWindow * const) const {

            // TODO: Not yet implemented!
            return nullptr;
        }
        
        int Merger::settingsMode() const {
            return Settings::EnumProcessingMode::Merge;
        }
    }
}
