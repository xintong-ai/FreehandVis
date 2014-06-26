#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"

#include "qlabel.h"

inline QVector3D Leap2QVector(Leap::Vector v)
{
	return QVector3D(v.x, v.y, v.z);
}

void MyListener::onFrame(const Leap::Controller & ctl)
{

	if(timer->elapsed() > 20)
	{
		Leap::Frame f = ctl.frame();
		// This is a hack so that we avoid having to declare a signal and
		// use moc generated code.
		setObjectName(QString::number(f.id()));
		// emits objectNameChanged(QString)
		//emit translate2(SimpleTranslate(f));

		/////////////////
		//Leap::Vector origin, point1, point2;
		//GetRectangle(f, origin, point1, point2);
		//GetAbsoluteRectangle(f, origin, point1, point2);
		//QVector3D origin_q(origin.x, origin.y, origin.z);
		//QVector3D point1_q(point1.x, point1.y, point1.z);
		//QVector3D point2_q(point2.x, point2.y, point2.z);
		//emit UpdateRectangle(origin_q, point1_q, point2_q);

		//camera

		Leap::Vector center, xDir, yDir, zDir;
		GetSpace(f, center, xDir, yDir, zDir);
		//zDir = - zDir;
		emit UpdateCamera(Leap2QVector(center), Leap2QVector(xDir), Leap2QVector(yDir), Leap2QVector(zDir));

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
//
//void QtVTKRenderWindows::keyPressEvent(QKeyEvent *event)
//{
//	if(event->key() == Qt::Key_Up)
//    {
//    //    myLabel->setText("You pressed ESC");
//		double p[3];
//		camera->GetPosition(p);
//		p[2] +=10;
//		camera->SetPosition(p);
//    }
//}


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
	renVol->AddVolume( AddVolume(reader) );
	renVol->AddActor(AddOutline(reader));

	double dataCenter[3];
	input->GetCenter(dataCenter);

	camera = vtkSmartPointer<vtkCamera>::New();
	//camera->SetPosition(dataCenter[0] , dataCenter[1], dataCenter[2] - 66);
	camera->SetPosition(dataCenter[0], dataCenter[1] + 600 , dataCenter[2] + 0.1);
	camera->SetFocalPoint(dataCenter);//, 0, 0);
	//camera->SetRoll(90);
	renVol->SetActiveCamera(camera);


	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));

	controller.addListener(listener);
	controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
	this->ui->label_observer->connect(&listener, SIGNAL(objectNameChanged(QString)),
		SLOT(setText(QString)));
	connect(&listener, SIGNAL(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)), 
		this, SLOT(UpdateCamera(QVector3D, QVector3D, QVector3D, QVector3D)));

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

////http://en.wikipedia.org/wiki/Rotation_matrix
//inline QVector3D rotateX()
//{
//	QMatrix4x4 m;
//}

void QtVTKRenderWindows::UpdateCamera(QVector3D origin, QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	
	QMatrix4x4 m;

	QVector3D palmDir = xDir;
	QVector3D thumbDir = yDir;
	QVector3D fingerDir = -zDir;

	double dataCenter[3];
	input->GetCenter(dataCenter);

	//camera = vtkSmartPointer<vtkCamera>::New();
	////camera->SetPosition(dataCenter[0] , dataCenter[1], dataCenter[2] - 66);
	//camera->SetPosition(dataCenter[0], dataCenter[1] + 600 , dataCenter[2] + 0.1);


	m.rotate(- 90, 1.0, 0.0, 0.0);
	palmDir = m * palmDir;
	thumbDir = m * thumbDir;
	fingerDir = m * fingerDir;

	cout<< "palmDir:\t"<<palmDir.x()<<",\t"<<palmDir.y()<<",\t"<<palmDir.z()<<endl;
	cout<< "thumbDir:\t"<<thumbDir.x()<<",\t"<<thumbDir.y()<<",\t"<<thumbDir.z()<<endl;
	cout<< "fingerDir:\t"<<fingerDir.x()<<",\t"<<fingerDir.y()<<",\t"<<fingerDir.z()<<endl;

	
	QVector3D xAxis, yAxis, zAxis;
	xAxis = palmDir;
	yAxis =  - fingerDir;
	zAxis = - thumbDir;
	cout<< "***"<<endl;
	cout<< "xAxis:\t"<<xAxis.x()<<",\t"<<xAxis.y()<<",\t"<<xAxis.z()<<endl;
	cout<< "yAxis:\t"<<yAxis.x()<<",\t"<<yAxis.y()<<",\t"<<yAxis.z()<<endl;
	cout<< "zAxis:\t"<<zAxis.x()<<",\t"<<zAxis.y()<<",\t"<<zAxis.z()<<endl;


	double p[3];
	QVector3D cameraPos(dataCenter[0], dataCenter[1] + 600 , dataCenter[2] + 0.1);
	cameraPos[0] = QVector3D::dotProduct(cameraPos, xAxis);
	cameraPos[1] = QVector3D::dotProduct(cameraPos, yAxis);
	cameraPos[2] = QVector3D::dotProduct(cameraPos, zAxis);
	
	//camera->GetPosition(p);
	////QVector3D vp(p[0], p[1], p[2]);
	//camera->GetFocalPoint(p) ;
	//QVector3D cameraFocalPoint(p[0], p[1], p[2]);
	////vp[0] = QVector3D::dotProduct(vp, xDir);
	////vp[1] = QVector3D::dotProduct(vp, xDir);
	////vp[2] = QVector3D::dotProduct(vp, xDir);
	//
	//QVector3D cameraDir = - zDir;
	//QVector3D cameraPos = cameraFocalPoint - 512 * cameraDir;


	camera->SetPosition(cameraPos.x(), cameraPos.y(), cameraPos.z());
	QVector3D viewUp(0,0,-1);
	viewUp[0] = QVector3D::dotProduct(viewUp, xAxis);
	viewUp[1] = QVector3D::dotProduct(viewUp, yAxis);
	viewUp[2] = QVector3D::dotProduct(viewUp, zAxis);
	

	camera->SetViewUp(viewUp.x(), viewUp.y(), viewUp.z());
	this->ui->view1->repaint();

}
