#include "videooutput.h"
#include "logging.h"

#include <QMutex>
#include <QOpenGLContext>
#include <QSGSimpleTextureNode>
#include <QThread>
#include <QQuickWindow>

VideoOutput::VideoOutput( QQuickItem *parent ) : QQuickItem( parent ) {
    // Mandatory for our own drawing code to do anything
    setFlag( QQuickItem::ItemHasContents, true );

    emit aspectModeChanged();
    emit aspectRatioChanged();
    emit linearFilteringChanged();
    emit televisionChanged();
    emit ntscChanged();
    emit widescreenChanged();

    size_t newSize = this->format.videoSize.width() * this->format.videoSize.height() * this->format.videoBytesPerPixel;
    framebuffer = new uchar[ newSize ]();
    framebufferSize = newSize;
}

VideoOutput::~VideoOutput() {
    if( framebuffer ) {
        delete framebuffer;
    }
}

// Public

void VideoOutput::setState( Node::State state ) {
    this->state = state;
}

void VideoOutput::setFormat( LibretroVideoFormat format ) {
    // Update the property if the incoming format and related properties define a different aspect ratio than the one stored
    qreal newRatio = calculateAspectRatio( format );

    if( aspectRatio != newRatio || format.videoSize.width() != this->format.videoSize.width() || format.videoSize.height() != this->format.videoSize.height() ) {
        // Pretty-print the old and new aspect ratio (ex. "4:3")
        int oldAspectRatioX = this->format.videoSize.height() * aspectRatio;
        int oldAspectRatioY = this->format.videoSize.width() / aspectRatio;
        int newAspectRatioX = format.videoSize.height() * newRatio;
        int newAspectRatioY = format.videoSize.width() / newRatio;
        int oldGCD = greatestCommonDivisor( oldAspectRatioX, oldAspectRatioY );
        int newGCD = greatestCommonDivisor( newAspectRatioX, newAspectRatioY );
        int oldAspectRatioXInt = oldAspectRatioX / oldGCD;
        int oldAspectRatioYInt = oldAspectRatioY / oldGCD;
        int newAspectRatioXInt = newAspectRatioX / newGCD;
        int newAspectRatioYInt = newAspectRatioY / newGCD;
        qCDebug( phxVideo ).nospace() << "Aspect ratio changed to " << newAspectRatioXInt << ":" << newAspectRatioYInt
                                      << " (was " << oldAspectRatioXInt << ":" << oldAspectRatioYInt << ")" ;

        aspectRatio = newRatio;
        emit aspectRatioChanged();
    }

    // Allocate a new framebuffer if the incoming format defines a larger one than we already have room for
    size_t newSize = format.videoSize.width() * format.videoSize.height() * format.videoBytesPerPixel;

    if( newSize > framebufferSize ) {
        if( framebuffer ) {
            delete framebuffer;
        }

        qCDebug( phxVideo ).nospace() << "Expanded framebuffer to fit new size/format (size = " <<
                                      ( double )newSize / 1024.0 << " KB)";
        framebuffer = new uchar[ newSize ]();
        framebufferSize = newSize;
    }

    this->format = format;
}

void VideoOutput::data( QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) {
    this->mutex = mutex;

    // Copy framebuffer to our own buffer for later drawing
    if( state == Node::State::Playing ) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

        // Discard data that's too far from the past to matter anymore
        if( currentTime - timestamp > 500 ) {
            static qint64 lastMessage = 0;

            if( currentTime - lastMessage > 1000 ) {
                lastMessage = currentTime;
                // qCWarning( phxVideo ) << "Discarding" << bytes << "bytes of old video data from" <<
                //                           currentTime - timestamp << "ms ago";
            }

            return;
        }

        const uchar *newFramebuffer = ( const uchar * )data;

        // Make sure reads and writes to this buffer are atomic
        mutex->lock();

        // Copy framebuffer line by line as the consumer may pack the image with arbitrary garbage data at the end of each line
        for( int i = 0; i < this->format.videoSize.height(); i++ ) {
            // Don't read past the end of the given buffer
            Q_ASSERT( i * this->format.videoBytesPerLine < bytes );

            // Don't write past the end of our framebuffer
            Q_ASSERT( i * this->format.videoSize.width() * this->format.videoBytesPerPixel < framebufferSize );

            memcpy( framebuffer + i * this->format.videoSize.width() * this->format.videoBytesPerPixel,
                    newFramebuffer + i * this->format.videoBytesPerLine,
                    this->format.videoSize.width() * this->format.videoBytesPerPixel
                  );
        }

        mutex->unlock();

        // Schedule a call to updatePaintNode()
        update();
    }
}

void VideoOutput::setTextureID( GLuint textureID ) {
    this->textureID = textureID;
}

void VideoOutput::setAspectMode( int aspectMode ) {
    if( this->aspectMode != aspectMode ) {
        this->aspectMode = aspectMode;
        setFormat( this->format );
    }
}

void VideoOutput::setTelevision( bool television ) {
    if( this->television != television ) {
        this->television = television;
        setFormat( this->format );
    }
}

void VideoOutput::setNtsc( bool ntsc ) {
    if( this->ntsc != ntsc ) {
        this->ntsc = ntsc;
        setFormat( this->format );
    }
}

void VideoOutput::setWidescreen( bool widescreen ) {
    if( this->widescreen != widescreen ) {
        this->widescreen = widescreen;
        setFormat( this->format );
    }
}

void VideoOutput::classBegin() {
    QQuickItem::classBegin();
}

void VideoOutput::componentComplete() {
    QQuickItem::componentComplete();
    // Lock the mutex before we start drawing if in 3D mode
    connect( window(), &QQuickWindow::beforeRendering, this, [ & ]() {
        if( mutex && format.videoMode == HARDWARERENDER ) {
            lockedByUs = true;
            mutex->lock();
            //qDebug() << "VideoOutput lock";
        }
    }, Qt::DirectConnection );

    connect( window(), &QQuickWindow::afterRendering, this, [ & ]() {
        if( mutex && format.videoMode == HARDWARERENDER && lockedByUs ) {
            lockedByUs = false;
            mutex->unlock();
            //qDebug() << "VideoOutput unlock";
        }
    }, Qt::DirectConnection );
}

// Private

QSGNode *VideoOutput::updatePaintNode( QSGNode *storedNode, QQuickItem::UpdatePaintNodeData * ) {
    // Don't draw unless emulation is active
    if( state != Node::State::Playing && state != Node::State::Paused ) {
        // Schedule a call to updatePaintNode() for this Item anyway
        update();
        return 0;
    }

    // Create a new texture node for the renderer if one does not already exist
    QSGSimpleTextureNode *storedTextureNode = static_cast<QSGSimpleTextureNode *>( storedNode );

    if( !storedTextureNode ) {
        storedTextureNode = new QSGSimpleTextureNode();
    }

    // 2D rendering, create a texture from the stored buffer
    if( format.videoMode == SOFTWARERENDER ) {
        // Schedule old texture for deletion
        if( texture ) {
            texture->deleteLater();
            texture = nullptr;
        }

        // Create new Image that holds a reference to our framebuffer
        QImage image( ( const uchar * )framebuffer, format.videoSize.width(), format.videoSize.height(), format.videoPixelFormat );
        // Create a texture via a factory function (framebuffer contents are uploaded to GPU once QSG reads texture node)
        texture = window()->createTextureFromImage( image, QQuickWindow::TextureOwnsGLTexture );
    }

    // 3D rendering, use the stored texture name to render
    else if( textureID != 0 ) {
        if( !texture ) {
            texture = window()->createTextureFromId( textureID, QSize( format.videoSize.width(), format.videoSize.height() ) );
        }

        if( textureID != static_cast<GLuint>( texture->textureId() ) ) {
            delete texture;
            texture = window()->createTextureFromId( textureID, QSize( format.videoSize.width(), format.videoSize.height() ) );
        }
    }

    // Texture was not set yet, don't draw anything
    else {
        return 0;
    }

    // Ensure texture lives in rendering thread so it will be deleted only once it's no longer associated with
    // the texture node
    texture->moveToThread( window()->openglContext()->thread() );

    // Put this new texture into our QSG node and mark the node dirty so it'll be redrawn
    storedTextureNode->setTexture( texture );
    storedTextureNode->setRect( boundingRect() );
    storedTextureNode->setFiltering( linearFiltering ? QSGTexture::Linear : QSGTexture::Nearest );
    storedTextureNode->setTextureCoordinatesTransform(
        format.videoMode == SOFTWARERENDER ?
        QSGSimpleTextureNode::NoTransform :
        QSGSimpleTextureNode::MirrorVertically
    );

    storedTextureNode->markDirty( QSGNode::DirtyMaterial );

    // Schedule a call to updatePaintNode() for this Item for the next frame
    update();

    return storedTextureNode;
}

qreal VideoOutput::calculateAspectRatio( LibretroVideoFormat format ) {
    qreal newRatio = format.videoAspectRatio;

    if( widescreen ) {
        newRatio = 16.0 / 9.0;
    }

    return newRatio;
}

int VideoOutput::greatestCommonDivisor( int m, int n ) {
    int r;

    /* Check For Proper Input */
    if( ( m == 0 ) || ( n == 0 ) ) {
        return 0;
    } else if( ( m < 0 ) || ( n < 0 ) ) {
        return -1;
    }

    do {
        r = m % n;

        if( r == 0 ) {
            break;
        }

        m = n;
        n = r;
    } while( true );

    return n;
}
