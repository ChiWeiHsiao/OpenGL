//#include "textureLoader.h"
#include "mesh.h"
#include "FreeImage.h"
#include "glew.h"
#include "glut.h"
#include <cstdio>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
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

enum TexType{ no, single, multi, cube };
struct Model{	/*float*/
	char objName[20];
	float Sx, Sy, Sz, angle, Rx, Ry, Rz, Tx, Ty, Tz;
	bool useNewTexture = false;
	bool isLast = false;
	enum TexType type;
	int texSetID; //specify which texSet do this model use;
	//enum TexType type;
	//std::vector <std::string> texNames; //a vector for list of textute names
	
};


struct SingleTex{
	char filename[20];
	bool loaded = false;
	GLuint texObject;//?
};
struct MultiTex{
	char filename[2][20];
	bool loaded = false;
	GLuint texObject[2];
};
struct CubeTex{
	//char filename[6][30];
	char f1[30], f2[30], f3[30], f4[30], f5[30], f6[30];
	bool loaded = false;
	GLuint texObject;// [6];
};
struct SingleTex texSingle[20];
struct MultiTex texMulti[10];
struct CubeTex texCube[2];
int cnt_single, cnt_multi, cnt_cube;
////////////////////////////////////////////////////////////////////////
/* Variables */
std::vector<mesh*> objects;
int xdif, ydif, zdif;
int dis; //camera to center (x,z)
double pi = 3.14;
double theta;//the angle between camera(eye)-center(vat) in polar coordinate system
struct View view;
struct Light light[8];
int lightCount = 0;
struct Ambient ambient; //from environment
struct Model model[500]; //assume no more than 500 model in one scene
int modelCount = 0;
int drag = -1;//choose with keyboard '0'~'9', use mouse to drag selected object
int old_mouse_x = -1;
int old_mouse_y = -1;
int windowSize[2];


void display();
void reshape(GLsizei , GLsizei );
void keyboard(unsigned char, int, int);
void mouse(int x, int y);
//////////////////////////////// LOAD /////////////////////////////////////
void loadLight(char* lightFile);
void loadView(char* viewFile);
void loadScene(char* sceneFile);
void LoadTextureSingle(char* pFilename, GLuint &texObject);
void LoadTextureCube(struct CubeTex c, GLuint &texObject);
//void LoadTextureCube(char* pFilename, GLuint &texObject);
//void LoadTextureCube(char pFilename[6][30], GLuint &texObject);


int main(int argc, char** argv)
{
	char sceneFileName[30];
	char viewFileName[30];
	char lightFileName[30];
	/*printf("Please enter scene name: ");
	scanf("%s", sceneFileName);
	printf("Please enter view name: ");
	scanf("%s", viewFileName);
	printf("Please enter light name: ");
	scanf("%s", lightFileName);
	*/
	//寫死檔名 Chess park
	strcpy(sceneFileName, "Chess.scene");
	strcpy(viewFileName, "Chess.view");
	strcpy(lightFileName, "Chess.light");
	
	loadScene(sceneFileName);
	loadView(viewFileName);
	loadLight(lightFileName);
	
	//distance from camera to center, for keyboard rotate
	xdif = view.eye[0] - view.vat[0];
	ydif = view.eye[1] - view.vat[1];
	zdif = view.eye[2] - view.vat[2];
	//dis = sqrt(xdif*xdif + ydif*ydif + zdif*zdif); 
	dis = sqrt(xdif*xdif + zdif*zdif); //極座標系中的 r
	theta = pi / 2;	//initialize the angle
//	printf("dis = %d\n", dis);
//	printf("cos(theta) = %f\n", cos(theta) );
//	printf("current camera x = %d\n",view.eye[0]);
//	printf("current camera z = %d\n\n\n", view.eye[2]);

	for (int i = 0; i < modelCount; i++){
		//printf("%s\n", model[i].objName);//c
		objects.push_back ( new mesh(model[i].objName) );
	}
			//object = new mesh("venus.obj");
	/*load material files of object
	Example:
	materialManager.SetUpTexture(newmtlName, textureFilename);
	*/

	glutInit(&argc, argv);
	glutInitWindowSize(view.viewport[2], view.viewport[3]);
	glutInitWindowPosition(view.viewport[0], view.viewport[1]);	//0,0
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);//RGB?
	glutCreateWindow("Mesh Example");
	
	glewInit();
	FreeImage_Initialise();
	for (int i = 0; i < cnt_single; i++){
		glGenTextures(1, &texSingle[i].texObject);
		LoadTextureSingle(texSingle[i].filename, texSingle[i].texObject);
	}
	for (int i = 0; i < cnt_multi; i++){
		glGenTextures(2, texMulti[i].texObject); //specify as multi-texture
		LoadTextureSingle(texMulti[i].filename[0], texMulti[i].texObject[0]);
		LoadTextureSingle(texMulti[i].filename[1], texMulti[i].texObject[1]);
	}
	glGenTextures(1, &texCube[0].texObject);
	LoadTextureCube(texCube[0], texCube[0].texObject);
	printf("LOAD Cube Sucesss\n");
	/*for (int i = 0; i < cnt_cube; i++){
		printf("How many cube map: %d\n\n", i);
		glGenTextures(1, &texCube[i].texObject);
		LoadTextureCube(texCube[i].filename, texCube[i].texObject);
		printf("LOAD Cube Sucesss\n");
	}*/
	FreeImage_DeInitialise();

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
	/*glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);*/
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
		int lastMaterial = -1; 
		if (1){
				//printf("\ntype:%d, id:%d, name:%s\n", model[o].type, model[o].texSetID, texSingle[model[o].texSetID].filename);
			switch (model[o].type){
			case no:
				//printf("\n\nNOOOOOOOOOOOO\n\n");
				//glDisable(GL_TEXTURE_2D);
				//glDisable(GL_TEXTURE_CUBE_MAP);
				break;
			case single:
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texSingle[model[o].texSetID].texObject);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				break;
			case multi:
				//bind texture 0
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texMulti[model[o].texSetID].texObject[0]);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				//bind texture 1
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texMulti[model[o].texSetID].texObject[1]);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				break;
			case cube:
					//printf("New Cube\n");
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_GEN_R);
				glEnable(GL_TEXTURE_CUBE_MAP);

				//glBindTexture(GL_TEXTURE_CUBE_MAP, texCube[model[o].texSetID].texObject);
				glBindTexture(GL_TEXTURE_CUBE_MAP, texCube[0].texObject);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				printf("Bind Cube Sucesss\n");
					
				break;
			default:
					//printf("error\n\n");
				break;
			}
		}
		/*
		if (!texSets[2].loaded){
		//switch (texSets[model[o].texSetID].type){
		switch (texSets[0].type){
		case no:
		glDisable(GL_TEXTURE_2D);
		break;
		case single:
		///unbind the previous texture sets???
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		//Create a new Texture object, bind it
		//glGenTextures(texSets[model[o].texSetID].countFile, texObjects[model[o].texSetID]);
		glGenTextures(1, texObjects[0]);
		//LoadTextureSingle(texSets[model[o].texSetID].fileNames[0], *texObjects[model[o].texSetID]);
		printf("\n\n要bind:  %s\n", texSets[2].fileNames[0]);
		LoadTextureSingle("grass.bmp", *texObjects[0]);
		printf("BIND !\n");
		//glBindTexture(GL_TEXTURE_2D, *texObjects[model[o].texSetID]);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, *texObjects[0]);
		texSets[2].loaded = true;
		break;
		}
		}
		*/
		//draw all polygons of one object (one model)
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
				//bind them in display function 
			}
			/*modeling transformation*/
			glPushMatrix();
			glTranslatef(model[o].Tx, model[o].Ty, model[o].Tz);
			glRotatef(model[o].angle, model[o].Rx, model[o].Ry, model[o].Rz);
			glScalef(model[o].Sx, model[o].Sy, model[o].Sz);
			
			glBegin(GL_TRIANGLES);//GL_QUADS? 4 vertices
			for (size_t j = 0; j < 3; ++j)
			{
					glNormal3fv(objects[o]->nList[objects[o]->faceList[i][j].n].ptr);
					glTexCoord2fv(objects[o]->tList[objects[o]->faceList[i][j].t].ptr);
					//glVertex3fv(objects[o]->vList[objects[o]->faceList[i][j].v].ptr);
					
					if (model[o].type == multi){
						glMultiTexCoord2fv(GL_TEXTURE0, objects[o]->tList[objects[o]->faceList[i][j].t].ptr);
						glMultiTexCoord2fv(GL_TEXTURE1, objects[o]->tList[objects[o]->faceList[i][j].t].ptr);
					}
					else{
						//no or single or cube
						glTexCoord2fv(objects[o]->tList[objects[o]->faceList[i][j].t].ptr);
					}
					glVertex3fv(objects[o]->vList[objects[o]->faceList[i][j].v].ptr);
					
			}
			glEnd();
			glPopMatrix();
		}
		/*UNBIND*/
		if (1){
			switch (model[o].type){
			case no:
				//glDisable(GL_TEXTURE_2D);
				break;
			case single:
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_ALPHA_TEST);
				glBindTexture(GL_TEXTURE_2D, 0);
				break;
			case multi:
				//unbind texture 1
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
				//unbind texture 0
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
				break;
			case cube:
				printf("Unbind Cube\n");
				glDisable(GL_TEXTURE_CUBE_MAP);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);
				//glDisable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				break;
			default:
				break;
			}
		}

	}
	//glFlush();
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
	printf("===============key is : %c==============\n", key);
	switch (key)
	{
	case 'w':	//zoom in   
		//view.eye[2] += 2;
		if (xdif > 0.1 && ydif > 0.1 && zdif > 0.1){
			xdif = (view.eye[0] - view.vat[0]) * 0.8;
			ydif = (view.eye[1] - view.vat[1]) * 0.8;
			zdif = (view.eye[2] - view.vat[2]) * 0.8;
			dis = sqrt(xdif*xdif + zdif*zdif);//相機與中心點的距離變了,必須更新距離
			view.eye[0] = view.vat[0] + xdif;
			view.eye[1] = view.vat[1] + ydif;
			view.eye[2] = view.vat[2] + zdif;
		}
		else{
			printf("too close\n");
		}
		break;
	case 's':	//zoom out   
		xdif = (view.eye[0] - view.vat[0]) * 1.2;
		ydif = (view.eye[1] - view.vat[1]) * 1.2;
		zdif = (view.eye[2] - view.vat[2]) * 1.2;
		dis = sqrt(xdif*xdif + zdif*zdif);//相機與中心點的距離變了,必須更新距離
		view.eye[0] = view.vat[0] + xdif;
		view.eye[1] = view.vat[1] + ydif;
		view.eye[2] = view.vat[2] + zdif;
		break;
	case 'a':	//rotate left
		theta -= 0.05;
		//if (theta <= 2*pi) theta = 0;
		printf("theta = %f \n", theta);
		view.eye[0] = view.vat[0] + dis*sin(theta);
		view.eye[2] = view.vat[2] + dis*cos(theta);
		break;
	case 'd':	//rotate right
		theta += 0.05;
		//if (theta >= -2*pi) theta = 0;
		view.eye[0] = view.vat[0] + dis*sin(theta);
		view.eye[2] = view.vat[2] + dis*cos(theta);
		break;
	default:	//choose which object to drag 
		if ('1' <= key && key <= '9'){
			drag = key - '1'; //start from 0
			printf("Model %d, [ %s ] is selected!\n ", drag, model[drag].objName);
		}
		else{
			printf("not functional key\n");
		}
	}
	glutPostRedisplay();
	//glFlush();
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
	char tmps[200];//buffer for one line
	char *word;
	
	int curID;
	enum TexType curType;
	cnt_single = 0; cnt_multi = 0; cnt_cube = 0;
	modelCount = 0;

	fin = fopen(fileName, "r");
	if (fin == NULL){ printf("Cannot open in file. import\n"); return ; }
	FreeImage_Initialise();

	while(fgets(tmps, sizeof(tmps), fin)){
		word = strtok(tmps, "\n ");
		//specify the texture info of this model
		
		if (strcmp(word, "model") == 0){
			//specify which set of texturemap it use
			model[modelCount].type = curType;
			model[modelCount].texSetID = curID; //texSets.size() - 1;
			//specify the info of one model
			//ex."model house.obj 1.0 1.0 1.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0"
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
		else{
		//Create a new TexSet, and assign it to all following models
			model[modelCount].useNewTexture = true;
			if (modelCount > 0) model[modelCount-1].isLast = true;
			//GLuint newTexObject;
			if (strcmp(word, "no-texture") == 0){
				//no
				printf("\n============Type NO=======\n");
				curType = no;
				curID = -1;
			}
			else if (strcmp(word, "single-texture") == 0){
				//single-texture ImageFileName1
				word = strtok(NULL, "\n ");
				curType = single;
				curID = cnt_single;
				strcpy(texSingle[cnt_single].filename, word);
					//printf("====loadSceneSing==\n%s\n", texSingle[cnt_single].filename);
				cnt_single++;
			}
			else if (strcmp(word, "multi-texture") == 0){
			//multi-texture ImageFileName1 ImageFileName2
				curType = multi;
				curID = cnt_multi;
				for (int i = 0; i < 2; i++){
					word = strtok(NULL, "\n ");
					strcpy(texMulti[cnt_multi].filename[i], word);
				}
				for (int i = 0; i < 2; i++) printf("MULTI:  %s\n", texMulti[cnt_multi].filename[i]);
				cnt_multi++;
				
			}
			else if (strcmp(word, "cube-map") == 0){
			//cube-map Image1 Image2 Image3 Image4 Image5 Image6
				curID = cnt_cube;
				curType = cube;
				/*for (int i = 0; i < 6; i++){
					word = strtok(NULL, "\n ");
					strcpy(texCube[0].filename[i], word);
					printf("CUBE: %s\n", texCube[0].filename[i]);
				}*/
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f1, word);
				printf("CUBE: %s\n", texCube[0].f1);
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f2, word);
				printf("CUBE: %s\n", texCube[0].f2);
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f3, word);
				printf("CUBE: %s\n", texCube[0].f3);
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f4, word);
				printf("CUBE: %s\n", texCube[0].f4);
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f5, word);
				printf("CUBE: %s\n", texCube[0].f5);
				word = strtok(NULL, "\n ");
				strcpy(texCube[0].f6, word);
				printf("CUBE: %s\n", texCube[0].f6);
				//cnt_cube++;
			}
		}
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

void LoadTextureSingle(char* pFilename, GLuint &texObject)
{
	FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType(pFilename, 0), pFilename);
	FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
	int iWidth = FreeImage_GetWidth(p32BitsImage);
	int iHeight = FreeImage_GetHeight(p32BitsImage);

	glBindTexture(GL_TEXTURE_2D, texObject);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
	glGenerateMipmap(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_CUBE_MAP, GL_COMBINE_RGB, GL_DECAL); only for ENV map
	//glBindTexture(GL_TEXTURE_2D, texObject);//?

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	FreeImage_Unload(p32BitsImage);
	FreeImage_Unload(pImage);
	//printf("\nFreeImage: Sin/Mul\n");
}
//int a[][10] //char** pFilename
void LoadTextureCube(struct CubeTex c, GLuint &texObject)
{
	printf("======LOAD CUBE=======\n");
	printf("1:%s\n", c.f1);
	FIBITMAP* pImage0 = FreeImage_Load(FreeImage_GetFileType(c.f1, 0), c.f1);
	FIBITMAP *p32BitsImage0 = FreeImage_ConvertTo32Bits(pImage0);
	int iWidth0 = FreeImage_GetWidth(p32BitsImage0);
	int iHeight0 = FreeImage_GetHeight(p32BitsImage0);

	FIBITMAP* pImage1 = FreeImage_Load(FreeImage_GetFileType(c.f2, 0), c.f2);
	FIBITMAP *p32BitsImage1 = FreeImage_ConvertTo32Bits(pImage1);
	int iWidth1 = FreeImage_GetWidth(p32BitsImage1);
	int iHeight1 = FreeImage_GetHeight(p32BitsImage1);

	FIBITMAP* pImage2 = FreeImage_Load(FreeImage_GetFileType(c.f3, 0), c.f3);
	FIBITMAP *p32BitsImage2 = FreeImage_ConvertTo32Bits(pImage2);
	int iWidth2 = FreeImage_GetWidth(p32BitsImage2);
	int iHeight2 = FreeImage_GetHeight(p32BitsImage2);

	FIBITMAP* pImage3 = FreeImage_Load(FreeImage_GetFileType(c.f4, 0), c.f4);
	FIBITMAP *p32BitsImage3 = FreeImage_ConvertTo32Bits(pImage3);
	int iWidth3 = FreeImage_GetWidth(p32BitsImage3);
	int iHeight3 = FreeImage_GetHeight(p32BitsImage3);

	FIBITMAP* pImage4 = FreeImage_Load(FreeImage_GetFileType(c.f5, 0), c.f5);
	FIBITMAP *p32BitsImage4 = FreeImage_ConvertTo32Bits(pImage4);
	int iWidth4 = FreeImage_GetWidth(p32BitsImage4);
	int iHeight4 = FreeImage_GetHeight(p32BitsImage4);

	FIBITMAP* pImage5 = FreeImage_Load(FreeImage_GetFileType(c.f6, 0), c.f6);
	FIBITMAP *p32BitsImage5 = FreeImage_ConvertTo32Bits(pImage5);
	int iWidth5 = FreeImage_GetWidth(p32BitsImage5);
	int iHeight5 = FreeImage_GetHeight(p32BitsImage5);


	//glBindTexture(GL_TEXTURE_CUBE_MAP, texObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texCube[0].texObject);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexEnvf(GL_TEXTURE_CUBE_MAP, GL_COMBINE_RGB, GL_DECAL);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, iWidth0, iHeight0, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, iWidth1, iHeight1, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage1));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, iWidth2, iHeight2, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage2));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, iWidth3, iHeight3, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage3));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, iWidth4, iHeight4, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage4));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, iWidth5, iHeight5, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage5));
	
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	
	FreeImage_Unload(p32BitsImage0);
	FreeImage_Unload(pImage0);
	FreeImage_Unload(p32BitsImage1);
	FreeImage_Unload(pImage1);
	FreeImage_Unload(p32BitsImage2);
	FreeImage_Unload(pImage2);
	FreeImage_Unload(p32BitsImage3);
	FreeImage_Unload(pImage3);
	FreeImage_Unload(p32BitsImage4);
	FreeImage_Unload(pImage4);
	FreeImage_Unload(p32BitsImage5);
	FreeImage_Unload(pImage5);
	
}
