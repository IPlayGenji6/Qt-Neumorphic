#include "neumorphic.h"

Neumorphic::Neumorphic()
{
    mode = NeumorphicMode_Normal;
    m_blurRadius = 20;
    m_opacity = 255;
    m_offset.setX(3);
    m_offset.setY(3);
    m_shadowColor = QColor(150, 164, 187, m_opacity);
    m_highlightColor = QColor(255, 255, 255, m_opacity);
}

Neumorphic::Neumorphic(NeumorphicMode mode)
{
    this->mode = mode;
    m_blurRadius = 20;
    m_opacity = 255;
    m_offset.setX(3);
    m_offset.setY(3);
    m_shadowColor = QColor(150, 164, 187, m_opacity);
    m_highlightColor = QColor(255, 255, 255, m_opacity);
}

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE


QRectF Neumorphic::boundingRectFor(const QRectF& rect) const
{
    qreal wRatio = 1;
    qreal hRatio = 1;
    if (rect.width() > rect.height())
        wRatio = rect.width() / rect.height();
    else
        hRatio = rect.height() / rect.width();

    if (mode == NeumorphicMode_Normal)
        return rect.united(rect.translated(0, 0).adjusted(-m_blurRadius * wRatio, -m_blurRadius * hRatio, m_blurRadius * wRatio, m_blurRadius * hRatio));
    else if (mode == NeumorphicMode_Inset)
        return rect.united(rect.translated(0, 0)); //united取两矩形并集
    else if (mode == NeumorphicMode_Ridge)
        return rect.united(rect.translated(0, 0).adjusted(-m_blurRadius * wRatio, -m_blurRadius * hRatio, m_blurRadius * wRatio, m_blurRadius * hRatio));
    else
        return rect;
}

void Neumorphic::draw(QPainter *painter)
{
    PixmapPadMode pxMode = PadToEffectiveBoundingRect;
    QPoint pos; //pos = 外部widget在窗口中的位置
    const QPixmap px = sourcePixmap(Qt::DeviceCoordinates, &pos, pxMode);
    int pxWidth = px.width();
    int pxHeight = px.height();
    px.save("../px.png");

    if (px.isNull())
            return;

    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    QImage shadow(px.size(), QImage::Format_ARGB32_Premultiplied);
    shadow.setDevicePixelRatio(px.devicePixelRatioF());
    shadow.fill(0);

    QImage highlight(px.size(), QImage::Format_ARGB32_Premultiplied);
    highlight.setDevicePixelRatio(px.devicePixelRatioF());
    highlight.fill(0);

    QPainter shadowPainter(&shadow);
    shadowPainter.setCompositionMode(QPainter::CompositionMode_Source);
    shadowPainter.drawPixmap(0, 0, px);
    shadowPainter.end();

    QPainter highlightPainter(&highlight);
    highlightPainter.setCompositionMode(QPainter::CompositionMode_Source);
    highlightPainter.drawPixmap(0, 0, px);
    highlightPainter.end();

//    //模糊
//    QImage blurImage(shadow.size(), QImage::Format_ARGB32_Premultiplied);
//    blurImage.setDevicePixelRatio(px.devicePixelRatioF());
//    blurImage.fill(0);
//    QPainter blurPainter(&blurImage);
//    qt_blurImage(&blurPainter, shadow, 0, false, true);
//    blurPainter.end();
//    shadow = blurImage;
//    highlight = blurImage;

    if (mode == NeumorphicMode_Normal)
    {
        //画右下阴影
        shadowPainter.begin(&shadow);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        shadowPainter.fillRect(shadow.rect(), m_shadowColor);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath shadowPath;
        shadowPath.lineTo(pxWidth - m_offset.x()*2, 0);
        shadowPath.lineTo(0, pxHeight - m_offset.y()*2);
        shadowPainter.fillPath(shadowPath, QColor(255,0,0,255));
        shadowPainter.end();
        shadow.save("../shadow.png");

        //画左上高光
        highlightPainter.begin(&highlight);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        highlightPainter.fillRect(highlight.rect(), m_highlightColor);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath highlightPath(QPoint(pxWidth, pxHeight));
        highlightPath.lineTo(pxWidth, 0 + m_offset.y()*2);
        highlightPath.lineTo(0 + m_offset.x()*2, pxHeight);
        highlightPath.lineTo(pxWidth, pxHeight);
        highlightPainter.fillPath(highlightPath, QColor(255,0,0,255));
        highlightPainter.end();
        highlight.save("../highlight.png");

        //模糊方案2:先拼接光影到一张image上,再模糊image
        QImage blurShadowAndHighlightImage(shadow.size(), QImage::Format_ARGB32_Premultiplied);
        blurShadowAndHighlightImage.setDevicePixelRatio(px.devicePixelRatioF());
        blurShadowAndHighlightImage.fill(0);
        QPainter blurShadowAndHighlightPainter(&blurShadowAndHighlightImage);
        blurShadowAndHighlightPainter.drawImage(m_offset, shadow);
        blurShadowAndHighlightPainter.drawImage(-m_offset, highlight);
        blurShadowAndHighlightImage.save("../splicing.png");
        qt_blurImage(blurShadowAndHighlightImage, m_blurRadius, true);
        blurShadowAndHighlightImage.save("../final_blur.png");
        blurShadowAndHighlightPainter.end();

        //    painter->drawImage(pos + offset, shadow);
        //    painter->drawImage(pos - offset, highlight);
        painter->drawImage(pos, blurShadowAndHighlightImage);

        painter->drawPixmap(pos, px);
    }
    else if (mode == NeumorphicMode_Inset)
    {
        //画左上阴影
        shadowPainter.begin(&shadow);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        shadowPainter.fillRect(shadow.rect(), m_shadowColor);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath shadowPath(QPoint(pxWidth, pxHeight)); //剪切右下三角形
        shadowPath.lineTo(pxWidth, 0);
        shadowPath.lineTo(0, pxHeight);
        shadowPath.lineTo(pxWidth, pxHeight);
        shadowPainter.fillPath(shadowPath, QColor(255,0,0,255));
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        shadowPainter.drawPixmap(m_offset, px);
        shadowPainter.end();
        shadow.save("../shadow.png");

        //画右下高光
        highlightPainter.begin(&highlight);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        highlightPainter.fillRect(highlight.rect(), m_highlightColor);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath highlightPath(QPoint(0, 0));  //剪切左上三角形
        highlightPath.lineTo(pxWidth, 0);
        highlightPath.lineTo(0, pxHeight);
        highlightPath.lineTo(0, 0);
        highlightPainter.fillPath(highlightPath, QColor(255,0,0,255));
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        highlightPainter.drawPixmap(-m_offset, px);
        highlightPainter.end();
        highlight.save("../highlight.png");

        //模糊方案2:先拼接光影到一张image上,再模糊image
        QImage blurShadowAndHighlightImage(shadow.size(), QImage::Format_ARGB32_Premultiplied);
        blurShadowAndHighlightImage.setDevicePixelRatio(px.devicePixelRatioF());
        blurShadowAndHighlightImage.fill(0);
        QPainter blurShadowAndHighlightPainter(&blurShadowAndHighlightImage);
        QPoint shadowOffset(0, 0);
        blurShadowAndHighlightPainter.drawImage(shadowOffset, shadow);
        blurShadowAndHighlightPainter.drawImage(shadowOffset, highlight);
        blurShadowAndHighlightImage.save("../splicing.png");
        QImage sourceShape(blurShadowAndHighlightImage.size(), QImage::Format_ARGB32_Premultiplied);
        sourceShape.fill(0);
        shadowPainter.begin(&sourceShape);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_Source);
        shadowPainter.drawPixmap(0, 0, px);
        shadowPainter.end();
        qt_blurImage(blurShadowAndHighlightImage, m_blurRadius, true);
        blurShadowAndHighlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        blurShadowAndHighlightPainter.drawImage(QPoint(0, 0), sourceShape);
        blurShadowAndHighlightImage.save("../final_blur.png");
        blurShadowAndHighlightPainter.end();

        painter->drawPixmap(pos, px);

        painter->drawImage(pos, blurShadowAndHighlightImage);
    }
    else if (mode == NeumorphicMode_Ridge)
    {
        //画右下阴影
        shadowPainter.begin(&shadow);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        shadowPainter.fillRect(shadow.rect(), m_shadowColor);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath shadowPath;
        shadowPath.lineTo(pxWidth - m_offset.x()*2, 0);
        shadowPath.lineTo(0, pxHeight - m_offset.y()*2);
        shadowPainter.fillPath(shadowPath, QColor(255,0,0,255));
        shadowPainter.end();

        //画左上高光
        highlightPainter.begin(&highlight);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        highlightPainter.fillRect(highlight.rect(), m_highlightColor);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath highlightPath(QPoint(pxWidth, pxHeight));
        highlightPath.lineTo(pxWidth, 0 + m_offset.y()*2);
        highlightPath.lineTo(0 + m_offset.x()*2, pxHeight);
        highlightPath.lineTo(pxWidth, pxHeight);
        highlightPainter.fillPath(highlightPath, QColor(255,0,0,255));
        highlightPainter.end();

        //模糊方案2:先拼接光影到一张image上,再模糊image
        QImage blurShadowAndHighlightImage(shadow.size(), QImage::Format_ARGB32_Premultiplied);
        blurShadowAndHighlightImage.setDevicePixelRatio(px.devicePixelRatioF());
        blurShadowAndHighlightImage.fill(0);
        QPainter blurShadowAndHighlightPainter(&blurShadowAndHighlightImage);
        blurShadowAndHighlightPainter.drawImage(m_offset, shadow);
        blurShadowAndHighlightPainter.drawImage(-m_offset, highlight);
        qt_blurImage(blurShadowAndHighlightImage, m_blurRadius, true);
        blurShadowAndHighlightPainter.end();

        //图层1外圈光影
        painter->drawImage(pos, blurShadowAndHighlightImage);
        //图层2源像素图
        painter->drawPixmap(pos, px);

        qreal wRatio = 1;
        qreal hRatio = 1;
        if (px.width() > px.height())
            wRatio = px.width() / px.height();
        else
            hRatio = px.height() / px.width();
        int insetRectWidth = px.width() - 2*wRatio*m_blurRadius;
        int insetRectHeight = px.height() - 2*hRatio*m_blurRadius;

        shadow = QImage(QSize(insetRectWidth, insetRectHeight), QImage::Format_ARGB32_Premultiplied);
//        shadow.setDevicePixelRatio(px.devicePixelRatioF());
        shadow.fill(0);
        shadowPainter.begin(&shadow);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_Source);
        shadowPainter.drawPixmap(-wRatio*m_blurRadius, -hRatio*m_blurRadius, px);
        shadowPainter.end();
        shadow.save("../littleShadow.png");

        highlight = QImage(QSize(insetRectWidth, insetRectHeight), QImage::Format_ARGB32_Premultiplied);
//        shadow.setDevicePixelRatio(px.devicePixelRatioF());
        highlight.fill(0);
        highlightPainter.begin(&highlight);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_Source);
        highlightPainter.drawPixmap(-wRatio*m_blurRadius, -hRatio*m_blurRadius, px);
        highlightPainter.end();

        //画左上阴影
        shadowPainter.begin(&shadow);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        shadowPainter.fillRect(shadow.rect(), m_shadowColor);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath shadowPath2(QPoint(insetRectWidth, insetRectHeight)); //剪切右下三角形
        shadowPath2.lineTo(insetRectWidth, 0);
        shadowPath2.lineTo(0, insetRectHeight);
        shadowPath2.lineTo(insetRectWidth, insetRectHeight);
        shadowPainter.fillPath(shadowPath2, QColor(255,0,0,255));
        shadowPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        shadowPainter.drawPixmap(m_offset.x() - wRatio*m_blurRadius, m_offset.y() - hRatio*m_blurRadius, px);
        shadowPainter.end();
        shadow.save("../shadow2.png");

        //画右下高光
        highlightPainter.begin(&highlight);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        highlightPainter.fillRect(highlight.rect(), m_highlightColor);
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QPainterPath highlightPath2(QPoint(0, 0));  //剪切左上三角形
        highlightPath2.lineTo(insetRectWidth, 0);
        highlightPath2.lineTo(0, insetRectHeight);
        highlightPath2.lineTo(0, 0);
        highlightPainter.fillPath(highlightPath2, QColor(255,0,0,255));
        highlightPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        highlightPainter.drawPixmap(-m_offset.x()- wRatio*m_blurRadius, -m_offset.y() - hRatio*m_blurRadius, px);
        highlightPainter.end();
        highlight.save("../highlight2.png");

        //模糊方案2:先拼接光影到一张image上,再模糊image
        QImage blurShadowAndHighlightImage2(shadow.size(), QImage::Format_ARGB32_Premultiplied);
        blurShadowAndHighlightImage2.setDevicePixelRatio(px.devicePixelRatioF());
        blurShadowAndHighlightImage2.fill(0);
        QPainter blurShadowAndHighlightPainter2(&blurShadowAndHighlightImage2);
        QPoint shadowOffset(0, 0);
        blurShadowAndHighlightPainter2.drawImage(shadowOffset, shadow);
        blurShadowAndHighlightPainter2.drawImage(shadowOffset, highlight);
        blurShadowAndHighlightImage2.save("../splicing2.png");
        QImage sourceShape(blurShadowAndHighlightImage2.size(), QImage::Format_ARGB32_Premultiplied);
        sourceShape.fill(0);
        shadowPainter.begin(&sourceShape);
        shadowPainter.setCompositionMode(QPainter::CompositionMode_Source);
        shadowPainter.drawPixmap(-wRatio*m_blurRadius, -hRatio*m_blurRadius, px);
        shadowPainter.end();
        sourceShape.save("../sourceShape.png");
        qt_blurImage(blurShadowAndHighlightImage2, m_blurRadius, true);        
        blurShadowAndHighlightPainter2.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        blurShadowAndHighlightPainter2.drawImage(QPoint(0, 0), sourceShape);
        blurShadowAndHighlightPainter2.end();
        blurShadowAndHighlightImage2.save("../final_blur2.png");

        //图层3:内圈光影
        painter->drawImage(QPoint(pos.x() + wRatio*m_blurRadius, pos.y() + hRatio*m_blurRadius), blurShadowAndHighlightImage2);
    }
    painter->setWorldTransform(restoreTransform);
}



