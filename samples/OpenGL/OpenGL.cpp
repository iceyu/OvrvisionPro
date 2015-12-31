//
//

#include "ovrvision_pro.h"

#include <GL/gl.h> 
#include <GL/glu.h> 
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

void SWAPBUFFERS();	// platform depend function


#define GL_API_CHECK(x)	x

GLuint textureIDs[2];


OVR::OvrvisionPro ovrvision;	// OvrvisionPro camera
OVR::ROI size;

GLvoid createObjects();


/* OpenGL code */
GLvoid initializeGL(GLsizei width, GLsizei height)
{
	ovrvision.CheckGPU();

	// Open with OpenGL sharing mode
	if (ovrvision.Open(0, OVR::Camprop::OV_CAMHD_FULL, 0) == 0) 
		puts("Can't open OvrvisionPro");

	//////////////////////////////////////////////////////////////////////////////
	// 肌色推定

	// Set scaling and size of scaled image
	size = ovrvision.SetSkinScale(2);
	cv::Mat images[2];
	images[0].create(size.height, size.width, CV_8UC4);
	images[1].create(size.height, size.width, CV_8UC4);

	// Estimate skin color range with 60 frames
	ovrvision.DetectHand(60);
	for (bool done = false; done == false;)
	{
		ovrvision.Capture(OVR::Camqt::OV_CAMQT_DMS);
		done = ovrvision.GetScaledImageRGBA(images[0].data, images[1].data);
		cv::waitKey(1);
		cv::imshow("L", images[0]);
		cv::imshow("R", images[1]);
	}
	cv::destroyAllWindows();
	//////////////////////////////////////////////////////////////////////////////

	// GPU共有テクスチャー開始
	createObjects();
}

GLvoid resize(GLsizei width, GLsizei height)
{
	GLfloat aspect;

	//glViewport(0, 0, width, height);

	aspect = (GLfloat)width / height;

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(45.0, aspect, 3.0, 7.0);
	//glMatrixMode(GL_MODELVIEW);
}

GLvoid createObjects()
{
	// 左右用にテクスチャ2個割り当て
	glEnable(GL_TEXTURE_2D);
	glGenTextures(2, textureIDs);

	// GPU共有テクスチャーの縮小率設定と、そのサイズの取得
	size = ovrvision.SetSkinScale(2);

	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// GPU共有テクスチャーを生成
	ovrvision.CreateSkinTextures(size.width, size.height, textureIDs[0], textureIDs[1]);
}


GLvoid drawScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// テクスチャの更新
	ovrvision.Capture(OVR::Camqt::OV_CAMQT_DMS);
	glFinish();
//	int64 start = cv::getTickCount();
	ovrvision.UpdateSkinTextures(textureIDs[0], textureIDs[1]);
//	int64 stop = cv::getTickCount();
//	double usec = (stop - start) * 1000000 / cv::getTickFrequency();
//	printf("%f usec\n", usec);
//	usec = cv::getTickFrequency();
	
#ifdef _DEBUG
	cv::Mat left(size.height, size.width, CV_8UC4);
	cv::Mat right(size.height, size.width, CV_8UC4);
	ovrvision.InspectTextures(left.data, right.data, 3); // Get HSV images
	cv::imshow("Left", left);
	cv::imshow("Right", right);
#endif

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);

	// テクスチャー貼り付け
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glTexCoord2i(1, 0);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glTexCoord2i(1, 1);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glTexCoord2i(0, 1);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glEnd();
	glFinish();

	// platform depend function
	SWAPBUFFERS();
}
