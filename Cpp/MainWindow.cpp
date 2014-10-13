/* 
 * File:   MainWindow.cpp
 * Author: Eivind
 * 
 * Created on 26. september 2014, 15:55
 *
 * Debug/Tesing GUI for the EurobotLocomotion project.
 * Initialized by main.cpp
 */

#include <QtGui>
#include "MainWindow.h"

MainWindow::MainWindow() {
    //createButtons();
    createMainLayout();
    createMenu();
    createTableConfig();
    createTableR_RW();
   // setCentralWidget(tableR);
}


void MainWindow::createMainLayout() {
    window = new QWidget();
    
    
    mainL = new QHBoxLayout();
    window->setLayout(mainL);
    setCentralWidget(window);
}

void MainWindow::createMenu() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *newQ = new QAction(tr("&Something"), this);
    fileMenu->addAction(newQ);
  //  fileMenu->addAction("Thing");
}

void MainWindow::createTableR_RW() {
    QVBoxLayout *vtables = new QVBoxLayout();
    
    
   tableR = new QTableWidget(7, 2, this);   
   tableR->setFixedWidth(335);
   tableR->setEditTriggers(QAbstractItemView::NoEditTriggers);
   tableR->setHorizontalHeaderLabels(QStringList() 
	   << "Left servo" << "Right servo");
   tableR->setVerticalHeaderLabels(QStringList() 
	   << "[36] Present pos" << "[38] Present speed" << "[40] Present Load" << "[42] Present volt" 
	   << "[43] Present temp" << "[44] Registered" << "[46] Moving");
   vtables->addWidget(tableR);
   
   tableRW = new QTableWidget(7, 2, this);
   tableRW->setFixedWidth(335);
   tableRW->setHorizontalHeaderLabels(QStringList() 
	   << "Left servo" << "Right servo");
   tableRW->setVerticalHeaderLabels(QStringList() 
	   << "[30] Goal pos" << "[32] Moving speed" << "[34] Torque limit" << "[48] Punch" 
	   << "[68] Current" << "[70] Torque Control" << "[71] Goal Torque" << "[73] Goal acceleration");
   
   vtables->addWidget(tableRW);
       
       
   mainL->addLayout(vtables);
}



void MainWindow::createTableConfig() 
{   
    tableConfig = new QTableWidget(18, 2, this);
    tableConfig->setFixedWidth(345);
    
    tableConfig->setHorizontalHeaderLabels(QStringList()
	    << "Left servo" << "Right servo");
    tableConfig->setVerticalHeaderLabels(QStringList()
	    << "[ 3] ID" << "[ 4] Baud rate" << "[ 5] Return delay "<< "[ 6] CW angle lim." << "[ 8] CCW angle lim."
	    << "[11] Limit temp." << "[12] Limit volt(low)" << "[13] Limit volt(high)" << "[14] Max Torque" 
	    << "[16] Status return lvl" << "[17] Alarm LED" << "[18] Alarm Shutdown" << "[20] Multi-turn offset" 
	    << "[22] Resolution div." << "[25] LED enable" << "[26] D gain" << "[27] I gain" << "[28] P gain");
	    
    
    //QWidget *q = createSpinBox(0, 1023);
    createCells(0);
    createCells(1);
    
    mainL->addWidget(tableConfig);
}

void MainWindow::createCells(int id) {
    tableConfig->setCellWidget(0, id, createSpinBox(0, 254)); //ID
    tableConfig->setCellWidget(1, id, createSpinBox(0, 254)); //Baud
    tableConfig->setCellWidget(2, id, createSpinBox(0, 254)); //Return
    tableConfig->setCellWidget(3, id, createSpinBox(0, 4095)); //CW
    tableConfig->setCellWidget(4, id, createSpinBox(0, 4095)); //CCW
    

    //Temp should be non-editable
    tableConfig->setCellWidget(6, id, createSpinBox(0, 254)); //Lvolt
    tableConfig->setCellWidget(7, id, createSpinBox(0, 254)); //Hvolt
    tableConfig->setCellWidget(8, id, createSpinBox(0, 1023)); //MaxTorque
    
    tableConfig->setCellWidget(9, id, createSpinBox(0, 2)); //Return
    tableConfig->setCellWidget(10, id, createSpinBox(0, 254)); //AlarmLED
    tableConfig->setCellWidget(11, id, createSpinBox(0, 254)); //AlarmShutdown
    tableConfig->setCellWidget(12, id, createSpinBox(-24566, 24576)); //MTOffset
    
    tableConfig->setCellWidget(13, id, createSpinBox(1, 254)); //ResDiv
    tableConfig->setCellWidget(14, id, createSpinBox(0, 1)); //LED
    tableConfig->setCellWidget(15, id, createSpinBox(0, 254)); //Dgain
    tableConfig->setCellWidget(16, id, createSpinBox(0, 254)); //Igain
    tableConfig->setCellWidget(17, id, createSpinBox(0, 254)); //Pgain 
}

QWidget* MainWindow::createSpinBox(int low, int high) {
    QSpinBox *newBox = new QSpinBox();
    newBox->setMinimum(low);
    newBox->setMaximum(high);
    
    QWidget *box = newBox;
    return box;
}

/*
void MainWindow::createButtons()
{
    QButtonGroup *buttons = new QButtonGroup;

    QPushButton *button1 = new QPushButton;
    QPushButton *button2 = new QPushButton;
    QPushButton *button3 = new QPushButton;
    QPushButton *button4 = new QPushButton;
    QPushButton *button5 = new QPushButton;
    QPushButton *button6 = new QPushButton;   
    button1->setText("Left\nBackward");
    button2->setText("Left\nStop");
    button3->setText("Left\nBackward");
    
    button3->setText("Left\nBackward");
    button4->setText("Right\nForward");
    button3->setText("Left\nBackward");
	
    button4->setText("Right\nForward");
    button5->setText("Right\nStop");
    button6->setText("Right\nBackward");
    
    buttons->addButton(button1);
    buttons->addButton(button2);
    buttons->addButton(button3);
    buttons->addButton(button4);
    buttons->addButton(button5);
    buttons->addButton(button6); 
}*/