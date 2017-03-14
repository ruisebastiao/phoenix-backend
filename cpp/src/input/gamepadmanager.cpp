#include "gamepadmanager.h"
#include "logging.h"
#include "gamepad.h"

#include <SDL_haptic.h>
#include <SDL_joystick.h>

#include <SDL.h>

#include <QCoreApplication>
#include <QThread>

GamepadManager::GamepadManager() :
    m_gamepadListSize( 0 ),
    m_gamepads( 16, nullptr ),
    m_initialized( false )
{

    //putenv( "SDL_VIDEODRIVER=dummy" );
    // Init SDL
    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_VIDEO ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }

    qCDebug( phxInput ) << "SDL2 initialized";

}

GamepadManager::~GamepadManager() {
    qDeleteAll( m_gamepads );

    qCDebug( phxInput ) << "Shutting down SDL2.";

    SDL_Quit();

}

void GamepadManager::poll() {

    // DO NOT REFACTOR THIS INTO USING THE SDL_EVENT SYSTEM!!!!

    const int oldNumJoysticks = SDL_NumJoysticks();
    SDL_GameControllerUpdate();

    if ( SDL_NumJoysticks() > oldNumJoysticks || !m_initialized ) {
        for ( int i=0; i < SDL_NumJoysticks(); ++i ) {
            if ( SDL_TRUE == SDL_IsGameController( i ) ) {
                SDL_GameController *controller = SDL_GameControllerFromInstanceID( i );
                if ( SDL_GameControllerGetAttached( controller ) ) {
                    qCDebug(phxInput) << "is attached";
                } else {

                    Gamepad *gamepad = new Gamepad( i );

                    qCDebug( phxInput, "Gamepad %s was added.", qPrintable( gamepad->name() ) );

                    m_gamepadIndexMap[ gamepad->instanceID() ] = i;

                    m_gamepads[ m_gamepadListSize ] = gamepad;

                     ++m_gamepadListSize;
                }
            }
        }

    } else if ( SDL_NumJoysticks() < oldNumJoysticks) {
        qDebug() << "Joystick Removed...";
        for ( int i=0; i < m_gamepadListSize; ++i ) {
            Gamepad *gamepad = m_gamepads[ i ];
            if ( !gamepad->isAttached() ) {
                qDebug() << "This gamepad is not attached";
                const int index = m_gamepadIndexMap[ gamepad->instanceID() ];
                m_gamepadIndexMap.remove( gamepad->instanceID() );

                delete m_gamepads[ index ];
                m_gamepads[ index ] = nullptr;

                --m_gamepadListSize;

                qDebug() << m_gamepads[ index ];

            }
        }
    }

    for ( int i=0; i < m_gamepadListSize; ++i ) {
        Gamepad *gamepad = m_gamepads[ i ];
        if ( gamepad ) {
            gamepad->updateStates();

            //debugStates( gamepad );
        }
    }

    if ( !m_initialized ) {
        qCDebug( phxInput ) << "Polling for input";
        m_initialized = true;
    }

}

const Gamepad &GamepadManager::at(int t_index) {
    return *m_gamepads[ t_index ];
}

int GamepadManager::size() const {
    return m_gamepadListSize;
}

void GamepadManager::debugStates(Gamepad *t_gamepad ) {
    auto debug = qDebug();
    for ( int i=0; i < SDL_CONTROLLER_BUTTON_MAX; ++i ) {
        debug << t_gamepad->getButtonState( static_cast<SDL_GameControllerButton>( i ) );
    }

    for ( int i=0; i < SDL_CONTROLLER_AXIS_MAX; ++i ) {
        debug << t_gamepad->getAxisState( static_cast<SDL_GameControllerAxis>( i ) );
    }
}
