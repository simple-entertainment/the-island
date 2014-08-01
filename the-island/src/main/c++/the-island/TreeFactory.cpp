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

		const unsigned int VERTICES_IN_TRUNK_SEGMENT = SEGMENT_DIVISIONS * 4;
		const unsigned int VERTICES_IN_TRUNK_SEGMENTS = VERTICES_IN_TRUNK_SEGMENT * SEGMENTS;
		const unsigned int VERTICES_IN_TRUNK_TOP = SEGMENT_DIVISIONS + 1;
		const unsigned int VERTICES_IN_TRUNK = VERTICES_IN_TRUNK_SEGMENTS + VERTICES_IN_TRUNK_TOP;

		const unsigned int INDICES_IN_TRUNK_SEGMENT = SEGMENT_DIVISIONS * 6;
		const unsigned int INDICES_IN_TRUNK_SEGMENTS = INDICES_IN_TRUNK_SEGMENT * SEGMENTS;
		const unsigned int INDICES_IN_TRUNK_TOP = SEGMENT_DIVISIONS * 3;
		const unsigned int INDICES_IN_TRUNK = INDICES_IN_TRUNK_SEGMENTS + INDICES_IN_TRUNK_TOP;

		unsigned int VERTICES_IN_LEAF = SEGMENTS * 6;

		unsigned int INDICES_IN_LEAF = SEGMENTS * 12;

		vector<shared_ptr<Model>> bounds;

		vector<shared_ptr<Mesh>> leaves;

		vector<shared_ptr<Mesh>> trunks;

		unique_ptr<Entity> createBranch(const Vector3& position, float angleY, float scale);
		shared_ptr<Mesh> createLeaf(const Mesh& branch, shared_ptr<MeshBuffer> buffer);
		shared_ptr<Mesh> createTrunk(shared_ptr<MeshBuffer> buffer);
		void createTrunks();

		unique_ptr<Entity> createBranch(const Vector3& position, float angleY, float scale)
		{
			unique_ptr<Entity> branch(new Entity);

			unsigned int treeIndex = getRandomInt(0, TRUNK_COUNT - 1);
			shared_ptr<Mesh> trunk = trunks[treeIndex];
			shared_ptr<Mesh> leaf = leaves[treeIndex];
			shared_ptr<Model> bounds = TreeFactory::bounds[treeIndex];

			Vector3 scaleVector(1.0f, 1.0f, 1.0f);
			scaleVector *= scale;
			simplicity::scale(branch->getTransform(), scaleVector);

			rotate(branch->getTransform(), angleY, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			rotate(branch->getTransform(), MathConstants::PI * 0.65f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));

			setPosition(branch->getTransform(), position);

			branch->addSharedComponent(trunk);
			branch->addSharedComponent(leaf);
			branch->addSharedComponent(bounds);

			return move(branch);
		}

		shared_ptr<Mesh> createLeaf(const Mesh& branch, shared_ptr<MeshBuffer> buffer)
		{
			if (buffer == nullptr)
			{
				buffer = ModelFactory::getInstance()->createMeshBuffer(VERTICES_IN_LEAF, INDICES_IN_LEAF);
			}

			// Copy the branch vertex data to a local vector... argggh!
			// I think it would have been OK at the moment but if we put the branch and the leaf in the same buffer...
			// there be dragons!
			const MeshData& branchData = branch.getData();
			vector<Vertex> branchVertices(branchData.vertexCount);
			memcpy(branchVertices.data(), branchData.vertexData, branchData.vertexCount * sizeof(Vertex));
			branch.releaseData();

			unique_ptr<Mesh> leaf(new Mesh(buffer));
			MeshData& leafData = leaf->getData(false);

			// Vertices
			leafData.vertexCount = VERTICES_IN_LEAF;

			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				// Find the center of the segment because that's where we want the leaves to originate from.
				Vector3 segmentCenter(0.0f, 0.0f, 0.0f);
				unsigned int indexOffset = segment * VERTICES_IN_TRUNK_SEGMENT;

				for (unsigned int index = indexOffset; index < indexOffset + VERTICES_IN_TRUNK_SEGMENT; index++)
				{
					segmentCenter += branchVertices[index].position;
				}
				segmentCenter /= static_cast<float>(VERTICES_IN_TRUNK_SEGMENT);

				float scale = (1.0f - ((float) segment / SEGMENTS)) * 0.5f;

				float saturation0 = getRandomFloat(0.25f, 0.75f);
				ModelFactory::insertTriangleVertices(leafData.vertexData, segment * 6,
						segmentCenter + Vector3(scale * 10.0f, 0.0f, 0.0f), Vector3(-scale * 10.0f, scale * 2.0f, 0.0f),
						Vector3(-scale * 10.0f, -scale * 2.0f, 0.0f), Vector4(0.0f, saturation0, 0.0f, 1.0f));

				float saturation1 = getRandomFloat(0.25f, 0.75f);
				ModelFactory::insertTriangleVertices(leafData.vertexData, segment * 6 + 3,
						segmentCenter + Vector3(-scale * 10.0f, 0.0f, 0.0f), Vector3(scale * 10.0f, scale * 2.0f, 0.0f),
						Vector3(scale * 10.0f, -scale * 2.0f, 0.0f), Vector4(0.0f, saturation1, 0.0f, 1.0f));
			}

			// Indices
			leafData.indexCount = INDICES_IN_LEAF;

			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				ModelFactory::insertTriangleIndices(leafData.indexData, segment * 12, segment * 6);
				ModelFactory::insertTriangleIndices(leafData.indexData, segment * 12 + 3, segment * 6, true);
				ModelFactory::insertTriangleIndices(leafData.indexData, segment * 12 + 6, segment * 6 + 3);
				ModelFactory::insertTriangleIndices(leafData.indexData, segment * 12 + 9, segment * 6 + 3, true);
			}

			leaf->releaseData();

			return shared_ptr<Mesh>(move(leaf));
		}

		void createTree(const Vector3& position)
		{
			if (trunks.empty())
			{
				createTrunks();
			}

			unsigned int treeIndex = getRandomInt(0, TRUNK_COUNT - 1);
			shared_ptr<Mesh> trunk = trunks[treeIndex];
			const MeshData& trunkData = trunk->getData();

			// Add branches
			vector<unique_ptr<Entity>> branches;
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				// Find the center of the segment because that's where we want the branches to originate from.
				Vector3 segmentCenter(0.0f, 0.0f, 0.0f);
				unsigned int indexOffset = segment * VERTICES_IN_TRUNK_SEGMENT;

				for (unsigned int index = indexOffset; index < indexOffset + VERTICES_IN_TRUNK_SEGMENT; index++)
				{
					segmentCenter += trunkData.vertexData[index].position;
				}
				segmentCenter /= static_cast<float>(VERTICES_IN_TRUNK_SEGMENT);

				float angleY = MathConstants::PI * getRandomFloat(0.0f, 2.0f);
				float scale = (1.0f - ((float) segment / SEGMENTS)) * 0.5f;
				for (unsigned int branch = 0; branch < 3; branch++)
				{
					branches.push_back(move(createBranch(position + segmentCenter, angleY, scale)));
					angleY += MathConstants::PI * 2.0f / 3.0f;
				}
			}

			trunk->releaseData();

			// Assemble the tree!
			unique_ptr<Entity> tree(new Entity);
			Entity* rawTree = tree.get();
			setPosition(tree->getTransform(), position);
			tree->addSharedComponent(trunk);
			tree->addSharedComponent(bounds[treeIndex]);
			Simplicity::getScene()->addEntity(move(tree));

			for (unsigned int index = 0; index < branches.size(); index++)
			{
				Simplicity::getScene()->addEntity(move(branches[index]), *rawTree);
			}
		}

		shared_ptr<Mesh> createTrunk(shared_ptr<MeshBuffer> buffer)
		{
			if (buffer == nullptr)
			{
				buffer = ModelFactory::getInstance()->createMeshBuffer(VERTICES_IN_TRUNK, INDICES_IN_TRUNK);
			}

			unique_ptr<Mesh> trunk(new Mesh(buffer));
			MeshData& trunkData = trunk->getData(false);

			Vector4 color(0.47f, 0.24f, 0.0f, 1.0f);

			float segmentHeight = 2.5f;
			float segmentHeightDelta = segmentHeight / SEGMENTS * 0.5f;
			float segmentRadius = 0.5f;
			float segmentRadiusDelta = segmentRadius / SEGMENTS;

			// Vertices
			trunkData.vertexCount = VERTICES_IN_TRUNK;

			// Trunk Sides
			vector<unique_ptr<Entity>> branches;
			Vector3 center(0.0f, 0.0f, 0.0f);
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				unsigned int indexOffset = segment * VERTICES_IN_TRUNK_SEGMENT;

				ModelFactory::insertTunnelVertices(trunkData.vertexData, indexOffset, segmentRadius, segmentHeight,
						SEGMENT_DIVISIONS, center, color);

				if (segment > 0)
				{
					unsigned int indexOffsetPrevious = (segment - 1) * VERTICES_IN_TRUNK_SEGMENT;
					for (unsigned int segmentDivision = 0; segmentDivision < SEGMENT_DIVISIONS; segmentDivision++)
					{
						trunkData.vertexData[indexOffsetPrevious + segmentDivision * 4 + 1].position =
								trunkData.vertexData[indexOffset + segmentDivision * 4].position;
						trunkData.vertexData[indexOffsetPrevious + segmentDivision * 4 + 3].position =
								trunkData.vertexData[indexOffset + segmentDivision * 4 + 2].position;

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
			ModelFactory::insertCircleVertices(trunkData.vertexData, VERTICES_IN_TRUNK_SEGMENTS, segmentRadius,
					SEGMENT_DIVISIONS, center, color);

			// Rotate so it's standing upright
			ModelFunctions::rotateVertices(trunkData.vertexData, trunkData.vertexCount, MathConstants::PI * 0.5f,
					Vector3(1.0f, 0.0f, 0.0f));

			// Indices
			trunkData.indexCount = INDICES_IN_TRUNK;

			// Trunk Sides
			for (unsigned int segment = 0; segment < SEGMENTS; segment++)
			{
				unsigned int indexOffset = segment * INDICES_IN_TRUNK_SEGMENT;
				unsigned int vertexIndexOffset = segment * VERTICES_IN_TRUNK_SEGMENT;

				ModelFactory::insertTunnelIndices(trunkData.indexData, indexOffset, vertexIndexOffset,
						SEGMENT_DIVISIONS);
			}

			// Trunk Top
			ModelFactory::insertCircleIndices(trunkData.indexData, INDICES_IN_TRUNK_SEGMENTS,
					VERTICES_IN_TRUNK_SEGMENTS, SEGMENT_DIVISIONS, true);

			trunk->releaseData();

			return shared_ptr<Mesh>(move(trunk));
		}

		void createTrunks()
		{
			unsigned int vertexCount = VERTICES_IN_TRUNK * TRUNK_COUNT + VERTICES_IN_LEAF * TRUNK_COUNT;
			unsigned int indexCount = INDICES_IN_TRUNK * TRUNK_COUNT + INDICES_IN_LEAF * TRUNK_COUNT;
			shared_ptr<MeshBuffer> trunkBuffer =
					ModelFactory::getInstance()->createMeshBuffer(vertexCount, indexCount);

			trunks.reserve(TRUNK_COUNT);
			for (unsigned int index = 0; index < TRUNK_COUNT; index++)
			{
				shared_ptr<Mesh> trunk = createTrunk(trunkBuffer);
				shared_ptr<Mesh> leaf = createLeaf(*trunk, trunkBuffer);

				const MeshData& trunkData = trunk->getData();
				shared_ptr<Model> bound =
					ModelFunctions::getCircleBoundsXZ(trunkData.vertexData, trunkData.vertexCount);
				trunk->releaseData();

				trunks.push_back(trunk);
				leaves.push_back(leaf);
				bounds.push_back(bound);
			}
		}
	}
}
