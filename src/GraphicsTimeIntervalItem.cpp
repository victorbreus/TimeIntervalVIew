#include "GraphicsTimeIntervalItem.h"
#include <QPen>
#include <QTextOption>
#include <QPainter>
#include "TimeIntervalScene.h"
#include "TimeDefines.h"
#include <QTime>

GraphicsTimeIntervalItem::GraphicsTimeIntervalItem( const TimeIntervalCluster& timeIntervalCluster, QGraphicsItem* pParent /*= nullptr */ )
    : QGraphicsRectItem( pParent )
    , m_timeIntervalCluster( timeIntervalCluster )
{
    QRectF rect;
    rect.setTop( 45.0 );
    rect.setBottom( 75.0 );
    rect.setLeft( TimeIntervalScene::unitsPerHour * std::chrono::duration_cast<HoursF>( m_timeIntervalCluster.timestamp() - TimePoint( Milliseconds( 0 ) ) ).count() );
    rect.setRight( rect.left() + TimeIntervalScene::unitsPerHour * std::chrono::duration_cast<HoursF>( m_timeIntervalCluster.duration() ).count() );
    setRect( rect );

    QPen pen;
    pen.setCosmetic( true );
    pen.setWidthF( 1 );
    setPen( pen );

    if ( m_timeIntervalCluster.enriesCount() > 1 )
    {
        setBrush( QBrush( QColor( 200, 255, 200 ), Qt::SolidPattern ) );
        setToolTip( generateTooltip( timeIntervalCluster ) );
    }
    else
    {
        setBrush( QBrush( QColor( 200, 200, 255 ), Qt::SolidPattern ) );
    }
}

void GraphicsTimeIntervalItem::paint( QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget )
{
    QGraphicsRectItem::paint( pPainter, pOption, pWidget );

    double scaleX = scale() / pPainter->transform().m11();
    double scaleY = scale() / pPainter->transform().m22();

    QRectF captionRect = rect();
    captionRect.setWidth( captionRect.width() / scaleX );
    captionRect.translate( captionRect.left() / scaleX - captionRect.left(), 0 );
    captionRect.setHeight( captionRect.height() / scaleY );
    captionRect.translate( 0 , captionRect.top() / scaleY - captionRect.top() );
    captionRect.adjust( 4.0, 0, 0, 0 );
    captionRect.setWidth( std::min( captionRect.width(), 1000.0 ) );

    QTextOption textOption;
    textOption.setWrapMode( QTextOption::NoWrap );
    textOption.setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    pPainter->save();
    pPainter->scale( scaleX, scaleY );
    QFontMetrics fontMetrics( pPainter->font() );
    pPainter->drawText( captionRect, fontMetrics.elidedText( QString::fromStdString( m_timeIntervalCluster.caption() ), Qt::ElideRight, captionRect.width() ), textOption );
    pPainter->restore();
}

QString GraphicsTimeIntervalItem::generateTooltip( const TimeIntervalCluster& timeIntervalCluster ) const
{
    const size_t detailRecordCount = 10;
    QString result = QString()
        + "<style>"
        + "td{ background: #FAFAFA; }"
        + "th{ background: #F3F3F3; }"
        + "p{ text-align: center; }"
        + "</style>";

    result = result
        + "<table cellspacing='5' cellpadding ='5' align='center'>"
        + "<caption><h3>Details</h3></caption>"
        + "<tr><th>Record name</th><th>Timestamp</th><th>Duration</th></tr>";
    for ( size_t i = 0; i < std::min( detailRecordCount, timeIntervalCluster.enriesCount() ); ++i )
    {
        result = result
            + QString( "<tr><td>%1</td><td>%2</td><td>%3</td></tr>" )
            .arg( QString::fromStdString( timeIntervalCluster.entryAt( i ).caption() ) )
            .arg( QTime( 0, 0 ).addMSecs( std::chrono::duration_cast<Milliseconds>( timeIntervalCluster.entryAt( i ).timestamp() - TimePoint() ).count() ).toString( "hh:mm:ss.zzz" ) )
            .arg( QTime( 0, 0 ).addMSecs( timeIntervalCluster.entryAt( i ).duration().count() ).toString( "hh:mm:ss.zzz" ) );
    }
    result = result + "</table>";
    if ( timeIntervalCluster.enriesCount() > 10 )
    {
        result = result + QString( "<p>...and another %1 record%2</p>" ).arg( timeIntervalCluster.enriesCount() - 10 ).arg( ( timeIntervalCluster.enriesCount() - 10 ) != 1 ? "s" : "" );
    }
    return result;
}

