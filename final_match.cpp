//算分小班project
//李文新老师班
//match部分
//汪益成 1400012795
//马浩迪 1400012862
//杨文逸 1400012880

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include "bitmap.h"
#include <opencv2\opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

#define maxn 600			//定义图片的最长长度
#define dstLength 128		//目标图长宽
#define dstWidth 96

unsigned char match1[maxn][maxn];	//读入的第一个模板
unsigned char match2[maxn][maxn];	//读入的第二个模板

void Match(const char *t1_path, const char *t2_path)
{
	ifstream inFile1(t1_path, ios::in | ios::binary);
	ifstream inFile2(t2_path, ios::in | ios::binary);

	int white1, white2;
	white1 = 0;
	white2 = 0;

	//读入模板
	for (int i = 0; i < dstWidth; i++)
	{
		for (int j = 0; j < dstLength; j++)
		{
			//uchar->unsignd char
			inFile1.read((char *)&match1[i][j], sizeof(char));
			inFile2.read((char *)&match2[i][j], sizeof(char));

			if (match1[i][j] == 255)
				white1++;

			if (match2[i][j] == 255)
				white2++;
		}
	}
	//关闭文件
	inFile1.close();
	inFile2.close();

	int count;			//count记录匹配的白色像素点个数
	int max = 0;		//max记录最大的匹配值

						//几个方向移动图片进行匹配，输出最大值
	for (int a = 0; a <= 15; a++)
	{
		for (int b = 0; b <= 15; b++)
		{
			count = 0;	//清零
			for (int i = 0; i < dstWidth; i++)
			{
				for (int j = 0; j < dstLength; j++)
				{
					//如果坐标有效并且对应两张图的像素点均为识别区域
					if (i - a >= 0 && j - b >= 0 &&
						match1[i - a][j - b] == match2[i][j] &&
						match2[i][j] == 255)
						count++;	//计数加加
				}
			}
			if (count > max) max = count;

			count = 0;	//清零
			for (int i = 0; i < dstWidth; i++)
			{
				for (int j = 0; j < dstLength; j++)
				{
					//如果坐标有效并且对应两张图的像素点均为识别区域
					if (i - a >= 0 && j + b < dstLength &&
						match1[i - a][j + b] == match2[i][j] &&
						match2[i][j] == 255)
						count++;	//计数加加
				}
			}
			if (count > max) max = count;

			count = 0;	//清零
			for (int i = 0; i < dstWidth; i++)
			{
				for (int j = 0; j < dstLength; j++)
				{
					//如果坐标有效并且对应两张图的像素点均为识别区域
					if (i + a < dstWidth && j - b >= 0 &&
						match1[i + a][j - b] == match2[i][j] &&
						match2[i][j] == 255)
						count++;	//计数加加
				}
			}
			if (count > max) max = count;

			count = 0;	//清零
			for (int i = 0; i < dstWidth; i++)
			{
				for (int j = 0; j < dstLength; j++)
				{
					//如果坐标有效并且对应两张图的像素点均为识别区域
					if (i + a < dstWidth && j + b < dstLength &&
						match1[i + a][j + b] == match2[i][j] &&
						match2[i][j] == 255)
						count++;	//计数加加
				}
			}
			if (count > max) max = count;
		}
	}

	//将两个比值平均后输出
	double result;
	result = (double)max / white1;
	result += (double)max / white2;
	result = result / 2;
	cout << result << endl;
}

int main(int argc, char *argv[])
{
	char *t1_path = argv[1];
	char *t2_path = argv[2];
	Match(t1_path, t2_path);
	return 0;
}