﻿#ifndef QtVTKRenderWindows_H
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
#include "vtkLineSource.h"
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
#include "HandWidget.h"
#include "vtkCubeSource.h"
#include <vtkDataSetReader.h>

#include "vtkImageGridSource.h"
#include <vtkGPUVolumeRayCastMapper.h>
//#include "vtkSmartVolumeMapper.h"
//#include <vtkLensOpenGLGPUVolumeRayCastMapper.h>

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;

inline vtkSmartPointer<vtkMatrix4x4> QMatrix2vtkMatrix(QMatrix4x4 v)
{

	float data[16];
	v.copyDataTo(data);
	vtkSmartPointer<vtkMatrix4x4> ret = vtkMatrix4x4::New();
	for(int i = 0; i < 4; i++)	{
		for(int j = 0; j < 4; j++)	{
			ret->SetElement(i, j, data[i * 4 + j]);
		}
	}
	return ret;
}

class QtVTKRenderWindows : public QMainWindow
{
	Q_OBJECT
public:

	// Constructor/Destructor
	QtVTKRenderWindows(int argc, char *argv[]);
	~QtVTKRenderWindows() {}

	void ToggleStreamline();

	private slots:

		virtual void slotExit();
		void slotKeyPressed(vtkObject *a, unsigned long b, void *c, void *d, vtkCommand *command);
		void UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
		void UpdateSkeletonHand(TypeArray2 fingers, TypeArray palm, float sphereRadius);
		//void UpdateCameraGlobe(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir);
		//void UpdatePlane(QVector3D origin, QVector3D normal);
		//void UpdateLine(QVector3D point1, QVector3D point2);
		void UpdateGesture(int gesture);
		void UpdateRightHand(QVector3D thumbTip, QVector3D indexTip, QVector3D indexDir);
protected:

	LeapListener listener;
	Leap::Controller controller;

private:
	vtkSmartPointer<vtkVolume> AddVolume(vtkSmartPointer<vtkAlgorithmOutput> output);
	//vtkSmartPointer<vtkActor> AddOutline(vtkSmartPointer<vtkImageAlgorithm> reader);
	vtkSmartPointer<vtkActor> AddEarth();
	vtkSmartPointer<vtkActor> GetHandsActor();

	void AddBoxWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor);
	void AddLineWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor);
	void AddPlaneWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor);
	void AddCube();
	void AddAxisLines();
	void AddSnappingPlane();
	void AddStreamlines(vtkSmartPointer<vtkDataSetReader> reader);
	void AddBarChart(vtkSmartPointer<vtkDataSetReader> reader);
	void AddCamera();
	void AddMask();
	void AddSphere();
	//void UpdateMask(QVector3D p);
	void UpdateMask(QVector3D center, float radius, float threshold);

	//QVector3D NormlizedLeapCoords2DataCoords(QVector3D p);
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
	vtkSmartPointer<vtkTransform> volumeTransform;

	HandWidget _hw;

	//cube
	//vtkSmartPointer<vtkCubeSource> _cube;
	vtkSmartPointer<vtkActor> _cubeActor;

	//line
	vtkSmartPointer<vtkLineSource> _dirLines[3];
	vtkSmartPointer<vtkActor> _dirLinesActor[3];

	//snapping plane
	vtkSmartPointer<vtkPlaneSource> _snappingPlane;
	vtkSmartPointer<vtkActor> _snappingPlaneActor;
	double _scalerRange[2];

	vtkSmartPointer< vtkRenderer > _renVol;

	//streamlines
	vtkSmartPointer<vtkActor> _streamLineActor;
	bool _streamlineOn;

	//side
	vtkSmartPointer< vtkRenderer > _ren2;
	vtkSmartPointer<vtkLineSource> _seeds;

	//mask
	vtkGPUVolumeRayCastMapper *_volumeMapper;
	vtkSmartPointer<vtkImageData> _mask;
	vtkSmartPointer<vtkDataArray> _dataArray;
	int _iScale;
	vtkSmartPointer<vtkVolume> _maskVolume;
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> _maskVolumeMapper;
};

#endif // QtVTKRenderWindows_H
