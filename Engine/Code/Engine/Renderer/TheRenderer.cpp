#include "Engine/Renderer/TheRenderer.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "Glu32" ) // Link in the Glu32.lib static library

TheRenderer* TheRenderer::instance = nullptr;

const unsigned char TheRenderer::plainWhiteTexel[3] = { 255, 255, 255 };

TheRenderer::TheRenderer()
: m_defaultFont(BitmapFont::CreateOrGetFont("SquirrelFixedFont"))
, m_defaultTexture(Texture::CreateTextureFromData("PlainWhite", const_cast<uchar*>(plainWhiteTexel), 3, Vector2Int::ONE))
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	hookUpOpenGLPointers();
}

void TheRenderer::ClearScreen(float red, float green, float blue)
{
	glClearColor(red, green, blue, 1.f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TheRenderer::ClearScreen(const RGBA& color)
{
	glClearColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, 1.f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TheRenderer::SetOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
	glLoadIdentity();
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
}

void TheRenderer::SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
	glLoadIdentity();
	gluPerspective(fovDegreesY, aspect, nearDist, farDist); //Note: absent in OpenGL ES 2.0
}

void TheRenderer::DrawPoint(float x, float y, const RGBA& color /*= RGBA::WHITE*/)
{
	Vertex_PCT vertex;
	vertex.pos = Vector3(x, y, 0.0f);
	vertex.color = color;
	DrawVertexArray(&vertex, 1, POINTS);
}

void TheRenderer::DrawPoint(const Vector2& point, const RGBA& color /*= RGBA::WHITE*/)
{
	DrawPoint(point.x, point.y, color);
}

void TheRenderer::DrawPoint(const Vector3& point, const RGBA& color /*= RGBA::WHITE*/)
{
	Vertex_PCT vertex;
	vertex.pos = point;
	vertex.color = color;
	DrawVertexArray(&vertex, 1, POINTS);
}

void TheRenderer::DrawLine(const Vector2& start, const Vector2& end, const RGBA& color/* = RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
	glLineWidth(lineThickness);
	Vertex_PCT vertexes[2];
	vertexes[0].pos = start;
	vertexes[1].pos = end;
	vertexes[0].color = color;
	vertexes[1].color = color;
	DrawVertexArray(vertexes, 2, LINES);
}

void TheRenderer::DrawLine(const Vector3& start, const Vector3& end, const RGBA& color /*= RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
	glLineWidth(lineThickness);
	Vertex_PCT vertexes[2];
	vertexes[0].pos = start;
	vertexes[1].pos = end;
	vertexes[0].color = color;
	vertexes[1].color = color;
	DrawVertexArray(vertexes, 2, LINES);
}

void TheRenderer::SetColor(float red, float green, float blue, float alpha)
{
	glColor4f(red, green, blue, alpha);
}

void TheRenderer::SetColor(const RGBA& color)
{
	glColor4ub(color.red, color.green, color.blue, color.alpha);
}

void TheRenderer::SetPointSize(float size)
{
	glPointSize(size);
}

void TheRenderer::SetLineWidth(float width)
{
	glLineWidth(width);
}

void TheRenderer::PushMatrix()
{
	glPushMatrix();
}

void TheRenderer::PopMatrix()
{
	glPopMatrix();
}

void TheRenderer::Translate(float x, float y, float z)
{
	glTranslatef(x, y, z);
}

void TheRenderer::Translate(const Vector2& xy)
{
	glTranslatef(xy.x, xy.y, 0.0f);
}

void TheRenderer::Translate(const Vector3& xyz)
{
	glTranslatef(xyz.x, xyz.y, xyz.z);
}

void TheRenderer::Rotate(float rotationDegrees)
{
	glRotatef(rotationDegrees, 0.f, 0.f, 1.f);
}

void TheRenderer::Rotate(float rotationDegrees, float x, float y, float z)
{
	glRotatef(rotationDegrees, x, y, z);
}

void TheRenderer::Scale(float x, float y, float z)
{
	glScalef(x, y, z);
}

void TheRenderer::EnableAdditiveBlending()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void TheRenderer::EnableAlphaBlending()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void TheRenderer::EnableInvertedBlending()
{
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
}

void TheRenderer::EnableDepthTest(bool usingDepthTest)
{
	usingDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void TheRenderer::BindTexture(const Texture& texture)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture.m_openglTextureID);
}

void TheRenderer::UnbindTexture()
{
	glDisable(GL_TEXTURE_2D);
}

int TheRenderer::GenerateBufferID()
{
	GLuint vboID = 0;
	glGenBuffers(1, &vboID);
	return vboID;
}

void TheRenderer::DeleteBuffers(int vboID)
{
	const GLuint id = ((GLuint)vboID);
	glDeleteBuffers(1, &id);
}

void TheRenderer::BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_PCT) * numVerts, vertexes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TheRenderer::DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode /*= QUADS*/, Texture* texture /*= nullptr*/)
{
	if (!texture)
	{
		texture = m_defaultTexture;
	}
	BindTexture(*texture);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), &vertexes[0].pos);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), &vertexes[0].color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), &vertexes[0].texCoords);

	glDrawArrays(GetDrawMode(drawMode), 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void TheRenderer::DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode /*= QUADS*/, Texture* texture /*= nullptr*/)
{
	if (!texture)
	{
		texture = m_defaultTexture;
	}
	BindTexture(*texture);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, pos));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, color));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, texCoords));

	glDrawArrays(GetDrawMode(drawMode), 0, numVerts);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TheRenderer::DrawText2D
	( const Vector2& startBottomLeft
	, const std::string& asciiText
	, float cellWidth
	, float cellHeight
	, const RGBA& tint /*= RGBA::WHITE*/
	, bool drawShadow /*= false*/
	, const BitmapFont* font /*= nullptr*/)
{
	const float SHADOW_WIDTH_OFFSET = cellWidth / 10.0f;
	const float SHADOW_HEIGHT_OFFSET = cellHeight / -10.0f;
	const Vector2 SHADOW_OFFSET = Vector2(SHADOW_WIDTH_OFFSET, SHADOW_HEIGHT_OFFSET);
	if (font == nullptr)
	{
		font = m_defaultFont;
	}
	int stringLength = asciiText.size();
	Vector2 currentPosition = startBottomLeft;
	for (int i = 0; i < stringLength; i++)
	{
		unsigned char currentCharacter = asciiText[i];
		Vector2 topRight = currentPosition + Vector2(cellWidth, cellHeight);
		AABB2 quadBounds = AABB2(currentPosition, topRight);
		AABB2 glyphBounds =	font->GetTexCoordsForGlyph(currentCharacter);
		if (drawShadow)
		{
			AABB2 shadowBounds = AABB2(currentPosition + SHADOW_OFFSET, topRight + SHADOW_OFFSET);
			DrawTexturedAABB(shadowBounds, glyphBounds.mins, glyphBounds.maxs, font->GetTexture(), RGBA::BLACK);
		}
		DrawTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, font->GetTexture(), tint);
		currentPosition.x += cellWidth;
	}
}

unsigned char TheRenderer::GetDrawMode(DrawMode mode)
{
	switch (mode)
	{
	case QUADS:
		return GL_QUADS;
	case QUAD_STRIP:
		return GL_QUAD_STRIP;
	case POINTS:
		return GL_POINTS;
	case LINES:
		return GL_LINES;
	case LINE_LOOP:
		return GL_LINE_LOOP;
	case POLYGON:
		return GL_POLYGON;
	default:
		return GL_POINTS;
	}
}

void TheRenderer::EnableFaceCulling(bool enabled)
{
	enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void TheRenderer::DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
	Vertex_PCT* vertexes = new Vertex_PCT[numSides];
	const float radiansTotal = 2.0f * MathUtils::pi;
	const float radiansPerSide = radiansTotal / numSides;
	int index = 0;

	for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
	{
		float adjustedRadians = radians + radianOffset;
		float x = center.x + (radius * cos(adjustedRadians));
		float y = center.y + (radius * sin(adjustedRadians));

		vertexes[index].color = color;
		vertexes[index].pos = Vector2(x, y);
	}
	DrawVertexArray(vertexes, numSides, LINE_LOOP);

}

void TheRenderer::DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
	Vertex_PCT* vertexes = new Vertex_PCT[numSides];
	const float radiansTotal = 2.0f * MathUtils::pi;
	const float radiansPerSide = radiansTotal / numSides;
	int index = 0;

	for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
	{
		float adjustedRadians = radians + radianOffset;
		float x = center.x + (radius * cos(adjustedRadians));
		float y = center.y + (radius * sin(adjustedRadians));

		vertexes[index].color = color;
		vertexes[index].pos = Vector2(x, y);
	}
	DrawVertexArray(vertexes, numSides, POLYGON);
}

void TheRenderer::DrawAABB(const AABB2& bounds, const RGBA& color)
{
	const int NUM_VERTS = 4;
	SetColor(color);
	Vertex_PCT vertexes[NUM_VERTS];
	vertexes[0].color = color;
	vertexes[1].color = color;
	vertexes[2].color = color;
	vertexes[3].color = color;
	vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
	vertexes[1].pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
	vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
	vertexes[3].pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
	DrawVertexArray(vertexes, NUM_VERTS, QUADS);
}

void TheRenderer::DrawAABB(const AABB3& bounds, const RGBA& color)
{
	const int NUM_VERTS = 24;
	SetColor(color);
	Vertex_PCT vertexes[NUM_VERTS];
	for (int i = 0; i < NUM_VERTS; i++)
	{
		vertexes[i].color = color;
	}
	//Bottom
	vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

	//Top
	vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[5].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[6].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);

	//North
	vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

	//South
	vertexes[12].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[13].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[14].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[15].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);

	//East
	vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[17].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[18].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[19].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

	//West
	vertexes[20].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[21].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[22].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[23].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

	DrawVertexArray(vertexes, NUM_VERTS, QUADS);
}

void TheRenderer::DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color)
{
	const int NUM_VERTS = 20;
	SetColor(color);
	Vertex_PCT vertexes[NUM_VERTS];
	for (int i = 0; i < NUM_VERTS; i++)
	{
		vertexes[i].color = color;
	}
	//Bottom
	vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

	//West
	vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[5].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[6].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

	//North
	vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

	//East
	vertexes[12].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[13].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[14].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[15].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

	//South
	vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[17].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[18].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[19].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);

	DrawVertexArray(vertexes, NUM_VERTS, LINE_LOOP);
}

void TheRenderer::DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
	Vertex_PCT vertexes[4];
	Vertex_PCT vertex;
	vertex.color = color;
	vertex.pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
	vertex.texCoords = texCoordMins;
	vertexes[0] = vertex;
	vertex.pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
	vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
	vertexes[1] = vertex;
	vertex.pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
	vertex.texCoords = texCoordMaxs;
	vertexes[2] = vertex;
	vertex.pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
	vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
	vertexes[3] = vertex;
	TheRenderer::instance->DrawVertexArray(vertexes, 4, QUADS, texture);
}

void TheRenderer::DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
	Vertex_PCT vertexes[4];
	Vertex_PCT vertex;
	vertex.color = color;
	vertex.pos = face.verts[0];
	vertex.texCoords = texCoordMins;
	vertexes[0] = vertex;
	vertex.pos = face.verts[1];
	vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
	vertexes[1] = vertex;
	vertex.pos = face.verts[2];
	vertex.texCoords = texCoordMaxs;
	vertexes[2] = vertex;
	vertex.pos = face.verts[3];
	vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
	vertexes[3] = vertex;
	TheRenderer::instance->DrawVertexArray(vertexes, 4, QUADS, texture);
}

