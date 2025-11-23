#ifndef NIVEL_H
#define NIVEL_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>
#include <vector>

// Declaraciones
class FuerzaArmada;
class Obstaculo;
class Agente;

class Nivel : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel(int numeroNivel, QWidget *parent = nullptr,
                   qreal _v_alto = 800, qreal _v_ancho = 1000);
    ~Nivel();

signals:
    void volverAlMenu();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void actualizarJuego(); // El timer
    void onVolverClicked();

private:
    // Número del nivel
    qreal v_alto;
    qreal v_ancho;
    int numNivel;

    // Sistema gráfico
    QGraphicsView *vista;
    QGraphicsScene *escena;

    //Fondo
    QGraphicsPixmapItem *fondoScroll;
    qreal limx;
    qreal limy;
    qreal fondoOffsetX;
    qreal fondoOffsetY;

    // EL timer
    QTimer *timer;

    // Elementos del juego
    std::vector<FuerzaArmada*> enemigos;
    std::vector<Obstaculo*> obstaculos;
    FuerzaArmada *jugador;
    Agente *ia;

    // Input
    bool m_moveLeft;
    bool m_moveRight;
    bool m_moveUp;
    bool m_moveDown;

    // UI
    QPushButton *btnVolver;

    // Métodos de inicialización
    void inicializarUI();
    void inicializarEscena();
    void cargarElementosNivel();

    // Lógica del juego
    void actualizarFondo();
    void manejarColisiones();
    void actualizarIA();
};

#endif // NIVEL_H
