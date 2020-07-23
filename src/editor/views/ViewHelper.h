#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QColor>
#include <QPainter>
#include <QPainterPath>

#include "MooClock.h"

extern void drawCursor(const QPoint &position, QPainter &painter,
                       const QColor &color, const QString &username,
                       qint64 uid);
extern void drawCurrentPlayerPosition(QPainter &painter, MooClock *moo_clock,
                                      int height, qreal clockPerPx,
                                      bool drawHead);
extern void drawRepeatAndEndBars(QPainter &painter, const MooClock *moo_clock,
                                 qreal clockPerPx, int height);
extern void handleWheelEventWithModifier(QWheelEvent *event,
                                         PxtoneClient *client, bool scaleY);

extern QColor halfWhite, slightTint;

#endif  // VIEWHELPER_H
