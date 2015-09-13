#ifndef MYRIAD_QUEUEITEM_H
#define MYRIAD_QUEUEITEM_H

#include <QStandardItem>
#include <QString>
#include <QVariant>

namespace myriad {
    namespace modelview {
        
        /**
         * Unique codes that can be used to identify the various roles handled by all custom 
         * QStandardItem subclasses used by Myriad (currently just QueueItem, but defined outside of
         * that class for the sake of extensibility).
         */
        
        enum Role {
            PathRole = Qt::UserRole + 0b1
        };
        
        /**
         * Unique codes that can be used by Qt to identify the types of all custom QStandardItem 
         * subclasses used by Myriad (currently just QueueItem, but defined outside of that class
         * for the sake of extensibility).
         */
        
        enum Type {
            QueueItemType = QStandardItem::UserType + 0b1
        };

        /**
         * A class used to encapsulate the items of a QStandardItemModel when it is used to display
         * a collection of files or folders. This enables the file-list to be represented by a
         * single object while still retaining the whole path for each file, but displaying only its
         * name.
         */

        class QueueItem : public QStandardItem {

            public:
                
                /**
                * Constructs a new QueueItem that refers to a specified file or directory.
                * @param path The full filesystem path for this item.
                */
                
                explicit QueueItem(const QString& path);
                
                /**
                * Gets data associated with this item, which may be a full path usable in file
                * operations (if Role::PathRole is specified), a file or directory name only (if
                * Qt::DisplayRole is specified) or an icon (if Qt::DecorationRole is specified).
                * @param role The type of data to obtain.
                * @return The path, name or icon of the file or directory represented by this item.
                */

                QVariant data(int role) const override;
                
                /**
                 * Sets the data associated this item. This must be called with a full path usable
                 * in file operations, as the data associated with other roles of this item (i.e.
                 * its icon and its name) do not provide enough information to fully determine it.
                 * @param value The value to set this QueueItem with.
                 * @param role Must be Role::PathRole; otherwise, no action is taken.
                 */
                
                void setData(const QVariant& value, int role) override;
                
                /**
                * Returns a unique code identifying the type of this QStandardItem. The type codes
                * used by Myriad are enumerated in modelview::Type.
                */
                
                int type() const override;

            private:
                
                /**
                 * Sets the location of the file or directory that this item represents.
                 * @param path A full filesystem path indicating the new value for this item.
                 */
                
                void setPath(const QString& path);
                
                QString m_iconName;
                QString m_name;
                QString m_path;
        };
    }
}

#endif