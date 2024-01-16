#ifndef __OPENGL_WINDOW_H__
#define __OPENGL_WINDOW_H__

#include "opengl_helper.h"
#include <QTimer>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QLabel>

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLExtraFunctions
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
    explicit OpenGLWindow(const QString &modelPath, const QColor &color, QWidget *parent = Q_NULLPTR);
    ~OpenGLWindow();
    void setBgColor(const QColor &color);
    void setWheelScale(float wheelScale);
    void startAnimation(AnimationType animationType, int millisecond);
    void stopAnimation();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

public slots:
    void onFpsTimeOut();
    void onAnimationTimeOut();

private:
    void initializeFpsLabel();
    void initializeZoom();
    void initializeMesh();
    void paintMesh();
    bool compileGLSL();
    void releasePos(Qt::MouseButton mbType);
    int setRotation(int angle);

private:
    QScopedPointer<QOpenGLShaderProgram> m_shaderProgram;
    QVector<OpenGLHelper::ModelMesh> m_modelMeshs;
    int m_cameraDistance = 20;
    OpenGLHelper::CameraParam m_camera;
    std::array<GLclampf, 4> m_bgColor;
    int m_mouseFlag = Qt::NoButton;
    bool m_mousePress = false;
    QPoint m_mousePos;
    QTimer m_fpsTimer;
    QTimer m_animationTimer;
    int m_frameCount = 0;
    float m_wheelScale = 1.0;
    AnimationType m_animationType = Turntable;
    int m_animationPos;
    int m_animationLoopNum = 0;
    unsigned int m_glslProgramId = 0;
    QLabel *m_fpsLabel = nullptr;
};

#endif
