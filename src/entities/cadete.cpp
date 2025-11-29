#include "cadete.h"

Cadete::Cadete(qreal r, qreal x, qreal y, bool esjug)
    : FuerzaArmada(r,esjug)
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
    if (p->esDeJugador() && !this->jugador){
        vida -= p->getDaño();
        //qDebug() << "vida de jugador enemigo : " << vida;
    }
    if (!p->esDeJugador() && this->esJugador()){
        vida -= p->getDaño();
        //qDebug() << "vida de jugador : " << vida;
    }
    if (vida <= 0)
        this->muerto = true;
}

bool Cadete::esJugador() const
{
    return jugador;
}

