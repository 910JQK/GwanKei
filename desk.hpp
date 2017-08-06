#ifndef GWANKEI_DESK_HPP
#define GWANKEI_DESK_HPP

#include <QObject>
#include <QList>
#include <QString>
#include <QTimer>
#include "game.hpp"

using namespace GwanKei;
/*
class Desk : public QObject {
  Q_OBJECT
private:
  Game* game;
  bool started = false;
  Layout layout[4];
  bool ready_state[4] = {0};
  QString players[4];
  Player turn;
  QTimer timer;
  int step_count = 0;
public:
  Desk();
  ~Desk();
  QStringList get_players() const;
  bool is_player_available(Player player) const;
  bool is_player_ready(Player player) const;
signals:
  void players_changed(QStringList players);
  void ready_state_changed(QList<bool> ready_state);
  void status_changed(
      Player message_target,
      Game game,
      Player current_player,
      int wait_seconds
  );
public slots:
  void request_status_message(Player player);
  void add_player(Player player, QString name);
  void remove_player(Player player, QString name);
  void ready(Player player, Layout layout);
  void move(Player player, Cell from, Cell to);
  void skip(Player player);
  void timeout();
};
*/

#endif // GWANKEI_DESK_HPP
