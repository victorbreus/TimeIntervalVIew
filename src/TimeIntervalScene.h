#pragma once

#include <QGraphicsScene>
#include "TimeDefines.h"
#include "TimeIntervalDatabase.h"

class TimeIntervalScene : public QGraphicsScene
{
    Q_OBJECT
public:
    TimeIntervalScene( QObject* pParent = nullptr );
    void setDatabase( TimeIntervalDatabase* pDatabase );
    void setClusterization( Milliseconds clusterDuration, TimePoint startBound, TimePoint endBound );

    static constexpr double unitsPerHour = 3600.0 * 1000.0;

private:
    void createTimeScale();

private slots:
    void onDatabaseCleared();
    void onDatabasePopulated();

protected:
    void timerEvent( QTimerEvent* pTimerEvent );

private:
    TimeIntervalDatabase* m_pDatabase;
    QGraphicsItemGroup* m_pRecordsItemGroup;
    std::future<std::vector<TimeIntervalCluster>> m_clusterizationFuture;
    int m_clusterizationTimerId;

    Milliseconds m_clusterDuration;
    TimePoint m_startBound;
    TimePoint m_endBound;
};
