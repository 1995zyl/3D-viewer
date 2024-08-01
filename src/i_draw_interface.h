#ifndef __I_DRAW_INTERFACE_H__
#define __I_DRAW_INTERFACE_H__

#include <QSize>
#include <QColor>

class IDrawInterface
{
public:
    virtual ~IDrawInterface() {}

    virtual void resizeEx(const QSize& size) = 0;
    virtual void showEx() = 0;
    virtual void hideEx() = 0;
    virtual void setBgColor(const QColor &color) = 0;
    virtual void setWheelScale(float wheelScale) = 0;
    virtual void startAnimation(int animationType, int millisecond) = 0;
    virtual void stopAnimation() = 0;
    virtual QString getModelPath() const = 0;
};


#endif
