#define _CRT_SECURE_NO_WARNINGS
#include "Particle.h"
#include <stdio.h> //因为要用到sprintf函数。
#include <time.h>
#include <string>
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	Particle*p[40];
	float w;//实际上是wmax=1.2f;此处设置wmin=0.5;

	Particle*temp;
	float randX, randY, randZ;//定义风电机、光伏、微燃机数量
	srand((unsigned int)time(NULL));
	size_t i = 0;
	while(i<40)
	{
		//第一次初始化进行最大最小值约束
		randX = rand() % (NF_Max+1);//0-650
		randY = rand() % (NG_Max+1);//0-408
		randZ = rand() % (NW_Max+1);//0-6
		if (0.5*randX + 0.8*randY + 0.1 * randZ - AL_Max<0) continue;
		cout << "randX=" << randX << endl;
		cout << "randY=" << randY << endl;
		cout << "randZ=" << randZ << endl;
		p[i] = new Particle(randX, randY,randZ);
		cout << "The temp info is X:" << p[i]->getX() << " Y:" << p[i]->getY() << " Z:" << p[i]->getZ() << " P: " << p[i]->getP() << endl;
		i++;
	}
	//至此，就完成了粒子群的初始化。
	//////////////////////////////////////
	Coordinate gBest;  //全局最优解。
	int bestIndex = 0;
	float bestP; //最好的适应度。
	bestP = p[0]->getP();
	gBest = p[0]->getPBest();
	for (int i = 1; i<40; ++i)
	{
		if (p[i]->getP()<bestP)
		{
			bestP = p[i]->getP();
			gBest = p[i]->getPBest();
			bestIndex = i;
		}
	}

	/////////////////////////////////// 
	cout << "Now the initial gBest is X:" << gBest.x << " Y:" << gBest.y << " Z:" << gBest.z << endl;
	cout << "And the p[0] is X:" << p[0]->getX() << " Y:" << p[0]->getY() << " Z:" << p[0]->getZ() << endl;
	cout << "And the p[39] is X:" << p[39]->getX() << " Y:" << p[39]->getY() << " Z:" << p[39]->getZ() << endl;
	cout << "Now p[0].p=" << p[0]->getP() << endl;
	////////////////////////////////至此，已经寻找到初始时的种群最优。
	char buf[20];
	for (int i = 0; i<40; ++i)
	{
		sprintf(buf, "coordinate%d.dat", i);
		ofstream out(buf, ios::out);
		out.close();
	}
	//////////////////////////这样做是为了每运行一次都重复添加。
	for (int k = 0; k<100; ++k)   //k为迭代次数。
	{
		w = 0.9f - (0.9f - 0.4f)*k / 99.0f;  //这个因子很重要，既不能太大也不能太小。一开始自己就是设置得太大了导致出错。自己通过计算发现采用可变的惯性因子可以使得到的结果的精确度高一个数量级(达到1e-006)，而如果采用恒定的惯性因子，则只能得到1e-005的精度。
											 //////////////////////////一开始wmax=1.0,wmin=0.6,可以达到1e-006的精度，以为很高了，但是实际上wmax=0.9f,wmin=0.4f进可以很轻易地达到1e-11的水平，而这已经接近float的精度极限。

											 // w=0.85f;

		for (int i = 0; i<40; ++i)
		{
			temp = p[i];
			temp->setV(gBest, w);
			temp->setCoordinate();
			temp->setP();
			sprintf(buf, "coordinate%d.dat", i);
			temp->outputFile(buf);
		}
		bestP = p[0]->getP();
		gBest = p[0]->getPBest();
		for (int i = 1; i<40; ++i)
		{
			temp = p[i];
			if (temp->getP()<bestP)
			{
				bestP = temp->getP();
				gBest = temp->getPBest();
				bestIndex = i;
			}

		}
		cout << "Now gBest is X:" << gBest.x << " Y:" << gBest.y << " Z:" << gBest.z << " and the minP=" << p[bestIndex]->getP() << " L: " << p[bestIndex]->L << endl;
		cout << "bestIndex=" << bestIndex << endl;

	}
	system("pause");
	return 0;
}
