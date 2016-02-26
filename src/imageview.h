#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <memory>
#include <QFrame>

namespace myriad {
    namespace ui {
        
        /**
        * A widget that groups together a set of labels and an image preview widget, and together uses
        * these to display information about a particular image file.
        */
        
        class ImageView : public QFrame {
        Q_OBJECT
            
        public:
                
            /**
            * Constructs a new ImageInfoView widget in an empty state, with its labels and layout visible but with
            * no image (or associated information) displayed.
            */
            
            explicit ImageView(QWidget * parent = nullptr);
            
            /**
            * Releases resources owned by the ImageView. Note that the ImageView does not itself own any persistent
            * handle to the image it displays.
            */
            
        ~ImageView();
            
        private:
            
            class DetailsWidget;
            class PreviewWidget;
            
            struct Private;
            std::unique_ptr<Private> d;
        };
    }
}

#endif
