#pragma once

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <future>
#include "TimeDefines.h"

class TimeIntervalEntry
{
    friend class TimeIntervalDatabase;
public:
    TimeIntervalEntry( const std::string& caption, TimePoint timestamp, Milliseconds duration );
    std::string caption() const { return m_caption; };
    TimePoint timestamp() const { return m_timestamp; };
    Milliseconds duration() const { return m_duration; };

private:
    std::string     m_caption;
    TimePoint       m_timestamp;
    Milliseconds    m_duration;
};

typedef std::vector<TimeIntervalEntry> TimeIntervalEntries;
typedef std::shared_ptr<TimeIntervalEntries> TimeIntervalEntriesPtr;

class TimeIntervalCluster
{
    friend class TimeIntervalDatabase;
public:
    std::string caption() const;
    TimePoint timestamp() const;
    Milliseconds duration() const;
    TimeIntervalEntries::size_type enriesCount() const;
    TimeIntervalEntry entryAt( TimeIntervalEntries::size_type index ) const;
    bool isNull() const;

private:
    TimeIntervalEntries::size_type m_index;
    TimeIntervalEntries::size_type m_count;
    Milliseconds m_duration;
    TimeIntervalEntriesPtr m_pEntries;
};

typedef std::vector<TimeIntervalCluster> TimeIntervalClusters;

class TimeIntervalDatabase : public QObject
{
    Q_OBJECT
public:
    TimeIntervalDatabase( QObject* pParent = nullptr );
    ~TimeIntervalDatabase();
    void populate( TimeIntervalEntries::size_type count );
    std::future<TimeIntervalClusters> getClusterization( TimePoint startTime, TimePoint endTime, Milliseconds groupDuration );

signals:
    void databasePopulated();
    void databaseCleared();

protected:
    void timerEvent( QTimerEvent* pTimerEvent );

private:
    TimeIntervalEntriesPtr populateTask( TimeIntervalEntries::size_type count, const std::atomic_bool& cancelFlag );
    TimeIntervalClusters clusterizationTask( TimePoint startTime, TimePoint endTime, Milliseconds groupDuration, TimeIntervalEntriesPtr pEntries );

private:
    TimeIntervalEntriesPtr m_pEntries;
    std::future<TimeIntervalEntriesPtr> m_populateFuture;
    std::atomic_bool m_populateCancelFlag;
    int m_populateTimerId;
};
