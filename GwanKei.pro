QT       += core gui widgets webkitwidgets multimedia

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

TRANSLATIONS += Locale/zh_TW/gwankei.ts \
                Locale/zh_CN/gwankei.ts \
                Locale/ja_JP/gwankei.ts
