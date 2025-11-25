#include "cadete.h"

Cadete::Cadete(qreal r, qreal x, qreal y)
    : FuerzaArmada(r)
{
    setPos(x, y);
}

void Cadete::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    // Dibujar cuerpo (círculo verde)
    painter->setBrush(Qt::blue);
    painter->drawEllipse(boundingRect());

    // Dibujar arma en dirección del vector
    painter->setPen(QPen(Qt::black, 2));
    QPointF centro(0, 0);

    qreal largo = radio * 1.8;
    QPointF punta(direccion.x() * largo,
                  direccion.y() * largo);

    painter->drawLine(centro, punta);
}

void Cadete::recibirImpacto(Proyectil* p)
{
    vida -= p->getDaño();
    if (vida <= 0)
        this->muerto = true;
}

bool Cadete::esJugador() const
{
    return true;  // luego si quieres haces un flag para distinguir jugador/enemigo
}

