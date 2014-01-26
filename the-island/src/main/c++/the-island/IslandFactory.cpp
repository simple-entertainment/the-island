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
		void addDetail(vector<Vertex>& vertices, unsigned int index);
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
		void growGrass(const vector<Vertex>& vertices, unsigned int index);
		void growTree(const vector<Vertex>& vertices, unsigned int index);
		void initializeMaps(vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, unsigned int edgeLength);
		void setHeight(unsigned int radius, const vector<float>& profile, unsigned int x, unsigned int z,
				vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, float heightFactor,
				float slopeFactor);
		void smoothen(vector<Vertex>& vertices, unsigned int index);
		Vector3 getSmoothNormal(const vector<Vertex>& vertices, unsigned int x, unsigned int z);

		void addDetail(vector<Vertex>& vertices, unsigned int index)
		{
			Vector3 up(0.0, 1.0, 0.0);
			float maxY = max(vertices[index].position.Y(), max(vertices[index + 1].position.Y(),
					vertices[index + 2].position.Y()));

			// Cliffs!
			if (abs(dotProduct(vertices[index].normal, up)) < 0.2f)
			{
				vertices[index].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				vertices[index + 1].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				vertices[index + 2].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);

				return;
			}

			// Snow!
			if (maxY > 20.0f)
			{
				vertices[index].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
				vertices[index + 1].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
				vertices[index + 2].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);

				smoothen(vertices, index);

				return;
			}

			// Beaches!
			if ((abs(dotProduct(vertices[index].normal, up)) > 0.5f && maxY < 0.5f) ||
					maxY < 0.0f)
			{
				vertices[index].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				vertices[index + 1].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				vertices[index + 2].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);

				smoothen(vertices, index);

				return;
			}

			// Grass!
			growGrass(vertices, index);

			// Trees!
			if (MathFunctions::getRandomBool(0.025f))
			{
				growTree(vertices, index);
			}

			smoothen(vertices, index);
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
				addDetail(mesh->getVertices(), index);
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

		Vector3 getSmoothNormal(const vector<Vertex>& vertices, unsigned int x, unsigned int z)
		{
			unsigned int verticesPerGridElement = 6;
			unsigned int edgeLength = sqrt(vertices.size() / verticesPerGridElement);
			unsigned int gridElement = x * edgeLength + z;

			Vector3 normal(0.0f, 0.0f, 0.0f);

			if (x > 0 && z > 0)
			{
				unsigned int frontLeftGridElement = gridElement - edgeLength - 1;

				unsigned int p0 = frontLeftGridElement * verticesPerGridElement;
				unsigned int p1 = frontLeftGridElement * verticesPerGridElement + 1;
				unsigned int p2 = frontLeftGridElement * verticesPerGridElement + 2;
				unsigned int p3 = frontLeftGridElement * verticesPerGridElement + 5;

				Vector3 edge0 = vertices[p1].position - vertices[p0].position;
				edge0.normalize();
				Vector3 edge1 = vertices[p2].position - vertices[p0].position;
				edge1.normalize();
				normal += MathFunctions::crossProduct(edge0, edge1);

				Vector3 edge2 = vertices[p2].position - vertices[p0].position;
				edge2.normalize();
				Vector3 edge3 = vertices[p3].position - vertices[p0].position;
				edge3.normalize();
				normal += MathFunctions::crossProduct(edge2, edge3);
			}

			if (x > 0 && z < edgeLength)
			{
				unsigned int backLeftGridElement = gridElement - edgeLength;

				unsigned int p0 = backLeftGridElement * verticesPerGridElement;
				unsigned int p2 = backLeftGridElement * verticesPerGridElement + 2;
				unsigned int p3 = backLeftGridElement * verticesPerGridElement + 5;

				Vector3 edge2 = vertices[p2].position - vertices[p0].position;
				edge2.normalize();
				Vector3 edge3 = vertices[p3].position - vertices[p0].position;
				edge3.normalize();
				normal += MathFunctions::crossProduct(edge2, edge3);
			}

			if (z > 0 && x < edgeLength)
			{
				unsigned int frontRightGridElement = gridElement - 1;

				unsigned int p0 = frontRightGridElement * verticesPerGridElement;
				unsigned int p1 = frontRightGridElement * verticesPerGridElement + 1;
				unsigned int p2 = frontRightGridElement * verticesPerGridElement + 2;

				Vector3 edge0 = vertices[p1].position - vertices[p0].position;
				edge0.normalize();
				Vector3 edge1 = vertices[p2].position - vertices[p0].position;
				edge1.normalize();
				normal += MathFunctions::crossProduct(edge0, edge1);
			}

			if (x < edgeLength && z < edgeLength)
			{
				unsigned int backRightGridElement = gridElement;

				unsigned int p0 = backRightGridElement * verticesPerGridElement;
				unsigned int p1 = backRightGridElement * verticesPerGridElement + 1;
				unsigned int p2 = backRightGridElement * verticesPerGridElement + 2;
				unsigned int p3 = backRightGridElement * verticesPerGridElement + 5;

				Vector3 edge0 = vertices[p1].position - vertices[p0].position;
				edge0.normalize();
				Vector3 edge1 = vertices[p2].position - vertices[p0].position;
				edge1.normalize();
				normal += MathFunctions::crossProduct(edge0, edge1);

				Vector3 edge2 = vertices[p2].position - vertices[p0].position;
				edge2.normalize();
				Vector3 edge3 = vertices[p3].position - vertices[p0].position;
				edge3.normalize();
				normal += MathFunctions::crossProduct(edge2, edge3);
			}

			normal.normalize();

			return normal;
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

		void growGrass(const vector<Vertex>& vertices, unsigned int index)
		{
			unique_ptr<Entity> grass(new Entity);

			float averageBladeHeight = 0.5f;
			unsigned int blades = 20;
			vector<Vertex> bladeVertices(blades * 3);
			vector<unsigned int> bladeIndices(blades * 6);

			Vector3 center = (vertices[index].position + vertices[index + 1].position + vertices[index + 2].position) /
					3.0f;

			for (unsigned int blade = 0; blade < blades; blade++)
			{
				Vector3 grassPosition = center;
				for (unsigned int cornerIndex = index; cornerIndex < index + 3; cornerIndex++)
				{
					Vector3 toCorner = vertices[cornerIndex].position - grassPosition;
					grassPosition += toCorner * MathFunctions::getRandomFloat(0.0f, 1.0f);
				}

				float saturation = MathFunctions::getRandomFloat(0.25f, 0.75f);
				float height = MathFunctions::getRandomFloat(0.5f, 1.5f) * averageBladeHeight;
				float angle = MathFunctions::getRandomFloat(0.0f, 1.0f);

				ModelFactory::addTriangleVertexList(bladeVertices, blade * 3, Vector4(0.0f, saturation, 0.0f, 1.0f),
						grassPosition + Vector3(0.0f, height, 0.0f),
						//Vector3(height * 0.1f, -height, 0.0f),
						//Vector3(-height * 0.1f, -height, 0.0f));
						Vector3(sin(angle) * height * 0.1f, -height, cos(angle) * height * 0.1f),
						Vector3(-sin(angle) * height * 0.1f, -height, -cos(angle) * height * 0.1f));

				ModelFactory::addTriangleIndexList(bladeIndices, blade * 6, blade * 3);
				ModelFactory::addTriangleIndexList(bladeIndices, blade * 6 + 3, blade * 3, true);
			}

			unique_ptr<Model> grassModel =
					ModelFactory::getInstance().createMesh(bladeVertices, bladeIndices);
			grass->addUniqueComponent(move(grassModel));

			Simplicity::addEntity(move(grass));
		}

		void growTree(const vector<Vertex>& vertices, unsigned int index)
		{
			Vector3 center = (vertices[index].position + vertices[index + 1].position + vertices[index + 2].position) /
					3.0f;
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

		void smoothen(vector<Vertex>& vertices, unsigned int index)
		{
			unsigned int gridElement = index / 6;
			unsigned int edgeLength = sqrt(vertices.size() / 6);

			unsigned int x = gridElement / edgeLength;
			unsigned int z = gridElement % edgeLength;

			if (index % 2 == 0)
			{
				Vector3 before = vertices[index].normal;
				vertices[index].normal = getSmoothNormal(vertices, x, z);
				Vector3 after = vertices[index].normal;
				vertices[index + 1].normal = getSmoothNormal(vertices, x, z + 1);
				vertices[index + 2].normal = getSmoothNormal(vertices, x + 1, z + 1);
			}
			else
			{
				vertices[index].normal = getSmoothNormal(vertices, x, z);
				vertices[index + 1].normal = getSmoothNormal(vertices, x + 1, z + 1);
				vertices[index + 2].normal = getSmoothNormal(vertices, x + 1, z);
			}
		}
	}
}
