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
signals:
  void move(Cell from, Cell to);
public slots:
  virtual void status_changed(Game game, Player current_player) = 0;
};


class Brainless : public AI {
  Q_OBJECT
public:
  Brainless(Player player);
public slots:
  void status_changed(Game game, Player current_player);
};


#endif //GWANKEI_AI_HPP
