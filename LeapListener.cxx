#include "LeapListener.h"
#include "LeapInteraction.h"
#include <qvector3d.h>

inline QVector3D Leap2QVector(Leap::Vector v)
{
	return QVector3D(v.x, v.y, v.z);
}


void LeapListener::onFrame(const Leap::Controller & ctl)
{

	if(timer->elapsed() > 100)
	{
		Leap::Frame f = ctl.frame();
		setObjectName(QString::number(f.id()));
		// emits objectNameChanged(QString)
		//emit translate2(SimpleTranslate(f));


		Leap::Vector center, xDir, yDir, zDir;
		GetSpace(f, center, xDir, yDir, zDir);
		emit UpdateCamera(Leap2QVector(center), Leap2QVector(xDir), Leap2QVector(yDir), Leap2QVector(zDir));

		Leap::Vector toolTip, toolDir;
		GetTool(f, toolTip, toolDir);
		emit UpdatePlane(Leap2QVector(toolTip), Leap2QVector(toolDir));

		Leap::Vector point1, point2;
		GetTwoPoints(f, point1, point2);
		emit UpdateLine(Leap2QVector(point1), Leap2QVector(point2));

		timer->restart();
	}
}
