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
class Participantes;
class Obstaculo;
class Agente;

class Nivel : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel(int numeroNivel, QWidget *parent = nullptr);
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
    int numNivel;

    // Sistema gráfico
    QGraphicsView *vista;
    QGraphicsScene *escena;

    // EL timer
    QTimer *timer;

    // Elementos del juego
    std::vector<Participantes*> participantes;
    std::vector<Obstaculo*> obstaculos;
    Participantes *jugador;
    Agente *ia;

    // UI
    QPushButton *btnVolver;

    // Métodos de inicialización
    void inicializarUI();
    void inicializarEscena();
    void cargarElementosNivel();

    // Lógica del juego
    void manejarColisiones();
    void actualizarIA();
};

#endif // NIVEL_H
