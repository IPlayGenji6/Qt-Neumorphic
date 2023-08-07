#ifndef NEUMORPHIC_H
#define NEUMORPHIC_H

#include <QGraphicsEffect>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

class Neumorphic : public QGraphicsEffect
{
    Q_OBJECT

public:
    enum NeumorphicMode
    {
        NeumorphicMode_Normal = 1,
        NeumorphicMode_Inset = 2,
        NeumorphicMode_Ridge = 3
    };

    Neumorphic(QObject *parent = nullptr);
    Neumorphic(NeumorphicMode mode, QObject *parent = nullptr);

    inline qreal blurRadius() const { return m_blurRadius; }
    inline void setBlurRadius(qreal blurRadius)
    {
        m_blurRadius = blurRadius;
        updateBoundingRect();
    }
    inline qreal opacity() const { return m_opacity; }
    inline void setOpacity(qreal opacity)
    {
        m_opacity = opacity;
        updateBoundingRect();
    }
    inline QPoint offset() const { return m_offset; }
    inline void setOffset(int x, int y)
    {
        m_offset = QPoint(x, y);
        updateBoundingRect();
    }
    inline QColor shadowColor() const { return m_shadowColor.toRgb(); }
    inline void setShadowColor(QColor color)
    {
        m_shadowColor = color.toRgb();
        m_shadowColor.setAlpha(m_opacity);
        updateBoundingRect();
    };
    inline QColor highlightColor() const { return m_highlightColor.toRgb(); }
    inline void setHighlightColor(QColor color)
    {
        m_highlightColor = color.toRgb();
        m_highlightColor.setAlpha(m_opacity);
        updateBoundingRect();
    };

protected:
    QRectF boundingRectFor(const QRectF &rect) const override;
    void draw(QPainter *painter) override;

private:
    NeumorphicMode mode;

    qreal m_blurRadius;
    qreal m_opacity;
    QPoint m_offset;
    QColor m_shadowColor;
    QColor m_highlightColor;
};

#endif // NEUMORPHIC_H
