#ifndef MYRIAD_IMAGE_H
#define MYRIAD_IMAGE_H

#include <QByteArray>
#include <QList>
#include <QString>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include <pHash.h>
#pragma GCC diagnostic pop

namespace myriad {
    namespace processing {
        
        /**
         * Encapsulates Myriad's internal representation of the images it processes, storing
         * whatever data are needed to compare and appraise them.
         */
        
        class Image {
            
            public:
                
                /**
                 * Codes to be used in identifying the file format of an image. Certain file formats
                 * are relevant in Myriad's processing logic (for example, in suggesting a
                 * resolution for a duplicate pair) but others are not -- thus, the formats
                 * enumerated here are far from an exhaustive list of all the formats supported, and
                 * we provide an @c Other code accordingly.
                 */
                
                enum class Format {
                    BMP,
                    GIF,
                    JPEG,
                    PNG,
                    OTHER
                };
                
                /**
                 * Constructs a new image by reading relevant information from the corresponding
                 * file on disk and generating the perceptual hash that will be used to compare it
                 * with other images.
                 * @param path The filesystem path to the image file on disk.
                 */
                
                explicit Image(const QString& path);
                
                /**
                 * Compares perceptual hashes to determine whether this image is a visual (but not
                 * necessarily byte-wise) duplicate of another.
                 * @param rhs The image to compare to.
                 * @return The difference between this image and @p rhs, which is a floating-point
                 * number between @c 0.0 (if the images are the same) and @c 1.0.
                 */
                
                float difference(const Image& rhs) const;
                
                /**
                 * Gets the size of the file that the image was loaded from, in bytes.
                 * @return The image's file size.
                 */
                
                qint64 fileSize() const;
                
                /**
                 * Gets the height of the image, in pixels.
                 * @return The image's height.
                 */
                
                int height() const;
                
                /**
                 * Compares checksums to determine whether this image is a byte-wise identical
                 * duplicate of another.
                 * @param rhs The image to compare to.
                 * @return @c true if both images have the same checksum; @c false otherwise.
                 */
                
                bool isIdenticalTo(const Image& rhs) const;
                
                /**
                 * Gets the full filesystem path that the image was loaded from.
                 * @return The path to the image's file on disk.
                 */
                
                QString path() const;
                
                /**
                 * Gets the width of the image, in pixels.
                 * @return The image's width.
                 */
                
                int width() const;
            
            private:
                
                qint64 m_fileSize  = 0;
                Format m_format    = Format::OTHER;
                ulong64 m_hash     = 0;
                quint16 m_checksum = 0;
                
                int m_width  = 0;
                int m_height = 0;
                
                QString m_path;
        };
        
        /**
         * Gets a list of all image MIME types supported by Myriad.
         * @return The supported MIME type list.
         */
        
        QList<QByteArray> supportedMimeTypes();
    }
}

#endif
