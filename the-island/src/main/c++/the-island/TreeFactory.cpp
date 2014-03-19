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
		const unsigned int SEGMENTS = 4;
		const unsigned int SEGMENT_DIVISIONS = 5;

		const unsigned int TRUNK_COUNT = 5;

		vector<shared_ptr<Model>> bounds;

		vector<shared_ptr<Mesh>> leaves;

		vector<shared_ptr<Mesh>> trunks;

		unique_ptr<Entity> createBranch(const Vector3& position, float angleY, float scale);
		shared_ptr<Mesh> createLeaf(const Mesh& branch);
		shared_ptr<Mesh> createTrunk();
		void createTrunks();

		unique_ptr<Entity> createBranch(const Vector3& position, float angleY, float scale)
		{
			unique_ptr<Entity> branch(new Entity);

			unsigned int treeIndex = getRandomInt(0, TRUNK_COUNT - 1);
			shared_ptr<Mesh> trunk = trunks[treeIndex];
			shared_ptr<Mesh> leaf = leaves[treeIndex];
			unique_ptr<Model> bounds = ModelFunctions::getSquareBoundsXZ(trunk->getVertices());

			Vector3 scaleVector(1.0f, 1.0f, 1.0f);
			scaleVector *= scale;
			simplicity::scale(branch->getTransform(), scaleVector);

			rotate(branch->getTransform(), angleY, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			rotate(branch->getTransform(), MathConstants::PI * 0.65f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));

			setPosition(branch->getTransform(), position);

			branch->addSharedComponent(trunk);
			branch->addSharedComponent(leaf);
			branch->addUniqueComponent(move(bounds));

			return move(branch);
		}

		shared_ptr<Mesh> createLeaf(const Mesh& branch)
		{
			unsigned int verticesInTrunkSegment = SEGMENT_DIVISIONS * 4;

			// Vertices
			vector<Vertex> vertices(SEGMENTS * 6);
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				// Find the center of the segment because that's where we want the leaves to originate from.
				Vector3 segmentCenter(0.0f, 0.0f, 0.0f);
				unsigned int indexOffset = segment * verticesInTrunkSegment;

				for (unsigned int index = indexOffset; index < indexOffset + verticesInTrunkSegment; index++)
				{
					segmentCenter += branch.getVertices()[index].position;
				}
				segmentCenter /= verticesInTrunkSegment;

				float scale = (1.0f - ((float) segment / SEGMENTS)) * 0.5f;

				float saturation0 = getRandomFloat(0.25f, 0.75f);
				ModelFactory::insertTriangleVertices(vertices, segment * 6,
						segmentCenter + Vector3(scale * 10.0f, 0.0f, 0.0f), Vector3(-scale * 10.0f, scale * 2.0f, 0.0f),
						Vector3(-scale * 10.0f, -scale * 2.0f, 0.0f), Vector4(0.0f, saturation0, 0.0f, 1.0f));

				float saturation1 = getRandomFloat(0.25f, 0.75f);
				ModelFactory::insertTriangleVertices(vertices, segment * 6 + 3,
						segmentCenter + Vector3(-scale * 10.0f, 0.0f, 0.0f), Vector3(scale * 10.0f, scale * 2.0f, 0.0f),
						Vector3(scale * 10.0f, -scale * 2.0f, 0.0f), Vector4(0.0f, saturation1, 0.0f, 1.0f));
			}

			// Indices
			vector<unsigned int> indices(SEGMENTS * 12);
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				ModelFactory::insertTriangleIndices(indices, segment * 12, segment * 6);
				ModelFactory::insertTriangleIndices(indices, segment * 12 + 3, segment * 6, true);
				ModelFactory::insertTriangleIndices(indices, segment * 12 + 6, segment * 6 + 3);
				ModelFactory::insertTriangleIndices(indices, segment * 12 + 9, segment * 6 + 3, true);
			}

			unique_ptr<Mesh> leaf = ModelFactory::getInstance().createMesh(vertices, indices);
			return shared_ptr<Mesh>(move(leaf));
		}

		void createTree(const Vector3& position, float age)
		{
			unsigned int verticesInTrunkSegment = SEGMENT_DIVISIONS * 4;

			if (trunks.empty())
			{
				createTrunks();
			}

			unsigned int treeIndex = getRandomInt(0, TRUNK_COUNT - 1);
			shared_ptr<Mesh> trunk = trunks[treeIndex];

			// Add branches
			vector<unique_ptr<Entity>> branches;
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				// Find the center of the segment because that's where we want the branches to originate from.
				Vector3 segmentCenter(0.0f, 0.0f, 0.0f);
				unsigned int indexOffset = segment * verticesInTrunkSegment;

				for (unsigned int index = indexOffset; index < indexOffset + verticesInTrunkSegment; index++)
				{
					segmentCenter += trunk->getVertices()[index].position;
				}
				segmentCenter /= verticesInTrunkSegment;

				float angleY = MathConstants::PI * getRandomFloat(0.0f, 2.0f);
				float scale = (1.0f - ((float) segment / SEGMENTS)) * 0.5f;
				for (unsigned int branch = 0; branch < 3; branch++)
				{
					branches.push_back(move(createBranch(position + segmentCenter, angleY, scale)));
					angleY += MathConstants::PI * 2.0f / 3.0f;
				}
			}

			// Assemble the tree!
			unique_ptr<Entity> tree(new Entity);
			Entity* rawTree = tree.get();
			setPosition(tree->getTransform(), position);
			tree->addSharedComponent(trunk);
			tree->addSharedComponent(bounds[treeIndex]);
			Simplicity::addEntity(move(tree));

			for (unsigned int index = 0; index < branches.size(); index++)
			{
				Simplicity::addEntity(move(branches[index]), *rawTree);
			}
		}

		shared_ptr<Mesh> createTrunk()
		{
			Vector4 color(0.47f, 0.24f, 0.0f, 1.0f);

			float segmentHeight = 2.5f;
			float segmentHeightDelta = segmentHeight / SEGMENTS * 0.5f;
			float segmentRadius = 0.5f;
			float segmentRadiusDelta = segmentRadius / SEGMENTS;

			// Vertices
			unsigned int verticesInTrunkSegment = SEGMENT_DIVISIONS * 4;
			unsigned int verticesInTrunkSegments = verticesInTrunkSegment * SEGMENTS;
			unsigned int verticesInTrunkTop = SEGMENT_DIVISIONS + 1;
			vector<Vertex> vertices(verticesInTrunkSegments + verticesInTrunkTop);

			// Trunk Sides
			vector<unique_ptr<Entity>> branches;
			Vector3 center(0.0f, 0.0f, 0.0f);
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				unsigned int indexOffset = segment * verticesInTrunkSegment;

				ModelFactory::insertTunnelVertices(vertices, indexOffset, segmentRadius, segmentHeight,
						SEGMENT_DIVISIONS, center, color);

				if (segment > 0)
				{
					unsigned int indexOffsetPrevious = (segment - 1) * verticesInTrunkSegment;
					for (unsigned int segmentDivision = 0; segmentDivision < SEGMENT_DIVISIONS; segmentDivision++)
					{
						vertices[indexOffsetPrevious + segmentDivision * 4 + 1].position =
								vertices[indexOffset + segmentDivision * 4].position;
						vertices[indexOffsetPrevious + segmentDivision * 4 + 3].position =
								vertices[indexOffset + segmentDivision * 4 + 2].position;

						// TODO correct normals too...
					}
				}

				center.Z() -= segmentHeight;

				if (segment != SEGMENTS - 1)
				{
					center.X() += getRandomFloat(-segmentRadius, segmentRadius);
					center.Y() += getRandomFloat(-segmentRadius, segmentRadius);

					segmentHeight -= segmentHeightDelta;
					segmentRadius -= segmentRadiusDelta;
				}
			}

			// Trunk Top
			ModelFactory::insertCircleVertices(vertices, verticesInTrunkSegments, segmentRadius, SEGMENT_DIVISIONS,
					center, color);

			// Rotate so it's standing upright
			ModelFunctions::rotateVertices(vertices, MathConstants::PI * 0.5f, Vector3(1.0f, 0.0f, 0.0f));

			// Indices
			unsigned int indicesInTrunkSegment = SEGMENT_DIVISIONS * 6;
			unsigned int indicesInTrunkSegments = indicesInTrunkSegment * SEGMENTS;
			unsigned int indicesInTrunkTop = SEGMENT_DIVISIONS * 3;
			vector<unsigned int> indices(indicesInTrunkSegments + indicesInTrunkTop);

			// Trunk Sides
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				unsigned int indexOffset = segment * indicesInTrunkSegment;
				unsigned int vertexIndexOffset = segment * verticesInTrunkSegment;

				ModelFactory::insertTunnelIndices(indices, indexOffset, vertexIndexOffset, SEGMENT_DIVISIONS);
			}

			// Trunk Top
			ModelFactory::insertCircleIndices(indices, indicesInTrunkSegments, verticesInTrunkSegments,
					SEGMENT_DIVISIONS, true);

			unique_ptr<Mesh> trunk = ModelFactory::getInstance().createMesh(vertices, indices);
			return shared_ptr<Mesh>(move(trunk));
		}

		void createTrunks()
		{
			trunks.reserve(TRUNK_COUNT);
			for (unsigned int index = 0; index < TRUNK_COUNT; index++)
			{
				shared_ptr<Mesh> trunk = createTrunk();
				shared_ptr<Mesh> leaf = createLeaf(*trunk);
				shared_ptr<Model> bound(move(ModelFunctions::getCircleBoundsXZ(trunk->getVertices())));

				trunks.push_back(trunk);
				leaves.push_back(leaf);
				bounds.push_back(bound);
			}
		}
	}
}
