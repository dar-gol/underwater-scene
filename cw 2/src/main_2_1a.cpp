#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"

#include "Box.cpp"

GLuint program;
Core::Shader_Loader shaderLoader;

unsigned int VAO;

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// ZADANIE: W zalaczonym wyzej pliku Box.cpp znajduja sie tablice pozycji i kolorow wierzcholkow prostopadloscianu. Przesun go o wektor (0.5, 0.5, -0.2) i wyswietl go.
	// 
	// Do shadera uzytego w tym zadaniu nalezy przeslac nie tylko pozycje, ale rowniez kolory wierzcholkow.
	// W funkcji init należy przesłać pozycje i kolory do GPU 

	glUseProgram(program);

	// Powiąż stworzone VAO za pomocą funkcji glBindVertexArray
	// wykorzystaj glDrawArrays do narysowania prostopadłościanu

	glm::mat4 transformation;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);


	glUseProgram(0);
	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_2_1.vert", "shaders/shader_2_1.frag");
	// Dokładny opis działania funkcji z poniższych punktów można znaleźć na slajdach z wykładu, w dokumentacji https://www.khronos.org/registry/OpenGL-Refpages/gl4/ funkcje są opisane także na stronach https://learnopengl.com/Getting-started/Hello-Triangle oraz https://open.gl/drawing
	// Zainicjalizuj VertexArrayObject (VAO) i zapisz w nim atrybuty prostopadłościanu
	// I Stwórz VAO
	// 1. Stwórz nowy VAO za pomocą glGenVertexArrays i przypisz jego adres do zmiennej globalnej
	
	// 2. Powiąż stworzone VAO za pomocą funkcji glBindVertexArray

	// II Zainicjalizuj VBO
	// 3. Stwórz VertexBufferObject do przechowywania pozycji wierzchołków za pomocą funkcji glGenBuffers

	// 4. Powiąż stworzone VBO za pomocą funkcji glBindBuffer(GLenum target, GLuint )

	// 5. Zaalokuj w GPU miejsce na tablice wierzchołków i kolorów za pomocą glBufferData pamiętaj, żeby zaalokować dość miejsca dla punktów i kolorów

	// III Prześlij dane do karty graficznej
	// A) Prześlij pozycję wierzchołków do karty graficznej
	// 6. Aktywuj atrybut powiązany z pozycją wierchołków za pomocą glEnableVertexAttribArray(GLuint index). Indeks jest zapisany w shaderze wierzchołków w 3. lini. Można odpytać shader o indeks za pomocą funkcji glGetAttribLocation(GL uint program, const GLchar *name)

	// 7. Przekaż dane za pomocą glBufferSubData

	// 8. Zdefiniuj jak OpenGL powinien interpretować dane za pomocą glVertexAttribPointer

	// B) powtórz punkty 6-8 by przesłać  wartości odpowiadające za kolor 


	// 9. Uwolnij VAO za pomocą funkcji glBindVertexArray(0)

}

void shutdown()
{
	shaderLoader.DeleteProgram(program);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
