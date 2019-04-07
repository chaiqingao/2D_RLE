#include <iostream>
#include <opencv2/opencv.hpp>
#include <afx.h>
#include <string.h>
using namespace std;
using namespace cv;
int getMd(int i, int j);							//�����кż���ʮ����Mortonֵ
void getIJ(int Md, int &i, int &j);					//��ʮ����Mortonֵ�������к�
bool encode(Mat img, CString codeFileName);			//��ά�г̱���
bool decode(CString codeFileName, Mat &img);		//��ά�г̽���
int main(int argc, char *argv[])
{
	char srcImgFileName[200], codeFileName[200];
	strcpy(srcImgFileName, "../../data/test.jpg");	//ԭͼ���ļ���
	strcpy(codeFileName, "../../data/code.cqa");	//�����ļ���
	if (argc == 3)
	{
		strcpy(srcImgFileName, argv[1]);
		strcpy(codeFileName, argv[2]);
	}
	cout << "srcImgFileName: " << srcImgFileName << endl;
	cout << "codeFileName: " << codeFileName << endl;
	/*��ͼ��*/
	Mat img = imread(srcImgFileName, CV_LOAD_IMAGE_GRAYSCALE);
	if (img.empty())
	{
		cout << "Error: Unable to open image." << endl;
		//system("pause");
		return -1;
	}
	else cout << "Message: Successfully open image." << endl;
	/*��ά�г̱���*/
	if (!encode(img,codeFileName))
	{
		cout << "Error: Unable to write encoded data to file." << endl;
		//system("pause");
		return -2;
	}
	else cout << "Message: Successful encoding." << endl;
	/*��ά�г̽���*/
	Mat img_;
	if (!decode(codeFileName,img_))
	{
		cout << "Error: Unable to open encoded file." << endl;
		//system("pause");
		return -3;
	}
	else cout << "Message: Successful decoding." << endl;
	/*��ʾ����ͼ����жԱ�*/
	namedWindow("srcImg");
	namedWindow("dstImg");
	imshow("srcImg", img);
	imshow("dstImg",img_);
	waitKey(0);
	return 0;
}
int getMd(int i, int j)
{
	/*ʮ����Mortonֵ��i��j�еĶ��������ֽ����ϵó�*/
	int Md = 0;
	for (int t = 0; i || j; t+=2)
	{
		Md |= (i & 1) << (t + 1) | (j & 1) << t;	//ȡ��i��j�Ķ��������һλ���֣��ֱ�λ����t+1λ��tλ����λ��
		i >>= 1; j >>= 1;							//i��j��λ����һλ
	}
	return Md;
}
void getIJ(int Md, int &i, int &j)
{
	/*i��j��Morton�����ƽ���ȡ���õ�*/
	i = j = 0;	//��ʼ��Ϊ0
	for (int t = 0; Md; t++)
	{
		i |= (Md & 2) >> 1 << t;					//ȡ��Md�����Ƶ�����2λ���֣���λ����1λ���ٰ�λ����tλ����λ��
		j |= (Md & 1) << t;							//ȡ��Md�����Ƶ�����1λ���֣���λ����tλ����λ��
		Md >>= 2;									//Md��λ����2λ
	}
}
bool encode(Mat img, CString codeFileName)
{
	/*��ȡͼ������*/
	int rows = img.rows;
	int cols = img.cols;
	uchar* pData = img.data;
	/*���ļ�*/
	CFile outFile;
	if (!outFile.Open(codeFileName, CFile::modeWrite | CFile::modeCreate)) return false;
	/*��¼����������*/
	outFile.Write(&rows, sizeof(rows));
	outFile.Write(&cols, sizeof(cols));
	unsigned int index = 0;
	uchar value = pData[0];
	/*��¼��0��ֵ*/
	outFile.Write(&index, sizeof(index));
	outFile.Write(&value, sizeof(value));
	/*��1��min2PowSq����ȡ���Ҷ�ֵ����ǰһ�����бȽ�*/
	int i, j;
	for (int k = 1; k <= getMd(rows-1,cols-1); k++)
	{
		getIJ(k, i, j);								//��Md�������к�
		if (i >= rows || j >= cols) continue;		//���кŲ���ͼ��Χ��
		uchar newValue = pData[i*cols + j];
		if (newValue == value) continue;			//��ǰһ�����бȽ�
		index = k; value = newValue;				//����index��data
		/*��¼index��value*/
		outFile.Write(&index, sizeof(index));
		outFile.Write(&value, sizeof(value));
	}
	outFile.Close();
	return true;
}
bool decode(CString codeFileName, Mat &img)
{
	/*���ļ�*/
	CFile inFile;
	if (!inFile.Open(codeFileName, CFile::modeRead)) return false;
	/*��ȡͼ������������*/
	int rows, cols;
	inFile.Read(&rows, sizeof(rows));
	inFile.Read(&cols, sizeof(cols));
	img.create(rows, cols, CV_8UC1);
	uchar* pData = img.data;
	/*��ȡ��λ�г̱�����е�һ��*/
	unsigned int index = 0;
	uchar value = 0;
	inFile.Read(&index, sizeof(index));
	inFile.Read(&value, sizeof(value));
	/*��ȡ��λ�г̱��������һ��*/
	unsigned int newIndex;
	uchar newValue;
	int i, j;
	while (inFile.Read(&newIndex, sizeof(newIndex)) && inFile.Read(&newValue, sizeof(newValue)))
	{
		/*��Mdֵ��[index,newIndex)��Χ�ڵ����ػҶ�ֵ��Ϊvalue*/
		for (int k = index; k < (int)newIndex; k++)
		{
			getIJ(k, i, j);
			if (i >= rows || j >= cols) continue;
			pData[i*cols + j] = value;
		}
		index = newIndex; value = newValue;
	}
	/*��Mdֵ��[index,min2PowSq)��Χ�ڵ����ػҶ�ֵ��Ϊvalue*/
	for (int k = index; k <= getMd(rows - 1, cols - 1); k++)
	{
		getIJ(k, i, j);
		if (i >= rows || j >= cols) continue;
		pData[i*cols + j] = value;
	}
	inFile.Close();
	return true;
}