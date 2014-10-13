/* 
 * File:   MainWindow.h
 * Author: Eivind
 *
 * Created on 26. september 2014, 15:54
 */

#ifndef MAINWINDOW_H
#define	MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QTableWidget>
#include <QSlider>
#include <QSpinBox>
#include <stdio.h>


class MainWindow : public QMainWindow{
public:
    MainWindow();

   QHBoxLayout *mainL;
   
    
private:
    void createButtons();
    void createTableR_RW();
    void createTableConfig();
    void createMainLayout();
    void createMenu();
    void createCells(int id);
    QWidget* createSpinBox(int low, int high);

    
    QTableWidget *tableR;
    QTableWidget *tableRW;
    QTableWidget *tableConfig;
    QMenu *fileMenu;
    QWidget *window;
    
    
    
};

#endif	/* MAINWINDOW_H */
