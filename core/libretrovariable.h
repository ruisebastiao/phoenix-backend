#pragma once

#include <QVector>
#include <string>

struct retro_variable;

// Container class for a libretro core variable
class LibretroVariable {
    public:
        LibretroVariable() = default;
        LibretroVariable( const retro_variable *var );
        LibretroVariable( const std::string key );

        virtual ~LibretroVariable() = default;

        const std::string &key() const;

        const std::string &value( const std::string &default_ ) const;
        const std::string &value() const;

        const std::string &description() const;

        const QVector<std::string> &choices() const;

        bool setValue( const std::string &value );
        bool setValue( std::string &&t_value );

        bool isValid() const;

        bool operator ==( const LibretroVariable &t_var1 );

    private:
        // use std::strings instead of QStrings, since the later store data as 16bit chars
        // while cores probably use ASCII/utf-8 internally..
        std::string m_key;
        std::string m_value; // XXX: value should not be modified from the UI while a retro_run() call happens
        std::string m_description;
        QVector<std::string> m_choices;

};

QDebug operator<<( QDebug debug, const LibretroVariable &var );
