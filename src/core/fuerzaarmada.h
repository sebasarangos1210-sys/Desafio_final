#ifndef FUERZAARMADA_H
#define FUERZAARMADA_H
#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsView>
#include <QTimer>

class FuerzaArmada:public QObject, public QGraphicsPixmapItem
{

    Q_OBJECT
protected:
    qreal x = 200;
    qreal y = 200;

private:
    int spriteX = 0;
    int spriteY = 0;
    int spriteAncho = 60;
    int spriteAlto = 60;
    QPixmap hojaSprites;
    QPixmap sprite;
    int cont=0;
    QSize limites;
    QTimer *timerMov;
    int dx, dy;
    QGraphicsView *vista;
    // Municion array<int, canBalas >

public:
    virtual short int getMunucion();

};

#endif // FUERZAARMADA_H
