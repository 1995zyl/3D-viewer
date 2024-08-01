#include "render_container.h"
#include "opengl/opengl_window.h"
#include "qt3d/qt3d_window.h"
#include "spdlog/spdlog.h"

static std::array<int, 4> s_timerIntervals{600, 200, 150, 100};
static std::array<float, 7> s_wheelScales{0.10, 0.40, 0.70, 1.0, 2.0, 5.0, 10.0};

RenderContainer::RenderContainer(RenderMode renderMode, QWidget *parent)
    : QWidget(parent), m_renderMode(renderMode)
{
    reloadRenderWindow();
}

RenderContainer::~RenderContainer()
{
}

void RenderContainer::resizeEvent(QResizeEvent *e)
{
    m_renderWindow->resizeEx(e->size());
    return QWidget::resizeEvent(e);
}

void RenderContainer::setDrawMode(RenderMode renderMode)
{
    m_renderWindow->hideEx();
    m_renderMode = (RenderMode)renderMode;
    reloadRenderWindow();
    m_renderWindow->resizeEx(size());
    m_renderWindow->showEx();
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
    m_renderWindow->resizeEx(size());
    m_renderWindow->showEx();
}

void RenderContainer::setBgColor(const QColor &color)
{
    m_color = color;
    m_renderWindow->setBgColor(m_color);
}

void RenderContainer::setWheelScale(int index)
{
    if (index < 0 || index >= s_wheelScales.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, s_wheelScales.size());
        return;
    }

    m_renderWindow->setWheelScale(s_wheelScales[index]);
}

void RenderContainer::startAnimation(int index)
{
    m_animationType = index;
    m_renderWindow->stopAnimation();
    m_renderWindow->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                   s_timerIntervals[m_timerIntervalIndex]);
}

void RenderContainer::stopAnimation()
{
    m_renderWindow->stopAnimation();
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
        m_renderWindow->stopAnimation();
        m_renderWindow->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                       s_timerIntervals[m_timerIntervalIndex]);
    }
}

void RenderContainer::reloadRenderWindow()
{
    auto it = m_renderWindowLists.find(m_renderMode);
    if (m_renderWindowLists.end() != it && !(*it)->getModelPath().compare(m_modelPath))
    {
        m_renderWindow = *it;
    }
    else
    {
        switch (m_renderMode)
        {
        case QT3D_MODE:
            m_renderWindow.reset(new Qt3dWindow(m_modelPath, m_color, this));
            break;
        default:
            m_renderWindow.reset(new OpenGLWindow(m_modelPath, m_color, this));
            break;
        }
        m_renderWindowLists.insert(m_renderMode, m_renderWindow);
    }

    emit sigRenderWindowChange(m_renderMode);
}
