#ifndef __OPENGL_WINDOW_H__
#define __OPENGL_WINDOW_H__

#include "i_draw_interface.h"
#include "utils/model_loader_manager.h"
#include "utils/utils.h"
#include <QTimer>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QLabel>

class OpenGLWindow : public QOpenGLWidget,
                     public IDrawInterface,
                     protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    struct CameraParam
    {
        float m_fovy = 45.0;                // 观察者视角的大小
        float m_zoom = 1.0;                 // 观察者距被观察物体中心点的距离
        QVector3D m_eye{0, 0, m_zoom * 20}; // 观察者在被观察物体的三维坐标系中的位置
        QVector3D m_center{0.0, 0.0, 0.0};  // 观察者在被观察物体的三维坐标系中的位置
        QVector3D m_up{0.0, 1.0, 0.0};      // 观察者的头部朝向
        QMatrix4x4 m_projection;            // 透视矩阵
        QMatrix4x4 m_translation;           // 平移矩阵
        QMatrix4x4 m_rotation;              // 旋转矩阵
        int m_xRot = 0;                     // 绕x轴旋转的角度
        int m_yRot = 0;                     // 绕y轴旋转的角度
        int m_zRot = 0;                     // 绕z轴旋转的角度
        qreal m_xTrans = 0.0;               // 沿x轴移动的位置
        qreal m_yTrans = 0.0;               // 沿y轴移动的位置
        float m_zNear = 0.01;               // 透视矩阵视野最小值，太小容易出现黑影
        float m_zFar = 10000.0;             // 透视矩阵视野最大值
    };

public:
    explicit OpenGLWindow(const QString &modelPath, const QColor &color, QWidget *parent = Q_NULLPTR);
    ~OpenGLWindow();
    void resizeEx(const QSize &size) override;
    void showEx() override;
    void hideEx() override;
    void setBgColor(const QColor &color) override;
    void setWheelScale(float wheelScale) override;
    void startAnimation(int animationType) override;
    void stopAnimation() override;
    QString getModelPath() const override { return m_modelPath; }

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
    QString m_modelPath;
    QScopedPointer<QOpenGLShaderProgram> m_shaderProgram;
    std::shared_ptr<QVector<ModelLoadManager::ModelMesh>> m_modelMeshsPtr;
    int m_cameraDistance = 20;
    CameraParam m_camera;
    std::array<GLclampf, 4> m_bgColor;
    
    int m_mouseFlag = Qt::NoButton;
    bool m_mousePress = false;
    QPoint m_mousePos;

    QTimer m_fpsTimer;
    QTimer m_animationTimer;
    int m_frameCount = 0;
    float m_wheelScale = 1.0;
    AnimationHelper::AnimationType m_animationType = AnimationHelper::Turntable;
    int m_animationPos;
    int m_animationLoopNum = 0;
    unsigned int m_glslProgramId = 0;
    QLabel *m_fpsLabel = nullptr;
};

#endif
