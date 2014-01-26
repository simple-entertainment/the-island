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
#include <simplicity/math/MathConstants.h>
#include <simplicity/math/MathFunctions.h>
#include <simplicity/model/ModelFactory.h>
#include <simplicity/Simplicity.h>

#include "IslandFactory.h"
#include "RockFactory.h"
#include "TreeFactory.h"

using namespace simplicity;
using namespace std;

namespace theisland
{
	namespace IslandFactory
	{
		void addDetail(Vertex& vertex0, Vertex& vertex1, Vertex& vertex2);
		unsigned int adjustIndex(unsigned int index, int adjustment, string adjustmentAxis, string axis, int direction);
		void fillHeightMapSector(unsigned int radius, const vector<float>& profile, vector<vector<float>>& heightMap,
				vector<vector<float>>& slopeMap, string axis, int direction);
		float getAdjusted(const vector<vector<float>>& heightMap, unsigned int x, unsigned int z, int adjustment,
				string axis, int direction);
		void getFactors(unsigned int x, unsigned int z, const vector<vector<float>>& heightMap,
				const vector<vector<float>>& slopeMap, string axis, int direction, unsigned int beginIndex,
				unsigned int endIndex, float& heightFactor, float& slopeFactor);
		void getTraversalIndices(unsigned int radius, unsigned int currentRadius, string axis, int direction,
				unsigned int& beginIndexX, unsigned int& endIndexX, unsigned int& beginIndexZ, unsigned int& endIndexZ);
		void growGrass(const Vector3& position0, const Vector3& position1, const Vector3& position2);
		void growTree(const Vector3& position0, const Vector3& position1, const Vector3& position2);
		void initializeMaps(vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, unsigned int edgeLength);
		void setHeight(unsigned int radius, const vector<float>& profile, unsigned int x, unsigned int z,
				vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, float heightFactor,
				float slopeFactor);

		void addDetail(Vertex& vertex0, Vertex& vertex1, Vertex& vertex2)
		{
			Vector3 up(0.0, 1.0, 0.0);
			float maxY = max(vertex0.position.Y(), max(vertex1.position.Y(), vertex2.position.Y()));

			// Cliffs!
			if (abs(dotProduct(vertex0.normal, up)) < 0.2f)
			{
				vertex0.color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				vertex1.color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				vertex2.color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);

				return;
			}

			// Snow!
			if (maxY > 20.0f)
			{
				vertex0.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				vertex1.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				vertex2.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

				return;
			}

			// Beaches!
			if ((abs(dotProduct(vertex0.normal, up)) > 0.5f && maxY < 0.5f) ||
					maxY < 0.0f)
			{
				vertex0.color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				vertex1.color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				vertex2.color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);

				return;
			}

			// Grass!
			growGrass(vertex0.position, vertex1.position, vertex2.position);

			// Trees!
			if (MathFunctions::getRandomBool(0.025f))
			{
				growTree(vertex0.position, vertex1.position, vertex2.position);
			}
		}

		unsigned int adjustIndex(unsigned int index, int adjustment, string adjustmentAxis, string axis, int direction)
		{
			if (adjustmentAxis == axis)
			{
				return index - direction;
			}
			else
			{
				return index + adjustment;
			}
		}

		unique_ptr<Entity> createIsland(unsigned int radius, const vector<float>& profile)
		{
			unique_ptr<Entity> island(new Entity);

			// The Island!
			/////////////////////////
			vector<vector<float>> heightMap;
			vector<vector<float>> slopeMap;
			initializeMaps(heightMap, slopeMap, radius * 2 + 1);

			heightMap[radius][radius] = profile[0];

			fillHeightMapSector(radius, profile, heightMap, slopeMap, "x", -1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "x", 1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "z", -1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "z", 1);

			unique_ptr<Mesh> mesh = ModelFactory::getInstance().createHeightMapMesh(heightMap,
					Vector4(0.0f, 0.5f, 0.0f, 1.0f));

			for (unsigned int index = 0; index < mesh->getVertices().size(); index += 3)
			{
				addDetail(mesh->getVertices()[index], mesh->getVertices()[index + 1], mesh->getVertices()[index + 2]);
			}

			island->addUniqueComponent(move(mesh));

			// The Ocean!
			/////////////////////////
			unique_ptr<Entity> ocean(new Entity);
			MathFunctions::rotate(ocean->getTransformation(), MathConstants::PI * 0.5f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
			unique_ptr<Model> oceanModel =
					ModelFactory::getInstance().createSquareMesh(500.0f, Vector4(0.0f, 0.4f, 0.6f, 1.0f), false);
			ocean->addUniqueComponent(move(oceanModel));
			Simplicity::addEntity(move(ocean));

			for (float x = 0; x < radius * 2 + 1; x++)
			{
				for (float z = 0; z < radius * 2 + 1; z++)
				{
					if (heightMap[x][z] <= 0.0f)
					{
						continue;
					}

					// Rocks!
					/////////////////////////
					if (MathFunctions::getRandomBool(0.025f))
					{
						unique_ptr<Entity> rock = RockFactory::createRock(MathFunctions::getRandomFloat(0.25f, 0.75f));
						MathFunctions::setTranslation(rock->getTransformation(),
								Vector3(x - radius, heightMap[x][z], z - radius));
						Simplicity::addEntity(move(rock));
					}
				}
			}

			return move(island);
		}

		void fillHeightMapSector(unsigned int radius, const vector<float>& profile, vector<vector<float>>& heightMap,
				vector<vector<float>>& slopeMap, string axis, int direction)
		{
			for (unsigned int currentRadius = 1; currentRadius <= radius; currentRadius++)
			{
				unsigned int beginIndexX = 0;
				unsigned int endIndexX = 0;
				unsigned int beginIndexZ = 0;
				unsigned int endIndexZ = 0;
				getTraversalIndices(radius, currentRadius, axis, direction, beginIndexX, endIndexX, beginIndexZ,
						endIndexZ);

				for (unsigned int x = beginIndexX; x <= endIndexX; x++)
				{
					for (unsigned int z = beginIndexZ; z <= endIndexZ; z++)
					{
						float heightFactor = 0.0f;
						float slopeFactor = 0.0f;
						if (axis == "x")
						{
							getFactors(x, z, heightMap, slopeMap, axis, direction, beginIndexZ, endIndexZ, heightFactor,
									slopeFactor);
						}
						else if (axis == "z")
						{
							getFactors(x, z, heightMap, slopeMap, axis, direction, beginIndexX, endIndexX, heightFactor,
																slopeFactor);
						}

						setHeight(radius, profile, x, z, heightMap, slopeMap, heightFactor, slopeFactor);
					}
				}
			}
		}

		float getAdjusted(const vector<vector<float>>& source, unsigned int x, unsigned int z, int adjustment,
				string axis, int direction)
		{
			return source[adjustIndex(x, adjustment, "x", axis, direction)]
			              [adjustIndex(z, adjustment, "z", axis, direction)];
		}

		void getFactors(unsigned int x, unsigned int z, const vector<vector<float>>& heightMap,
				const vector<vector<float>>& slopeMap, string axis, int direction, unsigned int beginIndex,
				unsigned int endIndex, float& heightFactor, float& slopeFactor)
		{
			unsigned int traversalIndex = 0;
			if (axis == "x")
			{
				traversalIndex = z;
			}
			else if (axis == "z")
			{
				traversalIndex = x;
			}

			if (traversalIndex == beginIndex)
			{
				heightFactor = getAdjusted(heightMap, x, z, 1, axis, direction);
				slopeFactor = getAdjusted(slopeMap, x, z, 1, axis, direction);
			}
			else if (traversalIndex == endIndex)
			{
				heightFactor = getAdjusted(heightMap, x, z, -1, axis, direction);
				slopeFactor = getAdjusted(slopeMap, x, z, -1, axis, direction);
			}
			else if (traversalIndex == beginIndex + 1)
			{
				heightFactor =
						getAdjusted(heightMap, x, z, 0, axis, direction) +
						getAdjusted(heightMap, x, z, 1, axis, direction);
				heightFactor /= 2.0f;
				slopeFactor =
						getAdjusted(slopeMap, x, z, 0, axis, direction) +
						getAdjusted(slopeMap, x, z, 1, axis, direction);
				slopeFactor /= 2.0f;
			}
			else if (traversalIndex == endIndex - 1)
			{
				heightFactor =
						getAdjusted(heightMap, x, z, -1, axis, direction) +
						getAdjusted(heightMap, x, z, 0, axis, direction);
				heightFactor /= 2.0f;
				slopeFactor =
						getAdjusted(slopeMap, x, z, -1, axis, direction) +
						getAdjusted(slopeMap, x, z, 0, axis, direction);
				slopeFactor /= 2.0f;
			}
			else
			{
				heightFactor =
						getAdjusted(heightMap, x, z, -1, axis, direction) +
						getAdjusted(heightMap, x, z, 0, axis, direction) +
						getAdjusted(heightMap, x, z, 1, axis, direction);
				heightFactor /= 3.0f;
				slopeFactor =
						getAdjusted(slopeMap, x, z, -1, axis, direction) +
						getAdjusted(slopeMap, x, z, 0, axis, direction) +
						getAdjusted(slopeMap, x, z, 1, axis, direction);
				slopeFactor /= 3.0f;
			}
		}

		void getTraversalIndices(unsigned int radius, unsigned int currentRadius, string axis, int direction,
				unsigned int& beginIndexX, unsigned int& endIndexX, unsigned int& beginIndexZ, unsigned int& endIndexZ)
		{
			unsigned int beginIndex = radius - currentRadius;
			unsigned int endIndex = radius + currentRadius;

			if (axis == "x")
			{
				if (direction < 0)
				{
					beginIndexX = beginIndex;
					endIndexX = beginIndex;
				}
				else if (direction > 0)
				{
					beginIndexX = endIndex;
					endIndexX = endIndex;
				}

				beginIndexZ = beginIndex;
				endIndexZ = endIndex;
			}

			if (axis == "z")
			{
				beginIndexX = beginIndex;
				endIndexX = endIndex;

				if (direction < 0)
				{
					beginIndexZ = beginIndex;
					endIndexZ = beginIndex;
				}
				else if (direction > 0)
				{
					beginIndexZ = endIndex;
					endIndexZ = endIndex;
				}
			}
		}

		void growGrass(const Vector3& position0, const Vector3& position1, const Vector3& position2)
		{
			unique_ptr<Entity> grass(new Entity);

			unsigned int blades = 20;
			float averageBladeHeight = 0.5f;
			vector<Vertex> vertices(blades * 3);
			vector<unsigned int> indices(blades * 6);

			Vector3 center = (position0 + position1 + position2) / 3.0f;
			Vector3 corners[3];
			corners[0] = position0;
			corners[1] = position1;
			corners[2] = position2;

			for (unsigned int blade = 0; blade < blades; blade++)
			{
				Vector3 grassPosition = center;
				for (const Vector3& corner : corners)
				{
					Vector3 toCorner = corner - grassPosition;
					grassPosition += toCorner * MathFunctions::getRandomFloat(0.0f, 1.0f);
				}

				float saturation = MathFunctions::getRandomFloat(0.25f, 0.75f);
				float height = MathFunctions::getRandomFloat(0.5f, 1.5f) * averageBladeHeight;
				float angle = MathFunctions::getRandomFloat(0.0f, 1.0f);

				ModelFactory::addTriangleVertexList(vertices, blade * 3, Vector4(0.0f, saturation, 0.0f, 1.0f),
						grassPosition + Vector3(0.0f, height, 0.0f),
						//Vector3(height * 0.1f, -height, 0.0f),
						//Vector3(-height * 0.1f, -height, 0.0f));
						Vector3(sin(angle) * height * 0.1f, -height, cos(angle) * height * 0.1f),
						Vector3(-sin(angle) * height * 0.1f, -height, -cos(angle) * height * 0.1f));

				ModelFactory::addTriangleIndexList(indices, blade * 6, blade * 3);
				ModelFactory::addTriangleIndexList(indices, blade * 6 + 3, blade * 3, true);
			}

			unique_ptr<Model> grassModel =
					ModelFactory::getInstance().createMesh(vertices, indices);
			grass->addUniqueComponent(move(grassModel));

			Simplicity::addEntity(move(grass));
		}

		void growTree(const Vector3& position0, const Vector3& position1, const Vector3& position2)
		{
			Vector3 center = (position0 + position1 + position2) / 3.0f;
			Vector3 up(0.0f, 1.0f, 0.0f);

			if (dotProduct(center, up) > 0.8f)
			{
				center.Y() -= 0.1f;

				unique_ptr<Entity> tree = TreeFactory::createTree(MathFunctions::getRandomFloat(100.0f, 200.0f));
				MathFunctions::setTranslation(tree->getTransformation(), center);
				Simplicity::addEntity(move(tree));
			}
		}

		void initializeMaps(vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, unsigned int edgeLength)
		{
			heightMap.reserve(edgeLength);
			slopeMap.reserve(edgeLength);
			for (unsigned int x = 0; x < edgeLength; x++)
			{
				heightMap.push_back(vector<float>());
				heightMap[x].reserve(edgeLength);
				slopeMap.push_back(vector<float>());
				slopeMap[x].reserve(edgeLength);

				for (unsigned int z = 0; z < edgeLength; z++)
				{
					heightMap[x].push_back(0.0f);
					slopeMap[x].push_back(0.0f);
				}
			}
		}

		void setHeight(unsigned int radius, const vector<float>& profile, unsigned int x, unsigned int z,
				vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, float heightFactor,
				float slopeFactor)
		{
			if (MathFunctions::getRandomBool(0.8f))
			{
				heightMap[x][z] = heightFactor;// + slopeFactor;
			}
			else
			{
				float distance = floor(sqrt(pow(abs((float) radius - x), 2) + pow(abs((float) radius - z), 2)));
				heightMap[x][z] = profile[distance];
			}

			float randomization = MathFunctions::getRandomFloat(-0.1f, 0.1f);
			heightMap[x][z] += randomization;

			slopeMap[x][z] = heightMap[x][z] - heightFactor;
		}
	}
}
