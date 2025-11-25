#ifndef FUERZAARMADA_H
#define FUERZAARMADA_H

#include <QGraphicsItem>
#include <QPainter>
#include "vector2d.h"
#include "proyectil.h"

class FuerzaArmada : public QGraphicsItem
{
protected:
    Vector2D direccion;    // Unitario
    qreal radio;           // Para hitbox circular
    int vida = 10;
    qreal velocidad;
    bool muerto;

public:
    FuerzaArmada(qreal r = 10.0);

    // ----- MOVIMIENTO -----
    inline void setDireccion(const Vector2D &d) {
        if (d.magnitud2() > 0.0)
            direccion = d.normalizado();
    }

    inline Vector2D getDireccion() const { return direccion; }

    inline void setVelocidad(qreal v) { velocidad = v; }
    inline qreal getVelocidad() const { return velocidad; }

    // ----- VIDA -----
    inline void setVida(int v) { vida = v; }
    inline int getVida() const { return vida; }

    // ----- GRAFICOS -----
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // Cada subclase dibuja su propia representación
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;

    //---- Colisiones -----
    virtual void recibirImpacto(Proyectil* p) = 0;
    virtual bool esJugador() const = 0;

};

#endif // FUERZAARMADA_H
