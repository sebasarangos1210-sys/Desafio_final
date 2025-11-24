#include "bala.h"
#include "fuerzaarmada.h"

Bala::Bala(FuerzaArmada *emisor,
           const Vector2D &dir)
    : Proyectil(emisor,
                dir,       // dirección
                12.0,      // velocidad
                3.0,       // radio
                10)        // daño
{
    setZValue(-900);
}

void Bala::aplicarColision(FuerzaArmada *objetivo)
{
    // Resta vida
    objetivo->setVida( objetivo->getVida() - 10 );

    // Aquí podrías meter animaciones / efectos
}
