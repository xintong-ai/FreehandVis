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

vtkSmartPointer<vtkActor> QtVTKRenderWindows::AddOutline(vtkSmartPointer<vtkImageAlgorithm> reader)
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
	repPlane->SetPlaceFactor(1.1); // This must be set prior to placing the widget
	repPlane->PlaceWidget(reader->GetOutput()->GetBounds());
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
	vtkSmartPointer<vtkLineSource> line = vtkLineSource::New();
	line->SetPoint1(0, 0, 0);
	line->SetPoint2(50, 50,50);

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	//transform->RotateWXYZ(double angle, double x, double y, double z);
	//transform->RotateWXYZ(10, 0, 1, 0);
	QMatrix4x4 handrotateMatrix = handTranslation.inverted();
	transform->SetMatrix(QMatrix2vtkMatrix(handrotateMatrix));

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();

	transformFilter->SetTransform(transform);
	transformFilter->SetInputConnection(line->GetOutputPort());
	transformFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> polyMapper = vtkPolyDataMapper::New();
	polyMapper->SetInputConnection(transformFilter->GetOutputPort());
	vtkSmartPointer<vtkActor> polyActor = 
		vtkSmartPointer<vtkActor>::New();
	polyActor->SetMapper(polyMapper);
	return polyActor;
}


QtVTKRenderWindows::QtVTKRenderWindows( int argc, char *argv[])
{
	m.rotate(- 90, 1.0, 0.0, 0.0);

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
	vtkSmartPointer<vtkActor> outlineActor = AddOutline(reader);
	renVol->AddVolume( AddVolume(reader) );
	renVol->AddActor(outlineActor);

	AddPlaneWidget(reader, irenVol);

	AddLineWidget(reader, irenVol);
	//AddBoxWidget(reader, irenVol);

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);

	camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(dataCenter[0], dataCenter[1] + 600 , dataCenter[2] + 0.1);
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

	vtkSmartPointer<vtkEventQtSlotConnect> m_connections = vtkEventQtSlotConnect::New();
	m_connections->Connect(irenVol, vtkCommand::KeyPressEvent, 
		this, SLOT(slotKeyPressed(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 0, 1.0); 




	renVol->AddActor(GetHandsActor());
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

void QtVTKRenderWindows::UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	handTranslation.setToIdentity();
	handTranslation.setRow(0, QVector4D(xDir));
	handTranslation.setRow(1, QVector4D(yDir));
	handTranslation.setRow(2, QVector4D(zDir));

	double dataCenter[3];
	inputVolume->GetCenter(dataCenter);
	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);


	QVector3D viewDir = m * handTranslation * ( QVector3D(0,0,-1));
	//fixing the camera distance is a good idea
	QVector3D cameraPos = dataCenterQ/* - 8 * m  * origin */ - viewDir * 600;
	camera->SetPosition(cameraPos.x(), cameraPos.y(), cameraPos.z());

	QVector3D viewUp = m * handTranslation * yDir;
	camera->SetViewUp(viewUp.x(), viewUp.y(), viewUp.z());

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