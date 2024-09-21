#ifndef __VULKAN_WINDOW_H__
#define __VULKAN_WINDOW_H__

#include "i_draw_interface.h"
#include "vulkan_render.h"
#include <QWidget>
#include <QVulkanWindow>

class VulkanWindow;
class VulkanWindowContainer : public QWidget, public IDrawInterface
{
    Q_OBJECT
public:
    explicit VulkanWindowContainer(const QString &modelPath, const QColor &color, QWidget *parent = Q_NULLPTR);
    ~VulkanWindowContainer();
    void resizeEx(const QSize &size) override;
    void showEx() override;
    void hideEx() override;
    void setBgColor(const QColor &color) override;
    void setWheelScale(float wheelScale) override;
    void startAnimation(int animationType) override;
    void stopAnimation() override;
    QString getModelPath() const override { return m_modelPath; }

private:
    QString m_modelPath;
    VulkanWindow* m_vulkanWindow = nullptr;
};

class VulkanWindow : public QVulkanWindow
{
public:
    explicit VulkanWindow(const QString& modelPath, const QColor& color, QWindow *parent = nullptr);
    ~VulkanWindow();
    QVulkanWindowRenderer *createRenderer() override;
    void setBgColor(const QColor& color);
    void startAnimation(int animationType);
    void stopAnimation();

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    VulkanRenderer *m_renderer;
    QColor m_bgColor;
    std::shared_ptr<VulkanMesh> m_vulkanMeshPtr;
    bool m_mousePress = false;
    QPoint m_mousePos;
    int m_mouseFlag = Qt::NoButton;
    float m_wheelScale = 1.0;
};

#endif
