#include "Render_Utils.h"

#include <algorithm>

#include "glew.h"
#include "freeglut.h"


void Core::DrawContext(Core::RenderContext& context)
{

	glBindVertexArray(context.vertexArray);
	glDrawElements(
		GL_TRIANGLES,      // mode
		context.size,    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
	);
	glBindVertexArray(0);
}

