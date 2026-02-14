#include "graphicsinspector.h"
#include "ui_graphicsinspector.h"
#include "Emulator/Headers/defs.h"
#include "Emulator/Headers/GateArray.h"
#include "Emulator/Headers/CPC.h"

GraphicsInspector::GraphicsInspector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GraphicsInspector)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 640, 400);
    ui->graphicsView->setScene(scene);
    pixItem = 0;

    connect(ui->btnUpdate, &QPushButton::clicked, this, &GraphicsInspector::UpdateGraphics);
}

GraphicsInspector::~GraphicsInspector()
{
    delete ui;
}

void GraphicsInspector::UpdateGraphics()
{
    word baseAddress = 0xC000;
    if (ui->radioButton->isChecked()) baseAddress = 0x0000;
    if (ui->radioButton_2->isChecked()) baseAddress = 0x4000;
    if (ui->radioButton_3->isChecked()) baseAddress = 0x8000;
    int width = ui->inputWidth->text().toInt();
    int height = ui->inputHeight->text().toInt();
    int mode = GateArray::mode;
    static BYTE Pixels[640 * 400 * 3];
    for (int i = 0; i < 80; i++)
        for  (int j = 0; j < 25; j++)
            for (int k = 0; k < 8; k++)
            {
                word address = baseAddress + ((k * 0x0800 + j * 80 + i) % 0x4000);
                BYTE b = CPC::BaseRAM.MEM[address];
                int line = j * 8 + k;
                for (int l = 0; l < 8; l++)
                {
                    BYTE pen = GateArray::GetPenForPixel(mode, b, l);
                    const BYTE *color = GateArray::GetPaletteEntry(pen);
                    long pixelBase = (line * 640 * 2 + i * 8 + l) * 3;
                    if (pixelBase < 640 * 400 * 3 - 2)
                        for (int m = 0; m < 3; m++)
                        {
                            Pixels[pixelBase + m] = color[m];
                            Pixels[pixelBase + m + 640 * 3] = color[m];
                        }
                }
            }
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&Pixels[0], 640, 400, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(0, 0);
}

