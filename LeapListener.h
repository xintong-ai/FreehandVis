#ifndef LEAP_LISTENER_H
#define LEAP_LISTENER_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QVector3D>
#include <Leap.h>

class LeapListener : public QObject, public Leap::Listener {
	Q_OBJECT

signals:
	//void UpdateRectangle(QVector3D origin, QVector3D point1, QVector3D point2);
	void UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
	void UpdatePlane(QVector3D origin, QVector3D normal);
	void UpdateLine(QVector3D point1, QVector3D point2);
	void translate2(float v);

public:
	LeapListener()
	{
		//timer = new QTimer(this);
		timer = new QElapsedTimer();
		timer->start();
	}

	virtual void onFrame(const Leap::Controller & ctl);

private:
	QElapsedTimer *timer;


};

#endif	//LEAP_LISTENER_H