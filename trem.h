#ifndef TREM_H
#define TREM_H

#include <QThread>
#include <QAtomicInt>
#include <QPoint>
#include <QSemaphore>
#include <QVector>

// Os nove semaforos representam as regioes numeradas do enunciado.
struct Malha {
    QSemaphore regioes[9];
    Malha();
};

class Trem : public QThread {
    Q_OBJECT
public:
    Trem(int id, Malha *malha, QObject *parent = 0);
    void definirVelocidade(int valor);
    void encerrar();
    void run() override;
signals:
    void updateGUI(int id, int x, int y);
private:
    struct Ponto { QPoint pos; int acao; }; // 1: reservar; 10..18: liberar regiao
    void adicionar(int x, int y, int acao = 0);
    bool reservar();
    void liberar(int regiao);
    void liberarTodas();
    int id;
    Malha *malha;
    QAtomicInt velocidade;
    QAtomicInt executando;
    QVector<int> regioes;
    bool reservadas[9];
    QVector<Ponto> percurso;
};
#endif
