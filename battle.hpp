#ifndef GWANKEI_BATTLE_HPP
#define GWANKEI_BATTLE_HPP


#include <QObject>
#include "ai.hpp"
#include "desk.hpp"


using namespace GwanKei;


enum BattleType {
  BL_AI_2v2_NE,
  BL_AI_2v2_DE,
  LI_AI_1v1,
  LI_AI_2v2_NE,
  LI_AI_2v2_DE
};


class Battle : public QObject {
  Q_OBJECT
private:
  Player player;
public:
  Desk *desk;
  AI** ai;
  Battle(Player player, AI** ai, bool is_1v1, MaskMode mask_mode);
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


#endif // GWANKEI_BATTLE_HPP
