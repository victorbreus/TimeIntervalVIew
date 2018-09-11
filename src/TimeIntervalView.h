#pragma once

#include <QGraphicsView>
#include "TimeDefines.h"
#include <QTime>
#include <QTimeLine>
#include <QTimer>
#include <QScroller>

class TimeIntervalScene;

class TimeIntervalView: public QGraphicsView
{
    Q_OBJECT
public:
    TimeIntervalView( QWidget* pParent );
    TimeIntervalScene* intervalScene() const;
    Milliseconds visibleInterval() const;
    TimePoint visibleIntervalBegin() const;
    TimePoint visibleIntervalEnd() const;

protected:
    virtual void wheelEvent( QWheelEvent* pEvent );
    virtual void resizeEvent( QResizeEvent* pEvent );
    virtual void showEvent( QShowEvent* pEvent );

private slots:
    void onZoomAnimationTimer( qreal x );
    void onZoomAnimationFinished();
    void onScrollerStateChanged( QScroller::State newState );

private:
    void updateClusterization();

private:
    double m_mouseWheelAccumulator;
    QPointF m_zoomPoint;
    double m_zoomStep;
    QTimeLine* m_pZoomTimeLine;
    QRectF m_cachedRect;
};