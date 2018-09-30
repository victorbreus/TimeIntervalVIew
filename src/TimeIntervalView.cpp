#include "TimeIntervalView.h"
#include "TimeIntervalScene.h"
#include <QWheelEvent>
#include <QGuiApplication>
#include <QScreen>
#include <cmath>

TimeIntervalView::TimeIntervalView( QWidget* pParent )
    : QGraphicsView( pParent )
    , m_pZoomAnimationTimer( new QTimer( this ) )
    , m_zoomAccumulator( 0.0 )
    , m_zoomAccumulatorStep( 0.0 )
    , m_zoomSteadyPointRatio( 0.5 )
{
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    QScroller::grabGesture( viewport(), QScroller::LeftMouseButtonGesture );
    QScrollerProperties scrollerProperties = QScroller::scroller( viewport() )->scrollerProperties();
    scrollerProperties.setScrollMetric( QScrollerProperties::DecelerationFactor, 1.0 );
    QScroller::scroller( viewport() )->setScrollerProperties( scrollerProperties );
    QObject::connect( QScroller::scroller( viewport() ), &QScroller::stateChanged, this, &TimeIntervalView::onScrollerStateChanged );
    QObject::connect( m_pZoomAnimationTimer, &QTimer::timeout, this, &TimeIntervalView::onZoomAnimationTimer );
    m_pZoomAnimationTimer->setInterval( 20 );
}

TimeIntervalScene* TimeIntervalView::intervalScene() const
{
    return qobject_cast<TimeIntervalScene*>( scene() );
}

Milliseconds TimeIntervalView::visibleInterval() const
{
    return std::chrono::duration_cast<Milliseconds>( visibleIntervalEnd() - visibleIntervalBegin() );
}

TimePoint TimeIntervalView::visibleIntervalBegin() const
{
    if ( !intervalScene() )
    {
        return TimePoint( Milliseconds( 0 ) );
    }
    return TimePoint( std::chrono::duration_cast<Milliseconds>( HoursF( mapToScene( viewport()->geometry().topLeft() ).x() / TimeIntervalScene::unitsPerHour ) ) );
}

TimePoint TimeIntervalView::visibleIntervalEnd() const
{
    if ( !intervalScene() )
    {
        return TimePoint( Milliseconds( 0 ) );
    }
    return TimePoint( std::chrono::duration_cast<Milliseconds>( HoursF( mapToScene( viewport()->geometry().bottomRight() ).x() / TimeIntervalScene::unitsPerHour ) ) );
}

void TimeIntervalView::onZoomAnimationTimer()
{
    double zoomFactor = std::pow( 1.2, m_zoomAccumulatorStep / 120.0 );
    if ( std::fabs( m_zoomAccumulatorStep ) > std::fabs( m_zoomAccumulator ) )
    {
        m_zoomAccumulator = 0.0;
    }
    else
    {
        m_zoomAccumulator -= m_zoomAccumulatorStep;
    }
    QRectF visibleSceneRect = mapToScene( viewport()->geometry() ).boundingRect();
    QRectF zoomToRect( m_zoomSteadyPoint, m_zoomSteadyPoint );
    zoomToRect.setTop( sceneRect().top() );
    zoomToRect.setBottom( sceneRect().bottom() );
    zoomToRect.adjust( -visibleSceneRect.width() * zoomFactor * m_zoomSteadyPointRatio, 0.0, visibleSceneRect.width() * zoomFactor * ( 1.0 - m_zoomSteadyPointRatio ), 0.0 );
    zoomToRect = zoomToRect.intersected( sceneRect() );
    if ( zoomToRect.contains( sceneRect() ) || zoomToRect.width() < TimeIntervalScene::unitsPerHour / 60.0 )
    {
        m_zoomAccumulator = 0.0;
        m_zoomAccumulatorStep = 0.0;
    }
    fitInView( zoomToRect );
    m_cachedRect = zoomToRect;
    if ( fabs( m_zoomAccumulator ) < std::numeric_limits<double>::min() )
    {
        m_pZoomAnimationTimer->stop();
        updateClusterization();
    }
}

void TimeIntervalView::onScrollerStateChanged( QScroller::State newState )
{
    if ( newState == QScroller::Inactive )
    {
        updateClusterization();
    }
}

void TimeIntervalView::updateClusterization()
{
    if ( intervalScene() )
    {
        Milliseconds overlap( visibleInterval() * QGuiApplication::primaryScreen()->geometry().width() / viewport()->width() );
        TimePoint startBound = visibleIntervalBegin() - overlap;
        TimePoint endBound = visibleIntervalEnd() + overlap;
        Milliseconds clusterDuration = visibleInterval() * 100 / viewport()->width();
        intervalScene()->setClusterization( clusterDuration, startBound, endBound );
    }
}

void TimeIntervalView::wheelEvent( QWheelEvent* pEvent )
{
    if ( std::signbit( m_zoomAccumulator * pEvent->angleDelta().y() ) )
    {
        m_zoomAccumulator = pEvent->angleDelta().y();
    }
    else
    {
        m_zoomAccumulator += pEvent->angleDelta().y();
    }
    QRectF visibleSceneRect = mapToScene( viewport()->geometry() ).boundingRect();
    m_zoomSteadyPoint = mapToScene( QRect( pEvent->pos(), QSize( 2, 2 ) ) ).boundingRect().center();
    m_zoomSteadyPointRatio = ( m_zoomSteadyPoint.x() - visibleSceneRect.left() ) / visibleSceneRect.width();
    m_zoomSteadyPointRatio = std::max( m_zoomSteadyPointRatio, 0.0 );
    m_zoomSteadyPointRatio = std::min( m_zoomSteadyPointRatio, 1.0 );
    m_zoomAccumulatorStep = m_zoomAccumulator / 10.0;
    if ( !m_pZoomAnimationTimer->isActive() )
    {
        m_pZoomAnimationTimer->start();
    }
    pEvent->accept();
}

void TimeIntervalView::resizeEvent( QResizeEvent* pEvent )
{
    QGraphicsView::resizeEvent( pEvent );
    if ( !m_cachedRect.isNull() )
    {
        fitInView( m_cachedRect );
        updateClusterization();
    }
}

void TimeIntervalView::showEvent( QShowEvent* pEvent )
{
    QGraphicsView::showEvent( pEvent );
    if ( m_cachedRect.isNull() )
    {
        m_cachedRect = sceneRect();
        fitInView( m_cachedRect );
        updateClusterization();
    }
}
