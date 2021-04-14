
#include "Particle.h"
#include <fstream>
#include <iostream>
using namespace std;
float Particle::Xmax = NF_Max;   //自己通过测试证明，通过限制速度和位置坐标取得了良好的效果，可以很轻易地达到1e-006的精度以上，甚至于能达到1e-009的精度。原因就在于整个群体中的粒子都能更快地集中到适应度高的位置，自然群体最优解就更好。
float Particle::Xmin = 0;
float Particle::Ymax = NG_Max;
float Particle::Ymin = 0;
float Particle::Zmax = NW_Max;
float Particle::Zmin = 0;
float Particle::Vxmax = (Xmax - Xmin)/5;  //通常设置Vmax=Xmax-Xmin;  
float Particle::Vxmin = (0 - Vxmax);
float Particle::Vymax = (Ymax - Ymin)/5;
float Particle::Vymin = (0 - Vymax);
float Particle::Vzmax = (Zmax - Zmin)/5;
float Particle::Vzmin = (0 - Vzmax);

float Particle::k = 2;//惩罚系数,不满足条件的值加大处罚

float Particle::c1 = 2.0f;
float Particle::c2 = 2.0f;
float Particle::c3 = 2.0f;

Particle::Particle(float x, float y, float z)
{
	c.x = x;
	c.y = y;
	c.z = z;
	//p=100.0f; //先给它一个较大的适应度值（这里我们要得到的是一个较小的适应值）。
	L = 0.5*c.x + 0.8*c.y + 0.1 * c.z - AL_Max;
	p = 1.2*c.x + 0.1764*c.y + 0.11*c.z + 258.6 + k*(0 <= L ? 0 : L*L);
	Vx = (Xmax - Xmin) / 8.0f;  //////////////////////这里先采用第一种初始化方法，即给所有粒子一个相同的初始速度,为Vmax/8.0f,而Vmax=Xmax-Xmin=30-0=30;注意这个初始速度千万不能太大，自己测试发现它的大小对最终结果的精度影响也很大，有几个数量级。当然，也不能太小，自己测试发现在(Xmax-Xmin)/8.0f时可以得到比较高的精度。
	Vy = (Ymax - Ymin) / 8.0f;
	Vz = (Zmax - Zmin) / 8.0f;

	///初始时的pBest

	pBest.x = x;
	pBest.y = y;
	pBest.z = z;

}

void Particle::setP()   //这里既完成了适应度值p的设置，也完成了局部最优pBest的设置，所以不需要另外加一个setPBest()函数 。
{
	L = 0.5*c.x + 0.8*c.y + 0.1 * c.z - AL_Max;
	float temp = 1.2*c.x + 0.1764*c.y + 0.11*c.z + 258.6 + k*(0 <= L ? 0 : L*L);
	if (temp<p)
	{
		p = temp;
		pBest = c;
	}
}

float Particle::getP()const
{
	return p;
}

Coordinate Particle::getPBest()const
{
	return pBest;
}

void Particle::setV(Coordinate gBest, float w)  //w为惯性因子，
{
	float r1, r2, r3;
	r1 = rand() / (float)RAND_MAX;
	r2 = rand() / (float)RAND_MAX;
	r3 = rand() / (float)RAND_MAX;
	Vx = w*Vx + c1*r1*(pBest.x - c.x) + c2*r2*(gBest.x - c.x) + c3*r3*(gBest.x - c.x);
	if (Vx>Vxmax)
		Vx = Vxmax;
	else if (Vx<Vxmin)
		Vx = Vxmin;
	Vy = w*Vy + c1*r1*(pBest.y - c.y) + c2*r2*(gBest.y - c.y) + c3*r3*(gBest.y - c.y);
	if (Vy>Vymax)
		Vy = Vymax;
	else if (Vy<Vymin)
		Vy = Vymin;
	Vz = w*Vz + c1*r1*(pBest.z - c.z) + c2*r2*(gBest.z - c.z) + c3*r3*(gBest.z - c.z);
	if (Vz>Vzmax)
		Vz = Vzmax;
	else if (Vx<Vxmin)
		Vz = Vzmin;
}

float Particle::getVx()const
{
	return Vx;
}
float Particle::getVy()const
{
	return Vy;
}
float Particle::getVz()const
{
	return Vz;
}

//////////////////////必须先进行setV()的操作然后才能进行这步，否则坐标加的就不是第k+1次的速度了。
void Particle::setCoordinate()
{
	c.x = c.x + Vx;
	if (c.x>Xmax)
		c.x = Xmax;
	else if (c.x<Xmin)
		c.x = Xmin;
	c.y = c.y + Vy;
	if (c.y>Ymax)
		c.y = Ymax;
	else if (c.y<Ymin)
		c.y = Ymin;
	c.z = c.z + Vz;
	if (c.z>Zmax)
		c.z = Zmax;
	else if (c.z<Zmin)
		c.z = Zmin;
}

float Particle::getX()const
{
	return c.x;
}
float Particle::getY()const
{
	return c.y;
}
float Particle::getZ()const
{
	return c.z;
}

void Particle::outputFile(char Dir[])const
{
	ofstream out(Dir, ios::app);  //这是添加吧？

	out << this->getX() << " " << this->getY() << " " <<this->getZ() << " " << pBest.x << " " << pBest.y << " " <<pBest.z << endl;
	out.close();

}

ostream& operator<<(ostream &output, const Particle &right)
{
	output << "Now the current coordinates is X:" << right.getX() << " Y:" << right.getY() << " Z:" << right.getZ() << endl;
	output << "And the pBest is X:" << right.getPBest().x << " Y:" << right.getPBest().y << " Z:" << right.getPBest().z << endl;
	return output;
}
