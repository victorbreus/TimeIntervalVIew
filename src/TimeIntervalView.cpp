#include "TimeIntervalView.h"
#include "TimeIntervalScene.h"
#include <QWheelEvent>
#include <QTimeLine>
#include <QTime>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

TimeIntervalView::TimeIntervalView( QWidget* pParent )
    : QGraphicsView( pParent )
    , m_mouseWheelAccumulator( 0.0 )
    , m_pZoomTimeLine( nullptr )
{
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    QScroller::grabGesture( viewport(), QScroller::LeftMouseButtonGesture );
    QScrollerProperties scrollerProperties = QScroller::scroller( viewport() )->scrollerProperties();
    scrollerProperties.setScrollMetric( QScrollerProperties::DecelerationFactor, 1.0 );
    QScroller::scroller( viewport() )->setScrollerProperties( scrollerProperties );
    QObject:connect( QScroller::scroller( viewport() ), &QScroller::stateChanged, this, &TimeIntervalView::onScrollerStateChanged );
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
    return TimePoint( std::chrono::duration_cast<Milliseconds>( HoursF( mapToScene( 0, 0 ).x() / TimeIntervalScene::unitsPerHour ) ) );
}

TimePoint TimeIntervalView::visibleIntervalEnd() const
{
    if ( !intervalScene() )
    {
        return TimePoint( Milliseconds( 0 ) );
    }
    return TimePoint( std::chrono::duration_cast<Milliseconds>( HoursF( mapToScene( width(), 0 ).x() / TimeIntervalScene::unitsPerHour ) ) );
}

void TimeIntervalView::onZoomAnimationTimer( qreal x )
{
    if ( std::abs( m_mouseWheelAccumulator ) > std::numeric_limits<double>::min() )
    {
        double factor = std::pow( 1.2, m_zoomStep / 120.0 );
        if ( std::abs( m_zoomStep ) > std::abs( m_mouseWheelAccumulator ) )
        {
            m_mouseWheelAccumulator = 0.0;
        }
        else
        {
            m_mouseWheelAccumulator -= m_zoomStep;
        }

        QRectF zoomFromRect;
        zoomFromRect.setTopLeft( mapToScene( QPoint( 3, 0 ) ) );
        zoomFromRect.setBottomRight( mapToScene( QPoint( width() -3, height() ) ) );
        zoomFromRect.setBottom( 80.0 );
        zoomFromRect.setTop( 0.0 );

        QRectF zoomToRect = zoomFromRect;
        double grow = ( zoomFromRect.width() * factor - zoomFromRect.width() ) * ( 1.0 - x );
        double growLeft = ( m_zoomPoint.x()- zoomFromRect.left() ) / zoomFromRect.width();
        double growRight = ( zoomFromRect.right() - m_zoomPoint.x() ) / zoomFromRect.width();
        zoomToRect.adjust( -grow * growLeft, 0, grow * growRight, 0 );

        zoomToRect.setLeft( std::max( zoomToRect.left(), 0.0 ) );
        zoomToRect.setRight( std::min( zoomToRect.right(), TimeIntervalScene::unitsPerHour * 24 ) );

        if ( zoomToRect.width() > TimeIntervalScene::unitsPerHour / 60.0 )
        {
            fitInView( zoomToRect );
            m_cachedRect = zoomToRect;
        }
    }
}

void TimeIntervalView::onZoomAnimationFinished()
{
    updateClusterization();
    if ( m_pZoomTimeLine )
    {
        m_pZoomTimeLine->deleteLater();
        m_pZoomTimeLine = nullptr;
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
        Milliseconds overlap( visibleInterval() * QApplication::desktop()->screenGeometry().width() / width() );
        TimePoint startBound = visibleIntervalBegin() - overlap;
        TimePoint endBound = visibleIntervalEnd() + overlap;
        Milliseconds clusterDuration = visibleInterval() * 100 / width();
        intervalScene()->setClusterization( clusterDuration, startBound, endBound );
    }
}

void TimeIntervalView::wheelEvent( QWheelEvent* pEvent )
{
    QGraphicsView::wheelEvent( pEvent );
    if ( m_mouseWheelAccumulator * pEvent->delta() < 0 )
    {
        m_mouseWheelAccumulator = pEvent->delta();
    }
    else
    {
        m_mouseWheelAccumulator += pEvent->delta();
    }
    m_zoomPoint = mapToScene( QRect( pEvent->pos(), QSize( 2, 2 ) ) ).boundingRect().center();

    if ( m_pZoomTimeLine )
    {
        m_pZoomTimeLine->deleteLater();
        m_pZoomTimeLine = nullptr;
    }
    m_pZoomTimeLine = new QTimeLine( 250, this );
    m_pZoomTimeLine->setCurveShape( QTimeLine::LinearCurve );
    m_pZoomTimeLine->setUpdateInterval( 12 );
    QObject::connect( m_pZoomTimeLine, &QTimeLine::valueChanged, this, &TimeIntervalView::onZoomAnimationTimer );
    QObject::connect( m_pZoomTimeLine, &QTimeLine::finished, this, &TimeIntervalView::onZoomAnimationFinished );
    m_zoomStep = m_mouseWheelAccumulator / 10.0;
    m_pZoomTimeLine->start();
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
        m_cachedRect = QRectF( 0.0, 0.0, 24.0 * TimeIntervalScene::unitsPerHour, 80.0 );
        fitInView( m_cachedRect );
        updateClusterization();
    }
}
