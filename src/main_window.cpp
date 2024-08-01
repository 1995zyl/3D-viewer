#include "main_window.h"
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
    vLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    m_drawModeCom = new QComboBox();
    m_drawModeCom->addItem("opengl");
    m_drawModeCom->addItem("qt3d");
    m_drawModeCom->setCurrentIndex(0);

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

    hLayout->addWidget(m_drawModeCom);
    hLayout->addSpacing(4);
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
    m_renderContainer = new RenderContainer(RenderContainer::OPNEGL_MODE);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_renderContainer);
    m_renderContainer->setMinimumSize(800, 600);

    connect(m_drawModeCom, &QComboBox::currentIndexChanged, this, &MainWindow::onDrawModeChanged);
    connect(m_loadModelBtn, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(m_bgColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectBgColor);
    connect(m_wheelScaleCom, &QComboBox::currentIndexChanged, this, &MainWindow::onWheelScaleChanged);
    connect(m_animationCom, &QComboBox::currentIndexChanged, this, &MainWindow::onAnimationTypeChanged);
    connect(m_animationSpeedCom, &QComboBox::currentIndexChanged, this, &MainWindow::onAnimationShowSpeedChanged);
    connect(m_renderContainer, &RenderContainer::sigRenderWindowChange, this, &MainWindow::onRenderWindowChange);
}

void MainWindow::onDrawModeChanged(int index)
{
    m_renderContainer->setDrawMode((RenderContainer::RenderMode)index);
}

void MainWindow::onLoadModel()
{
    QString modelPath = QFileDialog::getOpenFileName(
        nullptr, tr("open 3D model"), QCoreApplication::applicationDirPath(), "*.glb;*.obj", nullptr, QFileDialog::ReadOnly);
    if (modelPath.isEmpty())
        return;

    m_renderContainer->loadModel(modelPath);
    setWindowTitle(QString("%1 - %2").arg(QFileInfo(modelPath).fileName()).arg(m_title));
    m_wheelScaleCom->setCurrentIndex(-1);
    m_animationCom->setCurrentIndex(-1);
    m_animationSpeedCom->setCurrentIndex(1);
}

void MainWindow::onSelectBgColor()
{
    m_renderContainer->setBgColor(QColorDialog::getColor(Qt::white, this));
}

void MainWindow::onWheelScaleChanged(int value)
{
    m_renderContainer->setWheelScale(value);
}

void MainWindow::onAnimationTypeChanged(int index)
{
    if (index == 0)
    {
        m_renderContainer->stopAnimation();
        m_animationCom->setCurrentIndex(-1);
        return;
    }
    m_renderContainer->startAnimation(index);
}

void MainWindow::onAnimationShowSpeedChanged(int value)
{
    m_renderContainer->setTimerInterval(value);
}

void MainWindow::onRenderWindowChange(RenderContainer::RenderMode renderMode)
{
    switch (renderMode)
    {
    case RenderContainer::QT3D_MODE:
        m_wheelScaleCom->setEnabled(false);
        m_animationCom->setEnabled(false);
        m_animationSpeedCom->setEnabled(false);
        break;
    default:
        m_wheelScaleCom->setEnabled(true);
        m_animationCom->setEnabled(true);
        m_animationSpeedCom->setEnabled(true);
        break;
    }
}
