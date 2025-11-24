#ifndef BALA_H
#define BALA_H

#include "proyectil.h"

class Bala : public Proyectil
{
public:
    Bala(FuerzaArmada *emisor,
         const Vector2D &dir);

    void aplicarColision(FuerzaArmada *objetivo) override;
};

#endif
