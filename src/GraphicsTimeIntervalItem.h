#pragma once

#include <QGraphicsRectItem>
#include "TimeIntervalDatabase.h"

class GraphicsTimeIntervalItem : public QGraphicsRectItem
{
public:
    GraphicsTimeIntervalItem( const TimeIntervalCluster& timeIntervalCluster, QGraphicsItem* pParent = nullptr );

protected:
    virtual void paint( QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget ) override;

private:
    QString generateTooltip( const TimeIntervalCluster& timeIntervalCluster ) const;

private:
    TimeIntervalCluster m_timeIntervalCluster;
};
