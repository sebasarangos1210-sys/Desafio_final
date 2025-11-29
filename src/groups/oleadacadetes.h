#ifndef OLEADACADETES_H
#define OLEADACADETES_H

#include "agente.h"
#include "cadete.h"
#include "obstaculo.h"

/**
 * @brief Controla las oleadas del Nivel 3.
 *        Se encarga de spawnear, actualizar y detectar fin de ronda.
 */
class OleadaCadetes : public Agente
{
public:
    OleadaCadetes(Nivel *nivelPtr);
    virtual ~OleadaCadetes();

    // ----- Implementación requerida por Agente -----
    void inicializarRondas(int total) override;
    void spawnRonda(int cantidad,
                    qreal radioMin, qreal radioMax,
                    qreal angMinRad, qreal angMaxRad) override;
    void actualizar() override;
    bool rondaCompletada() const override;
    bool juegoCompletado() const override;
    Vector2D react_colision(Obstaculo* obs, FuerzaArmada* cadet, qreal step);

    // --- setters de configuración del grupo (para Nivel) ---
    inline void setPuntosObjetivo(const std::vector<Vector2D> &pts) {
        puntosObjetivo = pts;
    }

    inline const std::vector<Vector2D>& getPuntosObjetivo() const {
        return puntosObjetivo;
    }

    inline void setRadioActivacion(qreal r) { radioActivacion = r; }
    inline qreal getRadioActivacion() const { return radioActivacion; }

private:
    // Puedes agregar atributos que necesites más adelante:
    int enemigosRestantes;

    // Geometría / comportamiento del grupo de cadetes
    std::vector<Vector2D> puntosObjetivo;  // slots de flanqueo / campamento
    qreal radioActivacion = 0.0;           // para modos de campamento/emboscada

};

#endif // OLEADACADETES_H
