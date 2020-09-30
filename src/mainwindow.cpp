#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QFile>
#include <QTextStream>
#include <QTableView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QEventLoop>
#include <QDesktopServices>
#include <QWebEngineView>
#include <QApplication>
#include <QThread>
#include <QSize>
#include <QFileInfo>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    model_=new QSqlTableModel(this);
    parser_ = new Parser(this);
    dbmanager_=new DataBaseManager();
    dbmanager_->moveToThread(&dbthread_);

    ui->setupUi(this);
    setWindowTitle("VkRequest");
    ui->updateIntervallcdNumber->display("1H");
    view_.resize(700,250);
    ui->dbProgressBar->reset();
    ui->saveFileButton->setEnabled(false);

    QObject::connect(parser_,&Parser::authorized,this,&MainWindow::onAuthorized);

    QObject::connect(parser_,&Parser::invalidGroupName,this,&MainWindow::onInvalidGroupName);
    QObject::connect(parser_,&Parser::invalidToken,this,&MainWindow::onInvalidToken);

    QObject::connect(parser_,&Parser::extractedMembers,dbmanager_,&DataBaseManager::onMembersRecieved);
    QObject::connect(parser_,&Parser::extractedID,dbmanager_,&DataBaseManager::onIDRecieved);
    QObject::connect(&dbthread_,&QThread::started,dbmanager_,&DataBaseManager::ConnectToDb,Qt::QueuedConnection);
    dbthread_.start();

    QObject::connect(this,&MainWindow::CommonMembersRequested,dbmanager_,&DataBaseManager::onCommonMembersRequested,Qt::QueuedConnection);
    QObject::connect(dbmanager_,&DataBaseManager::selectedCommonMembers,this,&MainWindow::onCommonMembersSelected,Qt::QueuedConnection);
    QObject::connect(dbmanager_,&DataBaseManager::parsingGroup,this,&MainWindow::onParseGroupStarted,Qt::QueuedConnection);

    QObject::connect(parser_,&Parser::finished,this,&MainWindow::onParserFree);

    QObject::connect(dbmanager_,&DataBaseManager::addedMembers,this,&MainWindow::onDbProgressChanged,Qt::QueuedConnection);

    QObject::connect(parser_,&Parser::successfullyFinishedParsingGroup,dbmanager_,&DataBaseManager::onParsingGroupFinished,Qt::QueuedConnection);

    QObject::connect(this,&MainWindow::UpdateAllRequested,dbmanager_,&DataBaseManager::onUpdateAllRequested,Qt::QueuedConnection);
    QObject::connect(dbmanager_,&DataBaseManager::ParseRequested,parser_,&Parser::onParseRequested);

    QObject::connect(parser_,&Parser::loginRequseted,this,&MainWindow::onLoginRequested);

    onParserUnavailable();

    QObject::connect(&update_timer_,&QTimer::timeout,this,[&](){emit UpdateAllRequested();});

    QObject::connect(&view_,&WebEngineView::closed,this,[&](){ui->authButton->setEnabled(true),ui->authLabel->hide();authLoading.stop();});

    sqlLoading.setFileName(":/icons/ae.gif");
    sqlLoading.setScaledSize(QSize(40,40));
    ui->loadingLabel->setMovie(&sqlLoading);

    authLoading.setFileName(":/icons/ae.gif");
    authLoading.setScaledSize(QSize(40,40));
    ui->authLabel->setMovie(&authLoading);

    ui->pathLineEdit->setText(RetrieveSavePath());

    ui->selectionTableView->hide();

    view_.setWindowFlag(Qt::WindowStaysOnTopHint);
    ui->authcheckBox->setEnabled(false);
    ui->authButton->setEnabled(false);
    RequestAuth();
}
WebEngineView::WebEngineView(QWidget *parent) : QWebEngineView(parent){}
WebEngineView::~WebEngineView(){}
void WebEngineView::closeEvent(QCloseEvent *event){
    emit closed();
    QWebEngineView::closeEvent(event);
}
void MainWindow::closeEvent(QCloseEvent *event){
    dbthread_.quit();
    dbmanager_->deleteLater();
    QMainWindow::closeEvent(event);
}
void MainWindow::onAuthorized(){
    onParserFree();
    ui->authcheckBox->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->authcheckBox->setCheckable(false);
    view_.hide();
    authLoading.stop();
    ui->authLabel->hide();
    ui->parseInputPlainEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
    dbthread_.quit();
    dbmanager_->deleteLater();
}
void MainWindow::onDbError(const QString& mes){
    QMessageBox* messagebox = new QMessageBox(this);
    messagebox->setText(mes);
    messagebox->show();
}
void MainWindow::BindQuery(QSqlQueryModel* query_model){
    QItemSelectionModel *m = ui->selectionTableView->selectionModel();
    ui->selectionTableView->setModel(query_model);
    delete m;
    ui->selectionTableView->show();


    ui->saveFileButton->setEnabled(true);
    ui->resultsButton->setEnabled(true);
}
QStringList MainWindow::SelectActiveGroups(){
    return ui->selectionPlainTextEdit->document()->toPlainText().split(',');
}
void MainWindow::SaveResultsToFile(){
    QString path = ui->pathLineEdit->text();
    QFileInfo info(path);
    if (info.isAbsolute()){
        UpdateConfig(path);
        path+='\\'+ui->fileNameLineEdit->text()+".txt";
        QFile output_file(path);
        if (output_file.open(QIODevice::WriteOnly)){
            QTextStream ostream(&output_file);
            if (!output_file.exists()){
                QMessageBox* mes = new QMessageBox(this);
                mes->setText("invalid path");
                mes->show();
            }
            ostream<<"groups: "<<'\n';
            auto end = selected_groups_.end();
            for (auto iter(selected_groups_.begin());iter!=end;++iter){
                ostream<<*iter<<' ';
            }
            ostream<<'\n'<<"members:"<<'\n';
            QAbstractItemModel * model = ui->selectionTableView->selectionModel()->model();
            int count(model->rowCount());
            for (int i=0;i<count;++i){
                ostream<<model->index(i,0).data().toString()<<'\n';
            }

        }else{
        }
        output_file.close();
        ui->saveFileButton->setEnabled(true);
    }else{
        ui->saveFileButton->setEnabled(true);
        QMessageBox* messagebox = new QMessageBox(this);
        messagebox->setText("invalid path: ");
        messagebox->show();
    }
}
void MainWindow::UpdateConfig(const QString &savepath){
    QFile config("config.txt");
    if (config.open(QIODevice::WriteOnly)){
        QTextStream ostream(&config);
        ostream<<savepath;
    }else{
    }
    config.close();
}
QString MainWindow::RetrieveSavePath(){
    QFile config("config.txt");
    if (config.exists()){
        QString res;
        if (config.open(QIODevice::ReadOnly)){
            QTextStream istream(&config);
            res=config.readLine();
        }
        config.close();
        return res;
    }
    return "";
}
void MainWindow::on_saveFileButton_clicked()
{
    ui->saveFileButton->clearFocus();
    ui->saveFileButton->setEnabled(false);
    SaveResultsToFile();
}
void MainWindow::on_resultsButton_clicked()
{
    ui->resultsButton->setEnabled(false);
    selected_groups_=SelectActiveGroups();
    emit CommonMembersRequested(selected_groups_);
}
void MainWindow::onCommonMembersSelected(QSqlQueryModel* model){
    BindQuery(model);
}
void MainWindow::RequestAuth()
{
    authLoading.start();
    ui->authLabel->show();
    parser_->ExecuteAuthorization(&view_);
}
void MainWindow::onInvalidGroupName(const QString& short_name){
    ui->loadingLabel->hide();
    sqlLoading.stop();
    QMessageBox* messagebox = new QMessageBox(this);
    messagebox->setText("invalid group name: "+short_name);
    messagebox->show();
    onParserFree();
}
void MainWindow::onInvalidToken(){
    ui->authcheckBox->setCheckState(Qt::CheckState::Unchecked);
    onParserUnavailable();
    RequestAuth();
}
void MainWindow::onDbProgressChanged(int offset,int count){
    if (!stopping_){
        ui->dbProgressBar->setMaximum(count);
        ui->dbProgressBar->setValue(offset);
        if (offset>=count){
            ui->dbProgressBar->reset();
            ui->groupNameLabel->setText("");
        }
    }
}
void MainWindow::onParseGroupStarted(const QString& group){
    if (!stopping_){
        ui->groupNameLabel->setText(group);
    }
}

void MainWindow::on_timeSelecthorizontalSlider_sliderMoved(int position)
{
        ui->updateIntervallcdNumber->display(QString::number(position)+"H");
}
void MainWindow::onParserFree(){
    if (!ui->autoUpdateCheckBox->isChecked()){
        ui->parseInputPlainEdit->setEnabled(true);
        ui->autoUpdateCheckBox->setEnabled(true);
        ui->timeSelecthorizontalSlider->setEnabled(true);
        ui->parsePushButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
    }
}
void MainWindow::onParserUnavailable(){
      ui->parseInputPlainEdit->setEnabled(false);
      ui->autoUpdateCheckBox->setEnabled(false);
      ui->timeSelecthorizontalSlider->setEnabled(false);
      ui->parsePushButton->clearFocus();
      ui->parsePushButton->setEnabled(false);
      ui->stopButton->setEnabled(true);
}

void MainWindow::on_autoUpdateCheckBox_stateChanged(int arg1)
{
    switch (arg1) {
    case 2:{
        stopping_=false;
        ui->stopButton->setEnabled(false);
        ui->loadingLabel->show();
        sqlLoading.start();
        onParserUnavailable();
        ui->autoUpdateCheckBox->setEnabled(true);
        int interval(ui->timeSelecthorizontalSlider->value());
        emit UpdateAllRequested();
        update_timer_.start(interval*1000*60*60);
        break;
    }
    case 0:{
        update_timer_.stop();
        RequestStop();
        ui->stopButton->setEnabled(true);
        break;
    }
    }
}
void MainWindow::RequestStop(){
    onParserFree();
    stopping_=true;
    ui->dbProgressBar->reset();
    ui->groupNameLabel->setText("");
    parser_->onStopRequested();
    sqlLoading.stop();
    ui->loadingLabel->hide();
}
void MainWindow::on_stopButton_clicked()
{
    RequestStop();
}
void MainWindow::onLoginRequested(){
    view_.show();
}

void MainWindow::on_parsePushButton_clicked()
{
    stopping_=false;
    onParserUnavailable();
    QStringList groups = ui->parseInputPlainEdit->document()->toPlainText().replace(' ',"").replace('\n',"").replace('\r',"").split(',',QString::SkipEmptyParts);
    ui->loadingLabel->show();
    sqlLoading.start();
    parser_->EnqueueGroups(groups);
    sqlLoading.stop();
    ui->loadingLabel->hide();
}

void MainWindow::on_authButton_clicked()
{
    ui->authButton->setEnabled(false);
    RequestAuth();
}
