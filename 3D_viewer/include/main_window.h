#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QWidget>
#include <QPushButton>
#include <QComboBox>

class OpenGLContainer;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onLoadModel();
    void onSelectBgColor();
    void onWheelScaleChanged(int index);
    void onAnimationTypeChanged(int index);
    void onAnimationShowSpeedChanged(int index);

private:
    void initUI();

private:
    QPushButton *m_loadModelBtn = nullptr;
    QPushButton *m_bgColorBtn = nullptr;
    QComboBox *m_wheelScaleCom = nullptr;
    QComboBox *m_animationCom = nullptr;
    QComboBox *m_animationSpeedCom = nullptr;
    OpenGLContainer * m_openGLContainer = nullptr;

    QString m_title;
};
#endif
