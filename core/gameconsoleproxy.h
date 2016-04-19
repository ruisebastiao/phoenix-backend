#pragma once

#include "backendcommon.h"

#include "controlhelper.h"

/*
 * GameConsoleProxy is a property proxy around its underlying GameConsole instance. Besides the many signals and slots
 * of the proxy system, all this class does is instantiate an instance of GameConsole and put it in its own thread.
 *
 * GameConsoleProxy is meant to be instantiated in your QML code.
 *
 * See GameConsole.h to see how to properly operate GameConsole.
 *
 * How the proxy system works:
 *   Writes via the GameConsoleProxy::set___() slot (step 1) cause a GameConsoleProxy::___changedProxy() (step 2) signal
 *   to be emitted, which is then handled by a GameConsole::set___() slot. GameConsole emits a GameConsole::___changed()
 *   signal (step 3) which we have connected to a GameConsoleProxy::set___Proxy() slot (step 4), which finally assigns
 *   the proxied value internally and emits a GameConsoleProxy___changed() signal to notify anyone concerned.
 *
 *   In total, per property, there are 2 GameConsoleProxy signals, 2 GameConsoleProxy slots, 1 GameConsole signal and
 *   1 GameConsole slot.
 *
 *   If the property is meant to be read-only, (step 1) and (step 2) are not needed.
 *
 * To add a new proxied property:
 *
 * - In GameConsole:
 *   - In GameConsole, set up a notifier signal and a setter slot along with a member
 *   - (Optional) If it's meant to go to some object like Core, in GameConsole set up a forwarder signal and make sure
 *     it gets registered inside GameConsole::connectCoreForwarder();
 *
 * - In GameConsoleProxy:
 *   - Set up an identical member here along with a getter, register with GameConsoleProxy::notifyAllProperties()
 *   - Set up 1 or 2 signals in GameConsoleProxy: A changed notifier (writable only) and a proxied change notifier
 *   - Set up 1 or 2 slots in GameConsoleProxy: A setter (writable only) and a proxied setter
 *   - If necessary, you can juggle two types for your property: one that's QML-friendly (ex. QVariantMap) and one
 *     that's more GameConsole/C++ friendly (ex. QStringMap)
 *     - Anything that touches QML gets the QML-friendly type (including the property, local member and getter)
 *     - Anything that goes to GameConsole gets the C++-friendly type (signals and slots that involve GameConsole)
 *     - Convert as needed
 *
 * To add a new forwarded method:
 *
 * - Add a slot to GameConsole
 * - Add a new public Q_INVOKABLE public method here, have it emit a forwarder signal, register in:
 *     GameConsoleProxy::connectGameConsoleProxy();
 * - (Optional) In GameConsole, set up a forwarder signal and register the forwarder inside :
 *     GameConsole::connectCoreForwarder();
 */

#include "videooutput.h"
#include "gamepadmanager.h"

#include <QQuickItem>
#include <QQmlParserStatus>

class QCoreApplication;
class GameConsole;
class QThread;

class GameConsoleProxy : public QObject, public QQmlParserStatus {
        Q_OBJECT
        Q_INTERFACES( QQmlParserStatus)

        // Settings

        // Producers and consumers that live in QML (and the QML thread)

        // Needed by: "libretro"
        Q_PROPERTY( VideoOutput *videoOutput READ getVideoOutput WRITE setVideoOutput NOTIFY videoOutputChanged )

        // Proxy
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( QVariant src READ src WRITE setSrc NOTIFY srcChanged)

        Q_PROPERTY( ControlHelper::State state READ getState NOTIFY stateChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )

        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )
        Q_PROPERTY( bool pausable READ getPausable NOTIFY pausableChanged )
        Q_PROPERTY( bool resettable READ getResettable NOTIFY resettableChanged )
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY rewindableChanged )

        Q_PROPERTY( QList<const Gamepad *> gamepadsConnected READ gamepadsConnected NOTIFY gamepadsConnectedChanged)
        Q_PROPERTY( const Gamepad * gamepadAdded READ gamepadAdded NOTIFY gamepadAddedChanged)



    public:
        explicit GameConsoleProxy( QObject *parent = 0 );
        ~GameConsoleProxy() = default;

        // May be called from QML
        Q_INVOKABLE void load();
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void reset();

        void componentComplete() override;
        void classBegin() override;

        QVariant src() const;

        void setSrc( QVariant _src );


        QList<const Gamepad *> gamepadsConnected() const
        {
            return m_gamepadsConnected;
        }

        const Gamepad *gamepadAdded() const
        {
            return m_gamepadAdded;
        }

    signals:
        void gamepadsConnectedChanged();
        void srcChanged( QVariant );
        void gamepadAddedChanged();

        // Tell GameConsole to clean up
        void shutdown();

        // GameConsole property proxy (Step 4: to anywhere)
        // These acknowledge that GameConsole's state has been changed
        void videoOutputChanged( VideoOutput *videoOutput );
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QVariantMap source );
        void stateChanged( ControlHelper::State state );
        void volumeChanged( qreal volume );
        void vsyncChanged( bool vsync );

        // GameConsole property proxy (Step 2: to GameConsole)
        // These tell GameConsole to change its state
        void videoOutputChangedProxy( VideoOutput *videoOutput );
        void playbackSpeedChangedProxy( qreal playbackSpeed );
        void volumeChangedProxy( qreal volume );
        void vsyncChangedProxy( bool vsync );

    private slots:
        // GameConsole property proxy (Step 1: from anywhere)
        // These change GameConsole's state ...once it gets around to it
        void setVideoOutput( VideoOutput *videoOutput );
        void setPlaybackSpeed( qreal playbackSpeed );
        void setSource( QVariantMap sourceQVariantMap );
        void setVolume( qreal volume );
        void setVsync( bool vsync );

        // GameConsole property proxy (Step 3: from GameConsole)
        // These tell us that GameConsole has acknowledged our request and changed state
        void setVideoOutputProxy( VideoOutput *videoOutput );
        void setPausableProxy( bool pausable );
        void setPlaybackSpeedProxy( qreal playbackSpeed );
        void setResettableProxy( bool resettable );
        void setRewindableProxy( bool rewindable );
        void setStateProxy( Control::State state );
        void setVolumeProxy( qreal volume );
        void setVsyncProxy( bool vsync );

    private:
        Gamepad *m_gamepadAdded;
        QList<const Gamepad *> m_gamepadsConnected;
        GameConsole *m_gameConsole;
        QThread *gameThread;

        // GameConsole property proxy
        void notifyAllProperties();
        void connectGameConsoleProxy();
        VideoOutput *videoOutput;

        QVariant m_src;

        ControlHelper::State state{ ControlHelper::STOPPED };

        qreal playbackSpeed{ 1.0 };
        qreal volume{ 1.0 };

        bool vsync{ false };
        bool resettable{ false };
        bool rewindable{ false };
        bool pausable{ false };


        VideoOutput *getVideoOutput() const;
        bool getPausable() const;
        qreal getPlaybackSpeed() const;
        bool getResettable() const;
        bool getRewindable() const;
        QVariantMap getSource() const;
        ControlHelper::State getState() const;
        qreal getVolume() const;
        bool getVsync() const;
};

Q_DECLARE_METATYPE( QList<const Gamepad *> )
Q_DECLARE_METATYPE( const Gamepad * )
