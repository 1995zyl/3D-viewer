#ifndef __RENDER_CONTAINER_H__
#define __RENDER_CONTAINER_H__

#include <QWidget>
#include "i_draw_interface.h"

class RenderContainer : public QWidget
{
    Q_OBJECT
public:
    enum RenderMode
    {
        OPNEGL_MODE,
        QT3D_MODE,
    };

public:
    explicit RenderContainer(RenderMode drawmode, QWidget *parent = nullptr);
    ~RenderContainer();
    void setDrawMode(RenderMode drawmode);
    void loadModel(const QString &modelPath);
    void setBgColor(const QColor &color);
    void setWheelScale(int index);
    void startAnimation(int index);
    void stopAnimation();
    void setTimerInterval(int index);

signals:
    void sigRenderWindowChange(RenderMode renderMode);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    void reloadRenderWindow();

private:
    QMap<RenderMode, IDrawInterface*> m_renderWindows;
    RenderMode m_renderMode;
    QColor m_color;
    int m_animationType = 0;
    int m_timerIntervalIndex = 1;
    QString m_modelPath;
};

#endif