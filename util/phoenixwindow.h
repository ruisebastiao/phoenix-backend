/*
 * Copyright © 2016 athairus
 *
 * This file is part of Phoenix.
 *
 * Phoenix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * PhoenixWindow extends QQuickWindow by allowing the user to toggle whether the window uses VSync (blocking on
 * swapBuffers() until VBlank ends) or not (immediately returning when calling swapBuffers()).
 */

#pragma once

#include <QObject>
#include <QOpenGLContext>
#include <QQuickWindow>

// A companion class that will run on whatever thread the scene graph runs on
class SceneGraphHelper : public QObject {
        Q_OBJECT

    public:
        explicit SceneGraphHelper() = default;

    public slots:
        void setVSync( QQuickWindow *window, QOpenGLContext *context, bool vsync );
};

class PhoenixWindow : public QQuickWindow {
        Q_OBJECT

    public:
        explicit PhoenixWindow( QQuickWindow *parent = 0 );
        ~PhoenixWindow();
        bool vsync{ true };

    signals:
        void vsyncChanged( bool vsync );

    public slots:
        void setVsync( bool vsync );

    private:
        bool sceneGraphIsInitialized { false };
        SceneGraphHelper *sceneGraphHelper { nullptr };
};