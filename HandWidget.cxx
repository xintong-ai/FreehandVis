#include "HandWidget.h"
#include <iostream>

HandWidget
	::HandWidget
	()
{
	_dataOrientation.rotate(-90, 1.0, 0.0, 0.0);
	leapDataHeight = 200;	// the height of the data center
	leapDataSize = 150;
	//	_handAttachedToCube = false;
	//	_cubeSize = 0;
	_cubeScale = 1;
	_dataScale = 1;
	_initialCubeDataRatio = 1.2;
	_mode = INTERACT_MODE::DETACH;
}

HandWidget
	::~HandWidget
	()
{

}



void HandWidget::SetHandOrientation(QVector3D xDir, QVector3D yDir, QVector3D zDir)
{
	_handOrientation.setToIdentity();
	_handOrientation.setColumn(0, QVector4D(xDir));
	_handOrientation.setColumn(1, QVector4D(yDir));
	_handOrientation.setColumn(2, QVector4D(zDir));
	_handOrientation = _dataOrientation * _handOrientation;
	//std::cout<< "_handOrientation:\t"<<_handOrientation.column(0).x()<<",\t"
	//	<<_handOrientation.column(0).y()<<",\t"<<_handOrientation.column(0).z()<<std::endl;
	//	_handOrientation = _dataOrientation * _handOrientation.inverted() * _dataOrientation.inverted();
}

void HandWidget::SetHandLocation(QVector3D v)
{
	_handLocation = LeapCoords2DataCoords(v);
}

QVector3D HandWidget::LeapCoords2DataCoords(QVector3D v)
{
	v = _dataOrientation * v;
	v += QVector3D(0, 0, GetLeapDataHeight());
	float scaleFactor = GetMaxDataSize() / GetLeapDataSize();

	v = v * scaleFactor;
	v += QVector3D(_dataSize[0] * 0.5, _dataSize[1] * 0.5, _dataSize[2] * 0.5);
	return v;
}

void HandWidget::SetDataSize(float x, float y, float z)
{
	_dataSize[0] = x;
	_dataSize[1] = y;
	_dataSize[2] = z;
	float maxDataSize = GetMaxDataSize();

	_cubeTranslation = QVector3D(x * 0.5, y * 0.5, z * 0.5);
	_dataTranslation = _cubeTranslation;
}

float HandWidget::GetScaleFactor()
{
	return GetMaxDataSize() / GetLeapDataSize();
}

QVector3D HandWidget::GetSnappingPoint()
{
	QVector3D ret;
	float cubeSize = GetCubeSize();
	switch(_snappedPlane)
	{
	case 0:
		ret = QVector3D(cubeSize * 0.5, 0, 0);
		break;
	case 1:
		ret = QVector3D(- cubeSize * 0.5, 0, 0);
		break;
	case 2:
		ret = QVector3D(0, cubeSize * 0.5, 0);
		break;
	case 3:
		ret = QVector3D(0, - cubeSize * 0.5, 0);
		break;
	case 4:
		ret = QVector3D(0, 0, cubeSize * 0.5);
		break;
	case 5:
		ret = QVector3D(0, 0, - cubeSize * 0.5);
		break;
	}
	return ret;
}

QMatrix4x4 HandWidget::GetSnappingRotation()
{
	QMatrix4x4 rotation;
	switch(_snappedOrientation)
	{
	case 0:
		break;
	case 1:
		rotation.rotate(90, -1, 0, 0);
		break;
	case 2:
		rotation.rotate(180, -1, 0, 0);
		break;
	case 3:
		rotation.rotate(270, -1, 0, 0);
		break;
	}
	switch(_snappedPlane)
	{
	case 0:
		break;
	case 1:
		rotation.rotate(-180, 0, 1, 0);
		break;
	case 2:
		rotation.rotate(-90, 0, 0, 1);
		break;
	case 3:
		rotation.rotate(90, 0, 0, 1);
		break;
	case 4:
		rotation.rotate(90, 0, 1, 0);
		break;
	case 5:
		rotation.rotate(-90, 0, 1, 0);
	}

	return rotation;
}

void HandWidget::UpdateTransformation()
{
	if(INTERACT_MODE::HAND_SNAP == _mode)
	{	
		_cubeDataOrientation = _handOrientation * GetSnappingRotation();
		QVector3D palmDir = _handOrientation.column(0).toVector3D().normalized();

		_cubeTranslation = palmDir * GetCubeSize() * 0.5;
		_cubeTranslation += _handLocation;

		_dataTranslation = _cubeTranslation;
	}
	else if(INTERACT_MODE::CUBE_TRANSLATE == _mode)
	{
		QVector3D dir = GetAdjustedHandOrientation().column(0).toVector3D().normalized();
		_cubeTranslation = dir * GetCubeSize() * 0.5;
		_cubeTranslation += _handLocation;
		_dataTranslation = _cubeTranslation + _dataTranslationRelative;
	}
	else if(INTERACT_MODE::CUBE_SCALE == _mode)
	{
		//QVector3D aa = GetCubeCenter();
		//QVector4D bb = GetAdjustedHandOrientation().column(0).normalized();
		//float cc = GetOriginalCubeSize();
		//float dd =  abs(QVector3D::dotProduct(_handLocation - GetCubeCenter(), QVector3D(GetAdjustedHandOrientation().column(0)).normalized()));

		_cubeScale = abs(QVector3D::dotProduct(_handLocation - GetCubeCenter(), QVector3D(GetAdjustedHandOrientation().column(0)).normalized()))
			/ (GetOriginalCubeSize() * 0.5);


		_dataScale = _dataScaleRelative * _cubeScale;

		//_dataTranslation = _dataTranslation + GetOriginalDataCenter() - _cubeTranslation;
		//_cubeTranslation = GetOriginalDataCenter();
	}

	_dataTransformation.setToIdentity();
	_cubeTransformation.setToIdentity();

	//translating
	_dataTransformation.translate(_dataTranslation);
	_cubeTransformation.translate(_cubeTranslation);

	//rotating: the cube and data share the same rotation
	_dataTransformation = _dataTransformation * _cubeDataOrientation;
	_cubeTransformation = _cubeTransformation * _cubeDataOrientation;

	_dataTransformation.scale(_dataScale);
	_cubeTransformation.scale(_cubeScale);

	//set the data center to the origin
	_dataTransformation.translate(-_dataSize[0] * 0.5, -_dataSize[1] * 0.5, -_dataSize[2] * 0.5);
	_cubeTransformation.translate(-_dataSize[0] * 0.5, -_dataSize[1] * 0.5, -_dataSize[2] * 0.5);

}

QMatrix4x4 HandWidget::GetCubeDataOrientation()
{
	return _cubeDataOrientation;
}

QMatrix4x4 HandWidget::GetHandOrientation()
{
	return _handOrientation;
}

QMatrix4x4 HandWidget::GetTmpOrientation()
{
	return GetSnappingRotation() * _handOrientation;
}

QMatrix4x4 HandWidget::GetAdjustedCubeOrientation()
{
	return GetSnappingRotation().inverted() * _cubeDataOrientation;
}

QMatrix4x4 HandWidget::GetAdjustedHandOrientation()	//this is not correct
{
	return _cubeDataOrientation * GetSnappingRotation().inverted();
}

QMatrix4x4 HandWidget::GetDataOrientation()
{
	return _dataOrientation;
}
QMatrix4x4 HandWidget::GetCubeTransformation()
{
	return _cubeTransformation;
}

QMatrix4x4 HandWidget::GetSnappedHandTransformation()
{
	return QMatrix4x4();
}

QVector3D HandWidget::LeapVec2DataVec(QVector3D v)
{
	return _dataOrientation * v * GetScaleFactor();
}

QMatrix4x4 HandWidget::GetDataTransformation()
{
	return _dataTransformation;
}

float HandWidget::GetCubeSize()
{
	return _cubeScale * GetMaxDataSize() * _initialCubeDataRatio;
}

float HandWidget::GetOriginalCubeSize()
{
	return GetMaxDataSize() * _initialCubeDataRatio;
}

void HandWidget::GetOrignalCubeCoords(QVector3D &min, QVector3D &max)
{
	float cubeSize = GetCubeSize();
	QVector3D halfCubeSize(cubeSize * 0.5, cubeSize * 0.5, cubeSize * 0.5);
	min = GetOriginalDataCenter() - halfCubeSize;
	max = GetOriginalDataCenter() + halfCubeSize;
}

// data center is the same as data translation
void HandWidget::GetDataCenter(float v[3])
{
	v[0] = _dataTranslation[0];
	v[1] = _dataTranslation[1];
	v[2] = _dataTranslation[2];
}

QVector3D HandWidget::GetOriginalDataCenter()
{
	return _dataSize * 0.5;
}

QVector3D HandWidget::GetCubeCenter()
{
	return _cubeTranslation;
}

float HandWidget::GetMaxDataSize()
{
	return std::max(_dataSize[0], std::max(_dataSize[1], _dataSize[2]));
}

float HandWidget::GetLeapDataHeight()
{
	return leapDataHeight;
}

float HandWidget::GetLeapDataSize()
{
	return leapDataSize;
}

//void HandWidget::ToggleHandAttachedToCube()
//{
//	_handAttachedToCube = !_handAttachedToCube;
//}

inline float Magnitude(QMatrix4x4 m)
{
	float ret = 0;
	for(int i = 0; i < 16; i++)	{
		ret += pow(m.constData()[i], 2);
	}
	return ret;
}

int HandWidget::GetSnappedPlane()
{
	if(INTERACT_MODE::DETACH != _mode && INTERACT_MODE::CUBE_SCALE != _mode )
		return 0;
	QMatrix4x4 orie[24];
	QVector3D xDirCube = QVector3D(_cubeDataOrientation.column(0));
	QVector3D yDirCube = QVector3D(_cubeDataOrientation.column(1));
	QVector3D zDirCube = QVector3D(_cubeDataOrientation.column(2));

	QVector3D xDirHand = QVector3D(_handOrientation.column(0));
	QVector3D yDirHand = QVector3D(_handOrientation.column(1));
	QVector3D zDirHand = QVector3D(_handOrientation.column(2));

	float iDiff[6];
	iDiff[0] = QVector3D::dotProduct(xDirHand, xDirCube);
	iDiff[1] = QVector3D::dotProduct(xDirHand,-xDirCube);
	iDiff[2] = QVector3D::dotProduct(xDirHand, yDirCube);
	iDiff[3] = QVector3D::dotProduct(xDirHand,-yDirCube);
	iDiff[4] = QVector3D::dotProduct(xDirHand, zDirCube);
	iDiff[5] = QVector3D::dotProduct(xDirHand,-zDirCube);

	//QMatrix4x4 rot, rot2;
	//float diff[24];
	//for(int i = 0; i < 6; i++)	{
	//	rot2.setToIdentity();
	//	switch(i)	{
	//	case 0:
	//		break;
	//	case 1:
	//		rot2.rotate(180, QVector3D(zDir));
	//		break;
	//	case 2:
	//		rot2.rotate(-90, QVector3D(yDir));
	//		break;
	//	case 3:
	//		rot2.rotate(90, QVector3D(yDir));
	//		break;
	//	case 4:
	//		rot2.rotate(-90, QVector3D(zDir));
	//		break;
	//	case 5:
	//		rot2.rotate(90, QVector3D(zDir));
	//		break;
	//	}
	//	for(int j = 0; j < 4; j++)	{
	//		rot.setToIdentity();
	//		rot.rotate(90 * j, QVector3D(xDir));
	//		orie[i * 4 + j] = rot * rot2 * _cubeOrientation;
	//		//QQuaternion quat;
	//		//orie[0][0].rotate(q
	//		diff[i * 4 + j] = Magnitude(orie[i * 4 + j] - _handOrientation);
	//	}
	//}
	int iMax;
	float max = - FLT_MAX;
	for(int i = 0; i < 6; i++)	{
		if(iDiff[i] > max)	{
			max = iDiff[i];
			iMax = i;
		}
	}
	QMatrix4x4 rotatedCube;
	QMatrix4x4 rotation;
	switch(iMax)	{
	case 0:
		break;
	case 1:
		rotation.rotate(180, yDirCube);
		break;
	case 2:
		rotation.rotate(90, zDirCube);
		break;
	case 3:
		rotation.rotate(-90, zDirCube);
		break;
	case 4:
		rotation.rotate(-90, yDirCube);
		break;
	case 5:
		rotation.rotate(90, yDirCube);
		break;
	}
	rotatedCube = rotation * _cubeDataOrientation;

	//QVector3D xDirCubeRotated = QVector3D(rotatedCube.column(0));
	QVector3D yDirCubeRotated = QVector3D(rotatedCube.column(1));
	QVector3D zDirCubeRotated = QVector3D(rotatedCube.column(2));

	float jDiff[4];
	jDiff[0] = QVector3D::dotProduct(yDirHand, yDirCubeRotated);
	jDiff[1] = QVector3D::dotProduct(yDirHand, zDirCubeRotated);
	jDiff[2] = QVector3D::dotProduct(yDirHand,-yDirCubeRotated);
	jDiff[3] = QVector3D::dotProduct(yDirHand,-zDirCubeRotated);

	int jMax;
	max = - FLT_MAX;
	for(int j = 0; j < 4; j++)	{
		if(jDiff[j] > max)	{
			max = jDiff[j];
			jMax = j;
		}
	}
	_snappedPlane = iMax;
	_snappedOrientation = jMax;
	//std::cout<<"_snappedPlane:\t"<<_snappedPlane<<std::endl;
	//std::cout<<"_snappedOrientation:\t"<<_snappedOrientation<<std::endl;
	return iMax * 4 + jMax;
}

void HandWidget::SetInteractMode(INTERACT_MODE m)
{
	if(INTERACT_MODE::CUBE_SCALE == m)
	{
		_dataScaleRelative = _dataScale / _cubeScale;
	}
	else if(INTERACT_MODE::CUBE_TRANSLATE == m)
	{
		_dataTranslationRelative = _dataTranslation - _cubeTranslation;
	}
	_mode = m;
}

void HandWidget::GetSnappedPlaneCoords(QVector3D &origin, QVector3D &point1, QVector3D &point2)
{
	switch(_snappedPlane)	{
	case 0:
		origin = QVector3D(0, 0, 0);
		point1 = QVector3D(0, 1, 0);
		point2 = QVector3D(0, 0, 1);
		break;
	case 1:
		origin = QVector3D(1, 0, 0);
		point1 = QVector3D(1, 0, 1);
		point2 = QVector3D(1, 1, 0);
		break;
	case 2:
		origin = QVector3D(0, 0, 0);
		point1 = QVector3D(1, 0, 0);
		point2 = QVector3D(0, 0, 1);
		break;
	case 3:
		origin = QVector3D(0, 1, 0);
		point1 = QVector3D(1, 1, 0);
		point2 = QVector3D(0, 1, 1);
		break;
	case 4:
		origin = QVector3D(0, 0, 0);
		point1 = QVector3D(1, 0, 0);
		point2 = QVector3D(0, 1, 0);
		break;
	case 5:
		origin = QVector3D(0, 0, 1);
		point1 = QVector3D(1, 0, 1);
		point2 = QVector3D(0, 1, 1);
		break;
	}
	QVector3D half(0.5, 0.5, 0.5);
	origin -= half;
	point1 -= half;
	point2 -= half;
	float cubeSize = GetOriginalCubeSize();
	QVector3D dataSizeQ(cubeSize, cubeSize, cubeSize);
	QVector3D dataCenter = GetOriginalDataCenter();

	origin = origin * dataSizeQ + dataCenter;
	point1 = point1 * dataSizeQ + dataCenter;
	point2 = point2 * dataSizeQ + dataCenter;
}

void HandWidget::SetSphereRadius(float v)
{
	_sphereRadius = v * GetScaleFactor();
}

void HandWidget::ResetTranslationScale()
{
	//_dataScale = _dataScale / _cubeScale;
	_cubeScale = 1;

	//_dataTranslation = _dataTranslation + GetOriginalDataCenter() - _cubeTranslation;
	_cubeTranslation = GetOriginalDataCenter();
}
