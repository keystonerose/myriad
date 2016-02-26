#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "imageview.h"

namespace myriad {
    namespace ui {
        
        /**
         * A widget that groups together the labels used to tabulate the detail info displayed below the image preview
         * in the ImageView.
         */
        
        class ImageView::DetailsWidget : public QWidget {
        
        public:
            
            /**
            * Sets up the labels used to display image detail text. Once constructed, the widget will contain a table of
            * appropriate labels.
            */
            
            explicit DetailsWidget(QWidget * const parent)
                : QWidget{parent} {
                    
                auto * const layout = new QFormLayout{this};

                layout->addRow(i18n("Filename:"),  m_fileNameLabel = new QLabel{});
                layout->addRow(i18n("Directory:"), m_dirLabel      = new QLabel{});
                layout->addRow(i18n("Width:"),     m_widthLabel    = new QLabel{});
                layout->addRow(i18n("Height:"),    m_heightLabel   = new QLabel{});
                layout->addRow(i18n("Format:"),    m_formatLabel   = new QLabel{});
                layout->addRow(i18n("File size:"), m_fileSizeLabel = new QLabel{});
            }
            
        private:
            
            QLabel * m_dirLabel;
            QLabel * m_fileNameLabel;
            QLabel * m_fileSizeLabel;
            QLabel * m_formatLabel;
            QLabel * m_heightLabel;
            QLabel * m_widthLabel;
        };
        
        /**
         * Lays out various image and text labels in such a way as to provide a preview of the ImageView's target image,
         * along with an optional icon and zoomed area as overlays. The zoom overlay is not visible by default.
         */
        
        class ImageView::PreviewWidget : public QWidget {
            
        public:
            
            /**
             * Sets up the labels used to preview the target image. Once constructed, the contents of the PreviewWidget
             * will themselves be constructed and appropriately laid out.
             */
            
            explicit PreviewWidget(QWidget * const parent) 
                : QWidget{parent} {

                setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
                
                auto * const layout = new QGridLayout{this};
                layout->setContentsMargins(0, 0, 0, 0);
                layout->addWidget(m_previewStack = new QWidget{}, 1, 1);
                
                auto * const stackLayout = new QStackedLayout{m_previewStack};
                stackLayout->setStackingMode(QStackedLayout::StackAll);
                
                stackLayout->addWidget(m_previewLabel = new QLabel{});
                m_previewLabel->setScaledContents(true);
                m_previewLabel->setMinimumSize(MinHeight, MinHeight);
                
                stackLayout->addWidget(m_zoomContainer = new QWidget{});
                stackLayout->setCurrentWidget(m_zoomContainer);
                m_zoomContainer->setMouseTracking(true);
                m_zoomContainer->installEventFilter(parent);
                
                auto * const zoomLayout = new QVBoxLayout{m_zoomContainer};
                zoomLayout->addWidget(m_zoomLabel = new QLabel{}, 0, Qt::AlignRight | Qt::AlignTop);
                
                // The visibility of the zoom overlay is handled at two levels:
                // - The zoom label itself is shown or hidden depending on whether the mouse is currently hovering over
                //   an appropriate part of the preview to zoom in on.
                // - The zoom container is shown or hidden depending on the PreviewWidget's zoom visibility setting. 
                //   See showZoom() and hideZoom().
                
                m_zoomLabel->setFrameStyle(QFrame::Plain);
                m_zoomLabel->setFrameShape(QFrame::Box);
                m_zoomLabel->hide();
            }
            
            /**
             * Hides the preview widget's zoom overlay.
             */
            
            void hideZoom() {
                m_zoomContainer->hide();
            }
            
            /**
             * Shows the preview widget's zoom overlay.
             */

            void showZoom() {
                
                auto * const layout = static_cast<QStackedLayout *>(m_previewStack->layout());
                layout->setCurrentWidget(m_zoomContainer);
                m_zoomContainer->show();
            }
            
        private:
            
            static constexpr int MinHeight = 50;  /** The minimum pixel height that this image view can be displayed with. */
            static constexpr int MinWidth  = 50;  /** The minimum pixel width that this image view can be displayed with. */
            
            // The hierarchy of widgets and layouts used to display the image preview is somewhat detailed, in order to
            // correctly contain and align both the preview label (which must have fixed but manually changeable size)
            // and a potential overlay icon.
            
            QLabel * m_iconLabel = nullptr;  /** Label used to display the overlay icon. */
            
            QLabel * m_previewLabel;  /** Label used to display the preview itself. */
            QLabel * m_zoomLabel;     /** Label used to dislpay a zoomed preview of the image. */
            
            QWidget * m_previewStack;   /** Centred widget containing the preview and icon. */
            QWidget * m_zoomContainer;  /** Widget used to properly position the zoom overlay. */
        };
        
        struct ImageView::Private {
            
            /**
             * Initialises the ImageView's private data upon construction.
             * @param q The owner instance of the public class.
             */

            explicit Private(ImageView * const q)
                : q{q},
                  m_detailsWidget{new DetailsWidget{q}},
                  m_previewWidget{new PreviewWidget{q}} {
            }
            
            ImageView * const q;
            
            DetailsWidget * const m_detailsWidget;
            PreviewWidget * const m_previewWidget;
            
            QLabel * m_titleLabel;
        };
        
        ImageView::ImageView(QWidget * const parent)
            : QFrame{parent}, d{std::make_unique<Private>(this)} {
            
            d->m_previewWidget->hideZoom();
            
            auto * const layout = new QVBoxLayout{this};
            layout->addWidget(d->m_titleLabel = new QLabel{}, 0, Qt::AlignHCenter);
            layout->addWidget(d->m_previewWidget, 1);
            layout->addWidget(d->m_detailsWidget, 0, Qt::AlignHCenter);
        }

        ImageView::~ImageView() = default;
    }
}
