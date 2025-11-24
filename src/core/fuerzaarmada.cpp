#include "fuerzaarmada.h"

FuerzaArmada::FuerzaArmada(qreal r)
    : direccion(0.0, -1.0), radio(r), vida(100), velocidad(3.0)
{
    setPos(0, 0); // posición base
}

QRectF FuerzaArmada::boundingRect() const
{
    return QRectF(-radio, -radio, 2*radio, 2*radio);
}

QPainterPath FuerzaArmada::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void FuerzaArmada::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    // Por defecto, dibuja un círculo gris.
    painter->setBrush(Qt::gray);
    painter->drawEllipse(boundingRect());
}
