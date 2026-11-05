#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QSlider>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>

class MalhaWidget : public QWidget {
public:
    explicit MalhaWidget(QWidget *parent = 0) : QWidget(parent) {
        setMinimumSize(600, 420);
        posicoes[0] = QPoint(300,60); posicoes[1] = QPoint(150,200);
        posicoes[2] = QPoint(450,200); posicoes[3] = QPoint(75,320);
        posicoes[4] = QPoint(300,380); posicoes[5] = QPoint(525,320);
    }
    void atualizar(int id, int x, int y) { posicoes[id] = QPoint(x,y); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#f8fafc"));
        p.translate((width()-600)/2.0, (height()-420)/2.0);
        p.setPen(QPen(QColor("#2563eb"), 3));
        p.drawRect(150,60,300,80); p.drawRect(150,140,150,120);
        p.drawRect(300,140,150,120); p.drawRect(75,260,150,120);
        p.drawRect(225,260,150,120); p.drawRect(375,260,150,120);
        const QColor cores[] = { QColor("#dc2626"), QColor("#ea580c"), QColor("#7c3aed"), QColor("#0891b2"), QColor("#16a34a"), QColor("#be185d") };

        for (int i = 0; i < 6; ++i) {
            QRect r(posicoes[i].x()-11,posicoes[i].y()-11,22,22);
            p.setPen(QPen(Qt::white, 2)); p.setBrush(cores[i]); p.drawRoundedRect(r, 4, 4);
            p.setPen(Qt::white); QFont f = p.font(); f.setBold(true); p.setFont(f);
            p.drawText(r, Qt::AlignCenter, QString(QChar('a'+i)));
        }
    }
private:
    QPoint posicoes[6];
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("Projeto dos Trens"));
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    desenho = new MalhaWidget(central);

    layout->addWidget(desenho, 1);
    QGroupBox *grupo = new QGroupBox(QString::fromUtf8("Controle de velocidade"), central);
    QGridLayout *grade = new QGridLayout(grupo);

    for (int i = 0; i < 6; ++i) {
        QLabel *nome = new QLabel(QString(QChar('a'+i)), grupo);
        controles[i] = new QSlider(Qt::Horizontal, grupo);
        controles[i]->setRange(0,200); controles[i]->setValue(80);
        QLabel *valor = new QLabel("80", grupo);
        grade->addWidget(nome, i/3, (i%3)*3);
        grade->addWidget(controles[i], i/3, (i%3)*3+1);
        grade->addWidget(valor, i/3, (i%3)*3+2);
        trens[i] = new Trem(i, &malha, this);
        connect(trens[i], &Trem::updateGUI, this, &MainWindow::updateInterface);
        connect(controles[i], &QSlider::valueChanged, trens[i], &Trem::definirVelocidade);
        connect(controles[i], &QSlider::valueChanged, valor, qOverload<int>(&QLabel::setNum));
    }

    QSlider *mestre = new QSlider(Qt::Horizontal, grupo);
    mestre->setRange(0,200); mestre->setValue(80);
    QLabel *valorMestre = new QLabel("80", grupo);
    grade->addWidget(new QLabel(QString::fromUtf8("m (geral)"), grupo),2,0);
    grade->addWidget(mestre,2,1,1,7); grade->addWidget(valorMestre,2,8);
    connect(mestre, &QSlider::valueChanged, valorMestre, qOverload<int>(&QLabel::setNum));

    for (int i = 0; i < 6; ++i) connect(mestre, &QSlider::valueChanged, controles[i], &QSlider::setValue);
    layout->addWidget(grupo);
    setCentralWidget(central);
    resize(760,620);

    for (int i = 0; i < 6; ++i) trens[i]->start();
}
void MainWindow::updateInterface(int id, int x, int y) { if (id >= 0 && id < 6) desenho->atualizar(id,x,y); }
MainWindow::~MainWindow() {
    for (int i = 0; i < 6; ++i) trens[i]->encerrar();
    for (int i = 0; i < 6; ++i) trens[i]->wait();
    delete ui;
}
