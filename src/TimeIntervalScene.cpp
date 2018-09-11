#include "timeintervalscene.h"
#include "GraphicsTimeIntervalItem.h"
#include "GraphicsFixedSizeTextItem.h"
#include "TimeIntervalDatabase.h"

TimeIntervalScene::TimeIntervalScene( QObject* pParent /*= nullptr */ )
    : QGraphicsScene( pParent )
    , m_pDatabase( nullptr )
    , m_pRecordsItemGroup( nullptr )
    , m_clusterizationTimerId( 0 )
{
    setSceneRect( 0, 0.0, 24.0 * unitsPerHour, 80.0 );
    setItemIndexMethod( QGraphicsScene::NoIndex );
    createTimeScale();
}

void TimeIntervalScene::setDatabase( TimeIntervalDatabase* pDatabase )
{
    QObject::connect( pDatabase, &TimeIntervalDatabase::databasePopulated, this, &TimeIntervalScene::onDatabasePopulated );
    QObject::connect( pDatabase, &TimeIntervalDatabase::databaseCleared, this, &TimeIntervalScene::onDatabaseCleared );
    m_pDatabase = pDatabase;
}

void TimeIntervalScene::createTimeScale()
{
    QPen pen;
    pen.setCosmetic( true );
    pen.setWidthF( 2 );
    for ( int i = 0; i < 25; ++i )
    {
        auto item = addLine( unitsPerHour * i, 10.0, unitsPerHour * i, 0.0, pen );
        GraphicsFixedSizeTextItem* textItem = new GraphicsFixedSizeTextItem( QString( "%1h").arg( i ) );
        textItem->setPos( unitsPerHour * i, 20.0 );
        addItem( textItem );
    }
}

void TimeIntervalScene::onDatabaseCleared()
{
    if ( m_pRecordsItemGroup )
    {
        removeItem( m_pRecordsItemGroup );
        delete m_pRecordsItemGroup;
        m_pRecordsItemGroup = nullptr;
    }
}

void TimeIntervalScene::onDatabasePopulated()
{
    if ( m_clusterDuration != Milliseconds() )
    {
        setClusterization( m_clusterDuration, m_startBound, m_endBound );
    }
}

void TimeIntervalScene::timerEvent( QTimerEvent* pTimerEvent )
{
    if ( m_clusterizationFuture.valid() )
    {
        if ( m_clusterizationFuture.wait_for( Milliseconds( 0 ) ) == std::future_status::ready )
        {
            if ( m_pRecordsItemGroup )
            {
                removeItem( m_pRecordsItemGroup );
                delete m_pRecordsItemGroup;
                m_pRecordsItemGroup = nullptr;
            }
            QList<QGraphicsItem*> items;
            auto clusters = m_clusterizationFuture.get();
            for ( const auto& cluster : clusters )
            {
                items.append( new GraphicsTimeIntervalItem( cluster ) );
            }
            m_pRecordsItemGroup = createItemGroup( items );
            m_pRecordsItemGroup->setHandlesChildEvents( false );
            killTimer( m_clusterizationTimerId );
        }
    }
}

void TimeIntervalScene::setClusterization( Milliseconds clusterDuration, TimePoint startBound, TimePoint endBound )
{
    m_clusterDuration = clusterDuration;
    m_startBound = startBound;
    m_endBound = endBound;
    if ( m_pDatabase )
    {
        if ( !m_clusterizationFuture.valid() )
        {
            m_clusterizationFuture = m_pDatabase->getClusterization( startBound, endBound, clusterDuration );
            m_clusterizationTimerId = startTimer( 20 );
        }
    }
}
