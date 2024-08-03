#include "render_container.h"
#include "opengl/opengl_window.h"
#include "qt3d/qt3d_window.h"
#include "spdlog/spdlog.h"

static const std::array<int, 4> s_timerIntervals{600, 200, 150, 100};
static const std::array<float, 7> s_wheelScales{0.10, 0.40, 0.70, 1.0, 2.0, 5.0, 10.0};

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
    if (index < 0 || index >= s_wheelScales.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, s_wheelScales.size());
        return;
    }

    m_renderWindows[m_renderMode]->setWheelScale(s_wheelScales[index]);
}

void RenderContainer::startAnimation(int index)
{
    m_animationType = index;
    m_renderWindows[m_renderMode]->stopAnimation();
    m_renderWindows[m_renderMode]->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                   s_timerIntervals[m_timerIntervalIndex]);
}

void RenderContainer::stopAnimation()
{
    m_renderWindows[m_renderMode]->stopAnimation();
}

void RenderContainer::setTimerInterval(int index)
{
    if (m_timerIntervalIndex == index || index < 0 || index >= s_timerIntervals.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, s_timerIntervals.size());
        return;
    }

    m_timerIntervalIndex = index;
    if (m_animationType != 0)
    {
        m_renderWindows[m_renderMode]->stopAnimation();
        m_renderWindows[m_renderMode]->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                       s_timerIntervals[m_timerIntervalIndex]);
    }
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
    case QT3D_MODE:
        m_renderWindows[m_renderMode] = new Qt3DWindowContainer(m_modelPath, m_color, this);
        break;
    default:
        m_renderWindows[m_renderMode] = new OpenGLWindow(m_modelPath, m_color, this);
        break;
    }
}
