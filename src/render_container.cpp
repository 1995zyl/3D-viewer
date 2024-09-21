#include "render_container.h"
#include "opengl/opengl_window.h"
#include "vulkan/vulkan_window.h"
#include "qt3d/qt3d_window.h"
#include "spdlog/spdlog.h"


static const std::array<float, 7> gWheelScales{0.10, 0.40, 0.70, 1.0, 2.0, 5.0, 10.0};

RenderContainer::RenderContainer(RenderMode renderMode, QWidget *parent)
    : QWidget(parent), m_renderMode(renderMode)
{
    reloadRenderWindow();
}

RenderContainer::~RenderContainer()
{
    for (const auto renderWindow : m_renderWindows)
    {
        if (renderWindow != nullptr)
            delete renderWindow;
    }
}

void RenderContainer::resizeEvent(QResizeEvent *e)
{
    m_renderWindows[m_renderMode]->resizeEx(e->size());
    return QWidget::resizeEvent(e);
}

void RenderContainer::setDrawMode(RenderMode renderMode)
{
    m_renderWindows[m_renderMode]->hideEx();
    m_renderMode = (RenderMode)renderMode;
    reloadRenderWindow();
    m_renderWindows[m_renderMode]->resizeEx(size());
    m_renderWindows[m_renderMode]->showEx();
}

void RenderContainer::loadModel(const QString &modelPath)
{
    if (modelPath.isEmpty() || !m_modelPath.compare(modelPath))
    {
        spdlog::warn("modelPath is error. modelPath: {}, m_modelPath: {}", modelPath.toStdString(), m_modelPath.toStdString());
        return;
    }

    m_modelPath = modelPath;
    reloadRenderWindow();
    m_renderWindows[m_renderMode]->resizeEx(size());
    m_renderWindows[m_renderMode]->showEx();
}

void RenderContainer::setBgColor(const QColor &color)
{
    m_color = color;
    m_renderWindows[m_renderMode]->setBgColor(m_color);
}

void RenderContainer::setWheelScale(int index)
{
    if (index < 0 || index >= gWheelScales.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, gWheelScales.size());
        return;
    }

    m_renderWindows[m_renderMode]->setWheelScale(gWheelScales[index]);
}

void RenderContainer::startAnimation(int index)
{
    m_animationType = index;
    m_renderWindows[m_renderMode]->stopAnimation();
    m_renderWindows[m_renderMode]->startAnimation(m_animationType);
}

void RenderContainer::stopAnimation()
{
    m_renderWindows[m_renderMode]->stopAnimation();
}

void RenderContainer::reloadRenderWindow()
{
    emit sigRenderWindowChange(m_renderMode);

    auto it = m_renderWindows.find(m_renderMode);
    if (m_renderWindows.end() != it && !(*it)->getModelPath().compare(m_modelPath))
        return;
    
    if (m_renderWindows.end() == it)
        m_renderWindows.insert(m_renderMode, nullptr);
    else if (nullptr != m_renderWindows[m_renderMode])
    {
        delete m_renderWindows[m_renderMode];
        m_renderWindows[m_renderMode] = nullptr;
    }

    switch (m_renderMode)
    {
    case VULKAN_MODE:
        m_renderWindows[m_renderMode] = new VulkanWindowContainer(m_modelPath, m_color, this);
        break;
    case QT3D_MODE:
        m_renderWindows[m_renderMode] = new Qt3DWindowContainer(m_modelPath, m_color, this);
        break;
    default:
        m_renderWindows[m_renderMode] = new OpenGLWindow(m_modelPath, m_color, this);
        break;
    }
}
