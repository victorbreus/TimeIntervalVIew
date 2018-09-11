#pragma once

#include <QGraphicsRectItem>
#include "TimeIntervalDatabase.h"

class GraphicsTimeIntervalItem : public QGraphicsRectItem
{
public:
    GraphicsTimeIntervalItem( const TimeIntervalCluster& timeIntervalCluster, QGraphicsItem* parent = nullptr );

protected:
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

private:
    QString generateTooltip( const TimeIntervalCluster& timeIntervalCluster ) const;

private:
    TimeIntervalCluster m_timeIntervalCluster;
};
