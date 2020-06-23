#ifndef SEQUENCINGSERVER_H
#define SEQUENCINGSERVER_H

#include <QFile>
#include <QTcpServer>

#include "ServerSession.h"
#include "protocol/Data.h"
#include "protocol/RemoteAction.h"
class BroadcastServer : public QObject {
  Q_OBJECT
 public:
  // TODO: Add a configurable delay
  BroadcastServer(const QByteArray &data, int port, QObject *parent = nullptr,
                  int delay_msec = 0, double drop_rate = 0);
  ~BroadcastServer();
  int port();

 private slots:
  void newClient();

 private:
  void broadcastAction(const ClientAction &m, qint64 uid);
  void broadcastNewSession(const QString &username, qint64 uid);
  void broadcastDeleteSession(qint64 uid);
  QTcpServer *m_server;
  QList<ServerAction> m_history;
  std::list<ServerSession *> m_sessions;
  QByteArray m_data;
  int m_next_uid;
  int m_delay_msec;
  double m_drop_rate;
  void broadcastServerAction(const ServerAction &a);
  void broadcastUnreliable(const ServerAction &a);
};

#endif  // SEQUENCINGSERVER_H