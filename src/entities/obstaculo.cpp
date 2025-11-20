#include "obstaculo.h"

Obstaculo::Obstaculo()
    : m_position(0.0, 0.0)
{
    // Hitbox inicial sencilla: rectángulo tipo roca
    QVector<QPointF> puntos;
    puntos << QPointF(-15, -15)
           << QPointF( 15, -15)
           << QPointF( 15,  15)
           << QPointF(-15,  15);
    m_hitbox.setLocalPoints(puntos);
}

void Obstaculo::setPosition(const QPointF &pos)
{
    m_position = pos;
}

QPointF Obstaculo::position() const
{
    return m_position;
}

Hitbox &Obstaculo::hitbox()
{
    return m_hitbox;
}

const Hitbox &Obstaculo::hitbox() const
{
    return m_hitbox;
}
