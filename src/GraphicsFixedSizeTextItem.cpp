#include "GraphicsFixedSizeTextItem.h"
#include <QPainter>

GraphicsFixedSizeTextItem::GraphicsFixedSizeTextItem( QGraphicsItem* pParent /*= nullptr */ )
    : QGraphicsSimpleTextItem( pParent )
{
}

GraphicsFixedSizeTextItem::GraphicsFixedSizeTextItem( const QString& text, QGraphicsItem* pParent /*= nullptr */ )
    : QGraphicsSimpleTextItem( text, pParent )
{
}

void GraphicsFixedSizeTextItem::paint( QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget )
{
    pPainter->save();
    double scaleValueX = scale() / pPainter->transform().m11();
    double scaleValueY = scale() / pPainter->transform().m22();
    pPainter->scale( scaleValueX, scaleValueY );
    pPainter->translate( -boundingRect().width() / 2.0, 0.0 );
    QGraphicsSimpleTextItem::paint( pPainter, pOption, pWidget );
    pPainter->restore();
}
