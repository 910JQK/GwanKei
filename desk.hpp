#ifndef GWANKEI_DESK_HPP
#define GWANKEI_DESK_HPP

#include <QObject>
#include <QList>
#include <QString>
#include <QTimer>
#include <vector>
#include "game.hpp"

using namespace GwanKei;

const unsigned int WAITING_TIME = 30;

enum End4Player {
  Tie,
  Win,
  GG
  // 勿更改此處順序，作下標用
};

typedef std::vector<End4Player> Ending;

enum FailReason {
  FlagLost,          // 軍棋被扛
  NoLivingPiece,     // 無棋可走
  Surrender,         // 投降
  Timeout            // 超時
  // 順序也不要改
};

class Desk : public QObject {
  Q_OBJECT
private:
  Game* game;
  QTimer* timer;
  bool started = false;
  Layout layouts[4];
  bool ready_state[4] = {0};
  bool failed[4] = {0};
  QString players[4] = {"", "","",""};
  Player current_player;
  MaskMode mask_mode;
  bool is_1v1;
  int step_count = 0;
private:
  void change_status(int single_player = -1);
  void next_turn();
  void try_to_start();
  bool check_ending();
public:
  Desk(MaskMode mask_mode = NoExpose, bool is_1v1 = false);
  ~Desk();
  QString get_player_name(Player player) const;
  QStringList get_players() const;
  QList<bool> get_ready_state() const;
  bool is_player_available(Player player) const;
  bool is_1v1_desk() const;
signals:
  void players_changed(QStringList players);
  void ready_state_changed(QList<bool> ready_state);
  void status_changed(
      Player message_target,
      Game game,
      Player current_player,
      int wait_seconds
  );
  void fail(Player player, FailReason reason);
  void end(Ending ending);
public slots:
  void timeout(); // timer 
  void set_player(Player player, QString name);
  void remove_player(QString name);
  void request_status_message(Player player);
  void ready(Player player, Layout layout); // ready 人數夠多則自動開局
  void move(Player player, Cell from, Cell to);
  void skip(Player player);
};

#endif // GWANKEI_DESK_HPP
