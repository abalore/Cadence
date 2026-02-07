#ifndef GRAPHICSINSPECTOR_H
#define GRAPHICSINSPECTOR_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class GraphicsInspector;
}

class GraphicsInspector : public QDialog
{
    Q_OBJECT

public:
    explicit GraphicsInspector(QWidget *parent = nullptr);
    ~GraphicsInspector();
    void UpdateGraphics();
private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixItem;
    Ui::GraphicsInspector *ui;
};

#endif // GRAPHICSINSPECTOR_H
