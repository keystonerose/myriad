#ifndef MYRIAD_IMAGEINFO_H
#define MYRIAD_IMAGEINFO_H

#include <memory>

#include <QByteArray>
#include <QList>

class QString;

namespace myriad {
    namespace processing {
        
        /**
         * Encapsulates Myriad's internal representation of the images it processes, storing whatever data are needed to
         * compare and appraise them. ImageInfo objects may exist in an uninitialised state if they are constructed
         * without a path and have not yet had their read() method called, and can be cheaply copied and moved: this 
         * makes them ideal for storage by value within Qt's containers.
         */
        
        class ImageInfo {
        
        public:
            
            /**
             * Codes to be used in identifying the file format of an image. Certain file formats are relevant in
             * Myriad's processing logic (for example, in suggesting a resolution for a duplicate pair) but others are
             * not -- thus, the formats enumerated here are far from an exhaustive list of all the formats supported,
             * and we provide an @c OTHER code accordingly.
             */
            
            enum class Format {
                BMP,
                GIF,
                JPEG,
                PNG,
                OTHER
            };
            
            /**
             * Compares perceptual hashes to determine how visually similar two images are to each other. If either of
             * the ImageInfo objects provided is missing hash information (probably because it hasn't yet been loaded,
             * but possibly also because the hashing failed for whatever reason), a maximal difference value is
             * returned.
             * @return The visual difference between the images described by @p lhs and @p rhs, as a floating-point
             * number between @c 0.0 (if the images are the same) and @c 1.0 (if they are maximally different).
             */
            
            static float difference(const ImageInfo& lhs, const ImageInfo& rhs);
            
            /**
             * Compares checksums to determine whether the image files represented by two ImageInfo objects are
             * byte-wise identical duplicates of each other. If either of these objects is uninitialised, @c false is
             * returned.
             * @return @c true if the images described by @p lhs and @p rhs both have the same checksum; @c false
             * otherwise.
             */
            
            bool identical(const ImageInfo& lhs, const ImageInfo& rhs);
            
            /**
             * Constructs a new ImageInfo object, which will be in an uninitialised state (i.e. calls to isValid() will
             * return @c false) until read() is called. 
             */
            
            ImageInfo();
            
            /**
             * Constructs a new ImageInfo object by reading relevant information from the corresponding file on disk and
             * generating the perceptual hash that will be used to compare the image it describes it with other images.
             * @param path The path to the image file on disk that this ImageInfo object should describe.
             */
            
            explicit ImageInfo(const QString& path);
            
            ~ImageInfo();
            
            /**
             * Gets the size of the image file on disk that this ImageInfo object was read from, in bytes. Returns @c 0
             * if the object is in an uninitialised state.
             */
            
            qint64 fileSize() const;
            
            /**
             * Tests whether the ImageInfo object has been initialised via a call to read(), whether by calling that
             * method directly or by invoking the ImageInfo(const QString&) constructor.
             */
            
            bool isValid() const;
            
            /**
             * Gets the height of the image described by this ImageInfo object, in pixels. Returns @c 0 if the object is
             * in an uninitialised state.
             */
            
            int height() const;
            
            /**
             * Populates this ImageInfo object by reading relevant information about a specified image file on disk and
             * generating the perceptual hash that will be used to compare it with other images.
             * @param path The path to the image file on disk that this ImageInfo object should describe.
             */
            
            void read(const QString& path);
            
            /**
             * Gets the width of the image described by this ImageInfo object, in pixels. Returns @c 0 if the object is
             * in an uninitialised state.
             */
            
            int width() const;
        
        private:
            
            /**
             * Tests whether perceptual hash information has been generated for the ImageInfo object. Typically, hash
             * information will be available after read() has been called (either directly or by invoking the
             * ImageInfo(const QString&) constructor), but this could also return false if this call was unable to read
             * the image file from disk for whatever reason.
             */
            
            bool hasHash() const;
            
            // Note that we refer to the image data via a shared_ptr rather than the the more typical unique_ptr:
            // this is because we need ImageInfo objects to be copyable, and shared_ptrs automatically provide nice
            // reference-counted copy semantics for us.
            
            struct Data;
            std::shared_ptr<Data> m_data = nullptr;
        };
        
        /**
         * Gets a list of all image MIME types supported by Myriad.
         * @return The supported MIME type list.
         */
        
        QList<QByteArray> supportedMimeTypes();
    }
}

#endif
