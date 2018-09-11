#include "timeintervaldatabase.h"
#include <random>
#include <sstream>
#include <iostream>
#include <map>

TimeIntervalEntry::TimeIntervalEntry( const std::string& caption, TimePoint timestamp, Milliseconds duration )
    : m_caption( caption )
    , m_timestamp( timestamp )
    , m_duration( duration )
{
}

TimeIntervalDatabase::TimeIntervalDatabase( QObject* pParent /*= nullptr */ )
    : QObject( pParent )
    , m_populateCancelFlag( false )
    , m_populateTimerId( 0 )
{
}

TimeIntervalDatabase::~TimeIntervalDatabase()
{
    m_populateCancelFlag = true;
}

std::string TimeIntervalCluster::caption() const
{
    if ( isNull() )
    {
        return std::string();
    }
    if ( m_count == 1 )
    {
        return entryAt( 0 ).caption();
    }
    return std::to_string( m_count );
}

TimePoint TimeIntervalCluster::timestamp() const
{
    if ( isNull() )
    {
        return TimePoint();
    }
    return entryAt( 0 ).timestamp();
}

Milliseconds TimeIntervalCluster::duration() const
{
    return m_duration;
}

TimeIntervalEntries::size_type TimeIntervalCluster::enriesCount() const
{
    return m_count;
}

TimeIntervalEntry TimeIntervalCluster::entryAt( TimeIntervalEntries::size_type index ) const
{
    if ( isNull() )
    {
        assert( false );
        return TimeIntervalEntry( std::string(), TimePoint(), Milliseconds() );
    }
    return m_pEntries->at( m_index + index );
}

bool TimeIntervalCluster::isNull() const
{
    return !m_pEntries || m_count == 0;
}

void TimeIntervalDatabase::populate( TimeIntervalEntries::size_type count )
{
    if ( m_pEntries )
    {
        m_pEntries.reset();
        emit databaseCleared();
    }

    m_populateFuture = std::async( std::launch::async, &TimeIntervalDatabase::populateTask, this, count, std::ref( m_populateCancelFlag ) ); 
    m_populateTimerId = startTimer( 100 );
}

std::future<std::vector<TimeIntervalCluster>> TimeIntervalDatabase::getClusterization( TimePoint startTime, TimePoint endTime, Milliseconds groupDuration )
{
    return std::async( std::launch::async, &TimeIntervalDatabase::clusterizationTask, this, startTime, endTime, groupDuration, m_pEntries );
}

void TimeIntervalDatabase::timerEvent( QTimerEvent* timerEvent )
{
    if ( m_populateFuture.valid() )
    {
        if ( m_populateFuture.wait_for( Milliseconds( 10 ) ) == std::future_status::ready )
        {
            m_pEntries = m_populateFuture.get();
            killTimer( m_populateTimerId );
            m_populateTimerId = 0;
            m_populateFuture = std::future<std::shared_ptr<TimeIntervalEntries>>();
            emit databasePopulated();
        }
    }
}

TimeIntervalEntriesPtr TimeIntervalDatabase::populateTask( TimeIntervalEntries::size_type count, const std::atomic_bool& cancelFlag )
{
    std::random_device rd;
    std::default_random_engine gen( rd() );

    const int peakHoursCount = 4;
    std::uniform_real_distribution<> peakHoursDistribution( 1.0, 23.0 );
    std::vector<std::normal_distribution<double>> timestampDistributions;
    for ( int i = 0; i < peakHoursCount; ++i )
    {
        timestampDistributions.push_back( std::normal_distribution<double>( peakHoursDistribution( gen ), 1.0 ) );
    }
    std::exponential_distribution<double> durationDistr( 5.0 );

    std::vector<TimePoint> timestamps;
    timestamps.reserve( count );
    int generatedTimeStamps = 0;
    while ( generatedTimeStamps != count )
    {
        double generatedValue = timestampDistributions.at( generatedTimeStamps % peakHoursCount )( gen );
        if ( generatedValue > 0.0 & generatedValue < 24.0 )
        {
            timestamps.emplace_back( std::chrono::duration_cast<Milliseconds>( HoursF( generatedValue ) ) );
            ++generatedTimeStamps;
        }
    }
    if ( cancelFlag )
    {
        TimeIntervalEntriesPtr();
    }

    std::sort( timestamps.begin(), timestamps.end() );
    auto res = std::make_shared<TimeIntervalEntries>( TimeIntervalEntries() );
    res->reserve( timestamps.size() );

    for ( TimeIntervalEntries::size_type i = 0; i < count; ++i )
    {
        if ( cancelFlag )
        {
            return TimeIntervalEntriesPtr();
        }
        res->emplace_back( "Rec " + std::to_string( i ), timestamps.at( i ), std::chrono::duration_cast<Milliseconds>( HoursF( std::min( durationDistr( gen ) / 10.0, 3.0 ) ) ) );
    }

    return res;
}

std::vector<TimeIntervalCluster> TimeIntervalDatabase::clusterizationTask( TimePoint startTime, TimePoint endTime, Milliseconds groupDuration, TimeIntervalEntriesPtr pEntries )
{
    if ( !pEntries )
    {
        return std::vector<TimeIntervalCluster>();
    }
    std::vector<std::pair<TimeIntervalCluster, std::future<TimeIntervalEntries::const_iterator>>> calculatingClusters;

    auto iBoundStart = std::lower_bound( pEntries->cbegin(), pEntries->cend(), TimeIntervalEntry( std::string(), startTime, Milliseconds( 0 ) ),
        []( const auto& l, const auto& r ) { return l.m_timestamp < r.m_timestamp; } );

    auto iBoundEnd = std::lower_bound( pEntries->cbegin(), pEntries->cend(), TimeIntervalEntry( std::string(), endTime, Milliseconds( 0 ) ),
        []( const auto& l, const auto& r ) { return l.m_timestamp < r.m_timestamp; } );

    auto iClusterStart = iBoundStart;
    while ( iClusterStart != pEntries->cend() && iClusterStart < iBoundEnd )
    {
        auto iClusterEnd = std::lower_bound( iClusterStart, iBoundEnd, TimeIntervalEntry( std::string(), iClusterStart->m_timestamp + groupDuration, Milliseconds( 0 ) ),
            []( const auto& l, const auto& r ) { return l.m_timestamp < r.m_timestamp; } );

        TimeIntervalCluster cluster;
        cluster.m_index = iClusterStart - pEntries->begin();
        cluster.m_count = iClusterEnd - iClusterStart;
        cluster.m_duration = Milliseconds( 0 );
        cluster.m_pEntries = pEntries;

        calculatingClusters.push_back( std::make_pair( cluster,
            std::async( std::launch::async
                , &std::max_element<TimeIntervalEntries::const_iterator, bool( const TimeIntervalEntry&, const TimeIntervalEntry& )>
                , pEntries->cbegin()
                + cluster.m_index
                , pEntries->cbegin() + cluster.m_index + cluster.m_count
                , []( const auto& l, const auto& r ) { return ( l.m_timestamp + l.m_duration ) < ( r.m_timestamp + r.m_duration ); } ) ) );

        iClusterStart = iClusterEnd;
    }
    std::vector<TimeIntervalCluster> result;
    for ( auto& calculatingCluster : calculatingClusters )
    {
        auto i = calculatingCluster.second.get();
        calculatingCluster.first.m_duration = std::chrono::duration_cast<Milliseconds>( i->m_timestamp - calculatingCluster.first.timestamp() + i->m_duration );
        result.push_back( calculatingCluster.first );
    }

    return result;
}
