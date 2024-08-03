#ifndef __QT3D_WINDOW_H__
#define __QT3D_WINDOW_H__

#include "i_draw_interface.h"
#include <QWidget>
#include <Qt3dwindow>
#include <QCamera>
#include <QPointlight>
#include <QFirstpersoncameracontroller>
#include <Qt3DRender/private/qsceneimporter_p.h>

class Q3DWindowEx;
class Qt3DWindowContainer : public QWidget, public IDrawInterface
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
    explicit Qt3DWindowContainer(const QString &modelPath, const QColor &color, QWidget *parent = Q_NULLPTR);
    ~Qt3DWindowContainer();
    void resizeEx(const QSize &size) override;
    void showEx() override;
    void hideEx() override;
    void setBgColor(const QColor &color) override;
    QString getModelPath() const override { return m_modelPath; }
    void setWheelScale(float wheelScale){}
    void startAnimation(int animationType, int millisecond){}
    void stopAnimation(){}
    void setCameraPos(const QVector3D& cameraPos);

private:
	void init3D(const QColor& color);

private:
    QString m_modelPath;
    QWidget* m_3dContainer = nullptr;
    Q3DWindowEx* m_view = nullptr;
    Qt3DRender::QCamera* m_cameraEntity = nullptr;
    Qt3DCore::QEntity* m_lightEntity = nullptr;
    Qt3DRender::QPointLight* m_light = nullptr;
    Qt3DCore::QTransform* m_lightTransform = nullptr;
    Qt3DExtras::QFirstPersonCameraController* m_camController = nullptr;
    Qt3DRender::QSceneImporter* m_pSceneImporter = nullptr;
    QColor m_bgColor;
};

/// @brief 
class Q3DWindowEx : public Qt3DExtras::Qt3DWindow
{
	Q_OBJECT
public:
    explicit Q3DWindowEx(Qt3DWindowContainer* w = Q_NULLPTR);
    ~Q3DWindowEx();
	QVector3D getSuitableLightPos();
	QVector3D getSuitableCameraPos();

protected:
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:
	bool m_mousePress = false;
	QPoint m_mousePos;
	Qt3DWindowContainer* m_q3DWindowContainer = nullptr;
	QVector3D m_cameraPos;
};

#endif
