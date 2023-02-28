# Qt-Neumorphic
QSS没有box-shadow属性，直到看到了[Italink](https://github.com/Italink)大佬写的[QNeumorphism](https://github.com/Italink/QNeumorphism)，在调整参数时候发现blurRadius过大的时候阴影和高光会重合，导致光影混叠在一起，在此通过明暗交界线限制光影区域处理这一问题。

翻译一下代码：

在Qt中通过重写`QGraphicsEffect`子类`draw()`实现新拟态风格的控件（Neumorphic），绘制出一个具有立体感的阴影和高光效果。
具体来说，它将一个给定的QPixmap转换为QImage，并在其上面进行阴影和高光的绘制。
控件的模式有NeumorphicMode_Normal：默认效果、NeumorphicMode_Inset：内嵌效果、NeumorphicMode_Ridge：边缘脊突效果。
绘制阴影和高光的过程中，使用了QPainterPath和QPainter的组合模式来实现，同时qt_blurImage对生成的图像进行模糊处理，增加立体感。
最终，绘制出来的图像是一个QImage，然后使用painter->drawImage绘制到外部widget上。

<div align=center><img src="https://github.com/IPlayGenji6/Qt-Neumorphic/blob/main/qt_neumorphic.png"/></div>

# 用法/API
```c++
Neumorphic* neum = new Neumorphic(Neumorphic::NeumorphicMode_Normal);
pushButton->setGraphicsEffect(neum);
```

```c++
qreal blurRadius() const { return m_blurRadius; }
void setBlurRadius(qreal blurRadius) { m_blurRadius = blurRadius; updateBoundingRect(); }  //设置模糊半径
qreal opacity() const { return m_opacity; }
void setOpacity(qreal opacity) { m_opacity = opacity; updateBoundingRect(); }  //设置光影透明度
QPoint offset() const { return m_offset; }
void setOffset(int x, int y) { m_offset = QPoint(x, y); updateBoundingRect(); }  //设置光影偏移量
QColor shadowColor() const {return m_shadowColor.toRgb(); }
void setShadowColor(QColor color) {m_shadowColor = color.toRgb(); m_shadowColor.setAlpha(m_opacity); updateBoundingRect(); };  //设置阴影颜色
QColor highlightColor() const {return m_highlightColor.toRgb(); }
void setHighlightColor(QColor color) {m_highlightColor = color.toRgb(); m_highlightColor.setAlpha(m_opacity); updateBoundingRect(); }; //设置高光颜色
```
