#include "main_window.h"
#include "opengl_container.h"
#include <QLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_title(tr("3D viewer"))
{
    setWindowIcon(QIcon(":/ad-product.svg"));
    setWindowTitle(m_title);

    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    m_loadModelBtn = new QPushButton(tr("load model"));
    m_bgColorBtn = new QPushButton(tr("background color"));
    m_wheelScaleCom = new QComboBox();
    m_animationCom = new QComboBox();
    m_animationSpeedCom = new QComboBox();

    m_wheelScaleCom->setPlaceholderText(tr("wheel scale"));
    m_animationCom->setPlaceholderText(tr("quick animation"));
    m_wheelScaleCom->addItem("10%");
    m_wheelScaleCom->addItem("40%");
    m_wheelScaleCom->addItem("70%");
    m_wheelScaleCom->addItem("100%");
    m_wheelScaleCom->addItem("200%");
    m_wheelScaleCom->addItem("500%");
    m_wheelScaleCom->addItem("1000%");
    m_animationCom->addItem(tr("No"));
    m_animationCom->addItem(tr("Turntable"));
    m_animationCom->addItem(tr("Sway"));
    m_animationCom->addItem(tr("Hover"));
    m_animationSpeedCom->addItem("x 0.5");
    m_animationSpeedCom->addItem("x 1.0");
    m_animationSpeedCom->addItem("x 1.5");
    m_animationSpeedCom->addItem("x 2.0");
    m_wheelScaleCom->setCurrentIndex(-1);
    m_animationCom->setCurrentIndex(-1);
    m_animationSpeedCom->setCurrentIndex(1);
    m_wheelScaleCom->setEnabled(false);
    m_animationCom->setEnabled(false);
    m_animationSpeedCom->setEnabled(false);

    hLayout->addWidget(m_loadModelBtn);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_bgColorBtn);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_wheelScaleCom);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_animationCom);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_animationSpeedCom);
    hLayout->addStretch();
    m_openGLContainer = new OpenGLContainer;
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_openGLContainer);
    m_openGLContainer->setMinimumSize(800, 500);
    showMaximized();

    connect(m_loadModelBtn, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(m_bgColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectBgColor);
    connect(m_wheelScaleCom, &QComboBox::currentIndexChanged, this, &MainWindow::onWheelScaleChanged);
    connect(m_animationCom, &QComboBox::currentIndexChanged, this, &MainWindow::onAnimationTypeChanged);
    connect(m_animationSpeedCom, &QComboBox::currentIndexChanged, this, &MainWindow::onAnimationShowSpeedChanged);
}

void MainWindow::onLoadModel()
{
    QString modelPath = QFileDialog::getOpenFileName(nullptr, tr("open 3D model"),
                                                     QCoreApplication::applicationDirPath(), "*.glb;*.obj;*.fbx;", nullptr, QFileDialog::ReadOnly);
    if (modelPath.isEmpty())
        return;

    m_openGLContainer->loadModel(modelPath);
    setWindowTitle(QString("%1 - %2").arg(QFileInfo(modelPath).fileName()).arg(m_title));
    m_wheelScaleCom->setEnabled(true);
    m_animationCom->setEnabled(true);
    m_animationSpeedCom->setEnabled(true);
    m_wheelScaleCom->setCurrentIndex(-1);
    m_animationCom->setCurrentIndex(-1);
    m_animationSpeedCom->setCurrentIndex(1);
}

void MainWindow::onSelectBgColor()
{
    m_openGLContainer->setBgColor(QColorDialog::getColor(Qt::white, this));
}

void MainWindow::onWheelScaleChanged(int index)
{
    m_openGLContainer->setWheelScale(index);
}

void MainWindow::onAnimationTypeChanged(int index)
{
    if (index == 0)
    {
        m_openGLContainer->stopAnimation();
        m_animationCom->setCurrentIndex(-1);
        return;
    }
    m_openGLContainer->startAnimation(index);
}

void MainWindow::onAnimationShowSpeedChanged(int index)
{
    m_openGLContainer->setTimerInterval(index);
}