#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string.h>

using namespace std;

void loadLight(string lightFile);
void loadView(string viewFile);
void loadScene(string sceneFile);


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
struct View view;
struct Light light[8];
int lightCount = 0;
struct Ambient ambient; //from environment
struct Model model[100]; //assume no more than 100 model in one scene
int modelCount = 0;


/*Scene Example:
model house.obj 1.0 1.0 1.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0
model wall.obj 1.0 1.0 1.0 90.0 0.0 1.0 0.0 -200.0 0.0 0.0
model obj_file_name Sx Sy Sz Angle Rx Ry Rz Tx Ty Tz

obj_file_name: the file name of the mesh.
Sx Sy Sz: the scale value of each direction.
Angle Rx Ry Rz: "Angle" means the degree of rotation. Rx, Ry, and Rz are the rotation axis vector.
Tx Ty Tz: the transfer vector. It means that the origin of the model should move to (x, y, z).

Please note the matrix operation in OpenGL is in reversed order.
*/

/* Load Files*/
void loadScene(string fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	fin = fopen(fileName.c_str(), "r");
	if (fin == NULL){ printf("Cannot open in file. import\n"); return; }

	for (fgets(tmps, sizeof(tmps), fin)){

		if (modelCount >= 100){
			printf("More than 100 objects!\n");
			return -1;
		}

		word = strtok(tmps, " ");
		//model house.obj 1.0 1.0 1.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0
		word = strtok(tmps, " ");
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
	printf("Scene Loaded: %s\n", model[0].objName);
	fclose(fin);
}

void loadView(string fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	fin = fopen(fileName.c_str(), "r");
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

void loadLight(string fileName){
	FILE *fin;
	char tmps[100];
	char *word;
	fin = fopen(fileName.c_str(), "r");
	if (fin == NULL){ printf("Cannot open in file. import\n"); return; }
	//light
	for (fgets(tmps, sizeof(tmps), fin)){
		word = strtok(tmps, " ");

		if ( strcmp(word, "light") == 0 ){
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

