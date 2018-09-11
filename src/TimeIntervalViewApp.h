#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TimeIntervalViewApp.h"
#include "TimeIntervalScene.h"
#include "TimeIntervalDatabase.h"

class TimeIntervalViewApp : public QMainWindow
{
    Q_OBJECT
public:
    TimeIntervalViewApp( QWidget* pParent = nullptr );

private slots:
    void onInitializeButtonClicked();
    void onTimeIntervalDatabasePopulated();

private:
    Ui::TimeIntervalViewExample ui;
    TimeIntervalScene* m_pTimeIntervalScene;
    TimeIntervalDatabase* m_pTimeIntervalDatabase;
};
