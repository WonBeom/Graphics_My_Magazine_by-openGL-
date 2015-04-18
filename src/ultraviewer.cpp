/* 20103378 임종호  */
/* 20103389 최원범  */

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <iostream>
#include <time.h>
#include <SOIL/SOIL.h>

#include "data.h"

void initBuffer();
void onDisplay();
void onResize(int width, int height);
void mousewheel(int wheel, int direction, int x, int y);
void MyMouseClick(GLint Button, GLint State, GLint x, GLint y);
int LoadGLTexture();
void MyMouseMove(GLint X,GLint Y);
void idle();

GLuint mBackVertexBuffer[6];
GLuint mFrameVertexBuffer[15];
GLuint mColorBuffer;
GLuint mIndexBuffer;
GLuint mTexcoodBuffer[15];

GLfloat xtrans = 0;
GLfloat ytrans = 0;
GLfloat ztrans = 0;

// intrinsic parameter
GLdouble othl = -1.15f;
GLdouble othr = 1.15f;
GLdouble othup = 1.15f;
GLdouble othdw = -1.15f;

// extrinsic parameter
GLdouble ex = 0;
GLdouble ey = 0;
GLdouble cenx = 0;
GLdouble ceny = 0;

GLint StartX, StartY;

bool is_lRotate = false;
bool is_rRotate = false;

float angle = 0;
const float angle_rotate = 90;

long animation_time = 500; // milliseconds
long start_time = 0;
long current_time = 0;

bool drag = false;
GLuint texture_id[15];
GLuint* buffers;

int width = 512, height = 512;
void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("UltraViewer");

	if(glewInit() != GLEW_OK) 
	{
		std::cout << "GLEW init is failed" << std::endl;
	}

	initBuffer();
	glutDisplayFunc(onDisplay);
	glutReshapeFunc(onResize);
	glutIdleFunc(idle);

	glutMotionFunc(MyMouseMove);
	glutMouseFunc(MyMouseClick);
	glutMouseWheelFunc(mousewheel);

	glutMainLoop();

	delete []buffers;
}
void onResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(  othl,  othr,
		     othdw, othup,
			     0,   100);
	glMatrixMode(GL_MODELVIEW);
}
void idle()
{
	glutPostRedisplay();
}

void initBuffer()
{
	buffers = new GLuint[38];
	glGenBuffers(38, &buffers[0]);
	
	for(int i=0; i<6; i++)
	{
		mBackVertexBuffer[i] = buffers[i];
		// Create a vertex buffer & transfer vertices data 
		// from client space to server space.
		glBindBuffer(GL_ARRAY_BUFFER, mBackVertexBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(back_vertices) * 4, back_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		for(int j=2; j<24; j+=3)
			back_vertices[j] -= 1.0f;
	}
	for(int i=0; i<15; i++)
	{
		mFrameVertexBuffer[i] = buffers[i+6];
		// Create a vertex buffer & transfer vertices data 
		// from client space to server space.
		glBindBuffer(GL_ARRAY_BUFFER, mFrameVertexBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frame_vertices[i]) * 4, frame_vertices[i], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	for(int i=0; i<15; i++)
	{
		int k = ((frame_vertices[i][3]-frame_vertices[i][0])/(frame_vertices[i][10] - frame_vertices[i][1]) >= 1.3)? 2 : 
				((frame_vertices[i][3]-frame_vertices[i][0])/(frame_vertices[i][10] - frame_vertices[i][1]) <= 0.7)? 1 : 0;
		mTexcoodBuffer[i] = buffers[21+i];
		glBindBuffer(GL_ARRAY_BUFFER, mTexcoodBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord[0]), texcoord[k], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	// Create a color buffer & transfer color data 
	// from client space to server space.
	mColorBuffer = buffers[36];
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors) * 4, colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create a index buffer & transfer index data 
	// from client space to server space.
	mIndexBuffer = buffers[37];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	LoadGLTexture();
}

void onDisplay()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	onResize(width,height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(  ex,   ey,  0,
			  cenx, ceny, -1,
			      0,   1,  0);
	
	if(is_lRotate || is_rRotate) {
		current_time = clock();
		long elapse_time = current_time - start_time;

		if(elapse_time > animation_time) {
			is_lRotate = false;
			is_rRotate = false;
			angle = 0;
		}
		else {
			float delta = elapse_time / (float)animation_time;
			if(is_lRotate) 
				angle = ((1.0f-delta)*angle_rotate);
			else if(is_rRotate) 
				angle = ((delta)*angle_rotate);
		}
	}

	for(int i=0; i<6; i++)
	{
		glEnableClientState(GL_VERTEX_ARRAY);

		for(int j=0; j<=index[0][i]; j++)
		{
			int idx = index[1][i]+j;
			int k = 0;
			if(j==index[0][i])
			{
				glDisable(GL_TEXTURE_2D);
				glEnableClientState(GL_COLOR_ARRAY);

				glBindBuffer(GL_ARRAY_BUFFER, mBackVertexBuffer[i]);
				glVertexPointer(3, GL_FLOAT, 0, 0);
				glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
				glColorPointer(4, GL_FLOAT, 0, 0);
			}
			else
			{
				k = ((frame_vertices[idx][3]-frame_vertices[idx][0])/(frame_vertices[idx][10] - frame_vertices[idx][1]) >= 1.3)? 2 : 
					((frame_vertices[idx][3]-frame_vertices[idx][0])/(frame_vertices[idx][10] - frame_vertices[idx][1]) <= 0.7)? 1 : 0;

				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				
				glBindTexture(GL_TEXTURE_2D, texture_id[idx]);

				glBindBuffer(GL_ARRAY_BUFFER, mFrameVertexBuffer[idx]);
				glVertexPointer(3, GL_FLOAT, 0, 0);
				glBindBuffer(GL_ARRAY_BUFFER, mTexcoodBuffer[idx]);
				glTexCoordPointer(2, GL_FLOAT, 0, 0);
			}

			glPushMatrix();

			if(is_rRotate)
			{
				if((int)ztrans==0) ztrans=6;

				if(i<(int)ztrans-1) glTranslatef(0.0f, 0.0f, -7.0f+ztrans);
				else if(i==(int)ztrans-1)
				{
					glTranslatef(-1.15f, 0.0f, 0.0f);
					glRotatef(angle, 0, 1, 0);
					glTranslatef(1.15f, 0.0f, (GLfloat)i);
				}
				else glTranslatef(0.0f, 0.0f, ztrans-1.0f);

				ztrans=(GLfloat)((int)ztrans%6);
			}
			else if(is_lRotate)
			{
				if(i<(int)ztrans)
					glTranslatef(0.0f, 0.0f, -6.0f+ztrans);
				else if(i==(int)ztrans)
				{
					glTranslatef(-1.15f, 0.0f, 0.0f);
					glRotatef(angle, 0, 1, 0);
					glTranslatef(1.15f, 0.0f, (GLfloat)i);
				}
				else if(ztrans<i)
					glTranslatef(0.0f, 0.0f, ztrans);
			}
			else
			{
				if(i<(int)ztrans) glTranslatef(0.0f, 0.0f, -6.0f+ztrans);
				else glTranslatef(0.0f, 0.0f, ztrans);
			}		

			
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			gluLookAt(0,0,0,
					  0,0,-1,
					  0,1,0);
			glPushMatrix();

			if(k==1)
			{
				if(xtrans <= 0.5f)
					glTranslatef(xtrans, 0.0f, 0.0f);
				else
					glTranslatef(1.0f-xtrans, 0.0f, 0.0f);

				if(xtrans >= 1.0f)
					xtrans = 0.0f;
			}
			else if(k==2)
			{
				if(ytrans >= -0.5f)
					glTranslatef(0.0f, ytrans, 0.0f);
				else
					glTranslatef(0.0f, -1.0f-ytrans, 0.0f);

				if(ytrans <= -1.0f)
					ytrans = 0.0f;
			}
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
			if(j==index[0][i])
			{
				glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
				glEnable(GL_TEXTURE_2D);
			}
			else
				glDrawElements(GL_TRIANGLES, sizeof(indices)/2, GL_UNSIGNED_BYTE, 0);

			glPopMatrix();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		} // for(int j=0; j<index[i]; j++)

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} // for(int i=0; i<6; i++)
	
	glutSwapBuffers();

	xtrans += 0.005f;
	ytrans -= 0.005f;
}

int LoadGLTexture()
{
	for(int i=0; i<15; i++)
	{
		texture_id[i] = SOIL_load_OGL_texture( filename[i],
						SOIL_LOAD_AUTO,
						SOIL_CREATE_NEW_ID,
						SOIL_FLAG_INVERT_Y);

	if(texture_id[i] == 0)
		return false;

	glBindTexture(GL_TEXTURE_2D, texture_id[i]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	return true;
}

void mousewheel(int wheel, int direction, int x, int y)
{
	if(direction < 0)
	{
		othl = othdw *= 1.01;
		othr = othup *= 1.01;
	}

	else
	{
		othr = othup *=0.98;
		othl = othdw *=0.98;

		if(othr/1.15 <= 0.3)
		{
			othr = othup = 1.15f*0.3;
			othl = othdw = -1.15*0.3;
		}
	}
	
	if(othr - othl >= 2.3)
	{
		ex = cenx = 0;
		othr = 1.15f;
		othl = -1.15f;
	}
	if(othup - othdw >= 2.3)
	{
		ey = ceny = 0;
		othup = 1.15f;
		othdw = -1.15f;
	}
	if(ex+othr > 1.15f)
		ex = cenx -= (ex+othr-1.15);

	if(ey+othup > 1.15f)
		ey = ceny -= (ey+othup-1.15);

	if(ex+othl < -1.15f)
		ex = cenx -= (ex+othl+1.15);

	if(ey+othdw<-1.15f)
		ey = ceny -= (ey+othdw+1.15);
}

void MyMouseClick(GLint Button, GLint State, GLint x, GLint y)
{
	if(Button==GLUT_LEFT_BUTTON && State == GLUT_UP && x>384)
	{
		if(drag==false)
		{
			xtrans = ytrans = 0.0f;
			ex = ey = cenx = ceny = 0;
			othr = othup = 1.15f;
			othl = othdw = -1.15f;
			ztrans += 1.0f;
			if(ztrans>=6)
				ztrans = 0;

			start_time = clock();
			is_rRotate = true;
		}
		else
			drag=false;
	}
	else if(Button==GLUT_LEFT_BUTTON && State == GLUT_UP && x<128)
	{
		if(drag==false)
		{
			xtrans = ytrans = 0.0f;
			ex = ey = cenx = ceny = 0;
			othr = othup = 1.15f;
			othl = othdw = -1.15f;
			ztrans -= 1.0f;
			if(ztrans<0)
				ztrans = 5;

			start_time = clock();
			is_lRotate = true;
		}
		else
			drag=false;
	}

	StartX=x, StartY=y;
}

void MyMouseMove( GLint X,GLint Y)
{
	//limit boundary
	double limith =1.15;
	double limitl =-1.15;

	if(X!=StartX || Y!=StartY)
		drag=true;
	else
		return;

	ex = cenx -=(X-StartX)*0.0002f;
	ey = ceny +=(Y-StartY)*0.0002f;

	if(ex+othr >= limith)
		ex = cenx = limith-othr;

	if( ey+othup>=limith)
		ey = ceny =limith-othup;
		
	if(ex+othl<=limitl)
		ex = cenx = limitl-othl;
		
	if(ey+othdw<=limitl)
		ey= ceny = limitl-othdw;
}