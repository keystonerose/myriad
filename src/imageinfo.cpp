#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QString>
#include <QTemporaryFile>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include <pHash.h>
#pragma GCC diagnostic pop

#include "imageinfo.h"

namespace myriad {
    namespace processing {
        
        namespace {
            
            /**
             * Generates a checksum from the contents of the file at a specified path. 
             * @param path The filesystem path of the file to check.
             * @return A checksum of the file at @p path, or @c 0 if this file could not be opened.
             */
            
            quint16 checksumFromPath(const QString& path) {
                
                QFile file{path};
                if (!file.open(QIODevice::ReadOnly)) {
                    return 0;
                }
                
                const auto rawData  = file.readAll();
                const auto checksum = qChecksum(rawData, rawData.length());
                
                file.close();
                return checksum;
            }
            
            /**
             * Determines the image format code that corresponds to a named MIME type.
             * @param mimeName The name of the MIME type to find the corresponding format code for.
             * @return The format code corresponding to @p mimeName.
             */
            
            ImageInfo::Format formatFromMimeName(const QString& mimeName) {
                
                static const QHash<QString, ImageInfo::Format> formatsByMimeName{
                    {"image/bmp",  ImageInfo::Format::Bmp},
                    {"image/gif",  ImageInfo::Format::Gif},
                    {"image/jpeg", ImageInfo::Format::Jpeg},
                    {"image/png",  ImageInfo::Format::Png}
                };
                
                return formatsByMimeName.contains(mimeName) ? formatsByMimeName[mimeName] : ImageInfo::Format::Other;
            }
            
           /**
            * Determines a code for the image's file format by reading the MIME type of the file
            * specified by @p path.
            * @param path The filesystem path of the image to find the file format of.
            * @return A code describing the image's file format.
            */

            ImageInfo::Format formatFromPath(const QString& path) {
                
                QMimeDatabase mimeDb;
                const auto mimeName = mimeDb.mimeTypeForFile(path).name();
                return formatFromMimeName(mimeName);
            }
        }
        
        struct ImageInfo::Data {
            
            qint64 fileSize  = 0;
            Format format    = Format::Other;
            ulong64 hash     = 0;
            quint16 checksum = 0;
            
            int width  = 0;
            int height = 0;
        };
        
        // Even though we don't take any action within the default constructor or the destructor, it is necessary to
        // define these methods explicitly here. If we don't, the compiler provides inline versions in the header file,
        // at which point the ImageInfo::Data type used by the m_data pointer is incomplete.
        
        ImageInfo::ImageInfo()  = default;
        ImageInfo::~ImageInfo() = default;
        
        ImageInfo::ImageInfo(const QString& path) {
            read(path);
        }
        
        float ImageInfo::difference(const ImageInfo& lhs, const ImageInfo& rhs) {
            
            if (lhs.hasHash() && rhs.hasHash()) {
                return ph_hamming_distance(lhs.m_data->hash, rhs.m_data->hash) / 64.0;
            }
            else {
                return 1.0;
            }
        }
        
        qint64 ImageInfo::fileSize() const {
            return isNull() ? 0 : m_data->fileSize;
        }
        
        bool ImageInfo::hasHash() const {
            return !isNull() && m_data->hash != 0;
        }
        
        int ImageInfo::height() const {
            return isNull() ? 0 : m_data->height;
        }
        
        bool ImageInfo::identical(const ImageInfo& lhs, const ImageInfo& rhs) {
            
            if (!lhs.isNull() && !rhs.isNull()) {
                return lhs.m_data->checksum == rhs.m_data->checksum;
            }
            else {
                return false;
            }
        }
        
        bool ImageInfo::isNull() const {
            return m_data == nullptr;
        }
        
        void ImageInfo::read(const QString& path) {
            
            m_data = std::make_shared<Data>();

            const QFileInfo fileInfo{path};
            const QImage image{path};
            
            m_data->checksum = checksumFromPath(path);
            m_data->fileSize = fileInfo.size();
            m_data->format   = formatFromPath(path);
            
            m_data->width  = image.width();
            m_data->height = image.height();
            
            // The pHash library only supports JPEG and BMP files, so if the image is not already in one of those
            // formats, a temporary bitmap file is created and used to calculate the hash value.
            
            if (m_data->format == Format::Jpeg || m_data->format == Format::Bmp) {
                ph_dct_imagehash(path.toLatin1(), m_data->hash);
            }
            else {
                
                QTemporaryFile tempBmp{QDir::tempPath() + QLatin1String("/myriad_XXXXXX.bmp")};
                if (tempBmp.open()) {
                    
                    image.save(&tempBmp, "BMP");
                    tempBmp.close();
                    
                    const auto tempPath = QFileInfo{tempBmp}.filePath();
                    ph_dct_imagehash(tempPath.toLatin1(), m_data->hash);
                }
            }
        }
        
        void ImageInfo::setNull() {
            m_data = nullptr;
        }

        int ImageInfo::width() const {
            return isNull() ? 0 : m_data->width;
        }
        
        QList<QByteArray> supportedMimeTypes() {
            return QImageReader::supportedMimeTypes();
        }
    }
}
