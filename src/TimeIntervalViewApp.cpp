#include "TimeIntervalViewApp.h"
#include <QIntValidator>

TimeIntervalViewApp::TimeIntervalViewApp( QWidget* pParent )
    : QMainWindow( pParent )
    , m_pTimeIntervalScene( new TimeIntervalScene( this ) )
    , m_pTimeIntervalDatabase( new TimeIntervalDatabase( this ) )
{
    ui.setupUi( this );
    ui.recordsCountLineEdit->setValidator( new QIntValidator( 0, 100000000, ui.recordsCountLineEdit ) );
    QObject::connect( ui.initializePushButton, &QPushButton::clicked, this, &TimeIntervalViewApp::onInitializeButtonClicked );
    QObject::connect( ui.recordsCountLineEdit, &QLineEdit::returnPressed, ui.initializePushButton, &QPushButton::click );
    QObject::connect( m_pTimeIntervalDatabase, &TimeIntervalDatabase::databasePopulated, this, &TimeIntervalViewApp::onTimeIntervalDatabasePopulated );

    m_pTimeIntervalScene->setDatabase( m_pTimeIntervalDatabase );
    ui.timeIntervalView->setScene( m_pTimeIntervalScene );
}

void TimeIntervalViewApp::onInitializeButtonClicked()
{
    ui.initializePushButton->setText( "Please wait..." );
    ui.initializePushButton->setEnabled( false );
    m_pTimeIntervalDatabase->populate( ui.recordsCountLineEdit->text().toInt() );
}

void TimeIntervalViewApp::onTimeIntervalDatabasePopulated()
{
    ui.initializePushButton->setText( "Initialize" );
    ui.initializePushButton->setEnabled( true );
}
