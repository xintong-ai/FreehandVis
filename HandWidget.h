#ifndef HAND_WIDGET_H
#define HAND_WIDGET_H

#include "qmatrix4x4.h"

class HandWidget

{
public:
	HandWidget
();
	~HandWidget
();
	void SetHandOrientation(QVector3D xDir, QVector3D yDir, QVector3D zDir);
	void SetHandLocation(QVector3D v);
	void SetDataSize(float x, float y, float z);

	QMatrix4x4 GetDataTransformation();
	QMatrix4x4 GetCubeTransformation();

	QVector3D LeapCoords2DataCoords(QVector3D v);

	void UpdateTransformation();
	QVector3D LeapVec2DataVec(QVector3D v);
	float GetCubeSize();
	void GetDataCenter(float v[3]);
	float GetMaxDataSize();
	float GetLeapDataHeight();	// the height of the data center
	float GetLeapDataSize();
	QMatrix4x4 GetDataOrientation();
	float GetScaleFactor();
	void ToggleHandAttachedToCube();
	int GetSnappedPlane();
	QVector3D GetSnappingPoint();
	QMatrix4x4 GetSnappingRotation();
	QMatrix4x4 GetCubeOrientation();
	QMatrix4x4 GetHandOrientation();
	QMatrix4x4 GetTmpOrientation();
	QMatrix4x4 GetAdjustedCubeOrientation();
	QMatrix4x4 GetAdjustedHandOrientation();

	void GetSnappedPlaneCoords(QVector3D &origin, QVector3D &point1, QVector3D &point2);


private:
	QMatrix4x4 _handOrientation;
	QMatrix4x4 _dataOrientation;// the 'm'
	QVector3D _handLeapLocation;
	QVector3D _handDataLocation;
	float _dataSize[3];
	//float _maxDataSize;

	QMatrix4x4 _dataTransformation;
	QMatrix4x4 _cubeTransformation;
	QMatrix4x4 _cubeOrientation;

	float leapDataHeight;	// the height of the data center
	float leapDataSize;

	bool _handAttachedToCube;
	int _snappedPlane;
	int _snappedOrientation;



	//float _cubeSize;

};

#endif