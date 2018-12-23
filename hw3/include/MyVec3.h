#ifndef _MyVec3_H_
#define _MyVec3_H_
#endif // !_MyVec3_H_

class MyVec3 {
public:
	float x, y, z;
	float arr[3];

	MyVec3();
	MyVec3(const float &_x, const float &_y, const float &_z);
	MyVec3(const float _arr[]);

	float getX() { return x; }
	float getY() { return y; }
	float getZ() { return z; }

	void print();
	float* getArr();

	MyVec3 operator= (const MyVec3 &other);
	MyVec3 operator+ (const MyVec3 &other);
	MyVec3 operator+ (const float arr[]);
	MyVec3 operator- (const MyVec3 &other);
	MyVec3 operator- (const float arr[]);
	MyVec3 operator+= (const MyVec3 &other);
	MyVec3 operator-= (const MyVec3 &other);
};