#ifndef OBSTACULO_H
#define OBSTACULO_H

#include <QPointF>
#include "hitbox.h"

class Obstaculo
{
public:
    Obstaculo();

    void setPosition(const QPointF &pos);
    QPointF position() const;

    Hitbox &hitbox();
    const Hitbox &hitbox() const;

private:
    QPointF m_position;
    Hitbox m_hitbox;
};

#endif // OBSTACULO_H
