#ifndef __QT3D_WINDOW_H__
#define __QT3D_WINDOW_H__

#include "i_draw_interface.h"
#include <QWidget>
#include <Qt3dwindow>
#include <QCamera>
#include <QPointlight>
#include <QFirstpersoncameracontroller>

class Qt3dWindow : public QWidget,
                   public IDrawInterface
{
    Q_OBJECT
public:
    enum AnimationType
    {
        Turntable = 1,
        Sway,
        Hover,
    };

public:
    explicit Qt3dWindow(const QString &modelPath, const QColor &color, QWidget *parent = Q_NULLPTR);
    ~Qt3dWindow();
    void resizeEx(const QSize &size) override;
    void showEx() override;
    void hideEx() override;
    void setBgColor(const QColor &color) override;
    QString getModelPath() const override { return m_modelPath; }
    void setWheelScale(float wheelScale){}
    void startAnimation(int animationType, int millisecond){}
    void stopAnimation(){}

private:
	void init3D(const QColor& color);

private:
    QString m_modelPath;
    QWidget* m_3dContainer = nullptr;
    Qt3DExtras::Qt3DWindow* m_view = nullptr;
    Qt3DRender::QCamera* m_cameraEntity = nullptr;
    Qt3DCore::QEntity* m_lightEntity = nullptr;
    Qt3DRender::QPointLight* m_light = nullptr;
    Qt3DCore::QTransform* m_lightTransform = nullptr;
    Qt3DExtras::QFirstPersonCameraController* m_camController = nullptr;
    QColor m_bgColor;
    
};

#endif
