#include <QFormLayout>
#include <QLabel>

#include <KLocalizedString>

#include "imageview.h"

namespace myriad {
    namespace ui {
        
        class ImageView::DetailsGroup : public QWidget {
        
        public:
            
            /**
            * Sets up the labels used to display image detail text. Once constructed, the widget will contain a table of
            * appropriate labels.
            */
            
            explicit DetailsGroup(QWidget * const parent)
                : QWidget{parent} {
                    
                auto * const layout = new QFormLayout{this};

                layout->addRow(i18n("Filename:"), m_fileNameLabel);
                layout->addRow(i18n("Directory:"), m_dirLabel);
                layout->addRow(i18n("Width:"), m_widthLabel);
                layout->addRow(i18n("Height:"), m_heightLabel);
                layout->addRow(i18n("Format:"), m_formatLabel);
                layout->addRow(i18n("File size:"), m_fileSizeLabel);
            }
            
        private:
            
            QLabel * const m_dirLabel      = new QLabel{};
            QLabel * const m_fileNameLabel = new QLabel{};
            QLabel * const m_fileSizeLabel = new QLabel{};
            QLabel * const m_formatLabel   = new QLabel{};
            QLabel * const m_heightLabel   = new QLabel{};
            QLabel * const m_widthLabel    = new QLabel{};
        };
        
        struct ImageView::Private {
            
            /**
             * Initialises the ImageView's private data upon construction.
             * @param q The owner instance of the public class.
             */

            explicit Private(ImageView * const q)
                : q{q} {
            }
            
            /**
             * Sets up the labels used to display image detail text. This method should be called upon construction, and
             * once it has completed, the m_detailsGroup widget will contain 
             */
            
            ImageView * const q;
        };
        
        ImageView::ImageView(QWidget * const parent)
            : QFrame{parent}, d{std::make_unique<Private>(this)} {
            
        }

        ImageView::~ImageView() = default;
    }
}
