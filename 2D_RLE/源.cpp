#include <iostream>
#include <opencv2/opencv.hpp>
#include <afx.h>
#include <string.h>
using namespace std;
using namespace cv;
int getMd(int i, int j);							//由行列号计算十进制Morton值
void getIJ(int Md, int &i, int &j);					//由十进制Morton值计算行列号
bool encode(Mat img, CString codeFileName);			//二维行程编码
bool decode(CString codeFileName, Mat &img);		//二维行程解码
int main(int argc, char *argv[])
{
	char srcImgFileName[200], codeFileName[200];
	strcpy(srcImgFileName, "../../data/test.jpg");	//原图像文件名
	strcpy(codeFileName, "../../data/code.cqa");	//编码文件名
	if (argc == 3)
	{
		strcpy(srcImgFileName, argv[1]);
		strcpy(codeFileName, argv[2]);
	}
	cout << "srcImgFileName: " << srcImgFileName << endl;
	cout << "codeFileName: " << codeFileName << endl;
	/*打开图像*/
	Mat img = imread(srcImgFileName, CV_LOAD_IMAGE_GRAYSCALE);
	if (img.empty())
	{
		cout << "Error: Unable to open image." << endl;
		//system("pause");
		return -1;
	}
	else cout << "Message: Successfully open image." << endl;
	/*二维行程编码*/
	if (!encode(img,codeFileName))
	{
		cout << "Error: Unable to write encoded data to file." << endl;
		//system("pause");
		return -2;
	}
	else cout << "Message: Successful encoding." << endl;
	/*二维行程解码*/
	Mat img_;
	if (!decode(codeFileName,img_))
	{
		cout << "Error: Unable to open encoded file." << endl;
		//system("pause");
		return -3;
	}
	else cout << "Message: Successful decoding." << endl;
	/*显示两幅图像进行对比*/
	namedWindow("srcImg");
	namedWindow("dstImg");
	imshow("srcImg", img);
	imshow("dstImg",img_);
	waitKey(0);
	return 0;
}
int getMd(int i, int j)
{
	/*十进制Morton值由i和j中的二进制数字交叉结合得出*/
	int Md = 0;
	for (int t = 0; i || j; t+=2)
	{
		Md |= (i & 1) << (t + 1) | (j & 1) << t;	//取出i和j的二进制最后一位数字，分别按位左移t+1位、t位，按位或！
		i >>= 1; j >>= 1;							//i和j按位右移一位
	}
	return Md;
}
void getIJ(int Md, int &i, int &j)
{
	/*i和j由Morton二进制交叉取出得到*/
	i = j = 0;	//初始化为0
	for (int t = 0; Md; t++)
	{
		i |= (Md & 2) >> 1 << t;					//取出Md二进制倒数第2位数字，按位右移1位，再按位左移t位，按位或！
		j |= (Md & 1) << t;							//取出Md二进制倒数第1位数字，按位左移t位，按位或！
		Md >>= 2;									//Md按位右移2位
	}
}
bool encode(Mat img, CString codeFileName)
{
	/*获取图像数据*/
	int rows = img.rows;
	int cols = img.cols;
	uchar* pData = img.data;
	/*打开文件*/
	CFile outFile;
	if (!outFile.Open(codeFileName, CFile::modeWrite | CFile::modeCreate)) return false;
	/*记录行数和列数*/
	outFile.Write(&rows, sizeof(rows));
	outFile.Write(&cols, sizeof(cols));
	unsigned int index = 0;
	uchar value = pData[0];
	/*记录第0个值*/
	outFile.Write(&index, sizeof(index));
	outFile.Write(&value, sizeof(value));
	/*从1到min2PowSq依次取出灰度值并与前一个进行比较*/
	int i, j;
	for (int k = 1; k <= getMd(rows-1,cols-1); k++)
	{
		getIJ(k, i, j);								//由Md计算行列号
		if (i >= rows || j >= cols) continue;		//行列号不在图像范围内
		uchar newValue = pData[i*cols + j];
		if (newValue == value) continue;			//与前一个进行比较
		index = k; value = newValue;				//更新index和data
		/*记录index和value*/
		outFile.Write(&index, sizeof(index));
		outFile.Write(&value, sizeof(value));
	}
	outFile.Close();
	return true;
}
bool decode(CString codeFileName, Mat &img)
{
	/*打开文件*/
	CFile inFile;
	if (!inFile.Open(codeFileName, CFile::modeRead)) return false;
	/*读取图像行数和列数*/
	int rows, cols;
	inFile.Read(&rows, sizeof(rows));
	inFile.Read(&cols, sizeof(cols));
	img.create(rows, cols, CV_8UC1);
	uchar* pData = img.data;
	/*读取二位行程编码表中第一行*/
	unsigned int index = 0;
	uchar value = 0;
	inFile.Read(&index, sizeof(index));
	inFile.Read(&value, sizeof(value));
	/*读取二位行程编码表中下一行*/
	unsigned int newIndex;
	uchar newValue;
	int i, j;
	while (inFile.Read(&newIndex, sizeof(newIndex)) && inFile.Read(&newValue, sizeof(newValue)))
	{
		/*将Md值在[index,newIndex)范围内的像素灰度值设为value*/
		for (int k = index; k < (int)newIndex; k++)
		{
			getIJ(k, i, j);
			if (i >= rows || j >= cols) continue;
			pData[i*cols + j] = value;
		}
		index = newIndex; value = newValue;
	}
	/*将Md值在[index,min2PowSq)范围内的像素灰度值设为value*/
	for (int k = index; k <= getMd(rows - 1, cols - 1); k++)
	{
		getIJ(k, i, j);
		if (i >= rows || j >= cols) continue;
		pData[i*cols + j] = value;
	}
	inFile.Close();
	return true;
}