#include<stdio.h>
#include<GL/glut.h>
#include<math.h>
#include <irrKlang.h>

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

//Color
GLfloat WHITE[] = { 1,1,1 };
GLfloat RED[] = { 1,0,0 };
GLfloat GREEN[] = { 0,1,0 };
GLfloat MAGENTA[] = { 1,0,1 };

//A camera

class Camera {
	double theta;
	double y;
	double dTheta;
	double dy;

public:
	Camera(){
		theta = 0;
		y = 3;
		dTheta = 0.04;
		dy = 0.2;
	}
	double getX() { return 10 * cos(theta); }
	double getY() { return y; }
	double getZ() { return 10 * sin(theta); }
	void moveRight() { theta += dTheta; }
	void moveLeft() { theta -= dTheta; }
	void moveUp() { y += dy; }
	void moveDown() { if (y > dy) y -= dy; }
};

//A ball

class Ball {
	double radius;
	GLfloat* color;
	double coeff_of_restitution;
	//double gravity;
	double maximumHeight;
	double x;
	double y;
	double z;
	//double ui,vi;
	int direction;
	int n,m;
	bool inAir = true;
public:
	Ball(double r, GLfloat* c, double h, double x, double z, double coeff_of_restitution){
		radius = r;
		color = c;
		maximumHeight = h;
		direction = -1;
		y = h;
		this->x = x;
		this->z = z;
		n = 0;
		m = 0;
		this->coeff_of_restitution = coeff_of_restitution;
		//gravity = 9.8;
		//ui = 0.0;
		//vi = sqrt(2.0 * gravity * h);
	}

	void update() {
		//++m;
		//y += (ui*(0.001)*m) + (direction * (0.5) * gravity * pow(static_cast<double>(m) * (0.001), 2));
		
		y += direction * 0.05;
		//maximumHeight = (maximumHeight - radius) * pow(coeff_of_restitution, n) + radius;
		if (y >= maximumHeight && direction==1) {
			y = maximumHeight;
			inAir = (y > radius ? true : false);
			direction = -1;
			//maximumHeight = (maximumHeight-radius)*pow(coeff_of_restitution, n++) + radius;
			//ui = 0.0;
		}
		else if (y <= radius) {
			y = radius;
			direction = 1;
			maximumHeight = ((maximumHeight - radius) * pow(coeff_of_restitution, ++n)) + radius;
			if (inAir) {
				inAir = false;
				engine->play2D("../../media/bell.wav");
			}
			//vi = gravity*m*(1.0/600.0);
			//ui = coeff_of_restitution * vi;
			//ui = sqrt(2.0 * gravity * maximumHeight*1000000);
			//m = 0;
			//vi = ui;
		}
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glTranslated(x, y, z);
		glutSolidSphere(radius, 30, 30);
		glPopMatrix();
	}
};

//A checkerboard
class Checkerboard {
	int displayListId;
	int width;
	int depth;
public:
	Checkerboard(int width, int depth){
		this->width = width;
		this->depth = depth;
	}
	double centerx() { return width / 2; }
	double centerz() { return depth / 2; }
	void create() {
		displayListId = glGenLists(1);
		glNewList(displayListId, GL_COMPILE);
		//GLfloat lightPosition[] = { 4,3,7,1 };
		//glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glBegin(GL_QUADS);
		glNormal3d(0, 1, 0);
		for (int x = 0; x < width - 1; x++) {
			for (int z = 0; z < depth - 1; z++) {
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
					(x + z) % 2 == 0 ? RED : WHITE);
				glVertex3d(x, 0, z);
				glVertex3d(x + 1, 0, z);
				glVertex3d(x + 1, 0, z + 1);
				glVertex3d(x, 0, z + 1);

			}
		}
		glEnd();
		glEndList();
	}

	void draw() {
		glCallList(displayListId);
	}
};

//Global variable: a camera, a checkerboard and some balls
Checkerboard checkerboard(10, 10);
Camera camera;
Ball balls[] = {
	Ball(0.5,GREEN,5,6,1,0.8),
	Ball(0.5,MAGENTA,5,3,4,0.0),
	Ball(0.5,WHITE,5,1,7,1.0)
};

//Application-specific initialization: Set up global lighting parameters
//and create display lists
void init() {
	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, WHITE);
	glLightfv(GL_LIGHT0, GL_SPECULAR, WHITE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, 30);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	checkerboard.create();
}

//Draws one frame, the checkerboard then the balls, from the current camera position
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera.getX(), camera.getY(), camera.getZ(),
		checkerboard.centerx(), 0.0, checkerboard.centerz(),
		0.0, 1.0, 0.0);
	checkerboard.draw();
	for (int i = 0; i < sizeof(balls) / sizeof(Ball); i++) {
		balls[i].update();
	}
	glFlush();
	glutSwapBuffers();
}

//On reshape constructs a camera that perfectly fits the window
void reshape(GLint w, GLint h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, GLfloat(w) / GLfloat(h), 1.0, 150.0);
	glMatrixMode(GL_MODELVIEW);
}

//Requests to draw the next frame
void timer(int v) {
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, timer, v);
}

//Moves the camera according to key presses, then ask to refresh the display.
void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT: camera.moveLeft(); break;
	case GLUT_KEY_RIGHT: camera.moveRight(); break;
	case GLUT_KEY_UP: camera.moveUp(); break;
	case GLUT_KEY_DOWN: camera.moveDown(); break;
	}
	glutPostRedisplay();
}

//Initializes GLUT and enters the main loop
int main(int argc, char** argv) {
	irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Bouncing Balls");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutTimerFunc(100, timer, 0);
	init();
	glutMainLoop();
	engine->drop(); // delete engine
	return 0;
}
