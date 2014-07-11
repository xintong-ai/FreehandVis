#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include <QMatrix4x4>

//read data
#include "vtkSmartPointer.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkDICOMImageReader.h"
#include <vtkPlane.h>

//volume rendering
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkImageResample.h"

//outline
#include <vtkOutlineFilter.h>
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkCamera.h"
#include "vtkImageData.h"

#include <vtkEventQtSlotConnect.h>
#include "vtkImplicitPlaneWidget2.h"
#include "vtkImplicitPlaneRepresentation.h"

//globe
#include "vtkPlaneSource.h"
#include "vtkSphericalTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkPNMReader.h"
#include "vtkTexture.h"
#include "vtkDataSetMapper.h"
#include "vtkPolyData.h"

//widgets
#include "vtkLineWidget.h"
#include "vtkBoxWidget.h"
#include "vtkBoxRepresentation.h"

//Leap:
#include "LeapListener.h"

//primitives
//#include "vtkLineSource.h"
#include "vtkTransform.h"
//#include "vtkPointSource.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkPoints.h"
#include "vtkTubeFilter.h"
#include "vtkCellArray.h"
#include "vtkLine.h"
#include "vtkProperty.h"
#include "vtkStripper.h"


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
		void slotKeyPressed(vtkObject *a, unsigned long b, void *c, void *d, vtkCommand *command);
		void UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
		void UpdateSkeletonHand(TypeArray2 fingers, TypeArray palm );
		void UpdateCameraGlobe(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
		void UpdatePlane(QVector3D origin, QVector3D normal);
		void UpdateLine(QVector3D point1, QVector3D point2);
protected:

	LeapListener listener;
	Leap::Controller controller;

private:
	vtkSmartPointer<vtkVolume> AddVolume(vtkSmartPointer<vtkImageAlgorithm> reader);
	//vtkSmartPointer<vtkActor> AddOutline(vtkSmartPointer<vtkImageAlgorithm> reader);
	vtkSmartPointer<vtkActor> AddEarth();
	vtkSmartPointer<vtkActor> GetHandsActor();

	void AddBoxWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor);
	void AddLineWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor);
	void AddPlaneWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor);

	QVector3D NormlizedLeapCoords2DataCoords(QVector3D p);
	vtkSmartPointer<vtkCamera> camera;
	vtkSmartPointer<vtkCamera> cameraGlobe;
	vtkSmartPointer<vtkVolume> _volume;
	// Designer form
	Ui_QtVTKRenderWindows *ui;
	vtkSmartPointer<vtkImageData> inputVolume;
	vtkSmartPointer<vtkImplicitPlaneWidget2> implicitPlaneWidget;
	vtkSmartPointer<vtkImplicitPlaneRepresentation> repPlane;
	QMatrix4x4 handTranslation;
	vtkSmartPointer<vtkLineWidget> lineWidget;
	QMatrix4x4 m;				//rotation from Leap coordinate system to VTK coordinate system
	vtkSmartPointer<vtkPolyData> inputGlobe;

	vtkSmartPointer<vtkTransform> _leapTransform;
	vtkSmartPointer<vtkGlyph3D> m_vtkGlyph3D ;

	//std::vector<vtkSphereSource*> _fingerSpheres;
	//vtkSphereSource* oneSphere;
	//vtkSmartPointer<vtkPointSource> _fingerPoints;
	vtkSmartPointer<vtkPoints> m_vtkCenterPoints;
	vtkSmartPointer<vtkPolyData> m_vtkCenterPolyData;
	vtkSmartPointer<vtkSphereSource> m_vtkSphereSource;

	vtkSmartPointer<vtkPolyData> _lines;
	vtkSmartPointer<vtkPolyDataMapper> _lineMapper;
	vtkSmartPointer<vtkActor> lineActor;

	//rotate
	vtkSmartPointer<vtkActor> _outlineActor;
	QVector3D LeapCoords2DataCoords(QVector3D v);
	QVector3D LeapVec2DataVec(QVector3D v);
	vtkSmartPointer<vtkTransform> volumeTransform;
};

#endif // QtVTKRenderWindows_H
