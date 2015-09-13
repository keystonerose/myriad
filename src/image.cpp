#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QString>
#include <QTemporaryFile>

#include "image.h"

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
            
            Image::Format formatFromMimeName(const QString& mimeName) {
                
                static const QHash<QString, Image::Format> formatsByMimeName{
                    {"image/bmp",  Image::Format::BMP},
                    {"image/gif",  Image::Format::GIF},
                    {"image/jpeg", Image::Format::JPEG},
                    {"image/png",  Image::Format::PNG}
                };
                
                return formatsByMimeName.contains(mimeName) ? formatsByMimeName[mimeName] : Image::Format::OTHER;
            }
            
           /**
            * Determines a code for the image's file format by reading the MIME type of the file
            * specified by @p path.
            * @param path The filesystem path of the image to find the file format of.
            * @return A code describing the image's file format.
            */

            Image::Format formatFromPath(const QString& path) {
                
                QMimeDatabase mimeDb;
                const auto mimeName = mimeDb.mimeTypeForFile(path).name();
                return formatFromMimeName(mimeName);
            }
        }
        
        Image::Image(const QString& path)
            : m_path{path} {
            
            const QFileInfo fileInfo{path};
            const QImage image{path};
            
            m_checksum = checksumFromPath(path);
            m_fileSize = fileInfo.size();
            m_format   = formatFromPath(path);
            
            m_width  = image.width();
            m_height = image.height();
            
            // The pHash library only supports JPEG and BMP files, so if the image is not already in
            // one of those formats, a temporary bitmap file is created and used to calculate the
            // hash value.
            
            if (m_format == Format::JPEG || m_format == Format::BMP) {
                ph_dct_imagehash(path.toLatin1(), m_hash);
            }
            else {
                
                QTemporaryFile tempBmp{QDir::tempPath() + QLatin1String("/myriad_XXXXXX.bmp")};
                if (tempBmp.open()) {
                    
                    image.save(&tempBmp, "BMP");
                    tempBmp.close();
                    
                    const auto tempPath = QFileInfo{tempBmp}.filePath();
                    ph_dct_imagehash(tempPath.toLatin1(), m_hash);
                }
                else {
                    //TODO: Add some error handling
                }
            }
        }
        
        float Image::difference(const Image& rhs) const {
            return ph_hamming_distance(m_hash, rhs.m_hash) / 64.0;
        }
        
        qint64 Image::fileSize() const {
            return m_fileSize;
        }
        
        int Image::height() const {
            return m_height;
        }
        
        bool Image::isIdenticalTo(const Image& rhs) const {
            return m_checksum == rhs.m_checksum;
        }
        
        QString Image::path() const {
            return m_path;
        }

        int Image::width() const {
            return m_width;
        }
        
        QList<QByteArray> supportedMimeTypes() {
            return QImageReader::supportedMimeTypes();
        }
    }
}
