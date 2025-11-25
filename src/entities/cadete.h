#ifndef CADETE_H
#define CADETE_H
#include "fuerzaarmada.h"

class cadete:public FuerzaArmada
{
private:
    qreal radio;
public:
    QRectF boundingRect() const;
    cadete(qreal r, qreal _x, qreal _y);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // CADETE_H
