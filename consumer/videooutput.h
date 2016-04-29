#pragma once

#include "backendcommon.h"

#include "logging.h"
#include "node.h"

// FIXME: Remove once switched over to pipelinecommon.h
#include "producer.h"

/*
 * VideoOutput is a consumer of video data as provided by Core via CoreControl. As it's a QML item, it will always live
 * in the QML thread.
 */

class VideoOutput : public QQuickItem {
        Q_OBJECT

        Q_PROPERTY( qreal aspectRatio MEMBER aspectRatio NOTIFY aspectRatioChanged )
        Q_PROPERTY( bool linearFiltering MEMBER linearFiltering NOTIFY linearFilteringChanged )
        Q_PROPERTY( bool television MEMBER television WRITE setTelevision NOTIFY televisionChanged )
        Q_PROPERTY( bool ntsc MEMBER ntsc WRITE setNtsc NOTIFY ntscChanged )
        Q_PROPERTY( bool widescreen MEMBER widescreen WRITE setWidescreen NOTIFY widescreenChanged )

    public:
        explicit VideoOutput( QQuickItem *parent = 0 );
        ~VideoOutput();

        void setState( Node::State state );
        void setFormat( ProducerFormat consumerFmt );
        void data( void *data, size_t bytes, qint64 timestamp );

    signals:
        // Properties
        void aspectRatioChanged( qreal aspectRatio );
        void linearFilteringChanged( bool linearFiltering );
        void televisionChanged( bool television );
        void ntscChanged( bool ntsc );
        void widescreenChanged( bool widescreen );

    private:
        // Current state. Used to ignore data that arrives after already being told we're no longer playing
        Node::State state{ Node::State::Stopped };
        ProducerFormat format;

        // The framebuffer that holds the latest frame from Core
        uchar *framebuffer{ nullptr };
        size_t framebufferSize{ 0 };

        // Holds a pointer to the framebuffer via its underlying QImage, used by renderer to upload framebuffer to GPU
        QSGTexture *texture{ nullptr };

        // Called by render thread whenever it's time to render and needs an update from us
        // We'll assign the current texture to the stored node given to us, creating this stored node if it does not
        // already exist
        // Main thread (our thread) is guarantied to be suspended while this function executes, the rest run normally
        // paintData is a pointer to a QSGTransformNode which contains the transformation matrix (unused)
        QSGNode *updatePaintNode( QSGNode *storedNode, UpdatePaintNodeData * ) override;

        // The correct aspect ratio to display this picture in
        qreal aspectRatio{ 1.0 };
        qreal calculateAspectRatio( ProducerFormat format );

        // Linear vs nearest-neighbor filtering
        bool linearFiltering{ false };

        // Controls whether to use analog television mode or not (false = digital TV or handheld games, etc... anything
        // with a square PAR)
        bool television{ false };

        // Do we expect 480 vertical lines (NTSC, true) or 576 vertical lines (PAL, false)?
        // Ignored if television is false
        bool ntsc{ true };

        // Is this standard 4:3 (false) or is the image anamorphic widescreen (16:9 packed into a 4:3 frame, true)?
        // Ignored if television is false
        bool widescreen{ false };

        // Setters for the above 3 properties, will force a recheck of the aspect ratio if any are called
        void setTelevision( bool television );
        void setNtsc( bool ntsc );
        void setWidescreen( bool widescreen );

        // Helper for printing aspect ratios as fractions
        // Source: http://codereview.stackexchange.com/questions/37189/euclids-algorithm-greatest-common-divisor
        int greatestCommonDivisor( int m, int n );
};
