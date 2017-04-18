#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _vector{
	float x, y, z;
	struct _vector *next;
} vector;

GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* Diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Light location. */

int h, w;
float *board_vertices, *board_normals;
GLuint *black_elements, *white_elements;
vector position[2];
vector *moves;
float offset;
int current_figure;

typedef struct _mesh{
	float *vertices, *normals;
	GLuint *elements;
	GLuint vertexBuffer, normalBuffer, elementBuffer;
	int mode;
	int size;
} mesh;
mesh *board, *knight;

void drawMesh(mesh *m){
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexPointer(3, GL_FLOAT, 0, m->vertices);
	glEnableClientState(GL_VERTEX_ARRAY);

	//glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glNormalPointer(GL_FLOAT, 0, m->normals);
	glEnableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementBuffer);	
  	glDrawElements(m->mode, m->size, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void addElementBuffer(GLuint *destination, GLuint *data, int elements){
	glGenBuffers(1, destination);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *destination);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements*sizeof(GLuint), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void addArrayBuffer(GLuint *destination, float *data, int elements){
	glGenBuffers(1, destination);
	glBindBuffer(GL_ARRAY_BUFFER, *destination);
	glBufferData(GL_ARRAY_BUFFER, elements*sizeof(float), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

mesh * addMesh(float *vertices, float *normals, int vertexCount, GLuint *elements, int size, int mode){
	mesh *newMesh = (mesh *)malloc(sizeof(mesh));
	newMesh->normals = normals;
	newMesh->size = size;
	newMesh->mode = mode;
	newMesh->vertices = vertices;
	newMesh->elements = elements;

	GLuint buffer;	
	addArrayBuffer(&buffer, normals, vertexCount);	
	newMesh->normalBuffer = buffer;
	addArrayBuffer(&buffer, vertices, vertexCount);
	newMesh->vertexBuffer = buffer;
	addElementBuffer(&buffer, elements, size);
	newMesh->elementBuffer = buffer;
	return newMesh;
}

void drawKnight(int index){
	float scale = 0.6;
	glTranslatef(position[index].x, position[index].y, position[index].z);
	glScalef(scale, scale, scale);
	drawMesh(knight);
	glScalef(1.0/scale, 1.0/scale, 1.0/scale);
	glTranslatef(-position[index].x, -position[index].y, -position[index].z);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawMesh(board);
	
	drawKnight(0);
	drawKnight(1);

	glutSwapBuffers();
}

mesh * openMesh(char *path){
	char filename[100];

	int countElements = 0;
	int countVertices = 0;
	float *vertices;
	float *normals;
	int *elements;
	int i;
	printf("Loading %s\n", path);
	sprintf(filename, "%s/Elements.raw", path);
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) printf("Can't open elements\n");
	else{
		while(!feof(fp)) {
			fgetc(fp);
			countElements++;
		}
		countElements = countElements/(4);
		elements = (int *)malloc(sizeof(int)*countElements);
		fseek(fp, 0, SEEK_SET);
		fread(elements, sizeof(int), countElements, fp);
	}
	fclose(fp);

	sprintf(filename, "%s/Vertices.raw", path);
	fp = fopen(filename, "r");
	if(fp == NULL) printf("Can't open vertices\n");
	else{
		while(!feof(fp)) {
			fgetc(fp);
			countVertices++;
		}
		countVertices = countVertices/(4*3);
		vertices = (float *)malloc(sizeof(float)*countVertices*3);
		normals = (float *)malloc(sizeof(float)*countVertices*3);
		fseek(fp, 0, SEEK_SET);
		fread(vertices, sizeof(vector), countVertices, fp);
	}
	fclose(fp);

	sprintf(filename, "%s/Normals.raw", path);
	fp = fopen(filename, "r");
	if(fp == NULL) printf("Can't open normals\n");
	else{
		fread(normals, sizeof(vector), countVertices, fp);
	}
	fclose(fp);
	printf("Loaded: %d vertices, %d elements\n", countVertices, countElements);
	return addMesh(vertices, normals, countVertices*3, elements, countElements, GL_TRIANGLES);
}


void update(int value){
	offset += 0.01;
	if(offset >= 1 && moves->next != NULL){
		offset = 0;
		current_figure = (current_figure+1)%2;
		moves = moves->next;
	}
	if(moves->next->next != NULL){
		float off = 0.5-cos(10*offset/M_PI)/2;
		float dx = ((moves->next->next->x)-(moves->x))*off;
		float dy = ((moves->next->next->y)-(moves->y))*off;
		position[current_figure].x = dx+0.5+(moves->x);
		position[current_figure].y = sin(offset/(0.1*M_PI));
		position[current_figure].z = dy+0.5+(moves->y);
	}

	glutTimerFunc(30, update, 0);
	glutPostRedisplay();
}

void init(void)
{
	int i, j, index;
	h = 8;
	w = 8;	
	current_figure = 0;
	offset = 0.0;

	moves = (vector *)malloc(sizeof(vector));
	vector *move = moves;
	do{
		move->next = (vector *)malloc(sizeof(vector));
		move = move->next;
		scanf("%f %f", &(move->x), &(move->y));
	}
	while(!feof(stdin));

	/* Enable a single OpenGL light. */
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	/* Use depth buffering for hidden surface elimination. */
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	/* Setup the view of the cube. */
	glMatrixMode(GL_PROJECTION);
	gluPerspective( /* field of view in degree */ 70.0,
		/* aspect ratio */ 1.0,
		/* Z near */ 0.1, /* Z far */ 100.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(-2.5, 2, -2.5,  /* eye is at (0,0,5) */
		2.0, 0.0, 2.0,      /* center is at (0,0,0) */
		0.0, 1.0, 0.);      /* up is in positive Y direction */

	/* Adjust cube position to be asthetic angle. */
/*	glTranslatef(0.0, 0.0, -1.0);
	glRotatef(60, 1.0, 0.0, 0.0);
	glRotatef(-20, 0.0, 0.0, 1.0);*/

	board_vertices = (float *)malloc(3*(h+1)*(w+1)*sizeof(float));
	board_normals = (float *)malloc(3*(h+1)*(w+1)*sizeof(float));
	black_elements = (GLuint *)malloc(h*w*2*sizeof(GLuint));
	white_elements = (GLuint *)malloc(h*w*2*sizeof(GLuint));
	for(i = 0; i <= w; i++){
		for(j = 0; j <= h; j++){
			index = ( (i*(h+1))+j )*3;
			board_vertices[index+0] = i;
			board_vertices[index+1] = 0;
			board_vertices[index+2] = j;

			board_normals[index+0] = 1.0;
			board_normals[index+1] = 1.0;
			board_normals[index+2] = j/2.0;
		}
	}
	
	for(i = 0; i < w; i++){
		for(j = 0; j < h; j++){
			index = ( (i*h)+j )*2;
			if(j%2==1 && h%2==0) index-=2;
			if((i+j) % 2 == 0){
				black_elements[index+0] = ((i)*(h+1))+(j);
				black_elements[index+1] = ((i+1)*(h+1))+(j);
				black_elements[index+2] = ((i+1)*(h+1))+(j+1);
				black_elements[index+3] = ((i)*(h+1))+(j+1);
			}	
			else{
				white_elements[index+0] = ((i)*(h+1))+(j);
				white_elements[index+1] = ((i+1)*(h+1))+(j);
				white_elements[index+2] = ((i+1)*(h+1))+(j+1);
				white_elements[index+3] = ((i)*(h+1))+(j+1);
			}
		}
	}
	for(i = 0; i < h*w*2; i++){
		//printf("%d %d\n", black_elements[i], white_elements[i]);
	}
	printf("\n");
	board = addMesh(board_vertices, board_normals, 3*(h+1)*(w+1), white_elements, h*w*2, GL_QUADS);
	
	knight = openMesh("./Knight");
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Chess");
  glutDisplayFunc(display);
  glutTimerFunc(100, update, 0);
  init();
  printf("Started\n");
  glutMainLoop();
  return 0;
}
