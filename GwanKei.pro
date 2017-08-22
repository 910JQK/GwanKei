QT       += core gui widgets webkitwidgets

TARGET = GwanKei
TEMPLATE = app

DEFINES += "DEBUG"

SOURCES += main.cpp \
	battle.cpp \
	ai.cpp \
	gui.cpp \
	desk.cpp \
	game.cpp \
	core.cpp
HEADERS += battle.hpp \
           ai.hpp \
           gui.hpp \
           desk.hpp \
           game.hpp \
           core.hpp
