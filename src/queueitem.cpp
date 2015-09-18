#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QVariant>

#include "queueitem.h"

namespace myriad {
    namespace modelview {
        
        namespace {
            
            /**
             * Determines the MIME type of the resource indicated by the specified path, and returns the name of the
             * icon associated with that MIME type.
             * @param path The filesystem path indicating the resource to find the icon name for.
             * @return The name of the icon for the resource at @p path.
             */
            
            QString iconNameFromPath(const QString& path) {
                
                QMimeDatabase mimeDb;
                return mimeDb.mimeTypeForFile(path).iconName();
            }
            
            /**
             * Gets the file or directory name portion of a full filesystem path.
             * @param path The full path to extract the name from.
             */
            
            QString nameFromPath(const QString& path) {
                
                const QFileInfo fileInfo{path};
                return fileInfo.fileName();
            }
        }
        
        QueueItem::QueueItem(const QString& path) {
            setPath(path);
        }

        QVariant QueueItem::data(const int role) const {
            switch (role) {
                
                case Qt::DecorationRole:
                    return QIcon::fromTheme(m_iconName);

                case Qt::DisplayRole:
                    return m_name;
                    
                case PathRole:
                    return m_path;
                    
                default:
                    return QStandardItem::data(role);
            }
        }

        void QueueItem::setData(const QVariant& value, const int role) {
            
            // Only the full URL of the resource indicated by this items gives us enough information to determine the
            // data associated with all of its roles (namely icon, filename and URL); accordingly, we do nothing if
            // setData() is not being called with the UrlRole role.
            
            if (role != PathRole) {
                return;
            }
            
            const auto path = value.toString();
            if (!path.isEmpty()) {

                setPath(path);
                emitDataChanged();
            }
        }

        void QueueItem::setPath(const QString& path) {

            m_iconName = iconNameFromPath(path);
            m_name = nameFromPath(path);
            m_path = path;
        }

        int QueueItem::type() const {
            return QueueItemType;
        }
    }
}
