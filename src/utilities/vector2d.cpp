#include "vector2d.h"
/*
Vector2D::rotCanonicoY(const Vector2D &o) const{
    Vector2D unit;
    if(this->magnitud()!=1.0) unit = this->normalizado();
    qreal n_x = unit.y()*o.y() + -unit.x()*o.x();
    qreal n_y = unit.x()*o.y() + unit.y()*o.x();
    return Vector2D(n_x,n_y);
}
*/

// this = dirección forward (jugador -> mouse), NO tiene por qué venir normalizado
Vector2D Vector2D::rotCanonicoY(const Vector2D &local) const
{
    // forward = this normalizado
    Vector2D forward = this->normalizado();

    // right = perpendicular a forward (hacia la derecha)
    Vector2D right(forward.y(), -forward.x());

    // world = combinacion lineal: local.x * right + local.y * forward
    return right * local.x() + forward * local.y();
}
