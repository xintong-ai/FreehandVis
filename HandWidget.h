#ifndef HAND_WIDGET_H
#define HAND_WIDGET_H

#include "qmatrix4x4.h"

enum INTERACT_MODE
{
	DETACH,
	DETACH_NORMALIZE,
	HAND_SNAP,
	CUBE_DATA_TRANSLATE,
	CUBE_TRANSLATE,
	CUBE_DATA_SCALE,
	CUBE_SCALE,
};

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
	QVector3D GetDataSize();
	void SetInteractMode(INTERACT_MODE m);

	QMatrix4x4 GetDataTransformation();
	QMatrix4x4 GetCubeTransformation();
	QMatrix4x4 GetSnappedHandTransformation();

	QVector3D LeapCoords2DataCoords(QVector3D v);
	QVector<QVector3D> LeapCoords2DataCoords(QVector<QVector3D> v);
	QVector<QVector<QVector3D>> LeapCoords2DataCoords(QVector<QVector<QVector3D>> v);

	void UpdateTransformation();
	QVector3D LeapVec2DataVec(QVector3D v);
	float GetCubeSize();
	float GetOriginalCubeSize();
	void GetDataCenter(float v[3]);
	QVector3D GetDataCenter();
	QVector3D GetOriginalDataCenter();
	QVector3D GetCubeCenter();
	float GetMaxDataSize();
	float GetMinDataSize();
	float GetLeapDataHeight();	// the height of the data center
	float GetLeapDataSize();
	QMatrix4x4 GetDataOrientation();
	float GetScaleFactor();
	//void ToggleHandAttachedToCube();
	int GetSnappedPlane();
	QVector3D GetSnappingPoint();
	QMatrix4x4 GetSnappingRotation();
	QMatrix4x4 GetCubeDataOrientation();
	QMatrix4x4 GetHandOrientation();
	QMatrix4x4 GetTmpOrientation();
	QMatrix4x4 GetAdjustedCubeOrientation();
	QMatrix4x4 GetAdjustedHandOrientation();
	void ResetCube();
	void ResetCubeData();
	void SetSphereRadius(float v);

	void GetSnappedPlaneCoords(QVector3D &origin, QVector3D &point1, QVector3D &point2);

	void GetOrignalCubeCoords(QVector3D &min, QVector3D &max);

	QMatrix4x4 GetLeap2DataTransformation();

	void SetHands(QVector<QVector<QVector3D > > leftFingers, QVector<QVector3D > leftPalm);
	QVector3D GetLeftHandCenter();
	bool GetSnappingPlaneStatus();

	void SetRightHand(QVector3D thumbTip, QVector3D indexTip, QVector3D indexDir);
	void GetRightHandTwoTips(QVector3D &tip1, QVector3D &tip2);
	QVector3D GetRightIndexTip();

	QVector3D CameraCoords2DataCoords(QVector3D v);

private:
	QMatrix4x4 _handOrientation;
	QMatrix4x4 _dataOrientation;// the 'm'
	QVector3D _handLocation;
	QVector3D _dataSize;
	//float _maxDataSize;

	QMatrix4x4 _dataTransformation;
	QMatrix4x4 _cubeDataOrientation;
	QMatrix4x4 _leap2DataTransformation;

	QMatrix4x4 _ralativeDataTransformation;	//scaling and translation of data relative to cube
	QMatrix4x4 _cubeTransformation;

	float leapDataHeight;	// the height of the data center
	float leapDataSize;

	//bool _handAttachedToCube;
	int _snappedPlane;
	int _snappedOrientation;
	float _dataScale;
	float _dataScaleRelative;
	float _cubeScale;
	QVector3D _dataTranslationRelative;
	QVector3D _dataTranslationRelativeInCubeSpace;
	QVector3D _cubeTranslation;
	QVector3D _dataTranslation;
	float _sphereRadius;
	QVector3D _dataCenter;
	QVector3D _cubeCenter;

	INTERACT_MODE _mode;
	//float _cubeSize;
	float _initialCubeDataRatio;
	bool _snapped;

	QVector<QVector<QVector3D > > _leftFingers;
	QVector<QVector3D > _leftPalm;

	
	//right hand
	QVector3D _rightThumbTip;
	QVector3D _rightIndexTip;
	QVector3D _rightIndexDir;

};

#endif