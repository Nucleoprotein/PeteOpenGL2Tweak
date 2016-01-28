/***************************************************************************
 *   Copyright (C) 2011 by Blade_Arma                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef _GTE_ACCURACY_H_
#define _GTE_ACCURACY_H_

typedef struct OGLVertexTag
{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	GLfloat sow;
	GLfloat tow;

	union COLTAG
	{
		unsigned char col[4];
		unsigned int lcol;
	} c;
} OGLVertex;

extern void resetGteVertices();
extern bool checkGteVertex(s16 sx, s16 sy);
extern bool getGteVertex(s16 sx, s16 sy, OGLVertexTag* vertex);
extern void GPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz);

#endif // _GTE_ACCURACY_H_
