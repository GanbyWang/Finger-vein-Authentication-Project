//算分小班project
//李文新老师班
//enroll部分
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
#define srcLength 512		//源图长宽
#define srcWidth 384
#define dstLength 128		//目标图长宽
#define dstWidth 96
#define compressRatio 0.25	//压缩率
#define lowThreshold 80		//调用Canny函数时的阈值下界，越大轮廓越少
#define thresholdRatio 3	//调用Canny函数时阈值上界比下界的值
#define kernelSize 3		//调用Canny函数时内核算子的大小
#define impArea 12			//上下的不可能区域，认为不可能有轮廓在这个区域内
#define lredunArea 10		//去除左边部分图像，因为不大规整
#define rredunArea 4		//取出右边部分图像，因为不大规整
#define contourTol 5		//轮廓容忍值，与相邻轮廓的像素点差距在此之内则接受
#define startArea 10		//从图片中间上下多少像素点开始寻找轮廓
#define Rad2Deg(Rad) (Rad)* R2D		// 弧度到角度
#define R2D    57.295779513082323

int upContour[maxn];		//记录上轮廓纵坐标
int downContour[maxn];		//记录下轮廓纵坐标

unsigned char color[maxn][maxn];	//初始的灰度矩阵
unsigned char transV[maxn][maxn];	//二值化后的结果

std::vector<Point> midPoints;       //中线点集
Vec4f midLine;                      //存储中线容器，四个参数，前两个为直线的方向的单位向量，后两个值给出的是该直线通过的一个点。
int midP[2];

int dis = 8;				//参数dis表示点对应搜索半径
int t = 1;					//参数t表示圆周内突变值至少为t，越大对突变要求越高（灰度差越大）
int g = 108;				//参数g表示圆周内突变情况，越小对突变要求越高（突变数越多）

/* 压缩图片（调用opencv函数） */
Mat Compress(Mat & img)
{
	Mat tmp;
	resize(img, tmp, Size(0, 0), compressRatio, compressRatio, CV_INTER_LINEAR);
#ifdef DEBUG
	namedWindow("compressed");
	imshow("compressed", tmp);
	waitKey(0);
#endif // DEBUG
	Mat cmpImg = tmp;
	return cmpImg;
}

/* 该函数用于标记上下轮廓，输入参数为Mat类的图片，最终结果记录在upContour和downContour两个数组中 */
void Contour(Mat & img)
{
	Mat bwimg;				//用于存储调用完Canny函数后的图片

	/* 调用Canny函数 */
	Canny(img, bwimg, lowThreshold, lowThreshold*thresholdRatio, kernelSize);

	/* 用于调试，输出Canny函数调用后的处理结果 */
	/*#ifdef DEBUG
	namedWindow("edge");
	imshow("edge", bwimg);
	//imwrite("edge.bmp", bwimg);
	waitKey(0);
	#endif // DEBUG*/

	/* 初始化几个数组 */
	for (int i = 0; i < dstLength; i++)
	{
		upContour[i] = downContour[i] = 0;
	}

	bool edge[maxn][maxn];	//edge数组用于存储调用完Canny函数后的像素点（1表示白色，0表示黑色）
	for (int i = 0; i < dstWidth; i++)
	{
		for (int j = 0; j < dstLength; j++)
		{
			if (bwimg.at<uchar>(i, j) == 0) edge[i][j] = 0;	//将调用完Canny的图像转存到edge数组
			else
			{
				if (i > impArea && i < dstWidth - impArea)	//只有落在中间区域的白色像素点才可能是轮廓
					edge[i][j] = 1;
			}
		}
	}

	/* 该段程序用于寻找上下轮廓（从右往左） */
	for (int j = dstLength - 1; j >= 0; j--)
	{
		for (int i = dstWidth / 2 - startArea; i >= 0; i--)
		{
			if (edge[i][j] == 1)
			{
				if (upContour[j] == 0)
				{
					upContour[j] = i;

					if (j < dstLength - 1)
					{
						int k = j + 1;
						while (k < dstLength && upContour[k] == 0)
							k++;

						if (k < dstLength &&
							abs(upContour[j] - upContour[k]) > contourTol)
							upContour[j] = 0;
					}
				}
				else
				{
					int k = j + 1;
					while (k < dstLength && upContour[k] == 0)
						k++;

					if (k < dstLength &&
						abs(i - upContour[k]) < abs(upContour[j] - upContour[k]))
						upContour[j] = i;
				}
			}
		}

		for (int i = dstWidth / 2 + startArea; i < dstWidth; i++)
		{
			if (edge[i][j] == 1)
			{
				if (downContour[j] == 0)
				{
					downContour[j] = i;

					if (j < dstLength - 1)
					{
						int k = j + 1;
						while (k < dstLength && downContour[k] == 0)
							k++;

						if (k < dstLength &&
							abs(downContour[j] - downContour[k]) > contourTol)
							downContour[j] = 0;
					}
				}
				else
				{
					int k = j + 1;
					while (k < dstLength && downContour[k] == 0)
						k++;

					if (k < dstLength &&
						abs(i - downContour[k]) < abs(downContour[j] - downContour[k]))
						downContour[j] = i;
				}
			}
		}
	}

	/*#ifdef DEBUG
	Mat Contr = bwimg;
	for (int i = 0; i < dstWidth; i++)
	{
	for (int j = 0; j < dstLength; j++)
	{
	if (upContour[j] == i || downContour[j] == i)
	Contr.at<uchar>(i, j) = 255;
	else Contr.at<uchar>(i, j) = 0;
	}
	}

	namedWindow("contourRow");
	imshow("contourRow", Contr);
	waitKey(0);
	#endif // DEBUG*/


	/* 接下来的这段代码对可能出现的不连续情况进行处理 */
	for (int i = 0; i < dstLength; i++)
	{
		/* 如果轮廓开头有不连续的情况 */
		if (i == 0 && upContour[i] == 0)
		{
			int j = i + 1;				//j记录后一个不空的像素点
			while (upContour[j] == 0)
				j++;

			/* 直接将空缺的部分拿直线补全 */
			for (int k = 0; k < j; k++)
				upContour[k] = upContour[j];

			i = j - 1;
		}
		else if (i != 0 && upContour[i] == 0)	//其他情况
		{
			int j = i + 1;				//j记录后一个不空的像素点（注意不能超过范围）
			while (j < dstLength && upContour[j] == 0)
				j++;

			if (j == dstLength)			//如果空缺的部分是结尾不连续
			{
				/* 直接将空缺部分拿直线补全 */
				for (int k = i; k < dstLength; k++)
					upContour[k] = upContour[i - 1];
			}
			else						//其他情况要找到开始和终止的像素点
			{
				int diffY = j - i;		//计算y轴的像素差距
				int diffX = upContour[j] - upContour[i - 1];	//计算最靠近的两个像素点的x差距
				int ifNeg;				//记录是正增长还是负增长
				if (diffX < 0)			//如果是负增长，则标记ifNeg，并将diffX取正
				{
					diffX = -diffX;
					ifNeg = -1;
				}
				else ifNeg = 1;			//否则ifNeg置为1
				diffX--;				//统一将diffX减掉1（表示真正的x差距）

				int diff = diffX / diffY;		//计算横纵坐标的比值（是整除）
				if (ifNeg == -1) diff = -diff;	//根据正负更改符号
				if (diff != 0)			//如果比值不为0（即说明x轴的像素点差距比y轴大）
				{
					for (int k = i; k < j; k += 1)	//则根据间距进行赋值（重点在于尽量保持原来的斜率）
						upContour[k] = upContour[k - 1] + diff;
				}
				else					//否则则要保持一段直线再增加x值
				{
					if (diffX == 0)		//如果仅相差一个像素点，则直接拿横线填补
					{
						for (int k = i; k < j; k++)
							upContour[k] = upContour[k - 1];
						continue;
					}
					int r = diffY / diffX;			//r表示一段线段的长度
					for (int k = i; k < j; k++)
					{
						if ((k - i) % r == 0)		//这个条件表示需要将线段抬高（或降低）1个像素点
							upContour[k] = upContour[k - 1] + ifNeg;
						else			//否则保持不变
							upContour[k] = upContour[k - 1];
					}
				}
			}
		}
	}

	/* 接下来处理下轮廓，和上轮廓完全相同 */
	for (int i = 0; i < dstLength; i++)
	{
		/* 如果轮廓开头有不连续的情况 */
		if (i == 0 && downContour[i] == 0)
		{
			int j = i + 1;				//j记录后一个不空的像素点
			while (downContour[j] == 0)
				j++;

			/* 直接将空缺的部分拿直线补全 */
			for (int k = 0; k < j; k++)
				downContour[k] = downContour[j];

			i = j - 1;
		}
		else if (i != 0 && downContour[i] == 0)	//其他情况
		{
			int j = i + 1;				//j记录后一个不空的像素点（注意不能超过范围）
			while (j < dstLength && downContour[j] == 0)
				j++;

			if (j == dstLength)			//如果空缺的部分是结尾不连续
			{
				/* 直接将空缺部分拿直线补全 */
				for (int k = i; k < dstLength; k++)
					downContour[k] = downContour[i - 1];
			}
			else						//其他情况要找到开始和终止的像素点
			{
				int diffY = j - i;		//计算y轴的像素差距
				int diffX = downContour[j] - downContour[i - 1];	//计算最靠近的两个像素点的x差距
				int ifNeg;				//记录是正增长还是负增长
				if (diffX < 0)			//如果是负增长，则标记ifNeg，并将diffX取正
				{
					diffX = -diffX;
					ifNeg = -1;
				}
				else ifNeg = 1;			//否则ifNeg置为1
				diffX--;				//统一将diffX减掉1（表示真正的x差距）

				int diff = diffX / diffY;		//计算横纵坐标的比值（是整除）
				if (ifNeg == -1) diff = -diff;	//根据正负更改符号
				if (diff != 0)			//如果比值不为0（即说明x轴的像素点差距比y轴大）
				{
					for (int k = i; k < j; k += 1)	//则根据间距进行赋值（重点在于尽量保持原来的斜率）
						downContour[k] = downContour[k - 1] + diff;
				}
				else					//否则则要保持一段直线再增加x值
				{
					if (diffX == 0)		//如果仅相差一个像素点，则直接拿横线填补
					{
						for (int k = i; k < j; k++)
							downContour[k] = downContour[k - 1];
						continue;
					}
					int r = diffY / diffX;			//r表示一段线段的长度
					for (int k = i; k < j; k++)
					{
						if ((k - i) % r == 0)		//这个条件表示需要将线段抬高（或降低）1个像素点
							downContour[k] = downContour[k - 1] + ifNeg;
						else			//否则保持不变
							downContour[k] = downContour[k - 1];
					}
				}
			}
		}
	}

#ifdef DEBUG
	Mat Contr = bwimg;
	for (int i = 0; i < dstWidth; i++)
	{
		for (int j = 0; j < dstLength; j++)
		{
			if (upContour[j] == i || downContour[j] == i)
				Contr.at<uchar>(i, j) = 255;
			else Contr.at<uchar>(i, j) = 0;
		}
	}

	namedWindow("contour");
	imshow("contour", Contr);
	waitKey(0);
#endif // DEBUG

}

/* 将存有二值化后结果的数组转换成用Mat类 */
Mat trans(Mat & img)
{
	Mat show = img;
	for (int i = 0; i < dstWidth; i++)
	{
		for (int j = 0; j < dstLength; j++)
			show.at<uchar>(i, j) = transV[i][j];
	}

#ifdef DEBUG
	namedWindow("trans");
	imshow("trans", show);
	//imwrite("trans.bmp", show);
	waitKey(0);
#endif

	return show;
}

/* 中值滤波模糊（平滑）边界 */
/*Mat mSmooth(Mat & img)
{
	Mat src = img;
	Mat dst;

	//中值滤波
	medianBlur(src, dst, 3);

#ifdef DEBUG
	namedWindow("mSmoothed");
	imshow("mSmoothed", dst);
	//imwrite("smoothed.bmp", dst);
	waitKey(0);
#endif // DEBUG
	return dst;
}*/

//高斯滤波
Mat gSmooth(Mat & img)
{
	Mat src = img;
	Mat dst;

	//参数是按顺序写的
	//高斯滤波
	//src:输入图像
	//dst:输出图像
	//Size(5,5)模板大小，为奇数
	//x方向方差
	//Y方向方差
	//模板越大越模糊
	GaussianBlur(src, dst, Size(3, 3), 0, 0);

#ifdef DEBUG
	namedWindow("gSmoothed");
	imshow("gSmoothed", dst);
	//imwrite("smoothed.bmp", dst);
	waitKey(0);
#endif // DEBUG
	return dst;
}

//旋转函数
Mat rotate(Mat & img)
{
	Mat rot = img;
	//将中线上的点每四个取一个放入点集
	for (int i = 0; i < dstLength; i += 4)
	{
		int y = (upContour[i] + downContour[i]) / 2;
		midPoints.push_back(Point(i, y));
	}
	//划线
	//midLine[0][1][2][3]分别为sin cos 及线上某点坐标
	fitLine(Mat(midPoints), midLine, CV_DIST_L2, 0, 0.01, 0.01);
	double k1 = asin(midLine[1]);
	double k2 = acos(midLine[0]);
	double k3 = atan(midLine[1] / midLine[0]);
	double kkk = Rad2Deg(k3);//角度
	midP[0] = midLine[3];
	midP[1] = midLine[4];

	Mat rot_mat(2, 3, CV_8UC1);
	Point center = Point(midP[1], midP[0]);
	double angle = kkk + 0.47;

	rot_mat = getRotationMatrix2D(center, angle, 1);
	warpAffine(img, rot, rot_mat, img.size());

#ifdef DEBUG
	namedWindow("rotate");
	imshow("rotate", rot);
	waitKey(0);
#endif // DEBUG

	return rot;
}

//enroll函数
void Enroll(const char *src_path, const char *dst_path)
{
	Mat img;				//读进图片
	img = imread(src_path, CV_8UC1);

#ifdef DEBUG
	namedWindow("input");
	imshow("input", img);
	waitKey(0);
#endif // DEBUG

	img = Compress(img);	//将图片压缩
	Contour(img);			//寻找轮廓

	img = gSmooth(img);		//高斯模糊

	/* 将压缩图片放入color数组 */
	for (int i = 0; i < dstWidth; i++)
	{
		for (int j = 0; j < dstLength; j++)
			//uchar->unsignd char
			color[i][j] = img.at<uchar>(i, j);
	}

	for (int y = 0; y<dstWidth; ++y)							//分析图中每点是否为图特征点(圆形域)
	for (int x = 0; x < dstLength; ++x)
	{
		if (upContour[x] >= y || downContour[x] <= y)
		{
			transV[y][x] = 0;
			continue;       //手指轮廓外
		}
		int counter = 0;
		for (int dy = 0; dy <= dis; ++dy)
		{
			for (int dx = 0; dx*dx <= dis*dis - dy*dy; ++dx)
			{
				if (color[y + dy][x + dx] - color[y][x] <= t) counter++;
				if (dx&&color[y + dy][x - dx] - color[y][x] <= t) counter++;
				if (dy&&color[y - dy][x + dx] - color[y][x] <= t) counter++;
				if (dy&&dx&&color[y - dy][x - dx] - color[y][x] <= t) counter++;
			}
		}
		if (counter <= g)
		{
			transV[y][x] = 255;
		}
		else
			transV[y][x] = 0;
	}

	/* 将二值化后的图片左边一个区域的点全部涂黑（区域宽度由lredunArea和rredunArea定义） */
	for (int k = 0; k < lredunArea; k++)
	{
		for (int i = 0; i < dstWidth; i++)
			transV[i][k] = 0;
	}

	for (int k = 0; k < rredunArea; k++)
	{
		for (int i = 0; i < dstWidth; i++)
			transV[i][dstLength - 1 - k] = 0;
	}

	img = trans(img);		//将二值化后的数组转为Mat类
	img = rotate(img);		//旋转图片
	//img = mSmooth(img);	//再次进行滤波

	/*for (int i = 0; i < dstWidth; i++)
	{
	for (int j = 0; j < dstLength; j++)
	{
	if (img.at<uchar>(i, j) != 255)
	img.at<uchar>(i, j) = 0;
	}
	}*/

	/* 将数组存储到目的地址 */
	ofstream outFile(dst_path, ios::out | ios::binary);
	for (int y = 0; y < dstWidth; ++y)
	{
		for (int x = 0; x < dstLength; ++x)
		{
			unsigned char tmp = img.at<uchar>(y, x);
			outFile.write((char *)&tmp, sizeof(char));
		}
	}
	outFile.close();
}

int main(int argc, char *argv[])
{
	char *src_path = argv[1];
	char *dst_path = argv[2];
	Enroll(src_path, dst_path);
	return 0;
}