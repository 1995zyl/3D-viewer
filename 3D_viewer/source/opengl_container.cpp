#include "opengl_container.h"
#include "opengl_window.h"
#include "spdlog/spdlog.h"

static std::array<int, 4> s_timerIntervals{600, 200, 150, 100};
static std::array<float, 6> s_wheelScales{0.25, 0.5, 1.0, 2.5, 5.0};

OpenGLContainer::OpenGLContainer(QWidget *parent)
    : QWidget(parent), m_openGLWindow(new OpenGLWindow("", Qt::black, this))
{
}

OpenGLContainer::~OpenGLContainer()
{
}

void OpenGLContainer::resizeEvent(QResizeEvent *e)
{
    m_openGLWindow->resize(e->size());
    return QWidget::resizeEvent(e);
}

void OpenGLContainer::loadModel(const QString &modelPath)
{
    m_openGLWindow.reset(new OpenGLWindow(modelPath, m_color, this));
    m_openGLWindow->resize(size());
    m_openGLWindow->show();
}

void OpenGLContainer::setBgColor(const QColor &color)
{
    m_color = color;
    m_openGLWindow->setBgColor(m_color);
}

void OpenGLContainer::setWheelScale(int index)
{
    if (index < 0 || index >= s_wheelScales.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, s_wheelScales.size());
        return;
    }

    m_openGLWindow->setWheelScale(s_wheelScales[index]);
}

void OpenGLContainer::startAnimation(int index)
{
    m_animationType = index;
    m_openGLWindow->stopAnimation();
    m_openGLWindow->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                   s_timerIntervals[m_timerIntervalIndex]);
}

void OpenGLContainer::stopAnimation()
{
    m_openGLWindow->stopAnimation();
}

void OpenGLContainer::setTimerInterval(int index)
{
    if (m_timerIntervalIndex == index || index < 0 || index >= s_timerIntervals.size())
    {
        spdlog::error("index out of range. index: {}, size: {}", index, s_timerIntervals.size());
        return;
    }

    m_timerIntervalIndex = index;
    if (m_animationType != 0)
    {
        m_openGLWindow->stopAnimation();
        m_openGLWindow->startAnimation(static_cast<OpenGLWindow::AnimationType>(m_animationType),
                                       s_timerIntervals[m_timerIntervalIndex]);
    }
}