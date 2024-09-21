#include "vulkan_window.h"
#include "spdlog/spdlog.h"
#include <QGridLayout>
#include <QMouseEvent>


static QVulkanInstance* getVulkanInstance()
{
    auto f = []() -> QVulkanInstance* 
    {
        static QVulkanInstance gVulkanIns;
        if (!gVulkanIns.create())
            return nullptr;
        return &gVulkanIns;
    };
    static QVulkanInstance* pVulkanIns = f();
    return pVulkanIns;
}

/// <summary>
/// 
/// </summary>
/// <param name="modelPath"></param>
/// <param name="color"></param>
/// <param name="parent"></param>
VulkanWindowContainer::VulkanWindowContainer(const QString &modelPath, const QColor &color, QWidget *parent)
	: QWidget(parent), m_modelPath(modelPath)
{
    m_vulkanWindow = new VulkanWindow(m_modelPath, color);
    m_vulkanWindow->setVulkanInstance(getVulkanInstance());
    QWidget* wrapper = QWidget::createWindowContainer(m_vulkanWindow);
    wrapper->setFocusPolicy(Qt::StrongFocus);
    wrapper->setFocus();
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
}

VulkanWindowContainer::~VulkanWindowContainer()
{
    qDebug() << "VulkanWindowContainer::~VulkanWindowContainer()";
}

void VulkanWindowContainer::resizeEx(const QSize &size)
{
	resize(size);
}

void VulkanWindowContainer::showEx()
{
	show();
}

void VulkanWindowContainer::hideEx()
{
	hide();
}

void VulkanWindowContainer::setBgColor(const QColor &color)
{
    m_vulkanWindow->setBgColor(color);
}

void VulkanWindowContainer::setWheelScale(float wheelScale)
{

}

void VulkanWindowContainer::startAnimation(int animationType)
{
    m_vulkanWindow->startAnimation(animationType);
}

void VulkanWindowContainer::stopAnimation()
{
    m_vulkanWindow->stopAnimation();
}


/// <summary>
/// 
/// </summary>
/// <param name="modelPath"></param>
/// <param name="color"></param>
/// <param name="parent"></param>
VulkanWindow::VulkanWindow(const QString& modelPath, const QColor& color, QWindow *parent)
    : QVulkanWindow(parent), m_bgColor(color)
{
    if (!modelPath.isEmpty())
    {
        m_vulkanMeshPtr.reset(new VulkanMesh());
        m_vulkanMeshPtr->load(modelPath);
    }
}

VulkanWindow::~VulkanWindow()
{
    qDebug() << "VulkanWindow::~VulkanWindow()";
}

QVulkanWindowRenderer *VulkanWindow::createRenderer()
{
    m_renderer = new VulkanRenderer(this, m_bgColor, m_vulkanMeshPtr);
    return m_renderer;
}

void VulkanWindow::setBgColor(const QColor& color)
{
    if (m_renderer)
        m_renderer->setBgColor(color);
}

void VulkanWindow::startAnimation(int animationType)
{
    if (m_renderer)
        m_renderer->startAnimation(animationType);
}

void VulkanWindow::stopAnimation()
{
    if (m_renderer)
        m_renderer->stopAnimation();
}

void VulkanWindow::mousePressEvent(QMouseEvent *e)
{
    m_mousePress = true;
    m_mousePos = e->pos();
    switch (e->button())
    {
    case Qt::LeftButton:
        m_mouseFlag = Qt::LeftButton;
        break;
    case Qt::RightButton:
        m_mouseFlag = Qt::RightButton;
        break;
    case Qt::MiddleButton:
        m_mouseFlag = Qt::MiddleButton;
        break;
    }
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *e)
{
    m_mousePress = false;
}

void VulkanWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mousePress)
        return;

    QPoint diff = m_mousePos - e->pos();
    switch (m_mouseFlag)
    {
    case Qt::LeftButton:
    case Qt::MiddleButton:
        break;
    case Qt::RightButton:
    {
        if (diff.y())
            m_renderer->pitch(diff.y() / 10.0f);
        if (diff.x())
            m_renderer->yaw(diff.x() / 10.0f);
        break;
    }
    default:
        break;
    }

    m_mousePos = e->pos();
}

void VulkanWindow::keyPressEvent(QKeyEvent *e)
{
    const float amount = e->modifiers().testFlag(Qt::ShiftModifier) ? 1.0f : 0.1f;
    switch (e->key())
    {
    case Qt::Key_W:
        m_renderer->walk(amount);
        break;
    case Qt::Key_S:
        m_renderer->walk(-amount);
        break;
    case Qt::Key_A:
        m_renderer->strafe(-amount);
        break;
    case Qt::Key_D:
        m_renderer->strafe(amount);
        break;
    default:
        break;
    }
}

void VulkanWindow::wheelEvent(QWheelEvent* e)
{
    if (m_mousePress)
        return;

    if (e->angleDelta().y() > 0)
        m_renderer->walk(m_wheelScale);
    else
        m_renderer->walk(-m_wheelScale);
}
