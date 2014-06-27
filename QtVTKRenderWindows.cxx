#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"

#include "qlabel.h"

inline QVector3D Leap2QVector(Leap::Vector v)
{
	return QVector3D(v.x, v.y, v.z);
}

void MyListener::onFrame(const Leap::Controller & ctl)
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

		timer->restart();
	}
}

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
	opacityFun->AddPoint(641, .72, .5, 0.0 );
	opacityFun->AddPoint(3071, .71, 0.5, 0.0);

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

QtVTKRenderWindows::QtVTKRenderWindows( int argc, char *argv[])
{
	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	vtkSmartPointer<vtkImageAlgorithm> reader = ReadImageData(argc, argv);
	input = reader->GetOutput();

	//////////////////volume rendering
	vtkSmartPointer< vtkRenderer > renVol =
		vtkSmartPointer< vtkRenderer >::New();
	this->ui->view1->GetRenderWindow()->AddRenderer(renVol);
	vtkRenderWindowInteractor *irenVol = this->ui->view1->GetInteractor();

	// Add the volume to the scene
	vtkSmartPointer<vtkActor> outlineActor = AddOutline(reader);
	renVol->AddVolume( AddVolume(reader) );
	renVol->AddActor(outlineActor);

	double dataCenter[3];
	input->GetCenter(dataCenter);

	camera = vtkSmartPointer<vtkCamera>::New();
	//camera->SetPosition(dataCenter[0] , dataCenter[1], dataCenter[2] - 66);
	camera->SetPosition(dataCenter[0], dataCenter[1] + 600 , dataCenter[2] + 0.1);
	camera->SetFocalPoint(dataCenter);//, 0, 0);
	//camera->SetRoll(90);
	renVol->SetActiveCamera(camera);


	//plane widget
	//http://www.cnblogs.com/dawnWind/archive/2013/05/04/3D_11.html
	//http://www.vtk.org/Wiki/VTK/Examples/Cxx/Widgets/ImplicitPlaneWidget2
	rep = vtkSmartPointer<vtkImplicitPlaneRepresentation>::New();
	rep->SetPlaceFactor(1.1); // This must be set prior to placing the widget
	rep->PlaceWidget(outlineActor->GetBounds());
	implicitPlaneWidget = vtkImplicitPlaneWidget2::New();
	implicitPlaneWidget->SetInteractor(irenVol);
	implicitPlaneWidget->SetRepresentation(rep);
	implicitPlaneWidget->On();

	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));

	controller.addListener(listener);
	controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
	this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
		SLOT(setText(QString)));
	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)));

	connect(&listener, SIGNAL(UpdatePlane(QVector3D, QVector3D)), 
		this, SLOT(UpdatePlane(QVector3D, QVector3D)));

	vtkSmartPointer<vtkEventQtSlotConnect> m_connections = vtkEventQtSlotConnect::New();
	m_connections->Connect(irenVol, vtkCommand::KeyPressEvent, 
		this, SLOT(slotKeyPressed(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 0, 1.0); 

};

void QtVTKRenderWindows::slotExit()
{
	qApp->exit();
}

void QtVTKRenderWindows::ResetViews()
{
	// Render in response to changes.
	//this->Render();
}



void QtVTKRenderWindows::slotKeyPressed(vtkObject *a, unsigned long b, void *c, void *d, vtkCommand *command)
{
	//if ( true) {
	//    // consume event so the interactor doesn't get it
	//    command->AbortFlagOn();
	//}

	// //    myLabel->setText("You pressed ESC");
	double p[3];
	camera->GetPosition(p);
	//	p[1] +=20;
	//	camera->SetPosition(p);
	cout<<"camera position:"<<p[0]<<","<<p[1]<<","<<p[2]<<endl;
	camera->GetFocalPoint(p);
	cout<<"focal point:"<<p[0]<<","<<p[1]<<","<<p[2]<<endl;
	vtkIndent aa;
	camera->PrintSelf(cout,  aa);
}

void QtVTKRenderWindows::UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	QMatrix4x4 m;
	m.rotate(- 90, 1.0, 0.0, 0.0);

	handTranslation.setToIdentity();
	handTranslation.setRow(0, QVector4D(xDir));
	handTranslation.setRow(1, QVector4D(yDir));
	handTranslation.setRow(2, QVector4D(zDir));

	double dataCenter[3];
	input->GetCenter(dataCenter);
	QVector3D dataCenterQ(dataCenter[0], dataCenter[1], dataCenter[2]);

	
	QVector3D viewDir = m * handTranslation * ( QVector3D(0,0,-1));
	QVector3D cameraPos = dataCenterQ - viewDir * 600;
	camera->SetPosition(cameraPos.x(), cameraPos.y(), cameraPos.z());
	
	QVector3D viewUp = m * handTranslation * yDir;
	camera->SetViewUp(viewUp.x(), viewUp.y(), viewUp.z());
	
	this->ui->view1->repaint();
}


void QtVTKRenderWindows::UpdatePlane(QVector3D origin, QVector3D normal)
{
	//cout<< "origin:\t"<<origin.x()<<",\t"<<origin.y()<<",\t"<<origin.z()<<endl;
	QMatrix4x4 m;
	m.rotate(- 90, 1.0, 0.0, 0.0);
	origin = origin - QVector3D(0.5,0.5,0.5);
	origin = m * origin;
	origin = origin + QVector3D(0.5,0.5,0.5);

	normal = m * handTranslation * normal;

	//cout<< "origin:\t"<<origin.x()<<",\t"<<origin.y()<<",\t"<<origin.z()<<endl;

	double bbox[6];
	input->GetBounds(bbox);
	origin.setX(origin.x() * (bbox[1] - bbox[0]) + bbox[0]);
	origin.setY(origin.y() * (bbox[3] - bbox[2]) + bbox[2]);
	origin.setZ(origin.z() * (bbox[5] - bbox[4]) + bbox[4]);

	rep->SetOrigin(origin.x(), origin.y(), origin.z());
	rep->SetNormal(normal.x(), normal.y(), normal.z());
}
