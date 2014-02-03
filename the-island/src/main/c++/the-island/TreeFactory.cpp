/*
 * Copyright © 2014 Simple Entertainment Limited
 *
 * This file is part of The Island.
 *
 * The Island is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * The Island is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with The Island. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "TreeFactory.h"

using namespace simplicity;
using namespace std;

namespace theisland
{
	namespace TreeFactory
	{
		unique_ptr<Entity> createTree(float age)
		{
			unsigned int segments = 4;
			unsigned int segmentDivisions = 5;
			Vector4 color(0.47f, 0.24f, 0.0f, 1.0f);

			float segmentHeight = age / 400.0f * 5.0f;
			float segmentHeightDelta = segmentHeight / segments / 2.0f;
			float segmentRadius = age / 400.0f;
			float segmentRadiusDelta = segmentRadius / segments / 2.0f;

			// Vertices
			unsigned int verticesInTrunkSegment = segmentDivisions * 4;
			unsigned int verticesInTrunkSegments = verticesInTrunkSegment * segments;
			unsigned int verticesInTrunkTop = segmentDivisions + 1;
			vector<Vertex> vertices(verticesInTrunkSegments + verticesInTrunkTop);

			// Trunk Sides
			Vector3 center(0.0f, 0.0f, 0.0f);
			for (unsigned int segment = 0; segment < segments; segment++)
			{
				unsigned int indexOffset = segment * verticesInTrunkSegment;

				ModelFactory::addTunnelVertexList(vertices, indexOffset, segmentRadius, segmentHeight,
						segmentDivisions, center, color);

				if (segment > 0)
				{
					unsigned int indexOffsetPrevious = (segment - 1) * verticesInTrunkSegment;
					for (unsigned int segmentDivision = 0; segmentDivision < segmentDivisions; segmentDivision++)
					{
						vertices[indexOffsetPrevious + segmentDivision * 4 + 1].position =
								vertices[indexOffset + segmentDivision * 4].position;
						vertices[indexOffsetPrevious + segmentDivision * 4 + 3].position =
								vertices[indexOffset + segmentDivision * 4 + 2].position;

						// TODO correct normals too...
					}
				}

				center.Z() -= segmentHeight;

				if (segment != segments - 1)
				{
					center.X() += getRandomFloat(-segmentRadius, segmentRadius);
					center.Y() += getRandomFloat(-segmentRadius, segmentRadius);

					segmentHeight -= segmentHeightDelta;
					segmentRadius -= segmentRadiusDelta;
				}
			}

			// Trunk Top
			ModelFactory::addCircleVertexList(vertices, verticesInTrunkSegments, segmentRadius, segmentDivisions,
					center, color);

			// Indices
			unsigned int indicesInTrunkSegment = segmentDivisions * 6;
			unsigned int indicesInTrunkSegments = indicesInTrunkSegment * segments;
			unsigned int indicesInTrunkTop = segmentDivisions * 3;
			vector<unsigned int> indices(indicesInTrunkSegments + indicesInTrunkTop);

			// Trunk Sides
			for (unsigned int segment = 0; segment < segments; segment++)
			{
				unsigned int indexOffset = segment * indicesInTrunkSegment;
				unsigned int vertexIndexOffset = segment * verticesInTrunkSegment;

				ModelFactory::addTunnelIndexList(indices, indexOffset, vertexIndexOffset, segmentDivisions);
			}

			// Trunk Top
			ModelFactory::addCircleIndexList(indices, indicesInTrunkSegments, verticesInTrunkSegments,
					segmentDivisions, true);

			unique_ptr<Entity> tree(new Entity);
			rotate(tree->getTransform(), MathConstants::PI * 0.5f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
			unique_ptr<Mesh> mesh = ModelFactory::getInstance().createMesh(vertices, indices);
			unique_ptr<Model> bounds = ModelFunctions::getCircleBoundsXZ(mesh->getVertices());
			tree->addUniqueComponent(move(mesh));
			tree->addUniqueComponent(move(bounds));

			return move(tree);
		}
	}
}
