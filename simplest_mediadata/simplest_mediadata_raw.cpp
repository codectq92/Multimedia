/*************************************************************************
    > File Name: simplest_mediadata_main.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: Tue 14 Nov 2017 02:57:31 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/////////////////////////////RGB、YUV像素数据处理///////////////////////////////////////

//顺时针旋转90
static void YUV420PClockRot90(unsigned char* dest,unsigned char* src,int w,int h,FILE* fp)
{
	int nPos = 0;
    //旋转Y
	int k = 0;
	for(int i=0;i<w;i++)
	{
		for(int j = h -1;j >=0;j--)
		{
			dest[k++] = src[j*w + i];
		}
	}
	//旋转U
	nPos = w*h;
	for(int i=0;i<w/2;i++)
	{
		for(int j= h/2-1;j>=0;j--)
		{
			dest[k++] = src[nPos+ j*w/2 +i];
		}
	}
   
	//旋转V
	nPos = w*h*5/4;
	for(int i=0;i<w/2;i++)
	{
		for(int j= h/2-1;j>=0;j--)
		{
			dest[k++] = src[nPos+ j*w/2 +i];
		}
	}
	
	fwrite(dest,1,w*h,fp);
	fwrite(dest+w*h,1,w*h/4,fp);
	fwrite(dest+w*h*5/4,1,w*h/4,fp);
}

//顺时针旋转180
static void YUV420PClockRot180(unsigned char* dest,unsigned char* src,int w,int h,FILE* fp)
{ 
	int k = w*h-1;
	//旋转Y
    for(int i=0;i<w*h;i++)
	{
		*(dest + k--) = *(src + i);
	}
	
	//旋转U
	k = w*h*5/4 - 1;
	for(int i = w*h; i < w*h*5/4; i++)
	{	
		*(dest + k--) = *(src + i);
	}
	
	//旋转V
	k = w * h *3/2 -1;
	for(int i = w*h*5/4; i < w*h*3/2; i++)
	{
		*(dest + k--) = *(src + i);
	}
	
	fwrite(dest,1,w*h,fp);
	fwrite(dest+w*h,1,w*h/4,fp);
	fwrite(dest+w*h*5/4,1,w*h/4,fp);
}


/** 
 * Split Y, U, V planes in YUV420P file. 
 * @param url  Location of Input YUV file. 
 * @param w    Width of Input YUV file. 
 * @param h    Height of Input YUV file. 
 * @param num  Number of frames to process. 
 * 
 */
static int simplest_yuv420_split(const char *url, int w, int h,int num)
{
	FILE *fp  = fopen(url,"r+");  
	FILE *fp1 = fopen("yuv420p/output_420_y.y","w+");  
	FILE *fp2 = fopen("yuv420p/output_420_u.y","w+");  
    FILE *fp3 = fopen("yuv420p/output_420_v.y","w+");

	char yuv[40];
	sprintf(yuv,"yuv420p/output_%dx%d_yuv420p.yuv",w,h);
	FILE* fp4 = fopen(yuv,"w+");

	char yuvClockRot90[50];
	sprintf(yuvClockRot90,"yuv420p/output_clockrot90_%dx%d_yuv420p.yuv",h,w);
	FILE* fpClockRot90 = fopen(yuvClockRot90,"w+");

	char yuvClockRot180[50];
	sprintf(yuvClockRot180,"yuv420p/output_clockrot180_%dx%d_yuv420p.yuv",h,w);
	FILE* fpClockRot180 = fopen(yuvClockRot180,"w+");

    unsigned char *pic = (unsigned char *)malloc(w*h*3/2);

    unsigned char* picClockRot90 = (unsigned char*)malloc(w*h*3/2);
    unsigned char* picClockRot180 = (unsigned char*)malloc(w*h*3/2);
    
	for(int i=0;i<num;i++)
	{
		 fread(pic,1,w*h*3/2,fp);

		 //Y
		 fwrite(pic,1,w*h,fp1);
		 //U
		 fwrite(pic+w*h,1,w*h/4,fp2);
		 //V
		 fwrite(pic+w*h*5/4,1,w*h/4,fp3);

		 //重新合成yuv420p
         fwrite(pic,1,w*h,fp4);
		 fwrite(pic+w*h,1,w*h/4,fp4);
		 fwrite(pic+w*h*5/4,1,w*h/4,fp4);
       
         //旋转90
		 YUV420PClockRot90(picClockRot90,pic,w,h,fpClockRot90);
         //旋转180
		 YUV420PClockRot180(picClockRot180,pic,w,h,fpClockRot180);
	} 
	 
	free(pic);
	free(picClockRot90);
	free(picClockRot180);
	fclose(fp);  
	fclose(fp1);  
	fclose(fp2);  
	fclose(fp3);
	fclose(fp4);
	fclose(fpClockRot90);
	fclose(fpClockRot180);
}


/** 
* Split Y, U, V planes in YUV444P file. 
* @param url  Location of YUV file. 
* @param w    Width of Input YUV file. 
* @param h    Height of Input YUV file. 
* @param num  Number of frames to process.
*/
static int simplest_yuv444_split(const char *url, int w, int h,int num)
{
	FILE *fp = fopen(url,"r+");  
	FILE *fp1 = fopen("yuv444p/output_444_y.y","w+");  
	FILE *fp2 = fopen("yuv444p/output_444_u.y","w+");  
	FILE *fp3 = fopen("yuv444p/output_444_v.y","w+");


	char yuv444p[40];
	sprintf(yuv444p,"yuv444p/output_%dx%d_yuv444p.yuv",w,h);
	FILE* fp4 = fopen(yuv444p,"w+");

	unsigned char *pic = (unsigned char *)malloc(w*h*3);
    
     for(int i=0;i<num;i++)
	 {
		 fread(pic,1,w*h*3,fp);  
		 //Y  
		 fwrite(pic,1,w*h,fp1);  
		 //U  
		 fwrite(pic+w*h,1,w*h,fp2);  
		 //V 
		 fwrite(pic+w*h*2,1,w*h,fp3);

		 //重新合成yuv444p
         fwrite(pic,1,w*h,fp4);
		 fwrite(pic+w*h,1,w*h,fp4);
		 fwrite(pic+w*h*2,1,w*h,fp4);
	 }
	 
	 free(pic);  
	 fclose(fp);  
	 fclose(fp1);  
	 fclose(fp2);  
	 fclose(fp3);  
					     
	return 0;
}

/** 
* Convert YUV420P file to gray picture 
* @param url     Location of Input YUV file. 
* @param w       Width of Input YUV file. 
* @param h       Height of Input YUV file. 
* @param num     Number of frames to process. 
*/  
static int simplest_yuv420_gray(const char *url, int w, int h,int num)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("yuv420p/output_gray.yuv","w+");
	
	unsigned char *pic = (unsigned char *)malloc(w*h*3/2);
	
	for(int i=0;i<num;i++)
	{
		fread(pic,1,w*h*3/2,fp);
		//Gray
		memset(pic+w*h,128,w*h/2);
		fwrite(pic,1,w*h*3/2,fp1);
	}
	
	free(pic); 
	fclose(fp); 
	fclose(fp1);
	return 0;
}

/** 
* Halve Y value of YUV420P file 
* @param url     Location of Input YUV file. 
* @param w       Width of Input YUV file. 
* @param h       Height of Input YUV file. 
* @param num     Number of frames to process. 
*/  
static int simplest_yuv420_halfy(const char *url, int w, int h,int num)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("yuv420p/output_half.yuv","w+");
		    
	unsigned char *pic = (unsigned char *)malloc(w*h*3/2);

    for(int i=0;i<num;i++)
	{
		fread(pic,1,w*h*3/2,fp);
        //Half
		for(int j=0;j<w*h;j++)
		{
			unsigned char temp = pic[j]/2;
			pic[j] = temp;
		}
		fwrite(pic,1,w*h*3/2,fp1);
	}

	free(pic); 
	fclose(fp); 
	fclose(fp1);
	
	return 0;  
}

/** 
* Add border for YUV420P file 
* @param url     Location of Input YUV file. 
* @param w       Width of Input YUV file. 
* @param h       Height of Input YUV file. 
* @param border  Width of Border. 
* @param num     Number of frames to process. 
*/
static int simplest_yuv420_border(const char *url, int w, int h,int border,int num)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("yuv420p/output_border.yuv","w+");
	
	unsigned char *pic = (unsigned char *)malloc(w*h*3/2);

    for(int i=0;i<num;i++)
	{
		fread(pic,1,w*h*3/2,fp);
		//Y
		for(int j=0;j<h;j++)
		{
			for(int k=0;k<w;k++)
			{
				if(k<border|| k > (w-border) || j<border || j>(h-border))
				{
					pic[j*w+k] = 255;
				}
			}
		}
		fwrite(pic,1,w*h*3/2,fp1);
	}
	
	free(pic); 
	fclose(fp); 
	fclose(fp1);
	
	return 0;  
}

/** 
* Generate YUV420P gray scale bar. 
* @param width    Width of Output YUV file. 
* @param height   Height of Output YUV file. 
* @param ymin     Max value of Y 
* @param ymax     Min value of Y 
* @param barnum   Number of bars 
* @param url_out  Location of Output YUV file. 
*/
static int simplest_yuv420_graybar(int width, int height,int ymin,int ymax,int barnum,const char *url_out)
{
	int barwidth;
	float lum_inc;
	unsigned char lum_temp;
	int uv_width,uv_height;  
	FILE *fp = NULL;
	unsigned char *data_y = NULL;
    unsigned char *data_u = NULL;  
	unsigned char *data_v = NULL;
	int t = 0,i = 0,j = 0;
	
	barwidth = width/barnum;
	lum_inc = ((float)(ymax-ymin))/((float)(barnum-1));
	uv_width = width/2;
	uv_height = height/2;
				    
	data_y = (unsigned char *)malloc(width*height);
	data_u = (unsigned char *)malloc(uv_width*uv_height);  
	data_v = (unsigned char *)malloc(uv_width*uv_height);

	if((fp = fopen(url_out,"w+")) == NULL)
	{
		printf("Error: Cannot create file!");
		return -1;
	}
	
	//Output Info
	printf("Y, U, V value from picture's left to right:\n");

	for(t=0;t<(width/barwidth);t++)
	{
		lum_temp = ymin + (char)(t*lum_inc);  
		printf("%3d, 128, 128\n",lum_temp);  
	}

	//Gen Data
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			t = i/barwidth;
			lum_temp = ymin + (char)(t*lum_inc);
			data_y[j*width+i] = lum_temp;
		}
	}
	
	for(j=0;j<uv_height;j++)
	{
		for(i=0;i<uv_width;i++)
		{
			data_u[j*uv_width+i] = 128;
		}
	}
	
	for(j=0;j<uv_height;j++)
	{
		for(i=0;i<uv_width;i++)
		{
			data_v[j*uv_width+i] = 128;
		}
	}
	
	fwrite(data_y,width*height,1,fp);  
	fwrite(data_u,uv_width*uv_height,1,fp);  
	fwrite(data_v,uv_width*uv_height,1,fp);  
	
	fclose(fp);
	free(data_y);  
	free(data_u);  
	free(data_v);  
	return 0;
}

/** 
* Calculate PSNR between 2 YUV420P file 
* @param url1     Location of first Input YUV file. 
* @param url2     Location of another Input YUV file. 
* @param w        Width of Input YUV file. 
* @param h        Height of Input YUV file. 
* @param num      Number of frames to process. 
*/
static int simplest_yuv420_psnr(const char *url1,const char *url2,int w,int h,int num)
{
	FILE *fp1 = fopen(url1,"r+");  
	FILE *fp2 = fopen(url2,"r+");
	unsigned char *pic1 = (unsigned char *)malloc(w*h);
	unsigned char *pic2 = (unsigned char *)malloc(w*h);
	
	for(int i=0;i<num;i++)
	{
		fread(pic1,1,w*h,fp1);  
		fread(pic2,1,w*h,fp2);
		
		double mse_sum=0,mse=0,psnr=0;
		for(int j=0;j<w*h;j++)
		{
			mse_sum += pow((double)(pic1[j]-pic2[j]),2);
		}
		mse = mse_sum/(w*h); 
		psnr = 10*log10(255.0*255.0/mse); 
		printf("%5.3f\n",psnr);
		
		fseek(fp1,w*h/2,SEEK_CUR);
		fseek(fp2,w*h/2,SEEK_CUR);
	}

	free(pic1);
	free(pic2); 
	fclose(fp1);  
	fclose(fp2);
	return 0;
}

/** 
* Split R, G, B planes in RGB24 file. 
* @param url  Location of Input RGB file. 
* @param w    Width of Input RGB file. 
* @param h    Height of Input RGB file. 
* @param num  Number of frames to process. 
*/
static int simplest_rgb24_split(const char *url, int w, int h,int num)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("rgb24/output_r.y","w+");  
	FILE *fp2 = fopen("rgb24/output_g.y","w+");  
	FILE *fp3 = fopen("rgb24/output_b.y","w+"); 
				    
	unsigned char *pic = (unsigned char *)malloc(w*h*3);
	
	for(int i=0;i<num;i++)
	{
		fread(pic,1,w*h*3,fp);
		
		for(int j=0;j<w*h*3;j=j+3)
		{
			//R
			fwrite(pic+j,1,1,fp1);
			//G
			fwrite(pic+j+1,1,1,fp2);
			//B
			fwrite(pic+j+2,1,1,fp3);
		}
	}
    
	free(pic); 
	fclose(fp); 
	fclose(fp1);  
	fclose(fp2);
	fclose(fp3);
	
	return 0;
}

/** 
* Convert RGB24 file to BMP file 
* @param rgb24path    Location of input RGB file. 
* @param width        Width of input RGB file. 
* @param height       Height of input RGB file. 
* @param url_out      Location of Output BMP file. 
*/
static int simplest_rgb24_to_bmp(const char *rgb24path,int width,int height,const char *bmppath)
{	
	typedef struct tagBITMAPFILEHEADER{ 
		unsigned short      bfType;       //位图文件的类型，一定为19778，其转化为十六进制为0x4d42，对应的字符串为BM
		unsigned int        bfSize;       //文件大小，以字节为单位 4字节 
		unsigned short      bfReserverd1; //位图文件保留字，2字节，必须为0   
		unsigned short      bfReserverd2; //位图文件保留字，2字节，必须为0
		unsigned int        bfbfOffBits;  //位图文件头到数据的偏移量，以字节为单位，4字节
	}BITMAPFILEHEADER;

    typedef struct tagBITMAPINFOHEADER{
		unsigned int  biSize;               //该结构大小，字节为单位,4字节
		int  biWidth;                       //图形宽度以象素为单位,4字节 
		int  biHeight;                      //图形高度以象素为单位，4字节
		unsigned short biPlanes;            //目标设备的级别，必须为1
		unsigned short biBitcount;          //颜色深度，每个象素所需要的位数, 一般是24
		unsigned int  biCompression;        //位图的压缩类型,一般为0,4字节
		unsigned int  biSizeImage;          //位图的大小，以字节为单位,4字节，即上面结构体中文件大小减去偏移(bfSize-bfOffBits)
		int  biXPelsPermeter;               //位图水平分辨率，每米像素数,一般为0 
		int  biYPelsPermeter;               //位图垂直分辨率，每米像素数,一般为0
		unsigned int  biClrUsed;            //位图实际使用的颜色表中的颜色数,一般为0
		unsigned int  biClrImportant;       //位图显示过程中重要的颜色数,一般为0
	}BITMAPINFOHEADER;

	int i=0,j=0;
	BITMAPFILEHEADER m_BMPHeader = {0};
	BITMAPINFOHEADER  m_BMPInfoHeader = {0};
	//位图第一部分，文件信息
	m_BMPHeader.bfType = (unsigned short)(('M' << 8) | 'B');
	m_BMPHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height *3*sizeof(char)-2*sizeof(char);//去掉结构体BITMAPFILEHEADER补齐的2字节
	m_BMPHeader.bfReserverd1 = 0;
	m_BMPHeader.bfReserverd2 = 0;
	m_BMPHeader.bfbfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)-2*sizeof(char);//真正的数据的位置
	
	unsigned char *rgb24_buffer = NULL;
	FILE *fp_rgb24 = NULL,*fp_bmp = NULL;
	
	if((fp_rgb24 = fopen(rgb24path,"r"))==NULL){
		printf("Error: Cannot open input RGB24 file.\n");
		return -1;
	}
	
	if((fp_bmp = fopen(bmppath,"w"))==NULL){
		printf("Error: Cannot open output BMP file.\n");
		return -1;
	}
	
	rgb24_buffer = (unsigned char *)malloc(width*height*3);
	fread(rgb24_buffer,1,width*height*3,fp_rgb24);

	//位图第二部分，数据信息
	m_BMPInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BMPInfoHeader.biWidth = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.biHeight = -height;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了
    m_BMPInfoHeader.biPlanes = 1;
    m_BMPInfoHeader.biBitcount = 24;
    m_BMPInfoHeader.biCompression = 0;//不压缩
    m_BMPInfoHeader.biSizeImage = width * height * 3 * sizeof(char);
    m_BMPInfoHeader.biXPelsPermeter = 0;
	m_BMPInfoHeader.biYPelsPermeter = 0;
    m_BMPInfoHeader.biClrUsed = 0;
	m_BMPInfoHeader.biClrImportant = 0;


    fwrite(&m_BMPHeader,sizeof(m_BMPHeader.bfType)+sizeof(m_BMPHeader.bfSize)+sizeof(m_BMPHeader.bfReserverd1),1,fp_bmp); 
//	fwrite(&m_BMPHeader.bfType,sizeof(m_BMPHeader.bfType),1,fp_bmp);
//	fwrite(&m_BMPHeader.bfSize,sizeof(m_BMPHeader.bfSize),1,fp_bmp);
//	fwrite(&m_BMPHeader.bfReserverd1,sizeof(m_BMPHeader.bfReserverd1),1,fp_bmp);
	fwrite(&m_BMPHeader.bfReserverd2,sizeof(m_BMPHeader.bfReserverd2),1,fp_bmp);
	fwrite(&m_BMPHeader.bfbfOffBits,sizeof(m_BMPHeader.bfbfOffBits),1,fp_bmp);

    printf("====BMP文件头： %lu,  BMP图片头: %lu\n",sizeof(m_BMPHeader),sizeof(m_BMPInfoHeader));
	
	fwrite(&m_BMPInfoHeader,1,sizeof(m_BMPInfoHeader),fp_bmp);
	
	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	//So we change 'R' and 'B'
	for(j =0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			char temp = rgb24_buffer[(j*width+i)*3+2];
			rgb24_buffer[(j*width+i)*3+2] = rgb24_buffer[(j*width+i)*3+0];
			rgb24_buffer[(j*width+i)*3+0] = temp;
		}
	}
	
	fwrite(rgb24_buffer,3*width*height,1,fp_bmp);
	fclose(fp_rgb24);
	fclose(fp_bmp);
	free(rgb24_buffer);
	printf("Finish generate %s!\n",bmppath);

	return 0;
}

static unsigned char clip_value(unsigned char x,unsigned char min_val,unsigned char  max_val)
{
	if(x>max_val)
	{
		return max_val;
	}
	else if(x<min_val)
	{
		return min_val;
	}
	else
	{
		return x;
	}
}

//RGB24 to YUV420P
static bool RGB24_TO_YUV420(unsigned char *RgbBuf,int w,int h,unsigned char *yuvBuf)
{
	unsigned char*ptrY, *ptrU, *ptrV, *ptrRGB;
	memset(yuvBuf,0,w*h*3/2);
	ptrY = yuvBuf;
	ptrU = yuvBuf + w*h;
	ptrV = ptrU + (w*h*1/4);
	unsigned char y, u, v, r, g, b;
	
	for (int j = 0; j<h;j++)
	{
		ptrRGB = RgbBuf + w*j*3;
		for (int i = 0;i<w;i++)
		{
			r = *(ptrRGB++);
			g = *(ptrRGB++);
			b = *(ptrRGB++);
			y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16;
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
			*(ptrY++) = clip_value(y,0,255);
			
			if (j%2 == 0 && i%2 == 0)
			{
				*(ptrU++) = clip_value(u,0,255);
			}
			else
			{
				if (i%2 == 0)
				{
					*(ptrV++) = clip_value(v,0,255);
				}
			}
		}
	}

	return true;
}

/** 
* Convert RGB24 file to YUV420P file 
* @param url_in  Location of Input RGB file. 
* @param w       Width of Input RGB file. 
* @param h       Height of Input RGB file. 
* @param num     Number of frames to process. 
* @param url_out Location of Output YUV file. 
*/
static int simplest_rgb24_to_yuv420(const char *url_in, int w, int h,int num,const char *url_out)
{
	FILE *fp = fopen(url_in,"r+");
	FILE *fp1 = fopen(url_out,"w+");
	
	unsigned char *pic_rgb24 = (unsigned char *)malloc(w*h*3);
	unsigned char *pic_yuv420 = (unsigned char *)malloc(w*h*3/2);

    for(int i=0;i<num;i++)
	{
		fread(pic_rgb24,1,w*h*3,fp);
		RGB24_TO_YUV420(pic_rgb24,w,h,pic_yuv420);
		fwrite(pic_yuv420,1,w*h*3/2,fp1);
	}
	   
	free(pic_rgb24);
	free(pic_yuv420);
	fclose(fp);
	fclose(fp1);

	return 0;
}

/** 
* Generate RGB24 colorbar. 
* @param width    Width of Output RGB file. 
* @param height   Height of Output RGB file. 
* @param url_out  Location of Output RGB file. 
*/
static int simplest_rgb24_colorbar(int width, int height,const char *url_out)
{
	unsigned char *data = NULL;
	int barwidth;
	char filename[100] = {0};
	FILE *fp = NULL;
	int i = 0,j = 0;
	
	data = (unsigned char *)malloc(width*height*3);
	barwidth = width/8;
	
	if((fp = fopen(url_out,"w+")) == NULL){
		printf("Error: Cannot create file!");
		return -1;
	}
	
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			int barnum = i/barwidth;
			switch(barnum)
			{
				case 0:
				{
					data[(j*width+i)*3+0] = 255;
					data[(j*width+i)*3+1] = 255;
					data[(j*width+i)*3+2] = 255;
					break;  
				}
				case 1:
				{
					data[(j*width+i)*3+0] = 255;
					data[(j*width+i)*3+1] = 255;
					data[(j*width+i)*3+2] = 0;
					break;
				}
				case 2:
				{
					data[(j*width+i)*3+0] = 0;
					data[(j*width+i)*3+1] = 255;
					data[(j*width+i)*3+2] = 255;
					break;
				}
                case 3:
				{
					data[(j*width+i)*3+0] = 0;
					data[(j*width+i)*3+1] = 255;
					data[(j*width+i)*3+2] = 0;
					break;
				}
				case 4:
				{
					data[(j*width+i)*3+0] = 255;
					data[(j*width+i)*3+1] = 0;
					data[(j*width+i)*3+2] = 255;
					break;
				}
                case 5:
				{
					data[(j*width+i)*3+0] = 255;
					data[(j*width+i)*3+1] = 0;
					data[(j*width+i)*3+2] = 0;
					break;
				}
				case 6:
				{
					data[(j*width+i)*3+0] = 0;
					data[(j*width+i)*3+1] = 0;
					data[(j*width+i)*3+2] = 255;
					break;
				}
                case 7:
				{
					data[(j*width+i)*3+0] = 0;
					data[(j*width+i)*3+1] = 0;
					data[(j*width+i)*3+2] = 0;
					break;
				}
			}
		}
	}
	
	fwrite(data,width*height*3,1,fp);
	fclose(fp);
	free(data);
	
	return 0;
}

///////////////////////PCM音频采样数据处理////////////////////////////////////////////

/**
* Split Left and Right channel of 16LE PCM file.
* @param url  Location of PCM file.
*/
static int simplest_pcm16le_split(const char *url)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("pcm/output_l.pcm","w+");
	FILE *fp2 = fopen("pcm/output_r.pcm","w+");
	
	unsigned char *sample = (unsigned char *)malloc(4);
	
	while(!feof(fp))
	{
		fread(sample,1,4,fp);
		//L
		fwrite(sample,1,2,fp1);
		//R
		fwrite(sample+2,1,2,fp2);
	}
	
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	
	return 0;
}


/**
* Halve volume of Left channel of 16LE PCM file
* @param url  Location of PCM file.
*/
static int simplest_pcm16le_halfvolumeleft(const char *url)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("pcm/output_halfleft.pcm","w+");

	int cnt = 0;
	unsigned char *sample = (unsigned char *)malloc(4);

	while(!feof(fp))
	{
		short *samplenum = NULL;
		fread(sample,1,4,fp);

		samplenum = (short *)sample;
		*samplenum = *samplenum/2;

		//L
		fwrite(sample,1,2,fp1);
		//R
		fwrite(sample+2,1,2,fp1);

		cnt++;
	}

	printf("%s: Sample Cnt:%d\n",__FUNCTION__,cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);

	return 0;
}

/**
* Re-sample to double the speed of 16LE PCM file
* @param url  Location of PCM file.
*/
static int simplest_pcm16le_doublespeed(const char *url)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("pcm/output_doublespeed.pcm","w+");

	int cnt = 0;
	unsigned char *sample = (unsigned char *)malloc(4);

	while(!feof(fp))
	{
		fread(sample,1,4,fp);
		if(cnt%2 != 0)
		{
			//L
			fwrite(sample,1,2,fp1);
			//R
			fwrite(sample+2,1,2,fp1);
		}

		cnt++;
	}

	printf("%s: Sample Cnt:%d\n",__FUNCTION__,cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);

	return 0;
}

/**
* Convert PCM-16 data to PCM-8 data.
* @param url  Location of PCM file.
*/
static int simplest_pcm16le_to_pcm8(const char *url)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("pcm/output_8.pcm","w+");

	int cnt = 0;
	unsigned char *sample = (unsigned char *)malloc(4);

	while(!feof(fp))
	{
		short *samplenum16 = NULL;
		char samplenum8 = 0;
		unsigned char samplenum8_u = 0;
		fread(sample,1,4,fp);
        //(-32768-32767)
		samplenum16 = (short *)sample;
		samplenum8 = (*samplenum16) >> 8;
		//(0-255)
		samplenum8_u = samplenum8 + 128;
		//L
		fwrite(&samplenum8_u,1,1,fp1);

		samplenum16 = (short *)(sample+2);
		samplenum8 = (*samplenum16) >> 8;
		samplenum8_u = samplenum8 + 128;
		//R
        fwrite(&samplenum8_u,1,1,fp1);
		cnt++;
	}

	printf("%s: Sample Cnt:%d\n",__FUNCTION__,cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);

	return 0;
}

/**
* Cut a 16LE PCM single channel file.
* @param url        Location of PCM file.
* @param start_num  start point
* @param dur_num    how much point to cut
*/
static int simplest_pcm16le_cut_singlechannel(const char *url,int start_num,int dur_num)
{
	FILE *fp = fopen(url,"r+");
	FILE *fp1 = fopen("pcm/output_cut.pcm","w+");
	FILE *fp_stat = fopen("pcm/output_cut.txt","w+");

	unsigned char *sample = (unsigned char *)malloc(2);
	int cnt = 0;

	while(!feof(fp))
	{
		fread(sample,1,2,fp);
		if(cnt>start_num && cnt<=(start_num + dur_num))
		{
			fwrite(sample,1,2,fp1);
			short samplenum = sample[1];
			samplenum = samplenum*256;
			samplenum = samplenum + sample[0];
			fprintf(fp_stat,"%6d,",samplenum);

			if(cnt%10 == 0)
				fprintf(fp_stat,"\n");
		}

		cnt++;
	}

	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp_stat);

	return 0;
}

/**
* Convert PCM16LE raw data to WAVE format
* @param pcmpath      Input PCM file.
* @param channels     Channel number of PCM file.
* @param sample_rate  Sample rate of PCM file.
* @param wavepath     Output WAVE file.
*
*  WAVE文件是一种RIFF格式的文件。其基本块名称是“WAVE”，其中包含了两个子块“fmt”和“data”
*  从编程的角度简单说来就是由WAVE_HEADER、WAVE_FMT、WAVE_DATA、采样数据共4个部分组成
*  它的结构如下所示
*  WAVE_HEADER
*  WAVE_FMT
*  WAVE_DATA
*  PCM数据
*/
static int simplest_pcm16le_to_wave(const char *pcmpath,int channels,int sample_rate,const char *wavepath)
{
	typedef struct WAVE_HEADER{
		char           ChunkID[4];         //内容为"RIFF"
		unsigned int   ChunkSize;          //存储文件的字节数（不包含ChunkID和ChunkSize这8个字节）
		char           Format[4];          //内容为"WAVE"
	}WAVE_HEADER;

	typedef struct WAVE_FMT{
		char             Subchunk1ID[4];      //内容为"fmt "
		unsigned int     Subchunk1Size;       //存储该子块的字节数,为16（不含前面的Subchunk1ID和Subchunk1Size这8个字节
		unsigned short   AudioFormat;         //存储音频文件的编码格式，例如若为PCM则其存储值为1，若为其他非PCM格式的则有一定的压缩
		unsigned short   NumChannels;         //通道数，单通道(Mono)值为1，双通道(Stereo)值为2
		unsigned int     SampleRate;          //采样率，如8k，44.1k等
		unsigned int     ByteRate;            //每秒存储的字节数，其值=SampleRate * NumChannels * BitsPerSample/8
		unsigned short   BlockAlign;          //块对齐大小，其值=NumChannels * BitsPerSample/8
		unsigned short   BitsPerSample;       //每个采样点的bit数，一般为8,16,32等
	}WAVE_FMT;

	typedef struct WAVE_DATA{
		char         Subchunk2ID[4];       //内容为“data”
		unsigned int Subchunk2Size;        //PCM原始裸数据字节数
	}WAVE_DATA;

	if(channels==0 || sample_rate==0)
	{
		channels = 2;
		sample_rate = 44100;
	}

	int bits = 16;
	WAVE_HEADER pcmHEADER;
	WAVE_FMT    pcmFMT;
	WAVE_DATA   pcmDATA;

	short m_pcmData;
	FILE *fp,*fpout;

	fp = fopen(pcmpath, "r");
	if(fp == NULL)
	{
		printf("%s: open pcm file error\n",__FUNCTION__);
		return -1;
	}

	fpout = fopen(wavepath,"w+");
	if(fpout == NULL)
	{
		printf("%s: create wav file error\n",__FUNCTION__);
		return -1;
	}

	//WAVE_HEADER
	memcpy(pcmHEADER.ChunkID,"RIFF",strlen("RIFF"));
	memcpy(pcmHEADER.Format,"WAVE",strlen("WAVE"));
	fseek(fpout,sizeof(WAVE_HEADER),SEEK_SET);

	//WAVE_FMT
	pcmFMT.SampleRate = sample_rate;
	pcmFMT.NumChannels = channels;
	pcmFMT.BitsPerSample = bits;
	pcmFMT.ByteRate = pcmFMT.SampleRate * pcmFMT.NumChannels * pcmFMT.BitsPerSample / 8;
	memcpy(pcmFMT.Subchunk1ID,"fmt ",strlen("fmt "));
	pcmFMT.Subchunk1Size = 16;
	pcmFMT.BlockAlign = pcmFMT.NumChannels * pcmFMT.BitsPerSample / 8;
	pcmFMT.AudioFormat = 1;

	fwrite(&pcmFMT,sizeof(WAVE_FMT),1,fpout);

	//WAVE_DATA
	memcpy(pcmDATA.Subchunk2ID,"data",strlen("data"));
	pcmDATA.Subchunk2Size = 0;
	fseek(fpout,sizeof(WAVE_DATA),SEEK_CUR);

    fread(&m_pcmData,sizeof(short),1,fp);
	while(!feof(fp))
	{
		pcmDATA.Subchunk2Size += 2;
		fwrite(&m_pcmData,sizeof(short),1,fpout);
		fread(&m_pcmData,sizeof(short),1,fp);
	}

	pcmHEADER.ChunkSize = 36 + pcmDATA.Subchunk2Size;
	rewind(fpout);
	fwrite(&pcmHEADER,sizeof(WAVE_HEADER),1,fpout);
	fseek(fpout,sizeof(WAVE_FMT),SEEK_CUR);
	fwrite(&pcmDATA,sizeof(WAVE_DATA),1,fpout);

	fclose(fp);
	fclose(fpout);

	return 0;
}

//////////////////H.264视频码流解析/////////////////////////////////////////

typedef enum{
	NALU_TYPE_SLICE    = 1,      //不分区、非IDR的片
	NALU_TYPE_DPA      = 2,      //片分区A
	NALU_TYPE_DPB      = 3,      //片分区B
	NALU_TYPE_DPC      = 4,      //片分区C
	NALU_TYPE_IDR      = 5,      //IDR图像中的片
	NALU_TYPE_SEI      = 6,      //补充增强信息单元(SEI)
	NALU_TYPE_SPS      = 7,      //序列参数集 (SPS)
	NALU_TYPE_PPS      = 8,      //图像参数集 (PPS)
	NALU_TYPE_AUD      = 9,      //分界符
	NALU_TYPE_EOSEQ    = 10,     //序列结束
	NALU_TYPE_EOSTREAM = 11,     //碼流结束
	NALU_TYPE_FILL     = 12      //填充
}NaluType;

typedef enum{
	NALU_PRIORITY_DISPOSABLE = 0,
	NALU_PRIRITY_LOW         = 1,
	NALU_PRIORITY_HIGH       = 2,
	NALU_PRIORITY_HIGHEST    = 3
}NaluPriority;

typedef struct{
	int startcodeprefix_len;          //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned int len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned int max_size;            //! Nal Unit Buffer size
	int forbidden_bit;                //! should be always FALSE
	int nal_reference_idc;            //! NALU_PRIORITY_xxxx
	int nal_unit_type;                //! NALU_TYPE_xxxx
	char *buf;                        //! contains the first byte followed by the EBSP
}NALU_t;

static FILE* h264bitstream = NULL;    //the bit stream file
static int info2 = 0, info3 = 0;

static int FindStartCode2(unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1)
		return 0; //0x000001?
	else
		return 1;
}

static int FindStartCode3(unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1)
		return 0;//0x00000001?
	else
		return 1;
}

static int GetAnnexbNALU(NALU_t *nalu)
{
	int pos = 0;
	int StartCodeFound,rewind;
	unsigned char *Buf;

	if((Buf = (unsigned char*)calloc(nalu->max_size,sizeof(char))) == NULL)
		printf("%s: Could not allocate Buf memory\n",__FUNCTION__);

	nalu->startcodeprefix_len = 3;
	if(3 != fread(Buf,1,3,h264bitstream))
	{
		free(Buf);
		return 0;
	}

//先找3字节的startCode
	info2 = FindStartCode2(Buf);
	if(info2 != 1)    //不是3字节的StartCode
	{
		if(1 != fread(Buf+3,1,1,h264bitstream))
		{
			free(Buf);
			return 0;
		}
        //再找4字节的StartCode
		info3 = FindStartCode3(Buf);
		if(info3 != 1)
		{
            //未找到4字节的StartCode
			free(Buf);
			return -1;
		}
		else
		{
            //找到4字节的StartCode
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}
	else //找到3字节的StartCode
	{
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}

    //printf("==========haoge=====info2: %d   info3: %d   pos: %d  startcodeprefix_len: %d\n",info2,info3,pos,nalu->startcodeprefix_len);

	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

    //找到StartCode后，查找相邻的下一个StartCode的位置
	while(!StartCodeFound)
	{
		if(feof(h264bitstream))
		{
			nalu->len = (pos-1)-nalu->startcodeprefix_len;
			memcpy(nalu->buf,&Buf[nalu->startcodeprefix_len],nalu->len);
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			free(Buf);
			return pos-1;
		}

		Buf[pos++] = fgetc(h264bitstream);
		info3 = FindStartCode3(&Buf[pos-4]);
		if(info3 != 1)
			info2 = FindStartCode2(&Buf[pos-3]);
		StartCodeFound = (info2 == 1 || info3 == 1);
	}

	// Here, we have found another start code and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    rewind = (info3 == 1)? -4 : -3;

    //将文件指针重新定位到刚找到的StartCode的位置上
    if(0 != fseek(h264bitstream,rewind,SEEK_CUR))
    {
		free(Buf);
		printf("%s: Cannot fseek in the bit stream file",__FUNCTION__);
	}

	// Here the Start code, the complete NALU, and the next start code is in the Buf.
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;//NALU的大小，包括一个字节NALU头和EBSP数据
	memcpy(nalu->buf,&Buf[nalu->startcodeprefix_len], nalu->len);//从StartCode之后开始拷贝NALU字节流数据
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit,提取NALU头中的forbidden_bit (禁止位)
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit，提取NALU头中的nal_reference_bit (优先级)
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit,提取NALU头中的nal_unit_type (NAL类型)
	free(Buf);

	return (pos+rewind);
}

/**
* Analysis H.264 Bitstream
* @param url Location of input H.264 bitstream file.
*/
static int simplest_h264_parser(const char *url)
{
	NALU_t *n;
	int buffersize = 100000;

	//FILE *myout=fopen("output_log.txt","w+");
	FILE *myout = stdout;
	h264bitstream = fopen(url, "r+");
	if(h264bitstream == NULL)
	{
		printf("%s: Open file error\n",__FUNCTION__);
		return 0;
	}

	n = (NALU_t*)calloc(1,sizeof(NALU_t));
	if(n == NULL)
	{
		printf("%s: Alloc NALU Error\n",__FUNCTION__);
		return 0;
	}

	n->max_size = buffersize;
	n->buf = (char*)calloc(buffersize,sizeof(char));
	if(n->buf == NULL)
	{
		free(n);
		printf("%s: AllocNALU: n->buf",__FUNCTION__);
		return 0;
	}

	int data_offset = 0;
	int nal_num = 0;
	printf("-----+-------- NALU Table ------+---------+\n");
	printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
	printf("-----+---------+--------+-------+---------+\n");

	while(!feof(h264bitstream))
	{
		int data_lenth;
		data_lenth = GetAnnexbNALU(n);
		char type_str[20] = {0};

		switch(n->nal_unit_type)
		{
			case NALU_TYPE_SLICE:
				sprintf(type_str,"SLICE");
				break;
			case NALU_TYPE_DPA:
				sprintf(type_str,"DPA");
				break;
			case NALU_TYPE_DPB:
				sprintf(type_str,"DPB");
				break;
			case NALU_TYPE_DPC:
				sprintf(type_str,"DPC");
				break;
			case NALU_TYPE_IDR:
				sprintf(type_str,"IDR");
				break;
			case NALU_TYPE_SEI:
				sprintf(type_str,"SEI");
				break;
			case NALU_TYPE_SPS:
				sprintf(type_str,"SPS");
				break;
			case NALU_TYPE_PPS:
				sprintf(type_str,"PPS");
				break;
			case NALU_TYPE_AUD:
				sprintf(type_str,"AUD");
				break;
			case NALU_TYPE_EOSEQ:
				sprintf(type_str,"EOSEQ");
				break;
			case NALU_TYPE_EOSTREAM:
				sprintf(type_str,"EOSTREAM");
				break;
			case NALU_TYPE_FILL:
				sprintf(type_str,"FILL");
				break;
		}

		char idc_str[20] = {0};
		switch(n->nal_reference_idc >>5)
		{
			case NALU_PRIORITY_DISPOSABLE:
				sprintf(idc_str,"DISPOS");
				break;
			case NALU_PRIRITY_LOW:
				sprintf(idc_str,"LOW");
				break;
			case NALU_PRIORITY_HIGH:
				sprintf(idc_str,"HIGH");
				break;
			case NALU_PRIORITY_HIGHEST:
				sprintf(idc_str,"HIGHEST");
				break;
		}

		fprintf(myout,"%5d| %8d| %7s| %6s| %8d|\n",nal_num,data_offset,idc_str,type_str,n->len);
		data_offset = data_offset + data_lenth;
		nal_num++;
	}

	//Free
	if(n)
	{
		if(n->buf)
		{
			free(n->buf);
			n->buf = NULL;
		}
		free(n);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	simplest_yuv420_split("yuv420p/lena_256x256_yuv420p.yuv",256,256,1);
    simplest_yuv444_split("yuv444p/lena_256x256_yuv444p.yuv",256,256,1);
    simplest_yuv420_gray("yuv420p/lena_256x256_yuv420p.yuv",256,256,1);
    simplest_yuv420_halfy("yuv420p/lena_256x256_yuv420p.yuv",256,256,1);
    simplest_yuv420_border("yuv420p/lena_256x256_yuv420p.yuv",256,256,20,1);
    simplest_yuv420_graybar(640,360,0,255,10,"yuv420p/output_graybar_640x360.yuv");
	simplest_yuv420_psnr("yuv420p/lena_256x256_yuv420p.yuv","yuv420p/lena_distort_256x256_yuv420p.yuv",256,256,1);
	simplest_rgb24_split("rgb24/cie1931_500x500.rgb",500,500,1);
	simplest_rgb24_to_bmp("rgb24/lena_256x256_rgb24.rgb",256,256,"rgb24/output_lena.bmp");
	simplest_rgb24_to_yuv420("rgb24/lena_256x256_rgb24.rgb",256,256,1,"rgb24/output_lena_256x256_yuv420p.yuv");
	simplest_rgb24_colorbar(640, 360,"rgb24/colorbar_640x360.rgb");

	simplest_pcm16le_split("pcm/NocturneNo2inEflat_44.1k_s16le.pcm");
    simplest_pcm16le_halfvolumeleft("pcm/NocturneNo2inEflat_44.1k_s16le.pcm");
    simplest_pcm16le_doublespeed("pcm/NocturneNo2inEflat_44.1k_s16le.pcm");
    simplest_pcm16le_to_pcm8("pcm/NocturneNo2inEflat_44.1k_s16le.pcm");
    simplest_pcm16le_cut_singlechannel("pcm/drum.pcm",2360,120);
    simplest_pcm16le_to_wave("pcm/NocturneNo2inEflat_44.1k_s16le.pcm",2,44100,"pcm/output_nocturne.wav");

	simplest_h264_parser("h264/sintel.h264");

	return 0;
}

