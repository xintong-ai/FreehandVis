#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QVector3D>
#include <QMatrix4x4>

#include "vtkSmartPointer.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkDICOMImageReader.h"
#include <vtkSphereSource.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>

#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkImageResample.h"
#include <vtkOutlineFilter.h>
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkCamera.h"
#include "vtkImageData.h"
#include <QKeyEvent>
//#include "vtkMapper.h"
#include <vtkEventQtSlotConnect.h>

#include "LeapInteraction.h"

class MyListener : public QObject, public Leap::Listener {
	Q_OBJECT

signals:
	void UpdateRectangle(QVector3D origin, QVector3D point1, QVector3D point2);
	void UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
	void translate2(float v);

public:
	MyListener()
	{
		//timer = new QTimer(this);
		timer = new QElapsedTimer();
		timer->start();
	}

	virtual void onFrame(const Leap::Controller & ctl);

private:
	QElapsedTimer *timer;


};

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;


class QtVTKRenderWindows : public QMainWindow
{
	Q_OBJECT
public:

	// Constructor/Destructor
	QtVTKRenderWindows(int argc, char *argv[]);
	~QtVTKRenderWindows() {}

	private slots:

		virtual void slotExit();
		virtual void ResetViews();
		//virtual void Render();
		//virtual void UpdateSeedPlane(QVector3D origin, QVector3D point1, QVector3D point2);
		//virtual void UpdateSlicePlane(QVector3D origin, QVector3D point1, QVector3D point2);
//		void slotKeyPressed(vtkObject *, unsigned long, void *, void *, vtkCommand *command);
		void UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
		void slotKeyPressed(vtkObject *a, unsigned long b, void *c, void *d, vtkCommand *command);
protected:

	MyListener listener;
	Leap::Controller controller;

	//void keyPressEvent(QKeyEvent *);

private:
	vtkSmartPointer<vtkVolume> AddVolume(vtkSmartPointer<vtkImageAlgorithm> reader);
	vtkSmartPointer<vtkActor> AddOutline(vtkSmartPointer<vtkImageAlgorithm> reader);
	vtkSmartPointer<vtkCamera> camera;
	// Designer form
	Ui_QtVTKRenderWindows *ui;
	vtkSmartPointer<vtkImageData> input;

};

#endif // QtVTKRenderWindows_H
