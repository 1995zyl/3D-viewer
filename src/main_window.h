#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "render_container.h"
#include <QWidget>
#include <QPushButton>
#include <QComboBox>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onDrawModeChanged(int index);
    void onLoadModel();
    void onSelectBgColor();
    void onWheelScaleChanged(int value);
    void onAnimationTypeChanged(int index);
    void onAnimationShowSpeedChanged(int value);
    void onRenderWindowChange(RenderContainer::RenderMode renderMode);

private:
    void initUI();

private:
    QComboBox* m_drawModeCom = nullptr;
    QPushButton *m_loadModelBtn = nullptr;
    QPushButton *m_bgColorBtn = nullptr;
    QComboBox *m_wheelScaleCom = nullptr;
    QComboBox *m_animationCom = nullptr;
    QComboBox *m_animationSpeedCom = nullptr;
    RenderContainer *m_renderContainer = nullptr;

    QString m_title;
};
#endif
