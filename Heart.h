#ifndef HEART_H
#define HEART_H

#include <QWidget>
#include <QTimer>

class Heart : public QWidget
{
    Q_OBJECT

public:
    Heart(QWidget *parent = nullptr);
    ~Heart();

private:

    QTimer m_timer;
    static const int mc_interval = 500;    // image update interval 500ms
    const QList<qreal> m_heartScaleFactors{0.7, 0.8, 0.9, 1, 0.9, 0.8, 0.7};
    QPixmap m_pixmap;

    void paintEvent(QPaintEvent* event) override;

    void drawOnPixmap(const QPolygonF &&polygon);

    // use formula(laTex): 1-0.5\cdot x^{2}\; =\; \left( y-0.5\cdot \sqrt[3]{x^{2}} \right)^{2}
    // equal formula(mac Grapher): 1-0.5|_cdot_x^{2} = ({y-0.5|_cdot_|nsqrt{{3};{x^{2}}}})^{2}
    // we can infer ==> y1 = ...; y2 = ...
    QPolygonF getPath(qreal factor, const QPointF &center);

    // translate and scale based on getPath()
    QPolygonF getOutLine(const QPolygonF &polygon, const QPointF &center) const;

    QPolygonF getBackGround(const QRect& rect) const;

};
#endif // HEART_H
