#ifndef GWANKEI_BATTLE_HPP
#define GWANKEI_BATTLE_HPP


#include <QObject>
#include "ai.hpp"
#include "desk.hpp"


using namespace GwanKei;


enum BattleType {
  BL_AI_2v2_NE,
  BL_AI_2v2_DE
};


class Battle : public QObject {
  Q_OBJECT
private:
  Player player;
public:
  Desk *desk;
  Battle(Player player);
  ~Battle();
  void init_desk();
  Player get_player() const;
  QString get_player_name(Player player) const;
  static Battle* Create(BattleType type);
signals:
  void status_changed(Game game, Player current, int wait_sec);
  void fail(Player player, FailReason reason);
  void end(Ending ending);
public slots:
  void ready(Layout layout);
  void move(Cell from, Cell to);
};


class BL_AI_2v2 : public Battle {
  Q_OBJECT
public:
  Brainless** ai;
  BL_AI_2v2(Player player, MaskMode mask_mode);
};


#endif // GWANKEI_BATTLE_HPP
