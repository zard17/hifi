//
//  ViewFrustum.cpp
//  hifi
//
//  Created by Brad Hefta-Gaub on 04/11/13.
//
//  Simple view frustum class.
//
//

#include "ViewFrustum.h"
#include "voxels_Log.h"

using voxels_lib::printLog;

ViewFrustum::ViewFrustum() :
    _position(glm::vec3(0,0,0)),
    _direction(glm::vec3(0,0,0)),
    _up(glm::vec3(0,0,0)),
    _right(glm::vec3(0,0,0)),
    _fieldOfView(0.0),
    _aspectRatio(1.0),
    _nearClip(0.1),
    _farClip(500.0),
    _nearHeight(0.0),
    _nearWidth(0.0),
    _farHeight(0.0),
    _farWidth(0.0),
    _farCenter(glm::vec3(0,0,0)),
    _farTopLeft(glm::vec3(0,0,0)),
    _farTopRight(glm::vec3(0,0,0)),
    _farBottomLeft(glm::vec3(0,0,0)),
    _farBottomRight(glm::vec3(0,0,0)),
    _nearCenter(glm::vec3(0,0,0)),
    _nearTopLeft(glm::vec3(0,0,0)),
    _nearTopRight(glm::vec3(0,0,0)),
    _nearBottomLeft(glm::vec3(0,0,0)),
    _nearBottomRight(glm::vec3(0,0,0)) { }

/////////////////////////////////////////////////////////////////////////////////////
// ViewFrustum::calculateViewFrustum()
//
// Description: this will calculate the view frustum bounds for a given position
// 				and direction
//
// Notes on how/why this works: 
//     http://www.lighthouse3d.com/tutorials/view-frustum-culling/view-frustums-shape/
//
void ViewFrustum::calculate() {

    static const double PI_OVER_180			= 3.14159265359 / 180.0; // would be better if this was in a shared location
	
	glm::vec3 front    = _direction;
	
	// Calculating field of view.
	float fovInRadians = this->_fieldOfView * PI_OVER_180;
	
	float twoTimesTanHalfFOV = 2.0f * tan(fovInRadians/2.0f);

    // Do we need this?
	//tang = (float)tan(ANG2RAD * angle * 0.5) ;

    float nearClip = this->_nearClip;
    float farClip  = this->_farClip;
	
	this->_nearHeight = (twoTimesTanHalfFOV * nearClip);
	this->_nearWidth  = this->_nearHeight * this->_aspectRatio;
	this->_farHeight  = (twoTimesTanHalfFOV * farClip);
	this->_farWidth   = this->_farHeight * this->_aspectRatio;

	float farHalfHeight    = (this->_farHeight * 0.5f);
	float farHalfWidth     = (this->_farWidth  * 0.5f);
	this->_farCenter       = this->_position+front * farClip;
	this->_farTopLeft      = this->_farCenter  + (this->_up * farHalfHeight)  - (this->_right * farHalfWidth); 
	this->_farTopRight     = this->_farCenter  + (this->_up * farHalfHeight)  + (this->_right * farHalfWidth); 
	this->_farBottomLeft   = this->_farCenter  - (this->_up * farHalfHeight)  - (this->_right * farHalfWidth); 
	this->_farBottomRight  = this->_farCenter  - (this->_up * farHalfHeight)  + (this->_right * farHalfWidth); 

	float nearHalfHeight   = (this->_nearHeight * 0.5f);
	float nearHalfWidth    = (this->_nearWidth  * 0.5f);
	this->_nearCenter      = this->_position+front * nearClip;
	this->_nearTopLeft     = this->_nearCenter + (this->_up * nearHalfHeight) - (this->_right * nearHalfWidth); 
	this->_nearTopRight    = this->_nearCenter + (this->_up * nearHalfHeight) + (this->_right * nearHalfWidth); 
	this->_nearBottomLeft  = this->_nearCenter - (this->_up * nearHalfHeight) - (this->_right * nearHalfWidth); 
	this->_nearBottomRight = this->_nearCenter - (this->_up * nearHalfHeight) + (this->_right * nearHalfWidth); 

	// compute the six planes
	// the function set3Points assumes that the points
	// are given in counter clockwise order
	this->_planes[TOPP].set3Points(this->_nearTopRight,this->_nearTopLeft,this->_farTopLeft);
	this->_planes[BOTTOMP].set3Points(this->_nearBottomLeft,this->_nearBottomRight,this->_farBottomRight);
	this->_planes[LEFTP].set3Points(this->_nearTopLeft,this->_nearBottomLeft,this->_farBottomLeft);
	this->_planes[RIGHTP].set3Points(this->_nearBottomRight,this->_nearTopRight,this->_farBottomRight);
	this->_planes[NEARP].set3Points(this->_nearTopLeft,this->_nearTopRight,this->_nearBottomRight);
	this->_planes[FARP].set3Points(this->_farTopRight,this->_farTopLeft,this->_farBottomLeft);

}

void ViewFrustum::dump() {

    printLog("position.x=%f, position.y=%f, position.z=%f\n", this->_position.x, this->_position.y, this->_position.z);
    printLog("direction.x=%f, direction.y=%f, direction.z=%f\n", this->_direction.x, this->_direction.y, this->_direction.z);
    printLog("up.x=%f, up.y=%f, up.z=%f\n", this->_up.x, this->_up.y, this->_up.z);
    printLog("right.x=%f, right.y=%f, right.z=%f\n", this->_right.x, this->_right.y, this->_right.z);

    printLog("farDist=%f\n", this->_farClip);
    printLog("farHeight=%f\n", this->_farHeight);
    printLog("farWidth=%f\n", this->_farWidth);

    printLog("nearDist=%f\n", this->_nearClip);
    printLog("nearHeight=%f\n", this->_nearHeight);
    printLog("nearWidth=%f\n", this->_nearWidth);

    printLog("farCenter.x=%f,      farCenter.y=%f,      farCenter.z=%f\n",
    	this->_farCenter.x, this->_farCenter.y, this->_farCenter.z);
    printLog("farTopLeft.x=%f,     farTopLeft.y=%f,     farTopLeft.z=%f\n",
    	this->_farTopLeft.x, this->_farTopLeft.y, this->_farTopLeft.z);
    printLog("farTopRight.x=%f,    farTopRight.y=%f,    farTopRight.z=%f\n",
    	this->_farTopRight.x, this->_farTopRight.y, this->_farTopRight.z);
    printLog("farBottomLeft.x=%f,  farBottomLeft.y=%f,  farBottomLeft.z=%f\n",
    	this->_farBottomLeft.x, this->_farBottomLeft.y, this->_farBottomLeft.z);
    printLog("farBottomRight.x=%f, farBottomRight.y=%f, farBottomRight.z=%f\n",
    	this->_farBottomRight.x, this->_farBottomRight.y, this->_farBottomRight.z);

    printLog("nearCenter.x=%f,      nearCenter.y=%f,      nearCenter.z=%f\n",
    	this->_nearCenter.x, this->_nearCenter.y, this->_nearCenter.z);
    printLog("nearTopLeft.x=%f,     nearTopLeft.y=%f,     nearTopLeft.z=%f\n",
    	this->_nearTopLeft.x, this->_nearTopLeft.y, this->_nearTopLeft.z);
    printLog("nearTopRight.x=%f,    nearTopRight.y=%f,    nearTopRight.z=%f\n",
    	this->_nearTopRight.x, this->_nearTopRight.y, this->_nearTopRight.z);
    printLog("nearBottomLeft.x=%f,  nearBottomLeft.y=%f,  nearBottomLeft.z=%f\n",
    	this->_nearBottomLeft.x, this->_nearBottomLeft.y, this->_nearBottomLeft.z);
    printLog("nearBottomRight.x=%f, nearBottomRight.y=%f, nearBottomRight.z=%f\n",
    	this->_nearBottomRight.x, this->_nearBottomRight.y, this->_nearBottomRight.z);
}


int ViewFrustum::pointInFrustum(glm::vec3 &p) {
	int result = INSIDE;
	for(int i=0; i < 6; i++) {
		if (this->_planes[i].distance(p) < 0)
			return OUTSIDE;
	}
	return(result);
}

int ViewFrustum::sphereInFrustum(glm::vec3 &center, float radius) {
	int result = INSIDE;
	float distance;
	for(int i=0; i < 6; i++) {
		distance = this->_planes[i].distance(center);
		if (distance < -radius)
			return OUTSIDE;
		else if (distance < radius)
			result =  INTERSECT;
	}
	return(result);
}


int ViewFrustum::boxInFrustum(AABox &b) {
	int result = INSIDE;
	for(int i=0; i < 6; i++) {
		if (this->_planes[i].distance(b.getVertexP(this->_planes[i].normal)) < 0)
			return OUTSIDE;
		else if (this->_planes[i].distance(b.getVertexN(this->_planes[i].normal)) < 0)
			result =  INTERSECT;
	}
	return(result);
 }
