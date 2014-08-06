#include "HandWidget.h"
#include <iostream>

HandWidget
	::HandWidget
	()
{
	_dataOrientation.setToIdentity();//.rotate(-90, 1.0, 0.0, 0.0);
	leapDataHeight = 75;	// the height of the data bottom
	leapDataSize = 150;
	//	_handAttachedToCube = false;
	//	_cubeSize = 0;
	_cubeScale = 1;
	_dataScale = 1;
	_initialCubeDataRatio = 1.2;
	_mode = INTERACT_MODE::DETACH;
	_snapped = false;
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
	return _leap2DataTransformation.map(v);
}

QVector<QVector3D> HandWidget::LeapCoords2DataCoords(QVector<QVector3D> v)
{
	QVector<QVector3D> ret;
	for(int i = 0; i < v.size(); i++)
		ret.push_back(LeapCoords2DataCoords(v[i]));
	return ret;
}

QVector<QVector<QVector3D>> HandWidget::LeapCoords2DataCoords(QVector<QVector<QVector3D>> v)
{
	QVector<QVector<QVector3D>> ret;
	for(int i = 0; i < v.size(); i++)
		ret.push_back(LeapCoords2DataCoords(v[i]));
	return ret;
}

void HandWidget::SetDataSize(float x, float y, float z)
{
	_dataSize.setX(x);
	_dataSize.setY(y);
	_dataSize.setZ(z);
	float maxDataSize = GetMaxDataSize();

	_cubeTranslation = QVector3D(x * 0.5, y * 0.5, z * 0.5);
	_dataTranslation = _cubeTranslation;

	float scaleFactor = GetScaleFactor();

	_leap2DataTransformation.setToIdentity();
	_leap2DataTransformation.translate(_dataSize * 0.5);
	_leap2DataTransformation.translate(0, - (GetLeapDataHeight() * scaleFactor + GetOriginalCubeSize() * 0.5), 0);
	_leap2DataTransformation.scale(scaleFactor);
	_leap2DataTransformation = _leap2DataTransformation * _dataOrientation;
}

QVector3D HandWidget::GetDataSize()
{
	return _dataSize;
}


QMatrix4x4 HandWidget::GetLeap2DataTransformation()
{
	return _leap2DataTransformation;
}


float HandWidget::GetScaleFactor()
{
	return GetMaxDataSize() / GetLeapDataSize();
}

QVector3D HandWidget::GetSnappingPoint()
{
	QVector3D ret;
	float cubeSize = GetCubeSize();
	//std::cout<<"_snappedPlane:\t"<<_snappedPlane<<std::endl;
	switch(_snappedPlane)
	{
	case 0:
		ret = QVector3D(- 1, 0, 0);
		break;
	case 1:
		ret = QVector3D(1, 0, 0);
		break;
	case 2:
		ret = QVector3D(0, - 1, 0);
		break;
	case 3:
		ret = QVector3D(0, 1, 0);
		break;
	case 4:
		ret = QVector3D(0, 0, - 1);
		break;
	case 5:
		ret = QVector3D(0, 0, 1);
		break;
	}
	QVector3D dataCenter = GetOriginalDataCenter();
	ret = ret * 0.5 * cubeSize + dataCenter;
	return _cubeTransformation.map(ret);
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
	switch(_mode)
	{
	case INTERACT_MODE::HAND_SNAP:
		{	
			_cubeDataOrientation = _handOrientation * GetSnappingRotation();
			QVector3D palmDir = _handOrientation.column(0).toVector3D().normalized();

			_cubeTranslation = palmDir * GetCubeSize() * 0.5;
			_cubeTranslation += _handLocation;


			_dataTranslation = _cubeTranslation + _cubeDataOrientation.map(_dataTranslationRelativeInCubeSpace);
			break;
		}
	case INTERACT_MODE::CUBE_DATA_TRANSLATE:
		{
			QVector3D dir = GetAdjustedHandOrientation().column(0).toVector3D().normalized();
			_cubeTranslation = dir * GetCubeSize() * 0.5;
			_cubeTranslation += _handLocation;
			_dataTranslation = _cubeTranslation + _dataTranslationRelative;
			break;
		}
	case INTERACT_MODE::CUBE_DATA_SCALE:
		{
			_cubeScale = abs(QVector3D::dotProduct(_handLocation - GetCubeCenter(), QVector3D(GetAdjustedHandOrientation().column(0)).normalized()))
				/ (GetOriginalCubeSize() * 0.5);
			_dataScale = _dataScaleRelative * _cubeScale;
			_dataTranslation = _cubeTranslation + _cubeScale * _cubeDataOrientation.map(_dataTranslationRelativeInCubeSpace);
			break;
		}
	case INTERACT_MODE::CUBE_TRANSLATE:
		{
			QVector3D dir = GetAdjustedHandOrientation().column(0).toVector3D().normalized();
			_cubeTranslation = dir * GetCubeSize() * 0.5;
			_cubeTranslation += _handLocation;
			//_dataTranslation = _cubeTranslation + _dataTranslationRelative;
			break;
		}
	case INTERACT_MODE::CUBE_SCALE:
		{
			_cubeScale = abs(QVector3D::dotProduct(_handLocation - GetCubeCenter(), QVector3D(GetAdjustedHandOrientation().column(0)).normalized()))
				/ (GetOriginalCubeSize() * 0.5);
			//_dataScale = _dataScaleRelative * _cubeScale;
			//_dataTranslation = _cubeTranslation + _cubeScale * _cubeDataOrientation.map(_dataTranslationRelativeInCubeSpace);
			break;
		}
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
	_dataTransformation.translate(-_dataSize.x() * 0.5, -_dataSize.y() * 0.5, -_dataSize.z() * 0.5);
	_cubeTransformation.translate(-_dataSize.x() * 0.5, -_dataSize.y() * 0.5, -_dataSize.z() * 0.5);

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
	v[0] = _dataTranslation.x();
	v[1] = _dataTranslation.y();
	v[2] = _dataTranslation.z();
}

QVector3D HandWidget::GetDataCenter()
{
	return _dataTranslation;
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
	return std::max(_dataSize.x(), std::max(_dataSize.y(), _dataSize.z()));
}

float HandWidget::GetMinDataSize()
{
	return std::min(_dataSize.x(), std::min(_dataSize.y(), _dataSize.z()));
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
	if(INTERACT_MODE::DETACH != _mode && INTERACT_MODE::CUBE_DATA_SCALE != _mode && DETACH_NORMALIZE != _mode )
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

	QVector3D leftHandCenter = GetLeftHandCenter();
	float cubeSize = GetCubeSize();
	float dist = GetSnappingPoint().distanceToPoint(leftHandCenter);
	_snapped = dist < (cubeSize * 0.3);
	//if(_snapped)
	//	std::cout<<"_snapped:\t"<<int(_snapped)<<std::endl;

	return iMax * 4 + jMax;
}

void HandWidget::SetInteractMode(INTERACT_MODE m)
{
	_dataScaleRelative = _dataScale / _cubeScale;
	_dataTranslationRelative = _dataTranslation - _cubeTranslation;
	_dataTranslationRelativeInCubeSpace = _cubeDataOrientation.inverted().map(_dataTranslationRelative);
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

void HandWidget::ResetCube()
{
	//_dataScale = _dataScale / _cubeScale;
	_cubeScale = 1;

	//_dataTranslation = _dataTranslation + GetOriginalDataCenter() - _cubeTranslation;
	_cubeTranslation = GetOriginalDataCenter();
}

void HandWidget::ResetCubeData()
{
	float scaleChanged = 1.0 / _cubeScale;
	_dataScale = _dataScale / _cubeScale;
	_cubeScale = 1;

	//_dataTranslation = _dataTranslation + GetOriginalDataCenter() - _cubeTranslation;
	_cubeTranslation = GetOriginalDataCenter();
	_dataTranslation = _cubeTranslation + scaleChanged * _cubeDataOrientation.map(_dataTranslationRelativeInCubeSpace);
}



void HandWidget::SetHands(QVector<QVector<QVector3D > > leftFingers, QVector<QVector3D > leftPalm)
{
	_leftFingers = LeapCoords2DataCoords(leftFingers);
	_leftPalm = LeapCoords2DataCoords(leftPalm);
}

QVector3D HandWidget::GetLeftHandCenter()
{
	if(0 == _leftFingers.size() )
		return QVector3D(0, 0, 0);
	return _leftFingers[2][0];
}

bool HandWidget::GetSnappingPlaneStatus()
{
	return _snapped;
}

void HandWidget::SetRightHand(QVector3D thumbTip, QVector3D indexTip, QVector3D indexDir)
{
	_rightThumbTip = LeapCoords2DataCoords(thumbTip);
	_rightIndexTip = LeapCoords2DataCoords(indexTip);
	_rightIndexDir = LeapCoords2DataCoords(indexDir);
}

QVector3D HandWidget::CameraCoords2DataCoords(QVector3D v)
{
	return _dataTransformation.inverted().map(v);
}

void HandWidget::GetRightHandTwoTips(QVector3D &tip1, QVector3D &tip2)
{
	tip1 = _rightThumbTip;
	tip2 = _rightIndexTip;
}

QVector3D HandWidget::GetRightIndexTip()
{
	return _rightIndexTip;
}
