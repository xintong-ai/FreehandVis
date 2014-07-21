#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"
#include "qlabel.h"

vtkSmartPointer<vtkImageAlgorithm> ReadImageData(int argc, char *argv[])
{
	int count = 1;
	char *dirname = NULL;
	char *fileName=0;
	int fileType=0;
	vtkSmartPointer<vtkImageAlgorithm> readerRet;
	//while ( count < argc )
	//{
	if ( !strcmp( argv[count], "-DICOM" ) )
	{
		dirname = new char[strlen(argv[count+1])+1];
		sprintf( dirname, "%s", argv[count+1] );
		count += 2;
		vtkSmartPointer<vtkDICOMImageReader> reader =
			vtkSmartPointer<vtkDICOMImageReader>::New();
		reader->SetDirectoryName(dirname);
		readerRet = reader;
	}


	readerRet->Update();

	return readerRet;
	//readerRet = reader;
	//	reader->GetOutput()->GetDimensions(imageDims);
}

vtkSmartPointer<vtkVolume> QtVTKRenderWindows::AddVolume(vtkSmartPointer<vtkImageAlgorithm> reader)
{

	vtkSmartPointer<vtkImageResample> resample = vtkImageResample::New();

	// Create our volume and mapper
	vtkVolume *volume = vtkVolume::New();
	vtkSmartVolumeMapper *mapper = vtkSmartVolumeMapper::New();

	mapper->SetInputConnection( reader->GetOutputPort() );


	// Create our transfer function
	vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New();
	vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New();

	// Create the property and attach the transfer functions
	vtkVolumeProperty *property = vtkVolumeProperty::New();
	property->SetIndependentComponents(true);
	property->SetColor( colorFun );
	property->SetScalarOpacity( opacityFun );
	property->SetInterpolationTypeToLinear();

	// connect up the volume to the property and the mapper
	volume->SetProperty( property );
	volume->SetMapper( mapper );

	// Depending on the blend type selected as a command line option,
	// adjustthe transfer function
	colorFun->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0.0 );
	colorFun->AddRGBPoint( -16, 0.73, 0.25, 0.30, 0.49, .61 );
	colorFun->AddRGBPoint( 641, .90, .82, .56, .5, 0.0 );
	colorFun->AddRGBPoint( 3071, 1, 1, 1, .5, 0.0 );

	opacityFun->AddPoint(-3024, 0, 0.5, 0.0 );
	opacityFun->AddPoint(-16, 0, .49, .61 );
	opacityFun->AddPoint(641, .02, .5, 0.0 );
	opacityFun->AddPoint(3071, .01, 0.5, 0.0);

	mapper->SetBlendModeToComposite();
	property->ShadeOn();
	property->SetAmbient(0.1);
	property->SetDiffuse(0.9);
	property->SetSpecular(0.2);
	property->SetSpecularPower(10.0);
	property->SetScalarOpacityUnitDistance(0.8919);

	return volume;
}

vtkSmartPointer<vtkActor> AddOutline(vtkSmartPointer<vtkImageAlgorithm> reader)
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
	//_cube = vtkCubeSource::New();
	////_cube->set
	//float cubeHalfSize = _hw.GetCubeSize() * 0.5;
	//float dataCenter[3];
	//_hw.GetDataCenter(dataCenter);
	//_cube->SetBounds(
	//	dataCenter[0] - cubeHalfSize, 
	//	dataCenter[0] + cubeHalfSize, 
	//	dataCenter[1] - cubeHalfSize, 
	//	dataCenter[1] + cubeHalfSize, 
	//	dataCenter[2] - cubeHalfSize, 
	//	dataCenter[2] + cubeHalfSize);

	vtkSmartPointer<vtkPoints> cubePoints = vtkPoints::New();
	//vtkSmartPointer<vtkPolyData> cubePolyData = vtkPolyData::New();
	//m_vtkSphereSource = vtkSphereSource::New();
	//m_vtkSphereSource->SetRadius(4);
	//m_vtkGlyph3D = vtkGlyph3D::New();
	/*for(int i = 0; i < 26; i++)
		cubePoints->InsertNextPoint(10,10,10);*/
	QVector3D cubeMin, cubeMax;
	_hw.GetOrignalCubeCoords(cubeMin, cubeMax);
	cubePoints->InsertNextPoint(cubeMin[0], cubeMin[1], cubeMin[2]);
	cubePoints->InsertNextPoint(cubeMin[0], cubeMax[1], cubeMin[2]);
	cubePoints->InsertNextPoint(cubeMax[0], cubeMax[1], cubeMin[2]);
	cubePoints->InsertNextPoint(cubeMax[0], cubeMin[1], cubeMin[2]);

	cubePoints->InsertNextPoint(cubeMin[0], cubeMin[1], cubeMax[2]);
	cubePoints->InsertNextPoint(cubeMin[0], cubeMax[1], cubeMax[2]);
	cubePoints->InsertNextPoint(cubeMax[0], cubeMax[1], cubeMax[2]);
	cubePoints->InsertNextPoint(cubeMax[0], cubeMin[1], cubeMax[2]);

	//cubePolyData->SetPoints(cubePoints);
	/*m_vtkGlyph3D->SetSourceConnection(m_vtkSphereSource->GetOutputPort());
	m_vtkGlyph3D->SetInputData(m_vtkCenterPolyData);
	m_vtkGlyph3D->GeneratePointIdsOn();
*/
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

	//Create a mapper and actor
	//vtkSmartPointer<vtkPolyDataMapper> cubeMapper = 
	//	vtkSmartPointer<vtkPolyDataMapper>::New();
	//cubeMapper->SetInputData(lines);

	//_cubeActor->SetMapper(cubeMapper);

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

	////	handTransformFilter->SetInputConnection(m_vtkGlyph3D->GetOutputPort());
	//leapTransformFilter->SetInputConnection(tubeFilter->GetOutputPort());
	////handTransformFilter->SetInputData(_lines);
	//leapTransformFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();
	cubeMapper->SetInputConnection(tubeFilter->GetOutputPort());//handTransformFilter->GetOutputPort());
	//vtkSmartPointer<vtkActor> polyActor = 
	//	vtkSmartPointer<vtkActor>::New();
	_cubeActor = vtkSmartPointer<vtkActor>::New();
	_cubeActor->SetMapper(cubeMapper);
	_cubeActor->GetProperty()->SetColor(1.0,1.0, 1.0); // Give some color to the line
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

void QtVTKRenderWindows::AddBoxWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor)
{
	vtkSmartPointer<vtkBoxWidget> boxWidget = vtkBoxWidget::New();
	boxWidget->SetInputConnection(reader->GetOutputPort());
	boxWidget->PlaceWidget();
	boxWidget->SetInteractor(interactor);
	boxWidget->On();
}

void QtVTKRenderWindows::AddLineWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor)
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

void QtVTKRenderWindows::AddPlaneWidget(vtkSmartPointer<vtkImageAlgorithm> reader, vtkRenderWindowInteractor *interactor)
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
	implicitPlaneWidget->On();
}

vtkSmartPointer<vtkActor> QtVTKRenderWindows::GetHandsActor()
{

	////////transform
	_leapTransform = vtkSmartPointer<vtkTransform>::New();

	float dataCenter[3];
	_hw.GetDataCenter(dataCenter);

	_leapTransform->PostMultiply();
	_leapTransform->SetMatrix(QMatrix2vtkMatrix(_hw.GetDataOrientation()));
	_leapTransform->Translate(0, 0, _hw.GetLeapDataHeight());		//the volume center is 200 mm above the device
	float scaleFactor = _hw.GetScaleFactor();	//the side of the cube is 100 mm
	_leapTransform->Scale(scaleFactor, scaleFactor, scaleFactor);
	_leapTransform->Translate(dataCenter[0], dataCenter[1], dataCenter[2]);

	vtkSmartPointer<vtkTransformPolyDataFilter> leapTransformFilter = 
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	leapTransformFilter->SetTransform(_leapTransform);

	////////////spheres
	m_vtkCenterPoints = vtkPoints::New();
	m_vtkCenterPolyData = vtkPolyData::New();
	m_vtkSphereSource = vtkSphereSource::New();
	m_vtkSphereSource->SetRadius(4);
	m_vtkGlyph3D = vtkGlyph3D::New();
	for(int i = 0; i < 26; i++)
		m_vtkCenterPoints->InsertNextPoint(10,10,10);

	m_vtkCenterPolyData->SetPoints(m_vtkCenterPoints);
	m_vtkGlyph3D->SetSourceConnection(m_vtkSphereSource->GetOutputPort());
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


QtVTKRenderWindows::QtVTKRenderWindows( int argc, char *argv[])
{
	m.rotate(-90, 1.0, 0.0, 0.0);

	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	vtkSmartPointer<vtkImageAlgorithm> reader = ReadImageData(argc, argv);
	inputVolume = reader->GetOutput();

	//////////////////volume rendering
	vtkSmartPointer< vtkRenderer > renVol =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view1->GetRenderWindow()->AddRenderer(renVol);
	vtkRenderWindowInteractor *irenVol = this->ui->view1->GetInteractor();

	//direction lines
	_dirLines[0] = vtkLineSource::New();
	_dirLines[1] = vtkLineSource::New();
	_dirLines[2] = vtkLineSource::New();

	_dirLines[0]->SetPoint1(0, 0, 0);
	_dirLines[1]->SetPoint1(0, 0, 0);
	_dirLines[2]->SetPoint1(0, 0, 0);

	_dirLines[0]->SetPoint2(100, 0, 0);
	_dirLines[1]->SetPoint2(0, 100, 0);
	_dirLines[2]->SetPoint2(0, 0, 100);

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

	renVol->AddActor(_dirLinesActor[0]);
	renVol->AddActor(_dirLinesActor[1]);
	renVol->AddActor(_dirLinesActor[2]);



	// Add the volume to the scene
	_outlineActor = AddOutline(reader);

	_volume =  AddVolume(reader) ;
	renVol->AddVolume(_volume );
	renVol->AddActor(_outlineActor);

	AddPlaneWidget(reader, irenVol);

	AddLineWidget(reader, irenVol);
	//AddBoxWidget(reader, irenVol);

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);

	_hw.SetDataSize(dataCenter[0] * 2, dataCenter[1] * 2, dataCenter[2] * 2);

	AddCube();
	renVol->AddActor(_cubeActor);


	camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(dataCenter[0], dataCenter[1] + 800 , dataCenter[2] + 0.1);
	camera->SetFocalPoint(dataCenter);//, 0, 0);
	renVol->SetActiveCamera(camera);


	//snapping plane
	_snappingPlane = vtkSmartPointer<vtkPlaneSource>::New(); 
	_snappingPlane->SetOrigin(0, 0, 0); 
	//planesrc->SetNormal(planeN); 
	_snappingPlane->SetPoint1(0, dataCenter[1] * 2, 0);
	_snappingPlane->SetPoint2(0, 0, dataCenter[2] * 2);
	_snappingPlane->Update(); 

	vtkSmartPointer<vtkPolyDataMapper> snappingPlaneMapper;
	snappingPlaneMapper = vtkPolyDataMapper::New();
	snappingPlaneMapper->SetInputConnection(_snappingPlane->GetOutputPort());

	_snappingPlaneActor = vtkActor::New();
	_snappingPlaneActor->SetMapper(snappingPlaneMapper);
	_snappingPlaneActor->GetProperty()->SetColor(1, 1, 1);
	_snappingPlaneActor->GetProperty()->LightingOff();
	_snappingPlaneActor->GetProperty()->SetOpacity(0.5);
	renVol->AddActor(_snappingPlaneActor);


	//second rendering window
	vtkSmartPointer< vtkRenderer > renGlobe =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view4->GetRenderWindow()->AddRenderer(renGlobe);
	renGlobe->AddActor(AddEarth());

	inputGlobe->GetCenter(dataCenter);

	cameraGlobe = vtkSmartPointer<vtkCamera>::New();
	cameraGlobe->SetPosition(dataCenter[0], dataCenter[1] + 5 , dataCenter[2] + 0.001);
	cameraGlobe->SetFocalPoint(dataCenter);//, 0, 0);
	renGlobe->SetActiveCamera(cameraGlobe);

	qRegisterMetaType<TypeArray>("TypeArray");
	qRegisterMetaType<TypeArray2>("TypeArray2");

	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

	controller.addListener(listener);
	this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
		SLOT(setText(QString)));

	controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);


	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCameraGlobe(QVector3D, QVector3D, QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdatePlane(QVector3D, QVector3D)), 
		this, SLOT(UpdatePlane(QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateLine(QVector3D, QVector3D)), 
		this, SLOT(UpdateLine(QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateSkeletonHand(TypeArray2, TypeArray, float)), 
		this, SLOT(UpdateSkeletonHand(TypeArray2, TypeArray, float)), Qt::QueuedConnection);

	connect(&listener, SIGNAL(UpdateGesture(int)), 
		this, SLOT(UpdateGesture(int)));

	vtkSmartPointer<vtkEventQtSlotConnect> m_connections = vtkEventQtSlotConnect::New();
	m_connections->Connect(irenVol, vtkCommand::KeyPressEvent, 
		this, SLOT(slotKeyPressed(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 0, 1.0); 




	renVol->AddActor(GetHandsActor());
	//	renVol->AddActor(lineActor);
	volumeTransform = vtkSmartPointer<vtkTransform>::New();


};

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
		_hw.SetInteractMode(INTERACT_MODE::CUBE_TRANSLATE);
		break;
	case '3':
		_hw.SetInteractMode(INTERACT_MODE::CUBE_SCALE);
		break;
	case '4':
		break;
	case '5':
		break;
	case '0':
		_hw.SetInteractMode(INTERACT_MODE::DETACH);
		_hw.ResetTranslationScale();
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

	QMatrix4x4 cubeOrientation = _hw.GetAdjustedHandOrientation() * 100;
	_dirLines[0]->SetPoint2(cubeOrientation.column(0).x(), cubeOrientation.column(0).y(), cubeOrientation.column(0).z());
	_dirLines[1]->SetPoint2(cubeOrientation.column(1).x(), cubeOrientation.column(1).y(), cubeOrientation.column(1).z());
	_dirLines[2]->SetPoint2(cubeOrientation.column(2).x(), cubeOrientation.column(2).y(), cubeOrientation.column(2).z());
	_dirLines[0]->Modified();
	_dirLines[1]->Modified();
	_dirLines[2]->Modified();

	QVector3D o, p1, p2, planeCenter;
	_hw.GetSnappedPlaneCoords(o, p1, p2);
	_snappingPlane->SetOrigin(o.x(), o.y(), o.z());
	_snappingPlane->SetPoint1(p1.x(), p1.y(), p1.z());
	_snappingPlane->SetPoint2(p2.x(), p2.y(), p2.z());
	_snappingPlane->Modified();

	planeCenter = (p1 + p2) * 0.5;
	QMatrix4x4 planeTransformation;
	planeTransformation.translate(planeCenter);

	_volume->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	_outlineActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetDataTransformation()));
	_cubeActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetCubeTransformation()));
	_snappingPlaneActor->SetUserMatrix(QMatrix2vtkMatrix(_hw.GetCubeTransformation()));
	//_dirLinesActor[0]->SetUserMatrix(QMatrix2vtkMatrix(planeTransformation * _hw.GetCubeTransformation()));
	//_dirLinesActor[1]->SetUserMatrix(QMatrix2vtkMatrix(planeTransformation * _hw.GetCubeTransformation()));
	//_dirLinesActor[2]->SetUserMatrix(QMatrix2vtkMatrix(planeTransformation * _hw.GetCubeTransformation()));

	_hw.GetSnappedPlane();

	this->ui->view1->repaint();
}

void QtVTKRenderWindows::UpdateCameraGlobe(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	handTranslation.setToIdentity();
	handTranslation.setRow(0, QVector4D(xDir));
	handTranslation.setRow(1, QVector4D(yDir));
	handTranslation.setRow(2, QVector4D(zDir));

	double dataCenter[3];
	inputGlobe->GetCenter(dataCenter);
	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);
	QVector3D viewDir = m * handTranslation * ( QVector3D(0,0,-1));
	//fixing the camera distance is a good idea
	QVector3D cameraPos = dataCenterQ/* - 8 * m  * origin */ - viewDir * 5;
	cameraGlobe->SetPosition(cameraPos.x(), cameraPos.y(), cameraPos.z());

	QVector3D viewUp = m * handTranslation * yDir;
	cameraGlobe->SetViewUp(viewUp.x(), viewUp.y(), viewUp.z());

	this->ui->view4->repaint();
}


QVector3D QtVTKRenderWindows::NormlizedLeapCoords2DataCoords(QVector3D p)
{
	//cout<< "origin:\t"<<origin.x()<<",\t"<<origin.y()<<",\t"<<origin.z()<<endl;
	p = p - QVector3D(0.5,0.5,0.5);
	p = m * p;
	p = p + QVector3D(0.5,0.5,0.5);

	double bbox[6];
	inputVolume->GetBounds(bbox);
	p.setX(p.x() * (bbox[1] - bbox[0]) + bbox[0]);
	p.setY(p.y() * (bbox[3] - bbox[2]) + bbox[2]);
	p.setZ(p.z() * (bbox[5] - bbox[4]) + bbox[4]);
	return p;
}

void QtVTKRenderWindows::UpdatePlane(QVector3D origin, QVector3D normal)
{
	normal = m * handTranslation * normal;
	origin = NormlizedLeapCoords2DataCoords(origin); 

	repPlane->SetOrigin(origin.x(), origin.y(), origin.z());
	repPlane->SetNormal(normal.x(), normal.y(), normal.z());
}

void QtVTKRenderWindows::UpdateLine(QVector3D point1, QVector3D point2)
{
	if(this->lineWidget == NULL)
		return;
	point1 = NormlizedLeapCoords2DataCoords(point1);
	lineWidget->SetPoint1(point1.x(), point1.y(), point1.z());
	point2 = NormlizedLeapCoords2DataCoords(point2);
	lineWidget->SetPoint2(point2.x(), point2.y(), point2.z());
}

void QtVTKRenderWindows::UpdateSkeletonHand(TypeArray2 fingers, TypeArray palm, float sphereRadius)
{

	if(fingers.size() == 0)
		return;

	_hw.SetSphereRadius(sphereRadius);

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
