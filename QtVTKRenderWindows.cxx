#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"
#include "qlabel.h"

#include <vtkXMLReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLHyperOctreeReader.h>
#include <vtkXMLCompositeDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkHyperOctree.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkCellTypes.h>
#include <vtksys/SystemTools.hxx>

#include <vtkAlgorithmOutput.h>

#include <vtkStreamLine.h>
#include <vtkBarChartActor.h>
#include <vtkImageAccumulate.h>
#include <vtkPointSource.h>
//#include <vtkGPUVolumeRayCastMapper.h>

//-VTK D:\data\nek\nek_bin.vtk
//-VTK D:\data\ifront\ifront.vtk

vtkSmartPointer<vtkVolume> QtVTKRenderWindows::AddVolume(vtkSmartPointer<vtkAlgorithmOutput> output)
{

	vtkSmartPointer<vtkImageResample> resample = vtkImageResample::New();

	// Create our volume and mapper
	vtkVolume *volume = vtkVolume::New();
	//vtkOpenGLGPUVolumeRayCastMapper *mapper = vtkOpenGLGPUVolumeRayCastMapper::New();
	_volumeMapper = vtkGPUVolumeRayCastMapper::New();
	//http://www.paraview.org/Wiki/VTK/Examples/Cxx/Visualization/RenderPassExistingContext
	//mapper->SetInputData(

	_volumeMapper->SetInputConnection( output);

	// Create our transfer function
	vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New();
	vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New();

	// Create the property and attach the transfer functions
	vtkVolumeProperty *property = vtkVolumeProperty::New();
	property->SetIndependentComponents(true);
	property->SetColor( colorFun );
	property->SetScalarOpacity( opacityFun );
	property->SetInterpolationTypeToLinear();
	//property->SetInterpolationTypeToNearest();

	// connect up the volume to the property and the mapper
	volume->SetProperty( property );
	double range = _scalerRange[1] - _scalerRange[0];
	volume->SetMapper( _volumeMapper );

	// Depending on the blend type selected as a command line option,
	// adjustthe transfer function

	colorFun->AddRGBPoint( _scalerRange[0], 0.23, 0.299, 0.754);
	colorFun->AddRGBPoint( _scalerRange[0] + range * 0.25,  0.5764705882352941, 0.7098039215686275, 1.0);
	colorFun->AddRGBPoint( _scalerRange[0] + range * 0.5, 0.8745098039215686, 0.8627450980392157, 0.8549019607843137);
	colorFun->AddRGBPoint( _scalerRange[0] + range * 0.75, 0.9686274509803922, 0.6627450980392157, 0.5411764705882353);
	colorFun->AddRGBPoint( _scalerRange[1],  0.706, 0.016, 0.15);

	opacityFun->AddPoint(_scalerRange[0], 0);
	opacityFun->AddPoint(_scalerRange[0] + 0.25 * range, 0);
	opacityFun->AddPoint(_scalerRange[0] + 0.5 * range, 1);
	opacityFun->AddPoint(_scalerRange[1], 1);

	//opacityFun->AddPoint(_scalerRange[0], 0);
	//opacityFun->AddPoint(_scalerRange[0] + 0.25 * range, 0);
	//opacityFun->AddPoint(_scalerRange[0] + 0.37 * range, 0.2);
	//opacityFun->AddPoint(_scalerRange[0] + 0.5 * range, 0);
	//opacityFun->AddPoint(_scalerRange[0] + 0.75 * range, 0);
	//opacityFun->AddPoint(_scalerRange[1], 1);

	//opacityFun->AddPoint(_scalerRange[0], 1);
	//opacityFun->AddPoint(_scalerRange[1], 1);


	_volumeMapper->SetBlendModeToComposite();
	property->ShadeOn();
	property->SetAmbient(0.1);
	property->SetDiffuse(0.9);
	property->SetSpecular(0.2);
	property->SetSpecularPower(10.0);
	property->SetScalarOpacityUnitDistance(0.8919);

	return volume;
}

vtkSmartPointer<vtkActor> AddOutline(vtkSmartPointer<vtkAlgorithm> reader)
{
	vtkSmartPointer<vtkOutlineFilter> outline = vtkOutlineFilter::New();
	outline->SetInputConnection(reader->GetOutputPort());

	//this does not have to be vtkPolyDataMapper, it can also be vtkDataSetMapper
	//Why??
	vtkSmartPointer<vtkPolyDataMapper> outlineMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	outlineMapper->SetInputConnection(outline->GetOutputPort());

	vtkSmartPointer<vtkActor> outlineActor = 
		vtkSmartPointer<vtkActor>::New();
	outlineActor->SetMapper(outlineMapper);

	return outlineActor;
}

void QtVTKRenderWindows::AddCube()
{


	vtkSmartPointer<vtkPoints> cubePoints = vtkPoints::New();
	QVector3D cubeMin, cubeMax;
	_hw.GetOrignalCubeCoords(cubeMin, cubeMax);
	cubePoints->InsertNextPoint(cubeMin.x(), cubeMin.y(), cubeMin.z());
	cubePoints->InsertNextPoint(cubeMin.x(), cubeMax.y(), cubeMin.z());
	cubePoints->InsertNextPoint(cubeMax.x(), cubeMax.y(), cubeMin.z());
	cubePoints->InsertNextPoint(cubeMax.x(), cubeMin.y(), cubeMin.z());

	cubePoints->InsertNextPoint(cubeMin.x(), cubeMin.y(), cubeMax.z());
	cubePoints->InsertNextPoint(cubeMin.x(), cubeMax.y(), cubeMax.z());
	cubePoints->InsertNextPoint(cubeMax.x(), cubeMax.y(), cubeMax.z());
	cubePoints->InsertNextPoint(cubeMax.x(), cubeMin.y(), cubeMax.z());

	/////////lines
	//https://gitorious.org/vtkwikiexamples/wikiexamples/source/f1789a4a8887aba4f72f3159e19da2b6c56405e0:PolyData/Tube.cxx#L41

	vtkSmartPointer<vtkCellArray> lineCell = 
		vtkSmartPointer<vtkCellArray>::New();

	vtkSmartPointer<vtkLine> line = 
		vtkSmartPointer<vtkLine>::New();

	line->GetPointIds()->SetId(0,0);
	line->GetPointIds()->SetId(1,1);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,1);
	line->GetPointIds()->SetId(1,2);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,2);
	line->GetPointIds()->SetId(1,3);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,3);
	line->GetPointIds()->SetId(1,0);
	lineCell->InsertNextCell(line);

	///////////
	line->GetPointIds()->SetId(0,4);
	line->GetPointIds()->SetId(1,5);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,5);
	line->GetPointIds()->SetId(1,6);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,6);
	line->GetPointIds()->SetId(1,7);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,7);
	line->GetPointIds()->SetId(1,4);
	lineCell->InsertNextCell(line);

	//////////////

	line->GetPointIds()->SetId(0,0);
	line->GetPointIds()->SetId(1,4);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,1);
	line->GetPointIds()->SetId(1,5);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,2);
	line->GetPointIds()->SetId(1,6);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,3);
	line->GetPointIds()->SetId(1,7);
	lineCell->InsertNextCell(line);

	///////////////
	vtkSmartPointer<vtkPolyData> lines = vtkPolyData::New();
	lines->SetPoints(cubePoints);
	lines->SetLines(lineCell);

	////if using this one without stripper filter, the tubes are broken at the joint point
	vtkSmartPointer<vtkStripper> stripperFilter = vtkStripper::New();
	stripperFilter->SetInputData(lines);
	stripperFilter->Update();

	//Create a tube (cylinder) around the line
	vtkSmartPointer<vtkTubeFilter> tubeFilter = 
		vtkSmartPointer<vtkTubeFilter>::New();
	//tubeFilter->SetInputData(_lines);		
	tubeFilter->SetInputConnection(stripperFilter->GetOutputPort());
	tubeFilter->SetRadius(4); //default is .5
	tubeFilter->SetNumberOfSides(50);
	tubeFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();
	cubeMapper->SetInputConnection(tubeFilter->GetOutputPort());//handTransformFilter->GetOutputPort());
	_cubeActor = vtkSmartPointer<vtkActor>::New();
	_cubeActor->SetMapper(cubeMapper);
	_cubeActor->GetProperty()->SetColor(1.0,1.0, 1.0); // Give some color to the line

	_renVol->AddActor(_cubeActor);
}

vtkSmartPointer<vtkActor> QtVTKRenderWindows::AddEarth()
{
	//earth
	vtkSmartPointer<vtkPlaneSource> plane = vtkPlaneSource::New();
	plane->SetOrigin(1, 3.14159265359 - 0.0001, 0.0);
	plane->SetPoint1(1, 3.14159265359 - 0.0001, 6.28318530719);
	plane->SetPoint2(1, 0.0001, 0.0);
	plane->SetXResolution(19);
	plane->SetYResolution(9);

	vtkSmartPointer<vtkSphericalTransform> transform = vtkSphericalTransform::New();
	vtkSmartPointer<vtkTransformPolyDataFilter> tpoly = vtkTransformPolyDataFilter::New();
	tpoly->SetInputConnection(plane->GetOutputPort());
	tpoly->SetTransform(transform);

	vtkSmartPointer<vtkPNMReader> earth = vtkPNMReader::New();
	earth->SetFileName("D:/data/VTKDATA6.0.0/Data/earth.ppm");

	vtkSmartPointer<vtkTexture> texture = vtkTexture::New();
	texture->SetInputConnection(earth->GetOutputPort());
	texture->InterpolateOn();

	vtkSmartPointer<vtkDataSetMapper> mapper = vtkDataSetMapper::New();
	mapper->SetInputConnection(tpoly->GetOutputPort());

	vtkSmartPointer<vtkActor> world = vtkActor::New();
	world->SetMapper(mapper);
	world->SetTexture(texture);

	inputGlobe = tpoly->GetOutput();

	return world;
}

void QtVTKRenderWindows::AddBoxWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor)
{
	vtkSmartPointer<vtkBoxWidget> boxWidget = vtkBoxWidget::New();
	boxWidget->SetInputConnection(reader->GetOutputPort());
	boxWidget->PlaceWidget();
	boxWidget->SetInteractor(interactor);
	boxWidget->On();
}

void QtVTKRenderWindows::AddLineWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor)
{

	lineWidget = vtkLineWidget::New();
	lineWidget->SetInputConnection(reader->GetOutputPort());
	lineWidget->SetAlignToXAxis();
	lineWidget->PlaceWidget();
	lineWidget->ClampToBoundsOn();
	lineWidget->SetClampToBounds(1);
	lineWidget->SetInteractor(interactor);
	lineWidget->On();
}

void QtVTKRenderWindows::AddSphere()
{
	
	////////////spheres
	m_vtkSphereSource = vtkSphereSource::New();
	m_vtkSphereSource->SetRadius(4);
	m_vtkSphereSource->SetCenter(0,0,0);
	m_vtkSphereSource->SetPhiResolution(32);
	m_vtkSphereSource->SetThetaResolution(32);

	
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkPolyDataMapper::New();
	sphereMapper->SetInputConnection(m_vtkSphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> sphereActor = vtkActor::New();
	sphereActor->SetMapper(sphereMapper);
	//sphereActor->GetProperty()->SetOpacity(0.3);

	_renVol->AddActor(sphereActor);
}

void QtVTKRenderWindows::AddPlaneWidget(vtkSmartPointer<vtkAlgorithm> reader, vtkRenderWindowInteractor *interactor)
{
	//plane widget
	//http://www.cnblogs.com/dawnWind/archive/2013/05/04/3D_11.html
	//http://www.vtk.org/Wiki/VTK/Examples/Cxx/Widgets/ImplicitPlaneWidget2
	repPlane = vtkSmartPointer<vtkImplicitPlaneRepresentation>::New();
	repPlane->SetPlaceFactor(1.5); // This must be set prior to placing the widget
	repPlane->PlaceWidget(_outlineActor->GetBounds());
	/*repPlane->OutsideBoundsOff();
	repPlane->UseBoundsOff();*/
	implicitPlaneWidget = vtkImplicitPlaneWidget2::New();
	implicitPlaneWidget->SetInteractor(interactor);
	implicitPlaneWidget->SetRepresentation(repPlane);
	implicitPlaneWidget->Off();
}

vtkSmartPointer<vtkActor> QtVTKRenderWindows::GetHandsActor()
{

	////////transform
	_leapTransform = vtkSmartPointer<vtkTransform>::New();

	QVector3D dataCenter;
	dataCenter = _hw.GetOriginalDataCenter();
	_leapTransform->SetMatrix(QMatrix2vtkMatrix(_hw.GetLeap2DataTransformation()));
	vtkSmartPointer<vtkTransformPolyDataFilter> leapTransformFilter = 
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	leapTransformFilter->SetTransform(_leapTransform);



	////////hand
	m_vtkCenterPoints = vtkPoints::New();
	m_vtkCenterPolyData = vtkPolyData::New();
	
	m_vtkGlyph3D = vtkGlyph3D::New();
	for(int i = 0; i < 26; i++)
		m_vtkCenterPoints->InsertNextPoint(10,10,10);

	m_vtkCenterPolyData->SetPoints(m_vtkCenterPoints);
	//m_vtkGlyph3D->SetSourceConnection(m_vtkSphereSource->GetOutputPort());
	m_vtkGlyph3D->SetInputData(m_vtkCenterPolyData);
	m_vtkGlyph3D->GeneratePointIdsOn();

	/////////lines
	//https://gitorious.org/vtkwikiexamples/wikiexamples/source/f1789a4a8887aba4f72f3159e19da2b6c56405e0:PolyData/Tube.cxx#L41
	_lines = vtkPolyData::New();

	vtkSmartPointer<vtkCellArray> lineCell = 
		vtkSmartPointer<vtkCellArray>::New();

	vtkSmartPointer<vtkLine> line = 
		vtkSmartPointer<vtkLine>::New();

	line->GetPointIds()->SetId(0,0);
	line->GetPointIds()->SetId(1,1);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,1);
	line->GetPointIds()->SetId(1,2);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,2);
	line->GetPointIds()->SetId(1,3);
	lineCell->InsertNextCell(line);


	///////////
	line->GetPointIds()->SetId(0,4);
	line->GetPointIds()->SetId(1,5);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,5);
	line->GetPointIds()->SetId(1,6);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,6);
	line->GetPointIds()->SetId(1,7);
	lineCell->InsertNextCell(line);

	/////////////

	line->GetPointIds()->SetId(0,8);
	line->GetPointIds()->SetId(1,9);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,9);
	line->GetPointIds()->SetId(1,10);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,10);
	line->GetPointIds()->SetId(1,11);
	lineCell->InsertNextCell(line);


	/////////////////////

	line->GetPointIds()->SetId(0,12);
	line->GetPointIds()->SetId(1,13);
	lineCell->InsertNextCell(line);


	line->GetPointIds()->SetId(0,13);
	line->GetPointIds()->SetId(1,14);
	lineCell->InsertNextCell(line);


	line->GetPointIds()->SetId(0,14);
	line->GetPointIds()->SetId(1,15);
	lineCell->InsertNextCell(line);

	//////////////////////
	line->GetPointIds()->SetId(0,16);
	line->GetPointIds()->SetId(1,17);
	lineCell->InsertNextCell(line);


	line->GetPointIds()->SetId(0,17);
	line->GetPointIds()->SetId(1,18);
	lineCell->InsertNextCell(line);


	line->GetPointIds()->SetId(0,18);
	line->GetPointIds()->SetId(1,19);
	lineCell->InsertNextCell(line);

	////palm
	line->GetPointIds()->SetId(0,20);
	line->GetPointIds()->SetId(1,21);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,21);
	line->GetPointIds()->SetId(1,22);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,22);
	line->GetPointIds()->SetId(1,23);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,23);
	line->GetPointIds()->SetId(1,24);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,24);
	line->GetPointIds()->SetId(1,25);
	lineCell->InsertNextCell(line);

	line->GetPointIds()->SetId(0,25);
	line->GetPointIds()->SetId(1,20);
	lineCell->InsertNextCell(line);

	///////////////
	_lines->SetPoints(m_vtkCenterPoints);
	_lines->SetLines(lineCell);

	//Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> lineMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	lineMapper->SetInputData(_lines);
	lineActor = 
		vtkSmartPointer<vtkActor>::New();
	lineActor->SetMapper(lineMapper);

	////if using this one without stripper filter, the tubes are broken at the joint point
	vtkSmartPointer<vtkStripper> stripperFilter = vtkStripper::New();
	stripperFilter->SetInputData(_lines);
	stripperFilter->Update();

	//Create a tube (cylinder) around the line
	vtkSmartPointer<vtkTubeFilter> tubeFilter = 
		vtkSmartPointer<vtkTubeFilter>::New();
	//tubeFilter->SetInputData(_lines);		
	tubeFilter->SetInputConnection(stripperFilter->GetOutputPort());
	tubeFilter->SetRadius(4); //default is .5
	tubeFilter->SetNumberOfSides(50);
	tubeFilter->Update();

	//	handTransformFilter->SetInputConnection(m_vtkGlyph3D->GetOutputPort());
	leapTransformFilter->SetInputConnection(tubeFilter->GetOutputPort());
	//handTransformFilter->SetInputData(_lines);
	leapTransformFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> polyMapper = vtkPolyDataMapper::New();
	polyMapper->SetInputConnection(leapTransformFilter->GetOutputPort());//handTransformFilter->GetOutputPort());
	vtkSmartPointer<vtkActor> polyActor = 
		vtkSmartPointer<vtkActor>::New();
	polyActor->SetMapper(polyMapper);
	polyActor->GetProperty()->SetColor(0.0,0.8,0.8); // Give some color to the line
	return polyActor;
}

void QtVTKRenderWindows::AddSnappingPlane()
{
	QVector3D dataCenter = _hw.GetOriginalDataCenter();

	//snapping plane
	_snappingPlane = vtkSmartPointer<vtkPlaneSource>::New(); 
	_snappingPlane->SetOrigin(0, 0, 0); 
	//planesrc->SetNormal(planeN); 
	_snappingPlane->SetPoint1(0, dataCenter.y() * 2, 0);
	_snappingPlane->SetPoint2(0, 0, dataCenter.z() * 2);
	_snappingPlane->Update(); 

	vtkSmartPointer<vtkPolyDataMapper> snappingPlaneMapper;
	snappingPlaneMapper = vtkPolyDataMapper::New();
	snappingPlaneMapper->SetInputConnection(_snappingPlane->GetOutputPort());

	_snappingPlaneActor = vtkActor::New();
	_snappingPlaneActor->SetMapper(snappingPlaneMapper);
	_snappingPlaneActor->GetProperty()->SetColor(1, 1, 1);
	_snappingPlaneActor->GetProperty()->LightingOff();
	_snappingPlaneActor->GetProperty()->SetOpacity(0.5);
	_renVol->AddActor(_snappingPlaneActor);
}

void QtVTKRenderWindows::AddAxisLines()
{

	//direction lines
	_dirLines[0] = vtkLineSource::New();
	_dirLines[1] = vtkLineSource::New();
	_dirLines[2] = vtkLineSource::New();

	float axisSize = _hw.GetMaxDataSize() * 0.2;
	_dirLines[0]->SetPoint1(0, 0, 0);
	_dirLines[1]->SetPoint1(0, 0, 0);
	_dirLines[2]->SetPoint1(0, 0, 0);

	_dirLines[0]->SetPoint2(0 + axisSize, 0, 0);
	_dirLines[1]->SetPoint2(0, 0 + axisSize, 0);
	_dirLines[2]->SetPoint2(0, 0, 0 + axisSize);

	vtkSmartPointer<vtkPolyDataMapper> dirLinesMapper[3];
	dirLinesMapper[0] = vtkPolyDataMapper::New();
	dirLinesMapper[1] = vtkPolyDataMapper::New();
	dirLinesMapper[2] = vtkPolyDataMapper::New();

	dirLinesMapper[0]->SetInputConnection(_dirLines[0]->GetOutputPort());
	dirLinesMapper[1]->SetInputConnection(_dirLines[1]->GetOutputPort());
	dirLinesMapper[2]->SetInputConnection(_dirLines[2]->GetOutputPort());

	_dirLinesActor[0] = vtkActor::New();
	_dirLinesActor[1] = vtkActor::New();
	_dirLinesActor[2] = vtkActor::New();

	_dirLinesActor[0]->SetMapper(dirLinesMapper[0]);
	_dirLinesActor[1]->SetMapper(dirLinesMapper[1]);
	_dirLinesActor[2]->SetMapper(dirLinesMapper[2]);

	_dirLinesActor[0]->GetProperty()->SetColor(1, 0, 0);
	_dirLinesActor[1]->GetProperty()->SetColor(0, 1, 0);
	_dirLinesActor[2]->GetProperty()->SetColor(0, 0, 1);

	_dirLinesActor[0]->GetProperty()->SetLineWidth(4);
	_dirLinesActor[1]->GetProperty()->SetLineWidth(4);
	_dirLinesActor[2]->GetProperty()->SetLineWidth(4);

	_renVol->AddActor(_dirLinesActor[0]);
	_renVol->AddActor(_dirLinesActor[1]);
	_renVol->AddActor(_dirLinesActor[2]);
}

void QtVTKRenderWindows::AddBarChart(vtkSmartPointer<vtkDataSetReader> reader)
{
	//Use vtkChartXY  
	//http://www.na-mic.org/Wiki/images/1/18/NA-MIC-VTK-Charts-2011.pdf


	vtkSmartPointer<vtkImageAccumulate> histogram = 
		vtkSmartPointer<vtkImageAccumulate>::New();
	histogram->SetInputConnection( reader->GetOutputPort());
	//histogram->SetComponentExtent( 0,255,0,0,0,0 );
	//histogram->SetComponentOrigin( 0,0,0 );
	//histogram->SetComponentSpacing( 1,0,0 );
	//histogram->SetIgnoreZero( ignoreZero );
	histogram->Update();

	// Create a vtkBarChartActor
	vtkSmartPointer<vtkBarChartActor> barChart = 
		vtkSmartPointer<vtkBarChartActor>::New();

	// BUG 1: if input is not set first, the x-axis of the bar chart will be too long
	//
	barChart->SetInput(histogram->GetOutput());
	barChart->SetTitle( "Histogram" );

	//barChart->GetPositionCoordinate()->SetValue(0.05,0.05,0.0);
	//barChart->GetPosition2Coordinate()->SetValue(0.95,0.85,0.0);
	//barChart->GetProperty()->SetColor(1,1,1);

	// BUG 2: if the number of entries is not set to the number of data array tuples, the bar chart actor will crash.
	// The crash occurs whether the legend and or labels are visible or not.

	//barChart->GetLegendActor()->SetNumberOfEntries( dataObject->GetFieldData()->GetArray(0)->GetNumberOfTuples() );
	//barChart->LegendVisibilityOff();
	//barChart->LabelVisibilityOff();

	// BUG 3: the y-axis labels do not accurately reflect the range of data

	//int count = 0;
	//for( int i = 0; i < 256; ++i )
	//{
	//	for( int j = 0; j < numComponents; ++j )
	//	{
	//		barChart->SetBarColor( count++, colors[j] );
	//	}
	//}

	//// Visualize the histogram(s)
	//vtkSmartPointer<vtkRenderer> renderer = 
	//	vtkSmartPointer<vtkRenderer>::New();
	//_renHist->AddActor( barChart );
}

void QtVTKRenderWindows::AddCamera()
{
	float maxDataSize = _hw.GetMaxDataSize();
	QVector3D dataCenter = _hw.GetOriginalDataCenter();

	camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(dataCenter.x(), dataCenter.y() + 0.1, dataCenter.z() + maxDataSize * 4);
	camera->SetViewUp(0, 1, 0);
	camera->SetFocalPoint(dataCenter.x(), dataCenter.y(), dataCenter.z());//, 0, 0);
	//camera->SetClippingRange(0.1,1000);//dataCenter[2] - maxDataSize * 10, dataCenter[2] + maxDataSize * 10);
	//camera->SetViewAngle(60);
	_renVol->SetActiveCamera(camera);
}

void QtVTKRenderWindows::ToggleStreamline()
{
	_streamlineOn = !_streamlineOn;
}

QtVTKRenderWindows::QtVTKRenderWindows( int argc, char *argv[])
{
	_iScale = 1;
	_streamlineOn = false;
	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	//vtkSmartPointer<vtkAlgorithm> reader;
	//if("-DICOM" == argv[0])	{
	//	vtkSmartPointer<vtkImageAlgorithm> reader = ReadImageData(argc, argv);
	//	inputVolume = reader->GetOutput();
	//} else if("-VTK" == argv[0])	{
	vtkSmartPointer<vtkDataSetReader> reader =
		vtkSmartPointer<vtkDataSetReader>::New();
	reader->SetFileName(argv[2]);
	reader->Update();
	//	reader->GetOutput()->Register(reader);
	//	reader->GetOutput()->GetPointData();
	//	return vtkDataSet::SafeDownCast(reader->GetOutput());
	//}

	//////////////////volume rendering
	_renVol =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view1->GetRenderWindow()->AddRenderer(_renVol);
	vtkRenderWindowInteractor *irenVol = this->ui->view1->GetInteractor();

	_ren2 = vtkSmartPointer< vtkRenderer >::New();
	this->ui->view2->GetRenderWindow()->AddRenderer(_ren2);

	// Add the volume to the scene
	_outlineActor = AddOutline(reader);
	_renVol->AddActor(_outlineActor);

	reader->GetOutput()->GetPointData()->GetArray(0)->GetRange(_scalerRange);
	_dataArray = reader->GetOutput()->GetPointData()->GetArray(0);
	//printf("* Range: %f - %f\n", _scalerRange[0], _scalerRange[1]);

	_volume =  AddVolume(reader->GetOutputPort()) ;
	_renVol->AddVolume(_volume );


	//AddPlaneWidget(reader, irenVol);

	AddLineWidget(reader, irenVol);
	//AddBoxWidget(reader, irenVol);

	//inputVolume->GetCenter(dataCenter);
	double dataBounds[6];

	reader->GetOutput()->GetBounds(dataBounds);
	_hw.SetDataSize(dataBounds[1], dataBounds[3], dataBounds[5]);


	AddCube();
	AddSnappingPlane();
//	AddStreamlines(reader);
	AddAxisLines();
	//AddCamera();
	AddMask();
	AddSphere();

	_renVol->AddActor(GetHandsActor());

	////	_renVol->AddActor(lineActor);
	//volumeTransform = vtkSmartPointer<vtkTransform>::New();

	
	//AddBarChart(reader);
	////second rendering window
	//vtkSmartPointer< vtkRenderer > renGlobe =
	//	vtkSmartPointer< vtkRenderer >::New();
	//this->ui->view4->GetRenderWindow()->AddRenderer(renGlobe);
	//renGlobe->AddActor(AddEarth());

	//inputGlobe->GetCenter(dataCenter);

	//cameraGlobe = vtkSmartPointer<vtkCamera>::New();
	//cameraGlobe->SetPosition(dataCenter[0], dataCenter[1] + 5 , dataCenter[2] + 0.001);
	//cameraGlobe->SetFocalPoint(dataCenter);//, 0, 0);
	//renGlobe->SetActiveCamera(cameraGlobe);

	qRegisterMetaType<TypeArray>("TypeArray");
	qRegisterMetaType<TypeArray2>("TypeArray2");

	//// Set up action signals and slots
	//connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

	controller.addListener(listener);
	//this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
	//	SLOT(setText(QString)));

	//controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);


	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)));

	//connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
	//	this, SLOT(UpdateCameraGlobe(QVector3D, QVector3D, QVector3D, QVector3D)));

	//connect(&listener, SIGNAL(UpdatePlane(QVector3D, QVector3D)), 
	//	this, SLOT(UpdatePlane(QVector3D, QVector3D)));

	//connect(&listener, SIGNAL(UpdateLine(QVector3D, QVector3D)), 
	//	this, SLOT(UpdateLine(QVector3D, QVector3D)));
	connect(&listener, SIGNAL(UpdateRightHand(QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateRightHand(QVector3D, QVector3D, QVector3D)));



	connect(&listener, SIGNAL(UpdateSkeletonHand(TypeArray2, TypeArray, float)), 
		this, SLOT(UpdateSkeletonHand(TypeArray2, TypeArray, float)), Qt::QueuedConnection);

	//connect(&listener, SIGNAL(UpdateGesture(int)), 
	//	this, SLOT(UpdateGesture(int)));

	vtkSmartPointer<vtkEventQtSlotConnect> m_connections = vtkEventQtSlotConnect::New();
	m_connections->Connect(irenVol, vtkCommand::KeyPressEvent, 
		this, SLOT(slotKeyPressed(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 0, 1.0); 







};

void QtVTKRenderWindows::AddStreamlines(vtkSmartPointer<vtkDataSetReader> reader)
{
	//reader->Update();
	// Source of the streamlines
	_seeds = 
		vtkSmartPointer<vtkLineSource>::New();
	//QVector3D point1(2,2,26), point2(2,2,26);
	//QVector3D step = (point2 - point2) * 0.125;
	//for(int i = 0; i < 9; i++)	{
	//	QVector3D p = point1 + i * step;
	//	_seeds->InsertNextPoint(p.x(), p.y(), p.z());
	//}
	_seeds->SetResolution(8);
	_seeds->SetPoint1(-1,-1,-1);
	_seeds->SetPoint2(-2,-2,-2);

	// Streamline itself
	vtkSmartPointer<vtkStreamLine> streamLine = 
		vtkSmartPointer<vtkStreamLine>::New();
	streamLine->SetInputConnection(reader->GetOutputPort());
	streamLine->SetSourceConnection(_seeds->GetOutputPort());

	//streamLine->SetStartPosition(2,-2,30);
	// as alternative to the SetSource(), which can handle multiple
	// streamlines, you can set a SINGLE streamline from
	// SetStartPosition()
	streamLine->SetMaximumPropagationTime(8);
	streamLine->SetIntegrationStepLength(1);
	streamLine->SetStepLength(.1);
	streamLine->SetNumberOfThreads(1);
	streamLine->SetIntegrationDirectionToForward();
	streamLine->VorticityOn();

	//Create a tube (cylinder) around the line
	vtkSmartPointer<vtkTubeFilter> tubeFilter = 
		vtkSmartPointer<vtkTubeFilter>::New();
	//tubeFilter->SetInputData(_lines);		
	tubeFilter->SetInputConnection(streamLine->GetOutputPort());
	tubeFilter->SetRadius(0.5); //default is .5
	tubeFilter->SetNumberOfSides(8);
	tubeFilter->Update();

	//vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();
	//cubeMapper->SetInputConnection(tubeFilter->GetOutputPort());//handTransformFilter->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> streamLineMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	streamLineMapper->SetInputConnection(tubeFilter->GetOutputPort());

	_streamLineActor = 
		vtkSmartPointer<vtkActor>::New();
	//_streamLineActor->GetProperty()->LightingOff();
	_streamLineActor->GetProperty()->SetColor(0.2, 1.0, 0.2);
	_streamLineActor->SetMapper(streamLineMapper);
	_streamLineActor->VisibilityOn();
	_renVol->AddActor(_streamLineActor);
}


void QtVTKRenderWindows::slotExit()
{
	qApp->exit();
}

void QtVTKRenderWindows::slotKeyPressed(vtkObject *a, unsigned long b, void *c, void *d, vtkCommand *command)
{
	//vtkIndent aa;
	//cameraGlobe->PrintSelf(cout,  aa);
	//_hw.ToggleHandAttachedToCube();
	//cout << "Keypress callback" << endl;

	vtkRenderWindowInteractor *iren = 
		static_cast<vtkRenderWindowInteractor*>(a);

	cout << "Pressed: " << iren->GetKeySym() << endl;
	char cc = iren->GetKeySym()[0];
	switch(cc)
	{
	case '1':
		_hw.SetInteractMode(INTERACT_MODE::HAND_SNAP);
		break;
	case '2':
		_hw.SetInteractMode(INTERACT_MODE::CUBE_DATA_TRANSLATE);
		break;
	case '3':
		_hw.SetInteractMode(INTERACT_MODE::CUBE_DATA_SCALE);
		break;
	case '4':
		_hw.SetInteractMode(INTERACT_MODE::CUBE_TRANSLATE);
		break;
	case '5':
		_hw.SetInteractMode(INTERACT_MODE::CUBE_SCALE);
		break;
	case '6':
		ToggleStreamline();
		break;
	case '9':
		_hw.SetInteractMode(INTERACT_MODE::DETACH_NORMALIZE);
		_hw.ResetCubeData();
		break;
	case '0':
		_hw.SetInteractMode(INTERACT_MODE::DETACH);
		_hw.ResetCube();
		break;
	case 'o':
		_iScale -= 1;
		std::cout<<"iScale:\t"<<_iScale<<std::endl;
		break;
	case 'p':
		_iScale += 1;
		std::cout<<"iScale:\t"<<_iScale<<std::endl;
		break;
	default:
		_hw.SetInteractMode(INTERACT_MODE::DETACH);
		break;
	}
}

void QtVTKRenderWindows::UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	_hw.SetHandLocation(origin);
	_hw.SetHandOrientation(xDir, yDir, zDir);

	_hw.UpdateTransformation();
	_hw.GetSnappedPlane();

	QVector3D snappingPoint = _hw.GetSnappingPoint();
	QMatrix4x4 axisOrientation = _hw.GetAdjustedHandOrientation();
	float axisSize = _hw.GetMaxDataSize() * 0.2;
	axisOrientation *= axisSize;

	_dirLines[0]->SetPoint1(snappingPoint.x(), snappingPoint.y(), snappingPoint.z());
	_dirLines[1]->SetPoint1(snappingPoint.x(), snappingPoint.y(), snappingPoint.z());
	_dirLines[2]->SetPoint1(snappingPoint.x(), snappingPoint.y(), snappingPoint.z());

	QVector3D end0 = snappingPoint + QVector3D(axisOrientation.column(0));
	QVector3D end1 = snappingPoint + QVector3D(axisOrientation.column(1));
	QVector3D end2 = snappingPoint + QVector3D(axisOrientation.column(2));
	_dirLines[0]->SetPoint2(end0.x(), end0.y(), end0.z());
	_dirLines[1]->SetPoint2(end1.x(), end1.y(), end1.z());
	_dirLines[2]->SetPoint2(end2.x(), end2.y(), end2.z());

	//_dirLines[0]->SetPoint2(axisOrientation.column(0).x(), axisOrientation.column(0).y(), axisOrientation.column(0).z());
	//_dirLines[1]->SetPoint2(axisOrientation.column(1).x(), axisOrientation.column(1).y(), axisOrientation.column(1).z());
	//_dirLines[2]->SetPoint2(axisOrientation.column(2).x(), axisOrientation.column(2).y(), axisOrientation.column(2).z());
	_dirLines[0]->Modified();
	_dirLines[1]->Modified();
	_dirLines[2]->Modified();

	QVector3D o, p1, p2, planeCenter;
	_hw.GetSnappedPlaneCoords(o, p1, p2);

	if(_hw.GetSnappingPlaneStatus())	{
		_snappingPlaneActor->VisibilityOn();
		_snappingPlane->SetOrigin(o.x(), o.y(), o.z());
		_snappingPlane->SetPoint1(p1.x(), p1.y(), p1.z());
		_snappingPlane->SetPoint2(p2.x(), p2.y(), p2.z());
	}	else {
		_snappingPlaneActor->VisibilityOff();
	}
	_snappingPlane->Modified();

	planeCenter = (p1 + p2) * 0.5;
	QMatrix4x4 planeTransformation;
	planeTransformation.translate(planeCenter);

	_volume->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	_maskVolume->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	_outlineActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	_cubeActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetCubeTransformation()));
	_snappingPlaneActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetCubeTransformation()));

	//if(_streamlineOn)
	//{
	//	_streamLineActor->VisibilityOn();
	//	QVector3D rightTip1, rightTip2;
	//	QVector3D rightTipData1, rightTipData2;
	//	_hw.GetRightHandTwoTips(rightTip1, rightTip2);
	//	rightTipData1 = _hw.CameraCoords2DataCoords(rightTip1);
	//	rightTipData2 = _hw.CameraCoords2DataCoords(rightTip2);
	//	_seeds->SetPoint1(rightTipData1.x(), rightTipData1.y(), rightTipData1.z());
	//	_seeds->SetPoint2(rightTipData2.x(), rightTipData2.y(), rightTipData2.z());
	//	_seeds->Modified();
	//	lineWidget->SetPoint1(rightTip1.x(), rightTip1.y(), rightTip1.z());
	//	lineWidget->SetPoint2(rightTip2.x(), rightTip2.y(), rightTip2.z());
	//	lineWidget->Modified();
	//	_streamLineActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	//} else {
	//	_streamLineActor->VisibilityOff();
	//}

	//UpdateMask(QVector3D(0,0,0));
	float radius = _hw.GetMinDataSize() * 0.05;
	float threshold = _scalerRange[0] + 0.1 * _iScale * (_scalerRange[1] - _scalerRange[0] );
	UpdateMask(_hw.CameraCoords2DataCoords(_hw.GetRightIndexTip()), radius, threshold);
	QVector3D sphereCenter = _hw.GetRightIndexTip();
	m_vtkSphereSource->SetCenter(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
	m_vtkSphereSource->SetRadius(radius * _hw.GetScaleFactor());

	this->ui->view1->repaint();
	this->ui->view2->repaint();
}

//void QtVTKRenderWindows::UpdateCameraGlobe(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
//{
//	handTranslation.setToIdentity();
//	handTranslation.setRow(0, QVector4D(xDir));
//	handTranslation.setRow(1, QVector4D(yDir));
//	handTranslation.setRow(2, QVector4D(zDir));
//
//	double dataCenter[3];
//	inputGlobe->GetCenter(dataCenter);
//	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);
//	QVector3D viewDir = m * handTranslation * ( QVector3D(0,0,-1));
//	//fixing the camera distance is a good idea
//	QVector3D cameraPos = dataCenterQ/* - 8 * m  * origin */ - viewDir * 5;
//	cameraGlobe->SetPosition(cameraPos.x(), cameraPos.y(), cameraPos.z());
//
//	QVector3D viewUp = m * handTranslation * yDir;
//	cameraGlobe->SetViewUp(viewUp.x(), viewUp.y(), viewUp.z());
//
//	//this->ui->view4->repaint();
//}


//QVector3D QtVTKRenderWindows::NormlizedLeapCoords2DataCoords(QVector3D p)
//{
//	//cout<< "origin:\t"<<origin.x()<<",\t"<<origin.y()<<",\t"<<origin.z()<<endl;
//	p = p - QVector3D(0.5,0.5,0.5);
//	p = m * p;
//	p = p + QVector3D(0.5,0.5,0.5);
//
//	double bbox[6];
//	inputVolume->GetBounds(bbox);
//	p.setX(p.x() * (bbox[1] - bbox[0]) + bbox[0]);
//	p.setY(p.y() * (bbox[3] - bbox[2]) + bbox[2]);
//	p.setZ(p.z() * (bbox[5] - bbox[4]) + bbox[4]);
//	return p;
//}

//void QtVTKRenderWindows::UpdatePlane(QVector3D origin, QVector3D normal)
//{
//	normal = m * handTranslation * normal;
//	origin = NormlizedLeapCoords2DataCoords(origin); 
//
//	repPlane->SetOrigin(origin.x(), origin.y(), origin.z());
//	repPlane->SetNormal(normal.x(), normal.y(), normal.z());
//}

//void QtVTKRenderWindows::UpdateLine(QVector3D point1, QVector3D point2)
//{
//	if(this->lineWidget == NULL)
//		return;
//	point1 = NormlizedLeapCoords2DataCoords(point1);
//	lineWidget->SetPoint1(point1.x(), point1.y(), point1.z());
//	point2 = NormlizedLeapCoords2DataCoords(point2);
//	lineWidget->SetPoint2(point2.x(), point2.y(), point2.z());
//}

void QtVTKRenderWindows::UpdateSkeletonHand(TypeArray2 fingers, TypeArray palm, float sphereRadius)
{

	if(fingers.size() == 0)
		return;

	_hw.SetSphereRadius(sphereRadius);
	_hw.SetHands(fingers, palm);

	int k = 0;

	int nCells = 0;
	for(int i = 0; i < fingers.size(); i++)
	{
		for(int j = 0; j < fingers[i].size(); j++)	{
			QVector3D v = fingers[i][j];
			m_vtkCenterPolyData->GetPoints()->SetPoint(k++, v.x(), v.y(), v.z());
		}
	}
	for(int i = 0; i < palm.size(); i++)	{
		QVector3D v = palm[i];
		m_vtkCenterPolyData->GetPoints()->SetPoint(k++, v.x(), v.y(), v.z());
	}
	//http://www.paraview.org/Wiki/VTK/Examples/Cxx/Interaction/MoveAGlyph
	m_vtkCenterPolyData->Modified();
	_lines->Modified();
	this->ui->view1->repaint();
}

void QtVTKRenderWindows::UpdateGesture(int gesture)
{
	//if(1 == gesture)
	//	_hw.ToggleHandAttachedToCube();
	//cout<<"gesture = "<<gesture<<endl;
}


void QtVTKRenderWindows::UpdateRightHand(QVector3D thumbTip, QVector3D indexTip, QVector3D indexDir)
{
	_hw.SetRightHand(thumbTip, indexTip, indexDir);
}

void QtVTKRenderWindows::AddMask()
{
	// Make the mask
	vtkImageGridSource *grid = vtkImageGridSource::New();
	grid->SetDataScalarTypeToUnsignedChar();
	QVector3D dataSize = _hw.GetDataSize();
	grid->SetDataExtent(0, dataSize.x(), 
		0, dataSize.y(), 
		0, dataSize.z());
	grid->SetLineValue(unsigned char( 255)); // mask value
	//grid->SetFillValue(unsigned char( 0));
	grid->SetGridSpacing(1,1,1);
	grid->Update();
	_mask = grid->GetOutput();

	//UpdateMask(QVector3D());
	_volumeMapper->SetMaskInput(_mask);
	_volumeMapper->SetMaskTypeToBinary();


	///////////render mask/////////
	_maskVolumeMapper = vtkGPUVolumeRayCastMapper::New();
	_maskVolumeMapper->SetInputData(_mask);
	//maskVolumeMapper->Update();

	// Create our transfer function
	vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New();
	vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New();

	// Create the property and attach the transfer functions
	vtkVolumeProperty *property = vtkVolumeProperty::New();
	property->SetIndependentComponents(true);
	property->SetColor( colorFun );
	property->SetScalarOpacity( opacityFun );
	//property->SetInterpolationTypeToLinear();
	property->SetInterpolationTypeToNearest();

	// connect up the volume to the property and the mapper
	//maskVolume->SetMapper( maskVolumeMapper );

	colorFun->AddRGBPoint( 0,  0, 0, 0);
	colorFun->AddRGBPoint( 255,  1, 1, 1);

	opacityFun->AddPoint(0, 0);
	opacityFun->AddPoint(255, 1);


	_maskVolumeMapper->SetBlendModeToComposite();
	_maskVolumeMapper->SetMaskTypeToBinary();
	property->ShadeOn();
	property->SetAmbient(0.1);
	property->SetDiffuse(0.9);
	property->SetSpecular(0.2);
	property->SetSpecularPower(10.0);
	property->SetScalarOpacityUnitDistance(0.8919);

	_maskVolume = vtkVolume::New();
	_maskVolume->SetProperty( property );
	_maskVolume->SetMapper( _maskVolumeMapper );

	_ren2->AddVolume(_maskVolume);

}


void QtVTKRenderWindows::UpdateMask(QVector3D center, float radius, float threshold)
{
	//reader->GetOutput()->GetPointData()->GetArray("mag");
	unsigned char *ptr = static_cast< unsigned char * >(_mask->GetScalarPointer());
	const double radiusSq = radius * radius;//pow(_hw.GetMaxDataSize() * 0.5, 2); // 7cm spherical mask
	//QVector3D center = _hw.GetOriginalDataCenter();
	QVector3D dataSize = _hw.GetDataSize();
	dataSize.setX(dataSize.x() + 1); 
	dataSize.setY(dataSize.y() + 1); 
	dataSize.setZ(dataSize.z() + 1); 
	QVector3D spacing(1, 1, 1);
	int iRadius = radius + 1;
	for (int z = center.z() - iRadius; z < center.z() + iRadius; z++)
	//for (int z = 0; z < dataSize.z(); z++)
	{
		if(z < 0 || z >= dataSize.z())
			continue;
		for (int y = center.y() - iRadius; y < center.y() + iRadius ; y++)
		//for (int y = 0; y < dataSize.y(); y++)
		{
			if(y < 0 || y >= dataSize.y())
				continue;

			for (int x = center.x() - iRadius; x < center.x() + iRadius; x++)
			//for (int x = 0; x < dataSize.x(); x++)
			{
				if(x < 0 || x >= dataSize.x())
					continue;
				const double distanceSq =
					((double)x*spacing.x()-center.x()) * ((double)x*spacing.x()-center.x()) +
					((double)y*spacing.y()-center.y()) * ((double)y*spacing.y()-center.y()) +
					((double)z*spacing.z()-center.z()) * ((double)z*spacing.z()-center.z());

				int id = z * (int)dataSize.y() * (int)dataSize.z() + y * (int)dataSize.z() + x ;
				double value = _dataArray->GetTuple(id)[0];
				if(distanceSq < radiusSq && value < threshold)
				{
					ptr[id] = 0;
				}
//				*ptr = ( ? 255 : 0);
				//if(int(*ptr) <1)
				//	std::cout<<int(*ptr)<<","<<std::endl;
				//++ptr;
			}
		}
	}
	_volumeMapper->SetMaskInput(_mask);
//	cout<<"updated..."<<endl;
	//_maskVolumeMapper->SetInputData(_mask);
//	_volumeMapper->Modified();
	_volumeMapper->MaskNeedUpdate();
	//_maskVolumeMapper->MaskNeedUpdate();
	//_volumeMapper->Update();
}
