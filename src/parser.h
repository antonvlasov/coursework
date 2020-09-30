#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QNetworkAccessManager>
#include <QJsonDocument>

class GroupState;
/*!
 * Class which interacts with vk api to get members of a group.
 */
class Parser : public QObject
{
    Q_OBJECT
public:
    /*!
     * Connects the signals and slots of parser instance's members.
     * \param parent
     */
    explicit Parser(QObject *parent = nullptr);
    ~Parser();
    /*!
     * Redirects view to the authorization page. If user is not authorized automaticly
     * in 5 seconds, emits loginRequseted().
     * \param view QWebEngineView to be shown if required
     */
    void ExecuteAuthorization(QWebEngineView* view);
    /*!
     * Starts the process of parsing group. Only one group can be parsed at a time.
     * \param short_name group name
     */
    void ParseGroup(const QString& short_name);
    /*!
     * Enqueues groups for parsing in the same order they are given.
     * \param groups list of groups
     */
    void EnqueueGroups(const QStringList& groups);

signals:
    /*!
     * Signals that members of the group have have been extracted.
     * \param doc document containing members
     * \param group_id group ID
     * \param offset order number of the first member extracted
     * \param count ammount of members in group
     */
    void extractedMembers(QJsonDocument doc,int group_id,int offset,int count);
    /*!
     * Signals that the position of member that has been recieved or count have been changed.
     * \param offset position of member that has been recieved
     * \param count ammount of members in group
     */
    void updatedCount(int offset,int count);
    /*!
     * Signals that members of group have been treated.
     */
    void membersTreated();
    /*!
     * Signals that group ID has been extracted.
     * \param short_name group short name
     * \param id group ID
     */
    void extractedID(QString short_name,int id);
    /*!
     * Signals that time between responses has passed and whether parser still awaits for reply to previous response.
     * \param awaiting_response indicates whether parser still awaits for reply to previous response.
     */
    void timePassed(bool awaiting_response);

    /*!
     * Signals that recieved reply is error
     * \param reply recieved reply
     */
    void replyIsError(QNetworkReply* reply);
    /*!
     * Signals that recieved reply was not expected
     * \param reply recieved reply
     */
    void unexpectedReply(QNetworkReply* reply);
    /*!
     * Signals that group name is invalid
     * \param short_name groups short name
     */
    void invalidGroupName(const QString& short_name);
    /*!
     * Signals that token used for requests is invalid.
     */
    void invalidToken();

    /*!
     * Signals that authorization has been successfull and now parser has a valid token.
     */
    void authorized();
    /*!
     * Signals that group has been parsed successfully.
     * \param id group ID
     */
    void successfullyFinishedParsingGroup(int id);
    /*!
     * Signals that parsing all requested groups has been finished.
     */
    void finished();
    /*!
     * Signals that parsing group has been stopped.
     */
    void doneParsingGroup();
    /*!
     * Signals that stop of parsing has been requested.
     */
    void stopRequsted();
    /*!
     * Signals that manual login has been requested to get token.
     */
    void loginRequseted();
public slots:
    /*!
     * Action to do when parse of groups has been requested.
     * \param groups list of groups
     */
    void onParseRequested(const QStringList& groups);
    /*!
     * Action to do when token has been requested.
     * \param view WebEngineView which will be shown if manual login is required
     */
    void onTokenRequested(QWebEngineView* view);
    /*!
     * Action to do when stop of parsing is requested.
     */
    void onStopRequested();
private:
    /*!
     * Checks whether url contains a token.
     * \param url url which can contain token
     */
    void CheckForToken(const QUrl& url);
    /*!
     * Recieve the reply any reply and send it to an appropriate handler
     * \param reply reply which is recieved
     */
    void RecieveReply(QNetworkReply* reply);
    /*!
     * Request to get offset'th 1000 members of group
     * \param group group short name
     * \param offset offset of request
     */
    void ExecuteGetMembers(int group,int offset = 0);
    /*!
     * Request to get group ID
     * \param short_name group short name
     */
    void ExecuteGetGroupID(const QString& short_name);
    /*!
     * Treat members recieved from request sent by ExecuteGetMembers.
     * emits extractedMembers.
     * \param reply reply which is recieved
     */
    void TreatRecievedMembers(QNetworkReply* reply);
    /*!
     * Treat ID recieved from request sent by ExecuteGetGroupID.
     * \param reply reply which is recieved
     */
    void TreatRecievedID(QNetworkReply* reply);

    /*!
     * Gets offset which request previously sent by ExecuteGetMembers contains.
     * \param request previously sent by ExecuteGetMembers
     * \returns offset which request previously sent by ExecuteGetMembers contains
     */
    int GetOffset(QString request);
    /*!
     * Gets group ID contained in request previously sent by ExecuteGetMembers.
     * \param request previously sent by ExecuteGetMembers
     * \returns group ID contained in request previously sent by ExecuteGetMembers
     */
    int GetGroupID(QString request);
    /*!
     * Sets the parser's awaiting state. If the parser is waiting for response, no other
     * requests will be sent.
     * \param state awaiting state
     */
    void setAwaitingState(bool state);

    QNetworkAccessManager* manager_{nullptr};
    GroupState* state_;
    QTimer* timer_;
    const QUrl authorization_{"https://oauth.vk.com/authorize?client_id=7235450"
                             "&display=page&redirect_uri=https://oauth.vk.com/blank.html"
                             "&response_type=token&v=5.103&revoke=0"};
    QString token_;
    bool awaiting_response_{false};
    bool stop_requested_{false};
};
/*!
 * Class representing state of parse of a particular group.
 */
class GroupState : public QObject{
    Q_OBJECT
public:
    GroupState(QObject *parent = nullptr);
    ~GroupState();
    /*!
     * Resets members to default values.
     */
    void Reset();

    QString short_name_{""};
    int id_{0};
    int count_{0};
    int offset_{0};
signals:
    /*!
     * Signals that ID of the group has been set.
     */
    void IDSet();
    /*!
     * Signals that group should be parsed with offset.
     * \param id group ID
     * \param offset offset of request
     */
    void timePassed(int id,int offset);
    /*!
     * Signals that group has been successfully parsed.
     * \param id group ID
     */
    void finised(int id);
    /*!
     * Signals that parse has been interupted.
     */
    void stopped();
public slots:
    /*!
     * Action to do when id and short name are recieved.
     * \param short_name group short name
     * \param id group id
     */
    void onIDRecieved(QString short_name,int id);
    /*!
     * Action to do when offset or count are updated.
     * \param offset offset of last request
     * \param count ammount of members in group
     */
    void onCountRecieved(int offset,int count);
    /*!
     * Action to do when time between request has passed, depending on whether response is awaited.
     * \param awaiting_reponse indicates whether response is awaited
     */
    void onTimePassed(bool awaiting_reponse);
    /*!
     * Action to do when stop of parsing is requested.
     */
    void onStopRequested();
};

#endif // PARSER_H
