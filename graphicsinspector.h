#ifndef GRAPHICSINSPECTOR_H
#define GRAPHICSINSPECTOR_H

#include "defs.h"
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <vector>

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
protected:
    void showEvent(QShowEvent *event) override;
private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixItem;
    std::vector<BYTE> pixels;
    Ui::GraphicsInspector *ui;
};

#endif // GRAPHICSINSPECTOR_H
