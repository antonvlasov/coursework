#include "parser.h"

#include <QWebEngineView>
#include <QUrl>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>
#include <QJsonArray>

Parser::Parser(QObject *parent) : QObject(parent)
{
    manager_ = new QNetworkAccessManager(this);
    QObject::connect(manager_,&QNetworkAccessManager::finished,this,&Parser::RecieveReply);
    state_=new GroupState(this);
    timer_=new QTimer(state_);
    // parsing group
    QObject::connect(this,&Parser::extractedID,state_,&GroupState::onIDRecieved);
    QObject::connect(state_,&GroupState::timePassed,this,&Parser::ExecuteGetMembers);
    QObject::connect(this,&Parser::updatedCount,state_,&GroupState::onCountRecieved);
    QObject::connect(timer_,&QTimer::timeout,state_,[&](){emit timePassed(awaiting_response_);}); // awaiting_response shows whether new request should be sent
    QObject::connect(this,&Parser::timePassed,state_,&GroupState::onTimePassed);
    // reacting on finish of parsing
    QObject::connect(state_,&GroupState::finised,timer_,&QTimer::stop);
    QObject::connect(state_,&GroupState::finised,this,[&](int id){emit successfullyFinishedParsingGroup(id);emit doneParsingGroup();});
    // reacting on forced stop of parsing
    QObject::connect(state_,&GroupState::stopped,timer_,&QTimer::stop);
    QObject::connect(state_,&GroupState::stopped,this,[&](){emit doneParsingGroup();});
    // reacting on errors
    QObject::connect(this,&Parser::invalidGroupName,timer_,&QTimer::stop);

}
Parser::~Parser(){
}
void Parser::ExecuteAuthorization(QWebEngineView* view){
    QObject::connect(view,&QWebEngineView::urlChanged,this,&Parser::CheckForToken);
    view->load(authorization_);

    QTimer timer;
    QEventLoop loop;
    QObject::connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    timer.start(5000);
    loop.exec();
    if (!view->url().toString().startsWith("https://oauth.vk.com/blank.html#access_token=")){
        emit loginRequseted();
    }
}
void Parser::CheckForToken(const QUrl &url){
    QString str =url.toString();
    if (str.startsWith("https://oauth.vk.com/blank.html#access_token=")){
        str=str.section('=',1);
        str=str.split('&')[0];
        token_=str;
        emit authorized();
    }else{

    }
}
void Parser::RecieveReply(QNetworkReply *reply){
    QString request(reply->request().url().toString());
    if (reply->error()){
        emit replyIsError(reply);
    }
    else if (request.startsWith("https://api.vk.com/method/groups.getById")){
        TreatRecievedID(reply);
    }else if(request.startsWith("https://api.vk.com/method/groups.getMembers")){
        TreatRecievedMembers(reply);
    }else{
        emit unexpectedReply(reply);
    }
}
GroupState::GroupState(QObject* parent): QObject(parent){}
GroupState::~GroupState(){}
void GroupState::Reset(){
    short_name_="";
    id_=0;
    count_=0;
    offset_=0;
}
void GroupState::onIDRecieved(QString short_name,int id){
    short_name_=short_name;
    id_=id;
    emit IDSet();
}
void GroupState::onCountRecieved(int offset,int count){
    offset_=offset;
    count_=count;
}
void GroupState::onTimePassed(bool awaiting_response){
    if (offset_<=count_){
        if (!awaiting_response){
            emit timePassed(id_,offset_);
        }
    }else{
        emit finised(id_);
        Reset();
    }
}
void GroupState::onStopRequested(){
    emit stopped();
    Reset();
}
void Parser::setAwaitingState(bool state){
    awaiting_response_=state;
}
void Parser::onStopRequested(){
    stop_requested_=true;
    state_->onStopRequested();
}
void Parser::TreatRecievedID(QNetworkReply* reply){
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    int id = doc["response"][0]["id"].toInt();
    QString short_name = doc["response"][0]["screen_name"].toString();
    reply->close();
    reply->deleteLater();
    if (id==0){
        int error_code=doc["error"]["error_code"].toInt();
        switch (error_code){
        case 5:{
            emit invalidToken();
            break;
        }
        case 100:{
            short_name = doc["error"]["request_params"][0]["value"].toString();
            qDebug()<<"invalid name";
            emit invalidGroupName(short_name);
            break;
        }
        }
    }else{
        emit extractedID(short_name,id);
    }
}
void Parser::EnqueueGroups(const QStringList &groups){
    auto end = groups.cend();
    for (auto iter = groups.cbegin();iter!=end;++iter){
        ParseGroup(*iter);
        QEventLoop waiting_for_parse;
        QObject::connect(this,&Parser::doneParsingGroup,&waiting_for_parse,&QEventLoop::quit);
        waiting_for_parse.exec();
        if (stop_requested_){
            stop_requested_=false;
            break;
        }
    }
    emit finished();
}
void Parser::onParseRequested(const QStringList& groups){
    EnqueueGroups(groups);
}
void Parser::onTokenRequested(QWebEngineView* view){
    ExecuteAuthorization(view);
}
void Parser::ParseGroup(const QString& short_name){
    ExecuteGetGroupID(short_name);
    QEventLoop waiting_for_ID;
    QObject::connect(state_,&GroupState::IDSet,&waiting_for_ID,&QEventLoop::quit);
    waiting_for_ID.exec();

    timer_->start(333);
}
void Parser::ExecuteGetMembers(int id, int offset){
    QString count("1000");
    QString temp_url("https://api.vk.com/method/groups.getMembers?group_id=");
    temp_url.append(QString::number(id));
    temp_url.append("&offset=");
    temp_url.append(QString::number(offset));
    temp_url.append("&count="+count+"&v=5.52&access_token=");
    temp_url.append(token_);

    QUrl url(temp_url);
    QNetworkRequest request(url);
    setAwaitingState(true);
    manager_->get(request);
}
void Parser::ExecuteGetGroupID(const QString& short_name){
    QString temp_url("https://api.vk.com/method/groups.getById?group_id=");
    temp_url.append(short_name);
    temp_url.append("&v=5.52&access_token=");
    temp_url.append(token_);

    QUrl url(temp_url);
    QNetworkRequest request(url);
    manager_->get(request);
}
void Parser::TreatRecievedMembers(QNetworkReply *reply){
    setAwaitingState(false);
    QString request(reply->request().url().toString());
    int offset = GetOffset(request)+1000;

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->close();
    reply->deleteLater();

    int count=doc["response"]["count"].toInt();
    emit updatedCount(offset,count);

    emit extractedMembers(doc,GetGroupID(request),offset,count);

    emit membersTreated(); // should it be emitted? not used currently
}
int Parser::GetOffset(QString request){
    request=request.split('&')[1];
    request=request.split('=')[1];
    return request.toInt();
}
int Parser::GetGroupID(QString request){
    request=request.split('?')[1].split('&')[0].split('=')[1];
    return request.toInt();
}


