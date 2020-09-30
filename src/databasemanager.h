#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QJsonDocument>
/*!
 * \brief Interacts with database
 *
 * This class manages connection to sql server, creates the required database and
 * provides the required interface for program interaction with the database.
 */
class DataBaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseManager(QObject *parent = nullptr);
    /*!
     * Connect to local server via Windows Authentication and
     * create databases wich application uses if required.
     */
    void ConnectToDb();
    /*!
     * Adds groups to the group list if the groups is not already in the list
     * \param[in] short_name  group short name.
     * \param[in] id group id
     */
    void AddGroup(const QString& short_name,int id);
    /*!
     * Adds members of the group to the database if they are not already added.
     * Emits parsingGroup with group name as param; emits addedMembers(offset,count).
     * \param doc document containing list of members
     * \param group_id group id
     * \param offset the order number of first member to be added
     * \param count the ammount of members of the group
     */
    void AddMembers(QJsonDocument doc,int group_id,int offset,int count);
    /*!
     * Gets common members of requested groups if they exist in database. If the group doesn't
     * exits in the database, ignores it.
     * \param groups list of requested groups
     * \returns QSqlQueryModel representing the result
     */
    QSqlQueryModel* GetCommonGroupMembers(const QStringList& groups);
    /*!
     * Enqueues all groups in database for update, in antichronological last update order.
     * Emits ParseRequested with group names as param.
     */
    void EnqueueAllGroupsForUpdate();
    /*!
     * Creates database which is required for the application to work.
     */
    void CreateDatabase();
signals:
    /*!
     * Signals that offset'th member out of count has been added to database.
     * \param offset order number of the member who has been added
     * \param count ammount of members in group
     */
    void addedMembers(int offset,int count);
    /*!
     * Signals that common members have been selected.
     * \param result QSqlQueryModel representing the result
     */
    void selectedCommonMembers(QSqlQueryModel* result);
    /*!
     * Signals that group is now being parsed.
     * \param short_name group short name
     */
    void parsingGroup(const QString& short_name);
    /*!
     * Signals that parse of groups is requested.
     * \param groups groups to be parsed
     */
    void ParseRequested(const QStringList& groups);
    /*!
     * Signals that an error has occured while connecting to database.
     * \param mes error message
     */
    void Error(const QString& mes);
public slots:
    /*!
     * Action to do when members to be added to database are recieved.
     * \param doc list of members
     * \param group_id group ID
     * \param offset order number of the first member to be added
     * \param count ammount of members in group
     */
    void onMembersRecieved(QJsonDocument doc,int group_id,int offset,int count);
    /*!
     * Action to do when group's ID is recieved.
     * \param short_name group short name
     * \param id group ID
     */
    void onIDRecieved(const QString& short_name,int id);
    /*!
     * Action to do when common members of groups are requested.
     * \param groups list of requested groups
     */
    void onCommonMembersRequested(const QStringList& groups);
    /*!
     * Action to do when parsing groups is finished.
     * \param id group ID
     */
    void onParsingGroupFinished(int id);
    /*!
     * Action to do when update all group members requested.
     */
    void onUpdateAllRequested();
private:
    /*!
     * Prepares database for selecting common members of requested groups.
     * \param groups groups whose common members will be selected
     */
    void SetActiveGroups(const QStringList& groups);

    /*!
    * Represents the state of connection to the database.
    */
    bool connected{false};
};

#endif // DATABASEMANAGER_H
