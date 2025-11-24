#ifndef NIVEL_H
#define NIVEL_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>
#include <vector>

#include "vector2d.h"
#include "fuerzaarmada.h"
#include "cadete.h"
#include "bala.h"
#include "obstaculo.h"

class Agente;

class Nivel : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel(int numeroNivel, QWidget *parent = nullptr,
                   qreal _v_alto = 700, qreal _v_ancho = 1100);
    void disparar(FuerzaArmada *emisor);
    ~Nivel();

signals:
    void volverAlMenu();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void actualizarJuego();
    void onVolverClicked();
    void disparosEnemigos();

private:
    int numNivel;

    Vector2D viewportSize;
    Vector2D fondoSize;
    Vector2D limiteMapa;
    Vector2D camara;
    Vector2D mouseDir;

    QGraphicsView  *vista;
    QGraphicsScene *escena;
    QGraphicsPixmapItem *fondoScroll;

    QTimer *timer;

    std::vector<FuerzaArmada*> enemigos;
    std::vector<Obstaculo*> obstaculos;
    QTimer *timerDisparoEnemigos;

    Cadete *jugador;
    Agente *ia;

    bool m_moveLeft;
    bool m_moveRight;
    bool m_moveUp;
    bool m_moveDown;

    QPushButton *btnVolver;

    void inicializarUI();
    void inicializarEscena();
    void cargarElementosNivel();

    void actualizarFondo();
    void manejarColisiones();
    void actualizarIA();
    void actualizarPosicionFondo();

    std::vector<Proyectil*> proyectiles;

};

#endif // NIVEL_H
