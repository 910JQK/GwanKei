#ifndef GWANKEI_AI_HPP
#define GWANKEI_AI_HPP


#include <QObject>
#include "game.hpp"


using namespace GwanKei;


class AI : public QObject {
  Q_OBJECT
public:
  Player player;
  AI(Player player);
  virtual Layout get_layout() = 0;
signals:
  void move(Cell from, Cell to);
public slots:
  virtual void status_changed(Game game, Player current_player) = 0;
};


class Brainless : public AI {
  Q_OBJECT
public:
  Brainless(Player player);
  Layout get_layout();
public slots:
  void status_changed(Game game, Player current_player);
};


class LowIQ : public AI {
  Q_OBJECT
private:
  double aggressive;
  int least[105] = {0};
  bool is_proper_layout(const Layout& layout) const;
public:
  LowIQ(Player player, double aggressive = 0.5);
  Layout get_layout();
public slots:
  void status_changed(Game game, Player current_player);
};


#endif //GWANKEI_AI_HPP
