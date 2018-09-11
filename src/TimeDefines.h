#pragma once

#include <chrono>
#include <QMetaType>

typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
typedef std::chrono::milliseconds Milliseconds;
typedef std::chrono::duration<qreal, std::ratio<3600>> HoursF;

Q_DECLARE_METATYPE( TimePoint );
Q_DECLARE_METATYPE( Milliseconds );
