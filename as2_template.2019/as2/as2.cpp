// The template code for Assignment 2 
//

#include <stdio.h>
#include <stdlib.h>
#include <GL\glut.h>
#include <math.h>

#define ON 1
#define OFF 0


// Global variables
int window_width, window_height;    // Window dimensions
int PERSPECTIVE = OFF;
int COORDINATES = ON;
int OBJECTS = ON;
int MOUSEZERO = 1;
int MOUSETWO = 1;

int OLDX = 0;
int NEWX = 0;
int OLDY = 0;
int NEWY = 0;

float RHO = 10;
float PHI =  90*(3.14159/180);
float THETA = 90* (3.14159 / 180);

// Struct to keep track of cameras current position
typedef struct camera {
	float camerax, cameray, cameraz;
	float upx, upy, upz;
};

// Vertex and Face data structure sued in the mesh reader
// Feel free to change them
typedef struct _point {
  float x,y,z;
} point;

typedef struct _faceStruct {
  int v1,v2,v3;
  int n1,n2,n3;
} faceStruct;

int verts, faces, norms;    // Number of vertices, faces and normals in the system
point *vertList, *normList; // Vertex and Normal Lists
faceStruct *faceList;	    // Face List
camera c;

// The mesh reader itself
// It can read *very* simple obj files
void meshReader (char *filename,int sign)
{
  float x,y,z,len;
  int i;
  char letter;
  point v1,v2,crossP;
  int ix,iy,iz;
  int *normCount;
  FILE *fp;

  fp = fopen(filename, "r");
  if (fp == NULL) { 
    printf("Cannot open %s\n!", filename);
    exit(0);
  }

  // Count the number of vertices and faces
  while(!feof(fp))
    {
      fscanf(fp,"%c %f %f %f\n",&letter,&x,&y,&z);
      if (letter == 'v')
	{
	  verts++;
	}
      else
	{
	  faces++;
	}
    }

  fclose(fp);

  printf("verts : %d\n", verts);
  printf("faces : %d\n", faces);

  // Dynamic allocation of vertex and face lists
  faceList = (faceStruct *)malloc(sizeof(faceStruct)*faces);
  vertList = (point *)malloc(sizeof(point)*verts);
  normList = (point *)malloc(sizeof(point)*verts);

  fp = fopen(filename, "r");

  // Read the veritces
  for(i = 0;i < verts;i++)
    {
      fscanf(fp,"%c %f %f %f\n",&letter,&x,&y,&z);
      vertList[i].x = x;
      vertList[i].y = y;
      vertList[i].z = z;
    }

  // Read the faces
  for(i = 0;i < faces;i++)
    {
      fscanf(fp,"%c %d %d %d\n",&letter,&ix,&iy,&iz);
      faceList[i].v1 = ix - 1;
      faceList[i].v2 = iy - 1;
      faceList[i].v3 = iz - 1;
    }
  fclose(fp);


  // The part below calculates the normals of each vertex
  normCount = (int *)malloc(sizeof(int)*verts);
  for (i = 0;i < verts;i++)
    {
      normList[i].x = normList[i].y = normList[i].z = 0.0;
      normCount[i] = 0;
    }

  for(i = 0;i < faces;i++)
    {
      v1.x = vertList[faceList[i].v2].x - vertList[faceList[i].v1].x;
      v1.y = vertList[faceList[i].v2].y - vertList[faceList[i].v1].y;
      v1.z = vertList[faceList[i].v2].z - vertList[faceList[i].v1].z;
      v2.x = vertList[faceList[i].v3].x - vertList[faceList[i].v2].x;
      v2.y = vertList[faceList[i].v3].y - vertList[faceList[i].v2].y;
      v2.z = vertList[faceList[i].v3].z - vertList[faceList[i].v2].z;

      crossP.x = v1.y*v2.z - v1.z*v2.y;
      crossP.y = v1.z*v2.x - v1.x*v2.z;
      crossP.z = v1.x*v2.y - v1.y*v2.x;

      len = sqrt(crossP.x*crossP.x + crossP.y*crossP.y + crossP.z*crossP.z);

      crossP.x = -crossP.x/len;
      crossP.y = -crossP.y/len;
      crossP.z = -crossP.z/len;

      normList[faceList[i].v1].x = normList[faceList[i].v1].x + crossP.x;
      normList[faceList[i].v1].y = normList[faceList[i].v1].y + crossP.y;
      normList[faceList[i].v1].z = normList[faceList[i].v1].z + crossP.z;
      normList[faceList[i].v2].x = normList[faceList[i].v2].x + crossP.x;
      normList[faceList[i].v2].y = normList[faceList[i].v2].y + crossP.y;
      normList[faceList[i].v2].z = normList[faceList[i].v2].z + crossP.z;
      normList[faceList[i].v3].x = normList[faceList[i].v3].x + crossP.x;
      normList[faceList[i].v3].y = normList[faceList[i].v3].y + crossP.y;
      normList[faceList[i].v3].z = normList[faceList[i].v3].z + crossP.z;
      normCount[faceList[i].v1]++;
      normCount[faceList[i].v2]++;
      normCount[faceList[i].v3]++;
    }
  for (i = 0;i < verts;i++)
    {
      normList[i].x = (float)sign*normList[i].x / (float)normCount[i];
      normList[i].y = (float)sign*normList[i].y / (float)normCount[i];
      normList[i].z = (float)sign*normList[i].z / (float)normCount[i];
    }

}


void drawCoordinateAxis(void)
{

	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glEnd();

	glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

}

void drawObjects(void) {
	
	int i;
	point p1, p2, p3;

	// Draw object from obj file
	glColor3f(1, 0.2, 1);
	glBegin(GL_POLYGON);
	for (i = 0; i < faces; i++) {
		p1.x = vertList[faceList[i].v1].x;
		p1.y = vertList[faceList[i].v1].y;
		p1.z = vertList[faceList[i].v1].z;

		p2.x = vertList[faceList[i].v2].x;
		p2.y = vertList[faceList[i].v2].y;
		p2.z = vertList[faceList[i].v2].z;

		p3.x = vertList[faceList[i].v3].x;
		p3.y = vertList[faceList[i].v3].y;
		p3.z = vertList[faceList[i].v3].z;

		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
	}
	glEnd();
}

// Calculates the cameras position
void positionCamera(void) {
	if (RHO < 0)
		RHO = 0;
	c.camerax = RHO * sin(THETA * (3.14159 / 180)) * sin(PHI * (3.14159 / 180));
	c.cameray = RHO * cos(THETA * (3.14159 / 180));
	c.cameraz = RHO * sin(THETA * (3.14159 / 180)) * cos(PHI * (3.14159 / 180));

	c.upx = (RHO * sin(THETA * (3.14159 / 180) - 1) * sin(PHI * (3.14159 / 180))) - c.camerax;
	c.upy = (RHO * cos(THETA * (3.14159 / 180) - 1)) - c.cameray;
	c.upz = (RHO * sin(THETA * (3.14159 / 180) - 1) * cos(PHI * (3.14159 / 180))) - c.cameraz;

	printf("rho %p phicos %p phisin %p \n", RHO, sin(PHI * (3.14159 / 180)), cos(PHI * (3.14159 / 180)));

	gluLookAt(c.camerax, c.cameray, c.cameraz, 0, 0, 0, c.upx, c.upy, c.upz);
}

// The display function. It is called whenever the window needs
// redrawing (ie: overlapping window moves, resize, maximize)
// You should redraw your polygons here
void	display(void)
{
    // Clear the background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
	if (PERSPECTIVE) {
		// Perpective Projection 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)window_width / window_height, 0.01, 10000);
		glutSetWindowTitle("Assignment 2 Template (perspective)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Set the camera position, orientation and target
	}
	else {
		// Orthogonal Projection 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-2.5, 2.5, -2.5, 2.5, -10000, 10000);
		glutSetWindowTitle("Assignment 2 Template (orthogonal)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	positionCamera();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw the objects
	if (OBJECTS)
		drawObjects();

	// Draws the coordinate axis
	if (COORDINATES)
		drawCoordinateAxis();

    // (Note that the origin is lower left corner)
    // (Note also that the window spans (0,1) )
    // Finish drawing, update the frame buffer, and swap buffers
    glutSwapBuffers();
}


// This function is called whenever the window is resized. 
// Parameters are the new dimentions of the window
void	resize(int x,int y)
{
    glViewport(0,0,x,y);
    window_width = x;
    window_height = y;
    if (PERSPECTIVE) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60,(GLdouble) window_width/window_height,0.01, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
    }
    printf("Resized to %d %d\n",x,y);
}


// This function is called whenever the mouse is pressed or released
// button is a number 0 to 2 designating the button
// state is 1 for release 0 for press event
// x and y are the location of the mouse (in window-relative coordinates)
void	mouseButton(int button,int state,int x,int y)
{
	OLDX = x;
	OLDY = y;
	if (button == 0) {
		MOUSEZERO = !state;
	}
	if (button == 2) {
		MOUSETWO = !state;
	}
    printf("Mouse click at %d %d, button: %d, state %d\n",x,y,button,state);
	printf("Current rho: %p", RHO);
}


//This function is called whenever the mouse is moved with a mouse button held down.
// x and y are the location of the mouse (in window-relative coordinates)
void	mouseMotion(int x, int y)
{
	OLDX = NEWX;
	NEWX = x;
	OLDY = NEWY;
	NEWY = y;
	if (MOUSEZERO){
		if (OLDY > NEWY) {
			THETA = THETA + 3;
		}
		if (OLDY < NEWY) {
			THETA = THETA - 3;
		}
		if (OLDX > NEWX) {
			PHI = PHI + 2;
		}
		if (OLDX < NEWX) {
			PHI = PHI - 2;
		}

	}
	else if (MOUSETWO) {
		if (OLDY > NEWY) {
			RHO = RHO - .2;
		}
		else if (OLDY < NEWY) {
			RHO = RHO + .2;
		}
	}
	display();
	printf("Mouse is at %d, %d\n", x, y);
}


// This function is called whenever there is a keyboard input
// key is the ASCII value of the key pressed
// x and y are the location of the mouse
void	keyboard(unsigned char key, int x, int y)
{
    switch(key) {
    case '':
	case 'q':
	case 'Q':                           /* Quit */
		exit(1);
		break;
    case 'p':
    case 'P':
	// Toggle Projection Type (orthogonal, perspective)
		if (PERSPECTIVE) {
			// switch from perspective to orthogonal
			PERSPECTIVE = OFF;
		}
		else {
			// switch from orthogonal to perspective
			PERSPECTIVE = ON;
		}
		break;
	case 'a':
	case 'A':
		if (COORDINATES) {
			COORDINATES = OFF;
		}
		else {
			COORDINATES = ON;
		}
		break;
	case 's':
	case 'S':
		if (OBJECTS) {
			OBJECTS = OFF;
		}
		else {
			OBJECTS = ON;
		}
		break;
    default:
		break;
    }

    // Schedule a new display event
    glutPostRedisplay();
}


// Here's the main
int main(int argc, char* argv[])
{
	meshReader("teapot.obj", 0);

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Assignment 2 Template (orthogonal)");
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    // Initialize GL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.5,2.5,-2.5,2.5,-10000,10000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    // Switch to main loop
    glutMainLoop();
    return 0;        
}
