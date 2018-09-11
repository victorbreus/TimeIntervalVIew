#pragma once

#include <QGraphicsSimpleTextItem>

class GraphicsFixedSizeTextItem : public QGraphicsSimpleTextItem
{
public:
    GraphicsFixedSizeTextItem( QGraphicsItem* pParent = nullptr );
    GraphicsFixedSizeTextItem( const QString& text, QGraphicsItem* pParent = nullptr );

protected:
    virtual void paint( QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget ) override;
};
