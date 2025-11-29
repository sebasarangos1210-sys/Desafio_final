#ifndef AGENTE_H
#define AGENTE_H

#include <vector>
#include <qmath.h>
#include "vector2d.h"

class FuerzaArmada;
class Nivel;

// ---- Modos de comportamiento de un grupo ----
enum class ModoGrupo {
    AtaqueDirecto,   // van directo al jugador
    Flanqueo,        // se posicionan para rodear
    Campamento,      // se quedan quietos hasta que el jugador se acerque
    Emboscada        // esperan a que otro grupo entre en combate
};

// ---- Estado interno del grupo dentro de ese modo ----
enum class EstadoGrupo {
    Preparando,      // yendo a sus posiciones iniciales
    EnPosicion,      // ya colocados, esperando trigger
    Atacando,        // persiguen / disparan
    Muerto           // todos sus miembros muertos
};

/**
 * @brief Clase abstracta que representa un controlador de grupo/oleada.
 *        No es un participante del juego, sino un "manager" lógico.
 */
class Agente
{
protected:

    Nivel *nivel;                        // dueño del agente (nivel 1,2,3...)
    std::vector<FuerzaArmada*> grupo;    // unidades controladas por este agente

    int rondaActual;     // para uso interno del agente si maneja "sub-oleadas"
    int totalRondas;

    int rondaAsignada;   // en qué oleada global aparece este grupo (1,2,3,...)
    ModoGrupo modo;      // comportamiento principal
    EstadoGrupo estado;  // etapa actual dentro de ese modo
    bool activo;         // si el nivel lo está usando en este momento

public:
    Agente(Nivel *nivelPtr);
    virtual ~Agente();

    // ----- Gestión de oleadas -----
    virtual void inicializarRondas(int total) = 0;
    virtual void spawnRonda(int cantidad,
                            qreal radioMin, qreal radioMax,
                            qreal angMinRad, qreal angMaxRad) = 0;

    // ----- Actualización por frame -----
    virtual void actualizar() = 0;

    // ----- Estado -----
    virtual bool rondaCompletada() const = 0;
    virtual bool juegoCompletado() const = 0;

    // ----- Acceso -----
    inline const std::vector<FuerzaArmada*>& getGrupo() const { return grupo; }
    inline int getRondaActual() const { return rondaActual; }
    inline int getTotalRondas() const { return totalRondas; }

    // --- NUEVOS getters/setters básicos ---
    inline void      setModo(ModoGrupo m)          { modo = m; }
    inline ModoGrupo getModo() const               { return modo; }

    inline void        setEstado(EstadoGrupo e)    { estado = e; }
    inline EstadoGrupo getEstado() const           { return estado; }

    inline void setRondaAsignada(int r)            { rondaAsignada = r; }
    inline int  getRondaAsignada() const           { return rondaAsignada; }

    inline void setActivo(bool a)                  { activo = a; }
    inline bool estaActivo() const                 { return activo; }

};

#endif // AGENTE_H
