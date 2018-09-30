#pragma once

#include <QGraphicsView>
#include "TimeDefines.h"
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
    void onZoomAnimationTimer();
    void onScrollerStateChanged( QScroller::State newState );

private:
    void updateClusterization();

private:
    QTimer* m_pZoomAnimationTimer;
    double m_zoomAccumulator;
    double m_zoomAccumulatorStep;
    double m_zoomSteadyPointRatio;
    QPointF m_zoomSteadyPoint;
    QRectF m_cachedRect;
};
