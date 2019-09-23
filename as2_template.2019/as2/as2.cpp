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
int MOUSEONE = 1;
int MOUSETWO = 1;

int OLDX = 0;
int NEWX = 0;
int OLDY = 0;
int NEWY = 0;

float RHO = 5;
float PHI =  90*(3.14159/180);
float THETA = 90* (3.14159 / 180);

float WMatrix[16];

// Struct to keep track of cameras current position
typedef struct camera {
	float camerax, cameray, cameraz;
	float upx, upy, upz;
	float centerx, centery, centerz;
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

typedef float _matrix[16];

int verts, faces, norms;    // Number of vertices, faces and normals in the system
point *vertList, *normList; // Vertex and Normal Lists
faceStruct *faceList;	    // Face List
camera c;
point WOrigin;
float ObjectMatrix[16];

void myLookAt(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
void matrixMultiply(float*, float*);

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
	
	// Draws Object coordinates
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(ObjectMatrix[3], ObjectMatrix[7], ObjectMatrix[11]);
	glVertex3f(ObjectMatrix[3] + ObjectMatrix[0], ObjectMatrix[4] + ObjectMatrix[7], ObjectMatrix[8] + ObjectMatrix[11] + 1);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_LINES);
	glVertex3f(ObjectMatrix[3], ObjectMatrix[7], ObjectMatrix[11]);
	glVertex3f(ObjectMatrix[3] + ObjectMatrix[1], ObjectMatrix[7] + ObjectMatrix[5] + 1, ObjectMatrix[11] + ObjectMatrix[9]);
	glEnd();

	glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(ObjectMatrix[3], ObjectMatrix[7], ObjectMatrix[11]);
	glVertex3f(ObjectMatrix[3] + ObjectMatrix[2] + 1, ObjectMatrix[7] + ObjectMatrix[6], ObjectMatrix[11] + ObjectMatrix[10]);
	glEnd();


	// Draws World Coordinats
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(WOrigin.x, WOrigin.y, WOrigin.z);
	glVertex3f(WOrigin.x + 1, WOrigin.y, WOrigin.z);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_LINES);
	glVertex3f(WOrigin.x, WOrigin.y, WOrigin.z);
	glVertex3f(WOrigin.x, WOrigin.y + 1, WOrigin.z);
	glEnd();

	glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(WOrigin.x, WOrigin.y, WOrigin.z);
	glVertex3f(WOrigin.x, WOrigin.y, WOrigin.z + 1);
	glEnd();

}

void drawObjects(void) {
	
	int i;
	point p1, p2, p3;

	// Draw object from obj file
	
	/*
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
	*/
	
	glEnd(); for (int i = 0; i < faces; i++)
	{
		glColor3f(1.0f, 0.2f, 1.0f);
		glBegin(GL_TRIANGLES);
		glVertex3f(vertList[faceList[i].v1].x * ObjectMatrix[0] + vertList[faceList[i].v1].x * ObjectMatrix[1] + vertList[faceList[i].v1].x * ObjectMatrix[2] + ObjectMatrix[3], vertList[faceList[i].v1].y * ObjectMatrix[4] + vertList[faceList[i].v1].y * ObjectMatrix[5] + vertList[faceList[i].v1].y * ObjectMatrix[6] + ObjectMatrix[7], vertList[faceList[i].v1].z * ObjectMatrix[8] + vertList[faceList[i].v1].z * ObjectMatrix[9] + vertList[faceList[i].v1].z * ObjectMatrix[10] + ObjectMatrix[11]);
		glVertex3f(vertList[faceList[i].v2].x * ObjectMatrix[0] + vertList[faceList[i].v2].x * ObjectMatrix[1] + vertList[faceList[i].v2].x * ObjectMatrix[2] + ObjectMatrix[3], vertList[faceList[i].v2].y * ObjectMatrix[4] + vertList[faceList[i].v2].y * ObjectMatrix[5] + vertList[faceList[i].v2].y * ObjectMatrix[6] + ObjectMatrix[7], vertList[faceList[i].v2].z * ObjectMatrix[8] + vertList[faceList[i].v2].z * ObjectMatrix[9] + vertList[faceList[i].v2].z * ObjectMatrix[10] + ObjectMatrix[11]);
		glVertex3f(vertList[faceList[i].v3].x * ObjectMatrix[0] + vertList[faceList[i].v3].x * ObjectMatrix[1] + vertList[faceList[i].v3].x * ObjectMatrix[2] + ObjectMatrix[3], vertList[faceList[i].v3].y * ObjectMatrix[4] + vertList[faceList[i].v3].y * ObjectMatrix[5] + vertList[faceList[i].v3].y * ObjectMatrix[6] + ObjectMatrix[7], vertList[faceList[i].v3].z * ObjectMatrix[8] + vertList[faceList[i].v3].z * ObjectMatrix[9] + vertList[faceList[i].v3].z * ObjectMatrix[10] + ObjectMatrix[11]);
		glEnd();
	}
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

	myLookAt(c.camerax, c.cameray, c.cameraz, c.centerx, c.centery, c.centerz, c.upx, c.upy, c.upz);
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
	if (button == 1) {
		MOUSEONE = !state;
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
			THETA = THETA - 3;
		}
		if (OLDY < NEWY) {
			THETA = THETA + 3;
		}
		if (OLDX > NEWX) {
			PHI = PHI - 2;
		}
		if (OLDX < NEWX) {
			PHI = PHI + 2;
		}
	}
	else if (MOUSEONE) {

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

void myTranslate(float tMatrix[16]) {
	//float _cMatrix[16];
	//glGetFloatv(GL_MODELVIEW_MATRIX, _cMatrix);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(tMatrix);
	//matrixMultiply(tMatrix, _cMatrix);
	glMultMatrixf(tMatrix);
}


void myLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
	GLfloat centerx, GLfloat centery, GLfloat centerz,
	GLfloat upx, GLfloat upy, GLfloat upz)
{

	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) {			/* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	 * non-perpendicular unit-length vectors; so normalize x, y here
	 */

	mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

#define M(row,col)  m[col*4+row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
	glMultMatrixf(m);

	// Translate Eye to Origin 
	glTranslatef(-eyex, -eyey, -eyez);
	//float tMatrix[16] = { 1, 0, 0, -eyex, 0, 1, 0, -eyey, 0, 0, 1, -eyez, 0, 0, 0, 1 };
	//myTranslate(tMatrix);
}



void matrixMultiply(float m1[16], float m2[16]) {

	int row;
	int col;
	_matrix temp;

	int i = 0;

	for (row = 0; row <= 12; row = row + 4) {
		for (col = 0; col < 4; col++) {
			temp[i] = m1[row] * m2[col] + m1[row + 1] * m2[col + 4] + m1[row + 2] * m2[col + 8] + m1[row + 3] * m2[col + 12];
			i++;
		}
	}

	printf("Here is the matrix\n");
	for (row = 0; row < 16; row++) {
		m2[row] = temp[row];
		printf("%f\n",m2[row]);
	}
	printf("End\n");
}

void makeIdentity(float matrix[16]) {
	int i;
	for (i = 0; i < 16; i++) {
		if (i % 5 == 0)
			matrix[i] = 1;
		else
			matrix[i] = 0;
	}
}

point myCrossProduct(point p1, point p2) {
	point cross;
	cross.x = p1.y * p2.z - p1.z * p2.y;
	cross.y = p1.z * p2.x - p1.x * p2.z;
	cross.z = p1.x * p2.y - p1.y * p2.x;
	return cross;
}

float myDotProductViewTrans(point p1, point p2) {
	float result;

	result = -p1.x * p2.x + -p1.y * p2.y + -p1.z * p2.z;

	return result;
}

/*
void myLookAt(GLfloat cameraX, GLfloat cameraY, GLfloat cameraZ, GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat upX, GLfloat upY, GLfloat upZ) {
	float Mvw[16];
	makeIdentity(Mvw);

	point N;
	point n;
	point u;
	point v;

	// Calculations for n vector
	N.x = centerX - cameraX;
	N.y = centerY - cameraY;
	N.z = centerZ - cameraZ;

	float magN = sqrt(pow(N.x, 2) + pow(N.y, 2) + pow(N.z, 2));

	n.x = N.x / magN;
	n.y = N.y / magN;
	n.z = N.z / magN;


	point V;
	V.x = upX;
	V.y = upY;
	V.z = upZ;
	// Calculations for u vectors
	point Vn = myCrossProduct(V, n);

	float magVn = sqrt(pow(Vn.x, 2) + pow(Vn.y, 2) + pow(Vn.z, 2));

	u.x = Vn.x / magVn;
	u.y = Vn.y / magVn;
	u.z = Vn.z / magVn;

	v = myCrossProduct(n, u);

	Mvw[0] = u.x;
	Mvw[1] = u.y;
	Mvw[2] = u.z;
	Mvw[3] = myDotProductViewTrans(u, N);
	Mvw[4] = v.x;
	Mvw[5] = v.y;
	Mvw[6] = v.z;
	Mvw[7] = myDotProductViewTrans(v, N);
	Mvw[8] = n.x;
	Mvw[9] = n.y;
	Mvw[10] = n.z;
	Mvw[11] = myDotProductViewTrans(n, N);
	Mvw[12] = 0;
	Mvw[13] = 0;
	Mvw[14] = 0;
	Mvw[15] = 1;

	glMultMatrixf(Mvw);

	glTranslatef(cameraX, cameraY, cameraZ);
}
*/

// Here's the main
int main(int argc, char* argv[])
{
	meshReader("teapot.obj", 0);

	// For testing
	float m1[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	float m2[16] = { 100, 200, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	int i;

	matrixMultiply(m1, m2);

	makeIdentity(m1);
	for (i = 0; i < 16; i++) {
		printf("%f\n", m1[i]);
	}


	WOrigin.x = WOrigin.y = WOrigin.z = 0;
	makeIdentity(ObjectMatrix);

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
