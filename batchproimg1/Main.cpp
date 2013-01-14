////////////////////////////////////////////////////////////////////////////////
//
// Author : Vision Assistant
// Purpose: This file implements the algorithm prototyped in Vision Assistant.
//
// WARNING: This file was automatically generated.
//          Any changes you make to this file will be lost if you generate the
//          file again.
//
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//     Includes
//==============================================================================

#include <stdio.h>
#include <nivision.h>
#include "ImageProcessing.h"
#include <vector>
#include <afx.h>
//#include <stdafx.h>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
//==============================================================================
//  Defines
//==============================================================================

#define DISPLAY_WINDOW 0

//extern IVA_Data *ivaData;
struct point
{
	double x,y;
};
struct point3
{
	double x;
	double y;
	double z;
};
///////////////////////////////////////
//
////////////////////////////////////////
double Product(point A,point B,point C)//计算两个向量的叉积
{
	return (B.x-A.x)*(C.y-A.y)-(C.x-A.x)*(B.y-A.y);
}


//////////////////////////////////////
//A,B,C,D在逆时针方向
//如果E在ABCD之外，返回FLASE,否则返回TRUE
//注：此处依赖于A、B、C、D的位置关系，其位置不能调换 详细算法见编程之美 上面写反了
//////////////////////////////////////
BOOL isQuadrilateral(point A,point B,point C,point D,point E)
{
	if(Product(A,B,E)<0.0 && Product(B,C,E)<0.0 && Product(C,D,E)<0.0 && Product(D,A,E)<0.0)
	{
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////
//得到两点之间的距离
//////////////////////////////////////
double GetDistance(point A,point B)
{
	double result;
	result=sqrt(pow((A.x-B.x),2)+pow((A.y-B.y),2));
	return result;
}

//////////////////////////////////////
//得到三角形的面积
//////////////////////////////////////
double GetTriangleArea(double a,double b, double c)
{
	double result,p;
	p=(a+b+c)/2;
	result=sqrt(p*(p-a)*(p-b)*(p-c));
	return result;
}

//////////////////////////////////////
//得到光纤的线径 A,B,C,D逆时针排列
//////////////////////////////////////
double GetDiameter(point A,point B,point C,point D)
{
	double result,area1,area2;
	result=GetDistance(A,D);//临时变量 AB的线段长
	area1=GetTriangleArea(result,GetDistance(A,B),GetDistance(D,B));//ABD
	result=area1*2/result;//用三角形ABD算出的线径1
	area1=GetDistance(C,B);//临时变量，线段BC的长
	area2=GetTriangleArea(area1,GetDistance(B,D),GetDistance(D,C));//BCD
	area1=2*area2/area1;//用三角形BCD算出的线径2
	result=(result+area1)/2;//对计算得到的两个线径值做加权平均
	return result;
}
//全局变量
SYSTEMTIME st;
int mtime1;
int mtime2;
int mmtime1;
int mmtime2;
//SYSTEMTIME dst;
//
//==============================================================================
//  Main Function
//==============================================================================

int main (int argc, char *argv[])
{
    point3 filetemp[7117];
	int success = 1;
    int err = 0;
//    char** imagePath;       // Image Path
//    int cancelled;
    ImageType imageType;    // Image Type
    Image* image;           // Image
	vector <CString> v;
	//IVA_Data *ivaData;
    // IMAQ Vision creates windows in a separate thread
    imaqSetWindowThreadPolicy(IMAQ_SEPARATE_THREAD);
	//外层一个循环，用于更新imagePath
	CFileFind finder;
	BOOL bWorking = finder.FindFile(_T("c:\\imagestorep\\*.jpg"));
	int count=0;//use in debug
	int count1=0;
	int count2=0;
	int count3=0;
	imageType=IMAQ_IMAGE_U8;
	image = imaqCreateImage(imageType, 7);
	IVA_Data *ivaData;
	ivaData = IVA_InitData(9, 0);
	GetSystemTime(&st);
	mtime1=st.wSecond;
	mmtime1=st.wMilliseconds;
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		//cout <<(LPCTSTR) (finder.GetFileName()) <<endl;
		CString temp;
		temp=finder.GetFilePath();
		
    // Display the Load Image dialog
    //imagePath = imaqLoadImagePopup(NULL, "*.*", NULL, "Open Image", FALSE, IMAQ_BUTTON_LOAD, 0, 0, 1, 0, &cancelled, NULL);//可能可以读取多个图像文件，这里只用这个函数取得第一个图像文件的路径

    
        // Get the type of the image file to create an image of the right type
        //imaqGetFileInfo((LPCTSTR)temp, NULL, NULL, NULL, NULL, NULL, &imageType);//imagePath[0]

        // Create an IMAQ Vision image
        //image = imaqCreateImage(imageType, 7);

        // Read the image from disk
        imaqReadFile(image, (LPCTSTR)temp, NULL, NULL);//imagePath[0]

        // Vision Assistant Algorithm
		//IVA_Data *ivaData;
		//ivaData = IVA_InitData(9, 0);
        success = IVA_ProcessImage(image,ivaData);
        if (!success)
            err = imaqGetLastError();
		//在这里处理结果
		int temp1=int(ivaData->stepResults[4].results[0].resultVal.numVal);//这什么玩意 没注释害死人 这是有无黑斑的检测值

		//
		point A,B,C,D;//直线的四个端点
		A.x=ivaData->stepResults[8].results[0].resultVal.numVal;
		A.y=ivaData->stepResults[8].results[1].resultVal.numVal;
		B.x=ivaData->stepResults[7].results[0].resultVal.numVal;
		B.y=ivaData->stepResults[7].results[1].resultVal.numVal;
		C.x=ivaData->stepResults[7].results[2].resultVal.numVal;
		C.y=ivaData->stepResults[7].results[3].resultVal.numVal;
		D.x=ivaData->stepResults[8].results[2].resultVal.numVal;
		D.y=ivaData->stepResults[8].results[3].resultVal.numVal;
		double angle1,angle2,diameter;
		angle1=ivaData->stepResults[7].results[4].resultVal.numVal;
		angle2=ivaData->stepResults[8].results[4].resultVal.numVal;
		diameter=GetDiameter(A,B,C,D);
		//if (diameter<25 && diameter>10)
		//{
		//	diameter=diameter;
		//}
		filetemp[count].x=diameter;
		filetemp[count].y=angle2;
		filetemp[count].z=angle1;
		//
		if (temp1>0)
		{
			count1++;
			//判断在直线范围以内
			/*point A,B,C,D;
			A.x=ivaData->stepResults[8].results[0].resultVal.numVal;
			A.y=ivaData->stepResults[8].results[1].resultVal.numVal;
			B.x=ivaData->stepResults[7].results[0].resultVal.numVal;
			B.y=ivaData->stepResults[7].results[1].resultVal.numVal;
			C.x=ivaData->stepResults[7].results[2].resultVal.numVal;
			C.y=ivaData->stepResults[7].results[3].resultVal.numVal;
			D.x=ivaData->stepResults[8].results[2].resultVal.numVal;
			D.y=ivaData->stepResults[8].results[3].resultVal.numVal;
			double angle1,angle2,diameter;
			angle1=ivaData->stepResults[7].results[4].resultVal.numVal;
			angle2=ivaData->stepResults[8].results[4].resultVal.numVal;*/
			
			if (abs(angle1-angle2)<0.5)
			{
				
				for (int i=0;i<temp1;i++)
				{
					point E;
					E.x=ivaData->stepResults[4].results[i*2+1].resultVal.numVal;
					E.y=ivaData->stepResults[4].results[i*2+2].resultVal.numVal;
					if (isQuadrilateral(A,B,C,D,E))
					{
						count2++;
						break;
					}
				}
			}
			else
			{
				//表面有一层毛，影响检测，两个边沿不近似平行
			}
		}
		//IVA_DisposeData(ivaData);
        // Display the image
        //imaqMoveWindow(DISPLAY_WINDOW, imaqMakePoint(0,0));
        //imaqSetWindowPalette(DISPLAY_WINDOW, IMAQ_PALETTE_BINARY, NULL, 0);
        //imaqDisplayImage(image, DISPLAY_WINDOW, TRUE);

        // Wait for a key press before exiting
        

        // Dispose resources
        //imaqDispose(image);
		//2012-12-8mch
		count++;
		//cout <<count<<endl;
		if (!bWorking && count3<9)
		{
			bWorking = finder.FindFile(_T("c:\\imagestorep\\*.jpg"));
			count3++;
		}
	}
	GetSystemTime(&st);
	mtime2=st.wSecond;
	mmtime2=st.wMilliseconds;
	cout<<"stime= "<<mtime2-mtime1<<endl;
	cout<<"milltime= "<<mmtime2-mmtime1<<endl;
	IVA_DisposeData(ivaData);
	imaqDispose(image);
	cout<<"result=";
	cout<<count1<<endl;
	cout<<"result2=";
	cout<<count2<<endl;
	finder.Close();

	//文件操作

	std::ofstream file("d:\\try.txt",ios::out|ios::trunc);
	if(!file)
	{
		std::cout<<"不可以打开文件"<<std::endl;
		exit(1);
	}
	for (int k=0;k<103;k++)
	{
		file<<filetemp[k].x<<" "<<filetemp[k].y<<" "<<filetemp[k].z<<endl;
	}
	file.close();
    //imaqDispose(imagePath);
	printf ("Press Enter to exit.\n");
	getchar();
    return 0;
}
