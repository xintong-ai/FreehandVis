#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"
#include "qlabel.h"

class vtkIPWCallback : public vtkCommand
{
public:
	static vtkIPWCallback *New() 
	{ return new vtkIPWCallback; }
	virtual void Execute(vtkObject *caller, unsigned long, void*)
	{
		vtkImplicitPlaneWidget2 *planeWidget = 
			reinterpret_cast<vtkImplicitPlaneWidget2*>(caller);
		vtkImplicitPlaneRepresentation *rep = 
			reinterpret_cast<vtkImplicitPlaneRepresentation*>(planeWidget->GetRepresentation());
		rep->GetPlane(this->Plane);
	}
	vtkIPWCallback():Plane(0),Actor(0) {}
	vtkPlane *Plane;
	vtkActor *Actor;

};

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
	/*vtkSmartPointer<vtkBoxRepresentation> boxRep = vtkBoxRepresentation::New();
	boxRep->SetPlaceFactor(0.75);*/
	//boxRep->PlaceWidget(reader->GetOutput()->GetBounds());
	boxWidget->SetInputConnection(reader->GetOutputPort());
	boxWidget->PlaceWidget();
	boxWidget->SetInteractor(interactor);
	//boxWidget->SetRepresentation(boxRep);
	boxWidget->On();
	//boxWidget->rotate
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

vtkSmartPointer<vtkMatrix4x4> QMatrix2vtkMatrix(QMatrix4x4 v)
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



vtkSmartPointer<vtkActor> QtVTKRenderWindows::GetHandsActor()
{

	////////transform
	_leapTransform = vtkSmartPointer<vtkTransform>::New();

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);
	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);

	_leapTransform->PostMultiply();
	_leapTransform->SetMatrix(QMatrix2vtkMatrix(m));
	float scaleFactor = std::max(dataCenter[0], std::max(dataCenter[1], dataCenter[2])) / 100.0;
	_leapTransform->Translate(0, 0, 300);		//the volume is 300 mm above the device
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

	camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(dataCenter[0], dataCenter[1] + 800 , dataCenter[2] + 0.1);
	camera->SetFocalPoint(dataCenter);//, 0, 0);
	renVol->SetActiveCamera(camera);

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


	//plane
	// The callback will do the work
	//vtkSmartPointer<vtkPlane> plane =
	//	vtkSmartPointer<vtkPlane>::New();
	////vtkSmartPointer<vtkClipPolyData> clipper =
	////	vtkSmartPointer<vtkClipPolyData>::New();
	////clipper->SetClipFunction(plane);
	//vtkSmartPointer<vtkIPWCallback> myCallback = 
	//	vtkSmartPointer<vtkIPWCallback>::New();
	//myCallback->Plane = plane;
	////myCallback->Actor = actor;

	qRegisterMetaType<TypeArray>("TypeArray");
	qRegisterMetaType<TypeArray2>("TypeArray2");

	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

	controller.addListener(listener);
	this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
		SLOT(setText(QString)));

	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCameraGlobe(QVector3D, QVector3D, QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdatePlane(QVector3D, QVector3D)), 
		this, SLOT(UpdatePlane(QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateLine(QVector3D, QVector3D)), 
		this, SLOT(UpdateLine(QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdateSkeletonHand(TypeArray2, TypeArray)), 
		this, SLOT(UpdateSkeletonHand(TypeArray2, TypeArray)), Qt::QueuedConnection);

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
	vtkIndent aa;
	cameraGlobe->PrintSelf(cout,  aa);
}

QVector3D QtVTKRenderWindows::LeapVec2DataVec(QVector3D v)
{
	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);
	v = m * v;
	float scaleFactor = std::max(dataCenter[0], std::max(dataCenter[1], dataCenter[2])) / 100.0;
	v = v * scaleFactor;
	return v;

}

QVector3D QtVTKRenderWindows::LeapCoords2DataCoords(QVector3D v)
{
	//_leapTransform->SetMatrix(QMatrix2vtkMatrix(m));
	//float scaleFactor = std::max(dataCenter[0], std::max(dataCenter[1], dataCenter[2])) / 100.0;
	//_leapTransform->Translate(0, 0, 300);		//the volume is 300 mm above the device
	//_leapTransform->Scale(scaleFactor, scaleFactor, scaleFactor);
	//_leapTransform->Translate(dataCenter[0], dataCenter[1], dataCenter[2]);

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);
	v = m * v;
	v += QVector3D(0, 0,300);
	float scaleFactor = std::max(dataCenter[0], std::max(dataCenter[1], dataCenter[2])) / 100.0;
	v = v * scaleFactor;
	v += QVector3D(dataCenter[0], dataCenter[1], dataCenter[2]);
	return v;

}

void QtVTKRenderWindows::UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	handTranslation.setToIdentity();
	handTranslation.setRow(0, QVector4D(xDir));
	handTranslation.setRow(1, QVector4D(yDir));
	handTranslation.setRow(2, QVector4D(zDir));

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);
	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);

	//origin = m * origin;
	//origin += QVector3D(100, 100,300);
	////cout<<"origin1:"<<origin.x()<<"\t"<<origin.y()<<"\t"<<origin.z()<<endl;
	//origin = QVector3D(	origin.x() * dataCenter[0] / 100,
	//	origin.y() * dataCenter[1] / 100,
	//	origin.z() * dataCenter[2] / 100);
	origin = LeapCoords2DataCoords(origin);

	vtkSmartPointer<vtkTransform> handTransform = vtkSmartPointer<vtkTransform>::New();

	//this point is the origin of rotation in the data space,
	//which attaches to the hand center
	QVector3D fixPoint = LeapVec2DataVec(QVector3D(-150, 0, 0));

	bool withTranslation = true;
	volumeTransform->PostMultiply();
	volumeTransform->Identity();
	if(withTranslation)
	{
		volumeTransform->Translate(-fixPoint[0], -fixPoint[1], -fixPoint[2]);
	}
	volumeTransform->Translate(-dataCenter[0], -dataCenter[1], -dataCenter[2]);

	volumeTransform->Concatenate(QMatrix2vtkMatrix(m.inverted()));
	volumeTransform->Concatenate(QMatrix2vtkMatrix(handTranslation.inverted()));
	volumeTransform->Concatenate(QMatrix2vtkMatrix(m));

	if(withTranslation)
		volumeTransform->Translate(origin.x(), origin.y(), origin.z());
	else
		volumeTransform->Translate(dataCenter[0], dataCenter[1], dataCenter[2]);

	_volume->SetUserTransform(volumeTransform);
	_outlineActor->SetUserTransform(volumeTransform);


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

void QtVTKRenderWindows::UpdateSkeletonHand(TypeArray2 fingers, TypeArray palm )
{

	if(fingers.size() == 0)
		return;
	QVector<QVector<QVector3D>> tmp = fingers;
	//oneSphere->SetCenter(v.x(), v.y(), v.z());
	//m_vtkCenterPoints->Resize(0);
	//m_vtkCenterPolyData->Reset();
	//m_vtkCenterPoints->InsertNextPoint(50,50,50);
	//handTransformFilter->Update();
	int k = 0;

	int nCells = 0;
	for(int i = 0; i < fingers.size(); i++)
	{
		//	cout<<"sizes:"<<i<<","<<fingers[i].size()<<endl;
		for(int j = 0; j < fingers[i].size(); j++)	{
			QVector3D v = fingers[i][j];
			//	_fingerSpheres[k]->SetCenter(v.x(), v.y(), v.z());
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