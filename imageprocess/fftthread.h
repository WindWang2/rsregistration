#ifndef FFTTHREAD_H
#define FFTTHREAD_H

#include <QThread>

class FFTThread : public QThread
{
    Q_OBJECT
public:
    explicit FFTThread(QObject *parent = 0);
    void run();
    
signals:
    
public slots:
    
};

#endif // FFTTHREAD_H
