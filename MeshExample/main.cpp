#include "mesh.h"
#include "glut.h"
#include <cstdio>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <math.h>
std::vector<mesh*> objects ;
//mesh *object;

int windowSize[2];


void display();
void reshape(GLsizei , GLsizei );
void keyboard(unsigned char, int, int);
void mouse(int x, int y);

//////////////////////////////// LOAD /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void loadLight(char* lightFile);
void loadView(char* viewFile);
void loadScene(char* sceneFile);


struct View{	/*int*/
	int eye[3];
	int vat[3];
	int vup[3];
	int fovy;
	int dnear;
	int dfar;
	int viewport[4];
};

struct Light{	/*float*/
	float x, y, z, ar, ag, ab, dr, dg, db, sr, sg, sb;
};

struct Ambient{	/*float*/
	float r, g, b;
};

struct Model{	/*float*/
	char objName[20];
	float Sx, Sy, Sz, angle, Rx, Ry, Rz, Tx, Ty, Tz;
};

/* Variables */
int xdif, ydif, zdif;
int dis; //camera to center
double pi = 3.14;
double theta;//camera - center 在極座標系的初始關係
struct View view;
struct Light light[8];
int lightCount = 0;
struct Ambient ambient; //from environment
struct Model model[100]; //assume no more than 100 model in one scene
int modelCount = 0;
int drag = -1;//choose with keyboard '0'~'9', use mouse to drag selected object
int old_mouse_x = -1;
int old_mouse_y = -1;
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
	char sceneFileName[30];
	char viewFileName[30];
	char lightFileName[30];
	printf("Please enter scene name: ");
	scanf("%s", sceneFileName);
	printf("Please enter view name: ");
	scanf("%s", viewFileName);
	printf("Please enter light name: ");
	scanf("%s", lightFileName);
	loadScene(sceneFileName);
	loadView(viewFileName);
	loadLight(lightFileName);
	
	//distance from camera to center, for keyboard rotate
	xdif = view.eye[0] - view.vat[0];
	ydif = view.eye[1] - view.vat[1];
	zdif = view.eye[2] - view.vat[2];
	dis = sqrt(xdif*xdif + ydif*ydif + zdif*zdif); //極座標系中的 r
	theta = pi / 2;//camera - center 在極座標系的初始關係
	printf("dis = %d\n", dis);
	printf("cos(theta) = %f\n", cos(theta) );
	printf("current camera x = %d\n",view.eye[0]);
	printf("current camera z = %d\n\n\n", view.eye[2]);


	for (int i = 0; i < modelCount; i++){
		printf("%s\n", model[i].objName);//c
		objects.push_back ( new mesh(model[i].objName) );
	}
			//object = new mesh("venus.obj");
	/*load material files of object
	Example:
	materialManager.SetUpTexture(newmtlName, textureFilename);
	*/

	//objects[o]->mList[lastMaterial].map_Kd;


	glutInit(&argc, argv);
	glutInitWindowSize(view.viewport[2], view.viewport[3]);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("Mesh Example");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	
	glutMainLoop();

	return 0;
}

void setLight()
{
	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);
	// set light source 0
	GLfloat light0_specular[] = { light[0].sr, light[0].sg, light[0].sb, 1.0 };
	GLfloat light0_diffuse[] = { light[0].dr, light[0].dg, light[0].db, 1.0 };
	GLfloat light0_ambient[] = { light[0].ar, light[0].ag, light[0].ab, 1.0 };
	GLfloat light0_position[] = { light[0].x, light[0].y, light[0].z, 1.0 };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	// set light source 1
	GLfloat light1_specular[] = { light[1].sr, light[1].sg, light[1].sb, 1.0 };
	GLfloat light1_diffuse[] = { light[1].dr, light[1].dg, light[1].db, 1.0 };
	GLfloat light1_ambient[] = { light[1].ar, light[1].ag, light[1].ab, 1.0 };
	GLfloat light1_position[] = { light[1].x, light[1].y, light[1].z, 1.0 };
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	//set environment light
	GLfloat a[] = { ambient.r, ambient.g, ambient.b, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      // 清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer

	// viewport transformation
	glViewport(view.viewport[0], view.viewport[1], view.viewport[2], view.viewport[3]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(view.fovy, (GLfloat)windowSize[0]/(GLfloat)windowSize[1], view.dnear, view.dfar);
	
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(view.eye[0], view.eye[1], view.eye[2],// eye
				view.vat[0], view.vat[1], view.vat[2],// center
				view.vup[0], view.vup[1], view.vup[2]);// up

	//注意light位置的設定，要在gluLookAt之後
	setLight();

	int objectsSize = objects.size();
	for (int o = 0; o < objectsSize; o++){

		int lastMaterial = -1; //?
		//draw all polygons of one object
		for (size_t i = 0; i < objects[o]->fTotal; ++i)
		{
			// set material property if this face used different material
			if (lastMaterial != objects[o]->faceList[i].m)
			{
				lastMaterial = (int)objects[o]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, objects[o]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, objects[o]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, objects[o]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &objects[o]->mList[lastMaterial].Ns);

				//you can obtain the "texture name" by objects[o]->mList[lastMaterial].map_Kd
				//load them once in the main function before mainloop
				//bind them in display function here
				
				/*!! 在 main loop 前load texture*/
				/*!! 在這裡 bind texture*/

			}
			/*modeling transformation*/
			glPushMatrix();
			glTranslatef(model[o].Tx, model[o].Ty, model[o].Tz);
			glRotatef(model[o].angle, model[o].Rx, model[o].Ry, model[o].Rz);
			glScalef(model[o].Sx, model[o].Sy, model[o].Sz);
			//write the model
			glBegin(GL_TRIANGLES);
			for (size_t j = 0; j < 3; ++j)
			{
					//textex corrd. objects[o]->tList[objects[o]->faceList[i][j].t].ptr
					glNormal3fv(objects[o]->nList[objects[o]->faceList[i][j].n].ptr);
					glVertex3fv(objects[o]->vList[objects[o]->faceList[i][j].v].ptr);
			}
			glEnd();

			glPopMatrix();
		}
	}
	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}
// GLUT keyboard function
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':	//zoom in   
		//view.eye[2] += 2;
		if (xdif > 0.2 && ydif > 0.2 && zdif > 0.2){
			xdif = (view.eye[0] - view.vat[0]) * 0.8;
			ydif = (view.eye[1] - view.vat[1]) * 0.8;
			zdif = (view.eye[2] - view.vat[2]) * 0.8;
		}
		dis = sqrt(xdif*xdif + ydif*ydif + zdif*zdif);//相機與中心點的距離變了,必須更新距離
		view.eye[0] = view.vat[0] + xdif;
		view.eye[1] = view.vat[1] + ydif;
		view.eye[2] = view.vat[2] + zdif;
		glutPostRedisplay();
		break;
	case 's':	//zoom out   
		xdif = (view.eye[0] - view.vat[0]) * 1.2;
		ydif = (view.eye[1] - view.vat[1]) * 1.2;
		zdif = (view.eye[2] - view.vat[2]) * 1.2;
		dis = sqrt(xdif*xdif + ydif*ydif + zdif*zdif);//相機與中心點的距離變了,必須更新距離
		view.eye[0] = view.vat[0] + xdif;
		view.eye[1] = view.vat[1] + ydif;
		view.eye[2] = view.vat[2] + zdif;
		glutPostRedisplay();
		break;
	case 'a':	//rotate left
		theta += pi / 200;
		if (theta >= 2*pi) theta = 0;
		printf("theta = %f \n", theta);
		view.eye[0] = dis*cos(theta);
		view.eye[2] = dis*sin(theta);
		glutPostRedisplay();
		break;
	case 'd':	//rotate right
		theta -= pi / 200;
		if (theta <= -2*pi) theta = 0;
		printf("theta = %f \n", theta);
		view.eye[0] = dis*cos(theta);
		view.eye[2] = dis*sin(theta);
		glutPostRedisplay();
		break;
	default:	//choose which object to drag 
		if ('1' <= key && key <= '9'){
			drag = key - '1'; //start from 0
			printf("Model %d, [ %s ] is selected!\n ", drag, model[drag].objName);
		}
		else{
			printf("not functional key, 87777\n");
		}
		glutPostRedisplay();
	}
}
/*
- Right : Increase x
- Left : Decrease x
- Up : Increase y
- Down : Decrease y
*/

int x_move;
int y_move;

void mouse(int x, int y){
	//x+=x位移*dnear*dnear/100
	if (drag != -1){
		if (old_mouse_x == -1 && old_mouse_y == -1){
			old_mouse_x = x;
			old_mouse_y = y;
			printf("current mouse at ( %d,%d )\n", old_mouse_x, old_mouse_y);
		}
		//else if (drag != -1 && (abs(x_move)>10 || abs(y_move)>10) ){
		else{
			x_move = (x - old_mouse_x) * view.dnear * view.dnear / 100;
			y_move = (y - old_mouse_y) * view.dnear * view.dnear / 100;
			model[drag].Tx += x_move;
			model[drag].Ty -= y_move;
			printf("Object is at (%d, %d)\n", x, y, model[drag].Tx, model[drag].Ty);
			printf("drag === ( %d,%d ) <--- ( %d,%d )\n", x, y, old_mouse_x, old_mouse_y);
			old_mouse_x = x;
			old_mouse_y = y;
			glutPostRedisplay();
		}
	}

	
}


/* Load Files*/
void loadScene(char* fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	modelCount = 0;
	fin = fopen(fileName, "r");
	if (fin == NULL){ printf("Cannot open in file. import\n"); return ; }

	while(fgets(tmps, sizeof(tmps), fin)){

		word = strtok(tmps, " ");
		//model house.obj 1.0 1.0 1.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0
		//obj file name
		word = strtok(NULL, "\n ");
		strcpy(model[modelCount].objName, word);
		//Sx Sy Sz Angle Rx Ry Rz Tx Ty Tz
		word = strtok(NULL, "\n ");
		model[modelCount].Sx = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Sy = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Sz = atof(word);
		//angle Rx Ry Rz
		word = strtok(NULL, "\n ");
		model[modelCount].angle = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Rx = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Ry = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Rz = atof(word);
		//Tx Ty Tz
		word = strtok(NULL, "\n ");
		model[modelCount].Tx = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Ty = atof(word);
		word = strtok(NULL, "\n ");
		model[modelCount].Tz = atof(word);

		modelCount++;
	}
	printf("Model Count: %d\n", modelCount);
	fclose(fin);
}

void loadView(char* fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	fin = fopen(fileName, "r");
	if (fin == NULL){ printf("Cannot open in file. import\n"); return; }
	//eye
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	for (int i = 0; i < 3; i++){ word = strtok(NULL, "\n ");	view.eye[i] = atoi(word); }
	//vat
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	for (int i = 0; i < 3; i++){ word = strtok(NULL, "\n ");	view.vat[i] = atoi(word); }
	//vup
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	for (int i = 0; i < 3; i++){ word = strtok(NULL, "\n ");	view.vup[i] = atoi(word); }
	//fovy
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	word = strtok(NULL, "\n ");	view.fovy = atoi(word);
	//dnear
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	word = strtok(NULL, "\n ");	view.dnear = atoi(word);
	//dfar
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	word = strtok(NULL, "\n ");	view.dfar = atoi(word);
	//vup
	fgets(tmps, sizeof(tmps), fin);
	word = strtok(tmps, " ");
	for (int i = 0; i < 4; i++){ word = strtok(NULL, "\n ");	view.viewport[i] = atoi(word); }

	fclose(fin);
}

void loadLight(char* fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	fin = fopen(fileName, "r");
	if (fin == NULL) printf("Cannot open in file. import\n");
	//light

	//fgets(buf,100,scene);

	while(fgets(tmps, sizeof(tmps), fin)) {
		word = strtok(tmps, " ");

		if (strcmp(word, "light") == 0){
			// x y z ar ag ab dr dg db sr sg sb
			word = strtok(NULL, "\n ");
			light[lightCount].x = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].y = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].z = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].ar = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].ag = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].ab = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].dr = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].dg = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].db = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].sr = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].sg = atof(word);
			word = strtok(NULL, "\n ");
			light[lightCount].sb = atof(word);

			lightCount++;
		}
		else if (strcmp(word, "ambient") == 0){
			//r, g, b
			word = strtok(NULL, "\n ");
			ambient.r = atof(word);
			word = strtok(NULL, "\n ");
			ambient.g = atof(word);
			word = strtok(NULL, "\n ");
			ambient.b = atof(word);
		}
		else{
			printf("Something go wrong with light file!\n");
		}
	}

	fclose(fin);
}
