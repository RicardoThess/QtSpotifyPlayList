#include "mainwindow.h"
#include <QApplication>

#include <QSslSocket>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;

    //  Check the existens of the SSL Libraries
    //  In Windows they have to be installed separatley
    if (!QSslSocket::supportsSsl()) {

        QMessageBox msgError;
        msgError.setText("Instalation problem. SSL libraries are missing.");
        msgError.exec();

        qWarning () << "No SSL Support";
        exit (1);

        // necessary libeay32.dll and ssleay32.dll
    }
    qDebug () << QSslSocket::sslLibraryVersionString();

    w.show();

    return a.exec();
}
