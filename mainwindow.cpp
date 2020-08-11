#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    neuroplay = new NeuroplayPro();

    status = new QLabel;

    btnOpen = new QPushButton("Connect");
    connect(btnOpen, &QPushButton::clicked, [=]()
    {
        neuroplay->open();
    });

    editCmd = new QLineEdit;

    btnSend = new QPushButton("Send");
    connect(btnSend, &QPushButton::clicked, [=]()
    {
        QString cmd = editCmd->text();
        neuroplay->send(cmd);
    });
    connect(editCmd, &QLineEdit::editingFinished, btnSend, &QPushButton::click);

    tree = new QTreeWidget;
    tree->setColumnCount(2);
    tree->setColumnWidth(0, 200);
    tree->setHeaderHidden(true);

    log = new QTextEdit;

    ui->mainToolBar->addWidget(btnOpen);
    ui->mainToolBar->addWidget(editCmd);
    ui->mainToolBar->addWidget(btnSend);

    ui->statusBar->addWidget(status);

    QPushButton *btnSpectrum = new QPushButton("Spectrum");
    QPushButton *btnRawData = new QPushButton("RawData");

    QVBoxLayout *layButtons = new QVBoxLayout;
    layButtons->addWidget(btnSpectrum);
    layButtons->addWidget(btnRawData);

    QGridLayout *layout = new QGridLayout;
    ui->centralWidget->setLayout(layout);
    layout->addWidget(tree, 0, 0);
    layout->addLayout(layButtons, 0, 1);
    layout->addWidget(log, 1, 0, 1, 2);

    connect(neuroplay, &NeuroplayPro::connected, [=](){status->setText("connected");});
    connect(neuroplay, &NeuroplayPro::disconnected, [=](){status->setText("disconnected");});
    connect(neuroplay, &NeuroplayPro::response, [=](QString text){log->append(text);});
    connect(neuroplay, &NeuroplayPro::error, [=](QString text)
    {
        QMessageBox::warning(this, "NeuroplayPro error", text);
    });
    connect(neuroplay, &NeuroplayPro::deviceConnected, [=](NeuroplayDevice *device)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem({device->name(), "<click here to start>"});
        item->setData(1, 42, QVariant::fromValue<NeuroplayDevice*>(device));
        tree->addTopLevelItem(item);
        new QTreeWidgetItem(item, {"id", QString::number(device->id())});
        new QTreeWidgetItem(item, {"model", device->model()});
        new QTreeWidgetItem(item, {"serialNumber", device->serialNumber()});
        new QTreeWidgetItem(item, {"maxChannels", QString::number(device->maxChannels())});
        new QTreeWidgetItem(item, {"preferredChannelCount", QString::number(device->preferredChannelCount())});
        QTreeWidgetItem *modesItem = new QTreeWidgetItem(item, {"channelModes"});
        QStringList modes = device->channelModes();
        for (int i=0; i<modes.size(); i++)
        {
            QString mode = modes[i];
            QTreeWidgetItem *modeItem = new QTreeWidgetItem(modesItem, {mode, "<click here to start>"});
            modeItem->setData(1, 42, QVariant::fromValue<NeuroplayDevice*>(device));
            modeItem->setData(1, 43, device->channelModesValues()[i].first);
        }

        connect(device, &NeuroplayDevice::ready, [=]()
        {
            item->setBackgroundColor(0, Qt::green);
            item->setText(1, "started");

            connect(btnSpectrum, &QPushButton::clicked, [=]()
            {
                device->requestSpectrum();
                NeuroplayDevice::ChannelsData spectrum = device->spectrum();
                qDebug() << "spectrum: " << spectrum.size() << "x" << (spectrum.size()? spectrum[0].size(): 0);
            });

            connect(btnRawData, &QPushButton::clicked, [=]()
            {
                qDebug() << device->rawData();
//                device->rawData();
            });
        });
    });

    connect(tree, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int column)
    {
        if (column == 1)
        {
            if (item->data(1, 42).isValid())
            {
                NeuroplayDevice *device = item->data(1, 42).value<NeuroplayDevice*>();
                if (item->data(1, 43).isValid())
                {
                    int channelCount = item->data(1, 43).toInt();
                    device->start(channelCount);
                }
                else
                {
                    device->start();
                }
                item->setBackgroundColor(0, Qt::yellow);
                item->setText(1, "starting device...");
            }
        }
    });
}

MainWindow::~MainWindow()
{
    delete neuroplay;
    delete ui;
}