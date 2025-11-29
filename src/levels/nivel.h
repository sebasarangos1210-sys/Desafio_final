#ifndef NIVEL_H
#define NIVEL_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <vector>

#include "vector2d.h"
#include "fuerzaarmada.h"
#include "cadete.h"
#include "bala.h"
#include "obstaculo.h"
//#include "oleadacadetes.h"
#include "agente.h"

class Nivel : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel(int numeroNivel, QWidget *parent = nullptr,
                   qreal _v_alto = 700, qreal _v_ancho = 1100);
    void disparar(FuerzaArmada *emisor);
    inline QGraphicsPixmapItem* getFondoScroll() const { return fondoScroll; }
    inline QGraphicsScene* getEscena() const { return escena; }
    inline Vector2D getfondoSize() const { return fondoSize; }
    inline Vector2D getJugDir() const { return jugador->getDireccion(); }

    void registrarEnemigo(FuerzaArmada *e) { enemigos.push_back(e); }

    //registrar un agente en el nivel
    inline void registrarAgente(Agente *a) {
        if (a) agentes.push_back(a);
    }

    inline const std::vector<Agente*>& getAgentes() const {
        return agentes;
    }

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
    Vector2D ant_camara;
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
    std::vector<Agente*> agentes;
    int ronda_act;
    int total_rondas;

    bool m_moveLeft;
    bool m_moveRight;
    bool m_moveUp;
    bool m_moveDown;

    QWidget *hud;
    QPushButton *btnVolver;
    QLabel *lblVida;
    QLabel *lblEnemigos;
    QLabel *lblRonda;
    QLabel *lblBalas;
    QProgressBar *barraVida;

    void inicializarUI();
    void inicializarEscena();
    void cargarElementosNivel();

    bool jugadorTocaObstaculo() const;

    void devolverPosicionFondo();
    void actualizarFondo();
    void manejarColisiones();
    void actualizarIA();
    void actualizarPosicionFondo();
    void actualizarHUD();

    std::vector<Proyectil*> proyectiles;

};

#endif // NIVEL_H
