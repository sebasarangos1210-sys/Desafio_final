#include "cadete.h"

cadete::cadete(qreal r, qreal _x, qreal _y)
    :
    FuerzaArmada(),
    radio(r)
{
    this->x = _x;
    this->y = _y;
}

QRectF cadete::boundingRect() const
{
    return QRectF(x,y,2*radio,2*radio);
}

void cadete::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setBrush(Qt::darkGreen);
    painter->drawEllipse(cadete::boundingRect());
}
