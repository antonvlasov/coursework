#include "databasemanager.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTime>
#include <QDataStream>
#include <QFile>
#include <QTextStream>
#include <QSqlError>

DataBaseManager::DataBaseManager(QObject *parent) : QObject(parent)
{

}
void DataBaseManager::ConnectToDb(){
    QSqlDatabase database = QSqlDatabase::addDatabase("QODBC");
    database.setDatabaseName("DRIVER={SQL Server}; Server=.\\SQLEXPRESS;Trusted_Connection=yes;");
    if (database.open()){
        emit Error("database not opened");
    }else{
        connected=true;
    }
    QSqlQuery q;
    q.exec("use vk");
    if(q.lastError().isValid()){
        CreateDatabase();
    }
}
void DataBaseManager::CreateDatabase(){
    QFile source(":/db/script.sql");
    QString script;
    if (source.exists()){
        QString line;
        if (source.open(QIODevice::ReadOnly)){
            QTextStream istream(&script);
            do{
                line=source.readLine();
                line=line.trimmed();
                if (line == "GO"){
                    QSqlQuery q(script);
                    q.exec(script);
                    script="";
                }else{
                    script+=line;
                    script+=' ';
                }
            }while(!line.isNull());
        }else{
           emit Error("sql script missing");
        }
        source.close();
    }
}
void DataBaseManager::AddGroup(const QString &short_name, int id){
    QSqlQuery q;
    q.exec("use vk");
    q.prepare("exec InsertGroup @ID = ?, @short_name = ?");
    q.bindValue(0,id);
    q.bindValue(1,short_name);
    q.exec();
}
void DataBaseManager::AddMembers(QJsonDocument doc,int group_id,int offset,int count){
    QJsonArray arr = doc["response"]["items"].toArray();
    auto end = arr.end();
    QSqlQuery q;
    q.exec("use vk");

    q.prepare("exec GetGroupShortName @ID = ?");
    q.bindValue(0,group_id);
    q.exec();
    q.first();
    emit parsingGroup(q.value(0).toString());

    q.prepare("insert into member_list(group_id,user_id)"
              "values (?,?)");
    for (auto iter(arr.begin());iter!=end;++iter){
        q.bindValue(0,group_id);
        q.bindValue(1,iter->toInt());
        q.exec();
    }
    emit addedMembers(offset,count);
}
QSqlQueryModel* DataBaseManager::GetCommonGroupMembers(const QStringList& groups){
    SetActiveGroups(groups);
    QSqlQuery q("exec SelectUnion");

    QSqlQueryModel *query_model= new QSqlQueryModel(this);
    query_model->setQuery(q);
    return query_model;
}
void DataBaseManager::SetActiveGroups(const QStringList& groups){
    QSqlQuery q("use vk");
    q.exec("exec ClearActiveGroups");
    q.prepare("exec ActivateGroup @short_name = ?");
    auto end=groups.end();
    for (auto iter(groups.begin());iter!=end;++iter){
        q.bindValue(0,*iter);
        q.exec();
    }
}
void DataBaseManager::onParsingGroupFinished(int id){
    QSqlQuery q("use vk");
    q.prepare("exec UpdateGroup @ID = ?");
    q.bindValue(0,id);
    q.exec();
}
void DataBaseManager::EnqueueAllGroupsForUpdate(){
    QStringList groups;
    QSqlQuery q("use vk");
    q.exec("exec SortedGroups");
    while (q.next()){
        groups.push_back(q.value(0).toString());
    }
    emit ParseRequested(groups);
}
void DataBaseManager::onUpdateAllRequested(){
    EnqueueAllGroupsForUpdate();
}
void DataBaseManager::onMembersRecieved(QJsonDocument doc,int group_id,int offset,int count){
    AddMembers(doc,group_id,offset,count);
}
void DataBaseManager::onIDRecieved(const QString& short_name,int id){
    AddGroup(short_name,id);
}
void DataBaseManager::onCommonMembersRequested(const QStringList& groups){
    emit selectedCommonMembers(GetCommonGroupMembers(groups));
}
