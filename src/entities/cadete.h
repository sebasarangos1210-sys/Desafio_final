#ifndef CADETE_H
#define CADETE_H

#include "fuerzaarmada.h"

class Cadete : public FuerzaArmada
{
public:
    Cadete(qreal r = 10.0, qreal x = 0.0, qreal y = 0.0);

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void recibirImpacto(Proyectil* p) override;
    bool esJugador() const override;

};

#endif // CADETE_H
