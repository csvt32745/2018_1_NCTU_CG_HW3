#include <iostream>
#include <MyVec3.h>

MyVec3::MyVec3() {
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

MyVec3::MyVec3(const float &_x, const float &_y, const float&_z) {
	x = _x;
	y = _y;
	z = _z;
}

MyVec3::MyVec3(const float _arr[]) {
	x = _arr[0];
	y = _arr[1];
	z = _arr[2];
}


void MyVec3::print() {
	printf("%f, %f, %f\n", x, y, z);
}

float* MyVec3::getArr() {
	arr[0] = x;
	arr[1] = y;
	arr[2] = z;
	return arr;
}

MyVec3 MyVec3::operator= (const MyVec3 &other) {
	if (this != &other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	return *this;
}

MyVec3 MyVec3::operator+ (const MyVec3 &other) {
	MyVec3 tmp(x + other.x, y + other.y, z + other.z);
	return tmp;
}

MyVec3 MyVec3::operator+ (const float arr[]) {
	MyVec3 tmp(x + arr[0], y + arr[1], z + arr[2]);
	return tmp;
}

MyVec3 MyVec3::operator- (const MyVec3 &other) {
	MyVec3 tmp(x - other.x, y - other.y, z - other.z);
	return tmp;
}

MyVec3 MyVec3::operator- (const float arr[]) {
	MyVec3 tmp(x - arr[0], y - arr[1], z - arr[2]);
	return tmp;
}

MyVec3 MyVec3::operator+= (const MyVec3 &other) {
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

MyVec3 MyVec3::operator-= (const MyVec3 &other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}