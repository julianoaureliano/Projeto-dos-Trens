#include "trem.h"
#include <QtGlobal>

Malha::Malha() {
    for (int i = 0; i < 9; ++i) regioes[i].release();
}

void Trem::adicionar(int x, int y, int acao) {
    Ponto p = { QPoint(x, y), acao };
    percurso.append(p);
}

Trem::Trem(int numero, Malha *rede, QObject *parent)
    : QThread(parent), id(numero), malha(rede), velocidade(80), executando(1) {
    for (int i = 0; i < 9; ++i) reservadas[i] = false;
    // Coordenadas sao centros dos trens; cada trajeto e anti-horario.
    // Os pontos de reserva ficam a pelo menos 30 pixels dos cruzamentos.
    switch (id) {
    case 0: // a: regioes 1 e 2
        regioes << 0 << 1;
        adicionar(300,60); adicionar(150,60); adicionar(150,110,1);
        adicionar(150,140); adicionar(330,140,10); adicionar(450,140); adicionar(450,110,11);
        adicionar(450,60); adicionar(300,60); break;
    case 1: // b: regioes 1, 3, 4 e 5
        regioes << 0 << 2 << 3 << 4;
        adicionar(150,200); adicionar(150,230,1); adicionar(150,260);
        adicionar(255,260,13); adicionar(300,260); adicionar(300,230,14);
        adicionar(300,140); adicionar(270,140,12); adicionar(150,140);
        adicionar(150,170,10); adicionar(150,200); break;
    case 2: // c: regioes 2, 3, 6 e 7
        regioes << 1 << 2 << 5 << 6;
        adicionar(450,200); adicionar(450,170,1); adicionar(450,140);
        adicionar(300,140); adicionar(300,170,11); adicionar(300,260);
        adicionar(330,260,12); adicionar(405,260,15); adicionar(450,260);
        adicionar(450,230,16); adicionar(450,200); break;
    case 3: // d: regioes 4 e 8
        regioes << 3 << 7;
        adicionar(75,320); adicionar(75,380); adicionar(195,380,1);
        adicionar(225,380); adicionar(225,260); adicionar(195,260,17); adicionar(75,260);
        adicionar(75,290,13); adicionar(75,320); break;
    case 4: // e: regioes 5, 6, 8 e 9
        regioes << 4 << 5 << 7 << 8;
        adicionar(300,380); adicionar(345,380,1); adicionar(375,380);
        adicionar(375,260); adicionar(345,260,18); adicionar(270,260,15);
        adicionar(225,260); adicionar(225,290,14); adicionar(225,380);
        adicionar(255,380,17); adicionar(300,380); break;
    case 5: // f: regioes 7 e 9
        regioes << 6 << 8;
        adicionar(525,320); adicionar(525,290,1); adicionar(525,260);
        adicionar(375,260); adicionar(375,290,16); adicionar(375,380); adicionar(405,380,18);
        adicionar(525,380); adicionar(525,320); break;
    }
}

void Trem::definirVelocidade(int valor) { velocidade.storeRelease(qBound(0, valor, 200)); }
void Trem::encerrar() { executando.storeRelease(0); }

// A reserva e tentativa: se faltar qualquer regiao, devolvemos imediatamente
// todas as anteriores. Assim nenhum trem espera segurando uma reserva parcial,
// eliminando espera circular. Outros trens continuam tentando em suas threads.
bool Trem::reservar() {
    while (executando.loadAcquire()) {
        if (velocidade.loadAcquire() == 0) { msleep(10); continue; }
        int obtidos = 0;
        while (obtidos < regioes.size() && malha->regioes[regioes[obtidos]].tryAcquire()) ++obtidos;
        if (obtidos == regioes.size()) {
            for (int i = 0; i < regioes.size(); ++i) reservadas[regioes[i]] = true;
            return true;
        }
        while (obtidos > 0) malha->regioes[regioes[--obtidos]].release();
        msleep(2 + id); // reduz disputas repetidas entre as threads
    }
    return false;
}
void Trem::liberar(int regiao) {
    if (reservadas[regiao]) {
        reservadas[regiao] = false;
        malha->regioes[regiao].release();
    }
}
void Trem::liberarTodas() {
    for (int i = 0; i < 9; ++i) liberar(i);
}
void Trem::run() {
    QPoint pos = percurso.first().pos;
    int destino = 1;
    while (executando.loadAcquire()) {
        int v = velocidade.loadAcquire();
        if (v == 0) { msleep(10); continue; }
        const QPoint alvo = percurso[destino].pos;
        if (pos.x() != alvo.x()) pos.rx() += (alvo.x() > pos.x() ? 5 : -5);
        else if (pos.y() != alvo.y()) pos.ry() += (alvo.y() > pos.y() ? 5 : -5);
        emit updateGUI(id, pos.x(), pos.y());
        if (pos == alvo) {
            const int acao = percurso[destino].acao;
            if (acao == 1 && !reservar()) break;
            if (acao >= 10 && acao <= 18) liberar(acao - 10);
            destino = (destino + 1) % percurso.size();
            if (destino == 0) destino = 1;
        }
        // 200 corresponde a 1 ms por passo; 0 interrompe o movimento.
        msleep(static_cast<unsigned long>(1 + (200 - v) * (200 - v) / 400));
    }
    liberarTodas();
}
