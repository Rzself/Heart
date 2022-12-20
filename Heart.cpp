#include "Heart.h"
#include <QPainter>
#include <cmath>
#include <random>

Heart::Heart(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Heart");
    resize(1000, 800);

    connect(&m_timer, &QTimer::timeout, this, [this](){
        static int order = - 1;
        order = (order + 1) % m_heartScaleFactors.size();
        qreal factor = m_heartScaleFactors.at(order)*3;
        //factor = 3;

        QPointF point(width()/2, height()*57/100);
        QPolygonF polygon = getPath(factor, point);

        QPolygonF outline = getOutLine(polygon, point);
        polygon += outline;

        QPolygonF backGround = getBackGround(rect());
        polygon += backGround;

        drawOnPixmap(std::move(polygon));

        update();
    });
    m_timer.start(mc_interval);
}

Heart::~Heart()
{
}

void Heart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(rect(), m_pixmap);
}

void Heart::drawOnPixmap(const QPolygonF&& polygon)
{
    QPixmap tempPixmap = QPixmap(width(), height());
    QPainter painter(&tempPixmap);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

    QPen pen(QColor(232, 98, 113), 2);
    painter.setPen(pen);
    painter.setBrush(pen.color());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPoints(polygon);
//        for(int i = 0; i < polygon.size(); ++i)
//            painter.drawEllipse(polygon.at(i), 2, 2);
    m_pixmap = std::move(tempPixmap);
}

inline QPolygonF Heart::getPath(qreal factor, const QPointF &center)
{
    QPolygonF polygonUp;
    QPolygonF polygonDown;

    const qreal sqrt2 = std::sqrt(2.0) + 0.1;
    for(qreal x = -sqrt2; x < sqrt2; x += 0.01)
    {
        qreal boundary = 1 - 0.5*x*x;
        if(boundary < 0)
            continue;
        qreal y = 0.5*std::pow(x*x, 1.0/3) - std::sqrt(boundary);
        polygonDown.append(QPointF(100*x, - y*100));
        y = 0.5*std::pow(x*x, 1.0/3) + std::sqrt(boundary);
        polygonUp.append(QPointF(100*x, - y*100));
    }

    static auto translateScale = [](QPolygonF& polygon, const qreal factor, const QPointF& point){
        for(int i = 0; i < polygon.size(); ++i)
        {
            polygon[i] *= factor;
            polygon[i] += point;
        }
    };
    translateScale(polygonUp, factor, center);
    translateScale(polygonDown, factor, center);

//    std::sort(polygon.begin(), polygon.end(), [](const QPointF& left, const QPointF& right){
//        const qreal tolerate = 0.000001;
//        if(std::abs(left.y() - right.y()) < tolerate)
//            return left.y() < right.y();
//        return left.x() < right.x();
//    });

    const qreal Drift = factor;

    QPointF& startUp = polygonUp[0];
    QPointF& startDown = polygonDown[0];
    const QPointF middlePoint(startUp.x() - Drift, (startUp.y() + startDown.y())/2);
    polygonUp.insert(std::begin(polygonUp), middlePoint);
    polygonDown.insert(std::begin(polygonDown), middlePoint);

    QPointF& endUp = polygonUp[polygonUp.size() - 1];
    QPointF& endDown = polygonDown[polygonDown.size() - 1];
    const QPointF endMiddlePoint(endUp.x() + Drift, (endUp.y() + endDown.y())/2);
    polygonUp.append(endMiddlePoint);
    polygonDown.append(endMiddlePoint);

    static auto appendAppropriatePoints = [](QPolygonF& polygon){
        int polygonSize = polygon.size();
        for(int i = 1; i < polygonSize; ++i)
        {
            QPointF start = polygon[i - 1];
            QPointF end = polygon[i];
            QPointF& originEnd = polygon[polygonSize-1];
            const qreal spanY = end.y() - start.y();
            const uint sumPoint = std::abs(spanY*2) - uint(std::abs(spanY));
            if(sumPoint < 2)
                continue;
            const qreal spanX = std::abs(start.x() - end.x());

            for(uint j = 1; j < sumPoint; ++j)
            {
                const qreal x = start.x() + spanX*j/sumPoint;
                const qreal y = start.y() + (spanY*j)/sumPoint;
                polygon.append(QPointF(x, y));

                if(end == originEnd)
                {
                    // qDebug()<< sumPoint<< j<< x<< y<< start<< end;
                }
            }
        }
        // qDebug()<<"\n";
    };
    appendAppropriatePoints(polygonUp);
    appendAppropriatePoints(polygonDown);

    QPolygonF reversePolygonDown(polygonDown.rbegin(), polygonDown.rend());
    QPolygonF polygon = polygonUp + reversePolygonDown;
    polygon.removeFirst();
    polygon.removeLast();
    return polygon;
}

QPolygonF Heart::getOutLine(const QPolygonF &polygon, const QPointF& center) const
{
    static auto getOutlineFactors = [](){
        QList<qreal> factors;
        qreal span = 0.0008;
        for(qreal start = 1; start < 1.16; start += span)
        {
            span *= 1.1;
            factors.append(start);
            factors.append(1/start);
        }
        return factors;
    };

    // generate Random Number between 0 and span
    static auto getRandomNumber = [](const qreal span){
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(0, span);
            int mean = uniform_dist(e1);

            return mean;
        };

    // remove some data
    static auto randomRemove = [](QPolygonF &polygon){
        static const int span = 3;
        for(int i = polygon.size() - span-1; i > 0; i -= span)
        {
            int randomNumber = getRandomNumber(span);
            polygon.remove(i+randomNumber);
            randomNumber = getRandomNumber(span-1);
            polygon.remove(i+randomNumber);
        }
    };

    static const QList<qreal> outlineFactors = getOutlineFactors();
    QPolygonF outline;

    for(const auto& factor : outlineFactors)
    {
        const QPointF span = center*(1-factor);
        QPolygonF tempPolygon = polygon;
        for(auto& point : tempPolygon)
        {
            point *= factor;
            point += span;
        }

        randomRemove(tempPolygon);
        outline += tempPolygon;
    }

    return outline;
}

QPolygonF Heart::getBackGround(const QRect& rect) const
{
    static const qreal Span = 6;
    QPolygonF polygon;
    for(int w = 0; w < rect.width(); w += Span)
    {
        for(int h = 0; h < rect.height(); h += Span)
        {
            polygon.append(QPointF(w, h));
        }
    }

    return polygon;
}

