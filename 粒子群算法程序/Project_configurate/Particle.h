#ifndef PARTICLE_H
#define PARTICLE_H
//宏定义一些常用的变量
#define NF_Max 650   //风力发电机数量
#define NG_Max 100   //光伏数量
#define NW_Max 500   //微燃机数量
#define AL_Max 100
#include "Coordinate.h"
#include <math.h>
#include <iostream>
using namespace std;
class Particle
{
	friend ostream& operator<<(ostream &output, const Particle &right);

public:
	Particle(float x, float y, float z);
	void setP();
	float getP()const;

	//void setPBest();  //pBest的设置在setP()中就完成了。
	Coordinate getPBest()const;

	///////////////////////这是第一种方法，即采用恒定的学习因子。但是实际上可变的学习因子c1,c2可以使种群更快地收敛。此处，将两个维度的速度设置放在同一个函数中。
	void setV(Coordinate gBest, float w); //w为惯性因子。
	float getVx()const;
	float getVy()const;
	float getVz()const;

	void setCoordinate();
	float getX()const;
	float getY()const;
	float getZ()const;
	
	void outputFile(char Dir[])const;
	float L;	//L为功率总和，约束条件，调试使用
private:
	Coordinate c;
	float p;	//p为成本适应度
	
	Coordinate pBest;
	////////////////////二维的话就要有两个速度
	float Vx;
	float Vy;
	float Vz;
	static float k;//惩罚系数
	static float Xmax, Xmin;
	static float Ymax, Ymin;
	static float Zmax, Zmin;
	static float Vxmax, Vymax, Vzmax; //它们是用来对坐标和速度进行限制的，限制它只能在一定的范围内。
	static float Vxmin, Vymin, Vzmin;
	static float c1, c2, c3; //c1,c2,c3是学习因子。
						 //由于要对所有的对象进行比较之后才能得到群体最优，所以它还是不
						 //static Coordinate gBest; //这个是群体最优.整个群体共享一份就行，所以将它设置成static，但是注意千万不要以为static都是在初始化后就不能修改的，static const才是那样。

};

#endif