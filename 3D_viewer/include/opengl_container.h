#ifndef __OPENGL_WINDOW_CONTAINER_H__
#define __OPENGL_WINDOW_CONTAINER_H__

#include <QWidget>

class OpenGLWindow;
class OpenGLContainer : public QWidget
{
    Q_OBJECT
public:
    explicit OpenGLContainer(QWidget *parent = nullptr);
    ~OpenGLContainer();
    void loadModel(const QString &modelPath);
    void setBgColor(const QColor &color);
    void setWheelScale(int index);
    void startAnimation(int index);
    void stopAnimation();
    void setTimerInterval(int index);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    QScopedPointer<OpenGLWindow> m_openGLWindow;
    QColor m_color;
    int m_animationType = 0;
    int m_timerIntervalIndex = 1;
};

#endif