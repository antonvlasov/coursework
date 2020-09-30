#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "parser.h"
#include "databasemanager.h"

#include <QMainWindow>
#include <QSqlTableModel>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QMovie>
#include <QTimer>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
    class QWebEngineView;
}
QT_END_NAMESPACE
/*!
 * Emits closed() on closeEvent.
 */
class WebEngineView : public QWebEngineView{
    Q_OBJECT
public:
    WebEngineView(QWidget *parent = nullptr);
    ~WebEngineView() override;
signals:
    void closed();
protected:
    /*!
     * Emits closed, then calls parent's closeEvent(event).
     */
    void closeEvent(QCloseEvent *event) override;
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * Connects slots and signals of it's members; moves dbmanager_ to a separate thread.
     */
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    /*!
     * Saves common members to a file, file path is retrieved from ui.
     */
    void SaveResultsToFile();

    /*!
     * Binds QSqlQueryModel representing result of selecting common members to the table and shows the table on the ui.
     * \param query_model
     */
    void BindQuery(QSqlQueryModel* query_model);
signals:
    /*!
     * Signals that common members of groups are requested.
     * \param groups group list
     */
    void CommonMembersRequested(const QStringList& groups);
    /*!
     * Signals that parse of groups is requested.
     * \param groups  group list
     */
    void ParseRequested(const QStringList& groups);
    /*!
     * Signals that update of all group members has been requested.
     */
    void UpdateAllRequested();
    /*!
     * Signals that stop of update has been requested.
     */
    void StopUpdateRequested();
    /*!
     * Signals that parser is unavailable.
     */
    void ParserUnavailable();
    /*!
     * Signals that parser is free.
     */
    void ParserFree();

public slots:
    /*!
     * Action to do when authorization has been complete.
     */
    void onAuthorized();
    /*!
     * Action to do when group has invalid name.
     * \param short_name group name
     */
    void onInvalidGroupName(const QString& short_name);
    /*!
     * Action to do when token is invalid.
     */
    void onInvalidToken();
    /*!
     * Action to do when progress of adding members to database has changed.
     * \param offset offset of member added.
     * \param count ammount of members in group
     */
    void onDbProgressChanged(int offset,int count);
    /*!
     * Action to do when common members of group have been selected.
     * \param model QSqlQueryModel representing selection of common members
     */
    void onCommonMembersSelected(QSqlQueryModel* model);
    /*!
     * Action to do when parsing group has started.
     * \param group group ID
     */
    void onParseGroupStarted(const QString& group);
    /*!
     * Action to do when parser is free.
     */
    void onParserFree();
    /*!
     * Action to do when parser is free.
     */
    void onParserUnavailable();
    /*!
     * Action to do when login is requested.
     */
    void onLoginRequested();
    /*!
     * Action to do when dberror occurs.
     * \param mes error message
     */
    void onDbError(const QString& mes);
private slots:
    void on_saveFileButton_clicked();
    void on_resultsButton_clicked();
    void on_timeSelecthorizontalSlider_sliderMoved(int position);
    void on_autoUpdateCheckBox_stateChanged(int arg1);
    void on_stopButton_clicked();
    void on_parsePushButton_clicked();
    void on_authButton_clicked();

protected:
    /*!
     * Quits the thread which has been created before closing.
     */
    void closeEvent(QCloseEvent *event) override;
private:
    /*!
     * Updates config containing path of savefile
     * \param savepath path of savefile
     */
    void UpdateConfig(const QString& savepath);
    /*!
     * Retrieves path of savefile from config or empty string if config file does not exist.
     * \returns savepath path of savefile
     */
    QString RetrieveSavePath();
    /*!
     * Retrieves groups for selecting common members from the ui.
     * \return groups for selecting common members from the ui.
     */
    QStringList SelectActiveGroups();
    void RequestStop();
    void RequestAuth();

    Ui::MainWindow *ui;

    QMovie sqlLoading;
    QMovie authLoading;

    QSqlTableModel* model_;
    QStringList selected_groups_;

    WebEngineView view_;
    Parser* parser_;
    DataBaseManager* dbmanager_;
    QThread dbthread_;

    QTimer update_timer_;
    bool stopping_{false};
};


#endif // MAINWINDOW_H
