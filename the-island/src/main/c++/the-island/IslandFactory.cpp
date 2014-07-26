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
#include "EntityCategories.h"
#include "IslandFactory.h"
#include "RockFactory.h"
#include "TreeFactory.h"

using namespace simplicity;
using namespace std;

static const unsigned int CLIFF_SUBDIVIDE_MAX_DEPTH = 3;

namespace theisland
{
	namespace IslandFactory
	{
		void addDetail(MeshData& meshData, unsigned int vertexIndex);
		void addFoliage();
		unsigned int adjustIndex(unsigned int index, int adjustment, string adjustmentAxis, string axis, int direction);
		void divideTriangle(MeshData& meshData, unsigned int vertexIndex, unsigned int maxDepth, unsigned int depth = 1);
		void fillHeightMapSector(unsigned int radius, const vector<float>& profile, vector<vector<float>>& heightMap,
				vector<vector<float>>& slopeMap, string axis, int direction);
		float getAdjusted(const vector<vector<float>>& heightMap, unsigned int x, unsigned int z, int adjustment,
				string axis, int direction);
		void getFactors(unsigned int x, unsigned int z, const vector<vector<float>>& heightMap,
				const vector<vector<float>>& slopeMap, string axis, int direction, unsigned int beginIndex,
				unsigned int endIndex, float& heightFactor, float& slopeFactor);
		Vector3 getSmoothNormal(MeshData& meshData, unsigned int x, unsigned int z);
		void getTraversalIndices(unsigned int radius, unsigned int currentRadius, string axis, int direction,
				unsigned int& beginIndexX, unsigned int& endIndexX, unsigned int& beginIndexZ, unsigned int& endIndexZ);
		void growGrass(const MeshData& groundData, unsigned int vertexIndex);
		void initializeMaps(vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, unsigned int edgeLength);
		void setHeight(unsigned int radius, const vector<float>& profile, unsigned int x, unsigned int z,
				vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, float heightFactor);
		void smoothen(MeshData& meshData, unsigned int vertexIndex);

		vector<Vector3> rockPositions;
		vector<Vector3> treePositions;

		void addDetail(MeshData& meshData, unsigned int vertexIndex)
		{
			Vector3 up(0.0, 1.0, 0.0);
			Vector3 center = (meshData[vertexIndex].position + meshData[vertexIndex + 1].position +
					meshData[vertexIndex + 2].position) / 3.0f;
			float maxY = max(meshData[vertexIndex].position.Y(), max(meshData[vertexIndex + 1].position.Y(),
					meshData[vertexIndex + 2].position.Y()));

			// Rocks!
			/////////////////////////
			if (maxY > 0.0f && getRandomBool(0.025f))
			{
				rockPositions.push_back(center);
			}

			// Cliffs!
			if (fabs(dotProduct(meshData[vertexIndex].normal, up)) < 0.2f)
			{
				meshData[vertexIndex].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				meshData[vertexIndex + 1].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
				meshData[vertexIndex + 2].color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);

				divideTriangle(meshData, vertexIndex, CLIFF_SUBDIVIDE_MAX_DEPTH);

				return;
			}

			// Snow!
			if (maxY > 20.0f)
			{
				meshData[vertexIndex].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
				meshData[vertexIndex + 1].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
				meshData[vertexIndex + 2].color = Vector4(0.9f, 0.9f, 0.9f, 1.0f);

				smoothen(meshData, vertexIndex);

				return;
			}

			// Beaches!
			if ((fabs(dotProduct(meshData[vertexIndex].normal, up)) > 0.5f && maxY < 0.5f) ||
					maxY < 0.0f)
			{
				meshData[vertexIndex].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				meshData[vertexIndex + 1].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);
				meshData[vertexIndex + 2].color = Vector4(0.83f, 0.65f, 0.15f, 1.0f);

				smoothen(meshData, vertexIndex);

				return;
			}

			// Grass!
			//growGrass(meshData, vertexIndex);

			// Trees!
			if (getRandomBool(0.025f))
			{
				Vector3 up(0.0f, 1.0f, 0.0f);

				if (dotProduct(center, up) > 0.8f)
				{
					center.Y() -= 0.1f;

					treePositions.push_back(center);
				}
			}

			smoothen(meshData, vertexIndex);
		}

		void addFoliage()
		{
			for (Vector3& rockPosition : rockPositions)
			{
				RockFactory::createRock(rockPosition, getRandomFloat(0.25f, 0.75f));
			}
			rockPositions.clear();

			for (Vector3& treePosition : treePositions)
			{
				TreeFactory::createTree(treePosition);
			}
			treePositions.clear();
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

		void createIsland(unsigned int radius, const vector<float>& profile, unsigned int chunkSize)
		{
			unsigned int edgeLength = radius * 2 + 1;

			// The Island!
			/////////////////////////
			vector<vector<float>> heightMap;
			vector<vector<float>> slopeMap;
			initializeMaps(heightMap, slopeMap, edgeLength);

			heightMap[radius][radius] = profile[0];

			fillHeightMapSector(radius, profile, heightMap, slopeMap, "x", -1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "x", 1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "z", -1);
			fillHeightMapSector(radius, profile, heightMap, slopeMap, "z", 1);

			for (unsigned int x = 0; x < edgeLength - 1; x += chunkSize)
			{
				for (unsigned int z = 0; z < edgeLength - 1; z += chunkSize)
				{
					unique_ptr<Entity> chunk(new Entity(EntityCategories::GROUND));

					shared_ptr<MeshBuffer> buffer = ModelFactory::getInstance()->createBuffer(
							chunkSize * chunkSize * 6 * 10, 0, MeshBuffer::AccessHint::READ);
					unique_ptr<Mesh> mesh = ModelFactory::getInstance()->createHeightMapMesh(heightMap, x,
							x + chunkSize, z, z + chunkSize, buffer, Vector4(0.0f, 0.5f, 0.0f, 1.0f));
					MeshData& meshData = mesh->getData(true, true);

					unsigned int initialVertexCount = meshData.vertexCount;
					for (unsigned int vertexIndex = 0; vertexIndex < initialVertexCount; vertexIndex += 3)
					{
						addDetail(meshData, vertexIndex);
					}

					unique_ptr<Model> bounds =
						ModelFunctions::getSquareBoundsXZ(meshData.vertexData, meshData.vertexCount);

					mesh->releaseData();

					addFoliage();

					Body::Material material;
					material.mass = 0.0f;
					material.friction = 0.5f;
					material.restitution = 0.5f;
					unique_ptr<Body> body = PhysicsFactory::getInstance()->createBody(material, mesh.get(),
							chunk->getTransform(), false);

					chunk->addUniqueComponent(move(mesh));
					chunk->addUniqueComponent(move(bounds));
					chunk->addUniqueComponent(move(body));

					Simplicity::getScene()->addEntity(move(chunk));
				}
			}

			// The Sky!
			/////////////////////////
			unique_ptr<Entity> sky(new Entity);
			rotate(sky->getTransform(), MathConstants::PI * -0.5f, Vector3(1.0f, 0.0f, 0.0f));

			unique_ptr<Mesh> skyMesh = ModelFactory::getInstance()->createHemisphereMesh(1100.0f, 20,
					shared_ptr<MeshBuffer>(), Vector4(0.0f, 0.5f, 0.75f, 1.0f), true);

			sky->addUniqueComponent(move(skyMesh));
			Simplicity::getScene()->addEntity(move(sky));

			// The Ocean!
			/////////////////////////
			unique_ptr<Entity> ocean(new Entity);
			rotate(ocean->getTransform(), MathConstants::PI * -0.5f, Vector3(1.0f, 0.0f, 0.0f));

			unique_ptr<Mesh> oceanMesh =
					ModelFactory::getInstance()->createCylinderMesh(1200.0f, 500.0f, 20, shared_ptr<MeshBuffer>(),
							Vector4(0.0f, 0.4f, 0.6f, 1.0f), true);

			ocean->addUniqueComponent(move(oceanMesh));
			Simplicity::getScene()->addEntity(move(ocean));
		}

		void divideTriangle(MeshData& meshData, unsigned int vertexIndex, unsigned int maxDepth, unsigned int depth)
		{
			Vector3 center = (meshData[vertexIndex].position + meshData[vertexIndex + 1].position +
					meshData[vertexIndex + 2].position) / 3.0f;

			Vector3 divideCenter = center;
			divideCenter += (meshData[vertexIndex].position - center) * 0.5f * getRandomFloat(0.0f, 1.0f);
			divideCenter += (meshData[vertexIndex + 1].position - center) * 0.5f * getRandomFloat(0.0f, 1.0f);
			divideCenter += (meshData[vertexIndex + 2].position - center) * 0.5f * getRandomFloat(0.0f, 1.0f);
			divideCenter += meshData[vertexIndex].normal * getRandomFloat(-0.1f, 0.1f);

			Vertex triangle0[3];
			triangle0[0].color = meshData[vertexIndex].color;
			triangle0[0].position = divideCenter;
			triangle0[1].color = meshData[vertexIndex].color;
			triangle0[1].position = meshData[vertexIndex].position;
			triangle0[2].color = meshData[vertexIndex].color;
			triangle0[2].position = meshData[vertexIndex + 1].position;

			Vector3 edge0 = triangle0[1].position - triangle0[0].position;
			Vector3 edge1 = triangle0[2].position - triangle0[0].position;
			Vector3 normal = crossProduct(edge0, edge1);
			triangle0[0].normal = normal;
			triangle0[1].normal = normal;
			triangle0[2].normal = normal;

			Vertex triangle1[3];
			triangle1[0].color = meshData[vertexIndex].color;
			triangle1[0].position = divideCenter;
			triangle1[1].color = meshData[vertexIndex].color;
			triangle1[1].position = meshData[vertexIndex + 1].position;
			triangle1[2].color = meshData[vertexIndex].color;
			triangle1[2].position = meshData[vertexIndex + 2].position;

			edge0 = triangle1[1].position - triangle1[0].position;
			edge1 = triangle1[2].position - triangle1[0].position;
			normal = crossProduct(edge0, edge1);
			triangle1[0].normal = normal;
			triangle1[1].normal = normal;
			triangle1[2].normal = normal;

			Vertex triangle2[3];
			triangle2[0].color = meshData[vertexIndex].color;
			triangle2[0].position = divideCenter;
			triangle2[1].color = meshData[vertexIndex].color;
			triangle2[1].position = meshData[vertexIndex + 2].position;
			triangle2[2].color = meshData[vertexIndex].color;
			triangle2[2].position = meshData[vertexIndex].position;

			edge0 = triangle2[1].position - triangle2[0].position;
			edge1 = triangle2[2].position - triangle2[0].position;
			normal = crossProduct(edge0, edge1);
			triangle2[0].normal = normal;
			triangle2[1].normal = normal;
			triangle2[2].normal = normal;

			meshData.vertexCount += 6;

			memcpy(&meshData[vertexIndex], triangle0, sizeof(triangle0));
			memcpy(&meshData[meshData.vertexCount - 6], triangle1, sizeof(triangle1));
			memcpy(&meshData[meshData.vertexCount - 3], triangle2, sizeof(triangle2));

			if (depth < maxDepth)
			{
				// The vertex count will be modified by the following recursive calls so save it here.
				unsigned int vertexCount = meshData.vertexCount;

				divideTriangle(meshData, vertexIndex, maxDepth, depth + 1);
				divideTriangle(meshData, vertexCount - 6, maxDepth, depth + 1);
				divideTriangle(meshData, vertexCount - 3, maxDepth, depth + 1);
			}
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

						setHeight(radius, profile, x, z, heightMap, slopeMap, heightFactor);
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

		Vector3 getSmoothNormal(MeshData& meshData, unsigned int x, unsigned int z)
		{
			unsigned int verticesPerGridElement = 6;
			unsigned int edgeLength = static_cast<unsigned int>(sqrt(meshData.vertexCount / verticesPerGridElement));
			unsigned int gridElement = x * edgeLength + z;

			Vector3 normal(0.0f, 0.0f, 0.0f);

			if (x > 0 && z > 0)
			{
				unsigned int frontLeftGridElement = gridElement - edgeLength - 1;

				unsigned int p0 = frontLeftGridElement * verticesPerGridElement;
				unsigned int p1 = frontLeftGridElement * verticesPerGridElement + 1;
				unsigned int p2 = frontLeftGridElement * verticesPerGridElement + 2;
				unsigned int p3 = frontLeftGridElement * verticesPerGridElement + 5;

				Vector3 edge0 = meshData[p1].position - meshData[p0].position;
				edge0.normalize();
				Vector3 edge1 = meshData[p2].position - meshData[p0].position;
				edge1.normalize();
				normal += crossProduct(edge0, edge1);

				Vector3 edge2 = meshData[p2].position - meshData[p0].position;
				edge2.normalize();
				Vector3 edge3 = meshData[p3].position - meshData[p0].position;
				edge3.normalize();
				normal += crossProduct(edge2, edge3);
			}

			if (x > 0 && z < edgeLength)
			{
				unsigned int backLeftGridElement = gridElement - edgeLength;

				unsigned int p0 = backLeftGridElement * verticesPerGridElement;
				unsigned int p2 = backLeftGridElement * verticesPerGridElement + 2;
				unsigned int p3 = backLeftGridElement * verticesPerGridElement + 5;

				Vector3 edge2 = meshData[p2].position - meshData[p0].position;
				edge2.normalize();
				Vector3 edge3 = meshData[p3].position - meshData[p0].position;
				edge3.normalize();
				normal += crossProduct(edge2, edge3);
			}

			if (z > 0 && x < edgeLength)
			{
				unsigned int frontRightGridElement = gridElement - 1;

				unsigned int p0 = frontRightGridElement * verticesPerGridElement;
				unsigned int p1 = frontRightGridElement * verticesPerGridElement + 1;
				unsigned int p2 = frontRightGridElement * verticesPerGridElement + 2;

				Vector3 edge0 = meshData[p1].position - meshData[p0].position;
				edge0.normalize();
				Vector3 edge1 = meshData[p2].position - meshData[p0].position;
				edge1.normalize();
				normal += crossProduct(edge0, edge1);
			}

			if (x < edgeLength && z < edgeLength)
			{
				unsigned int backRightGridElement = gridElement;

				unsigned int p0 = backRightGridElement * verticesPerGridElement;
				unsigned int p1 = backRightGridElement * verticesPerGridElement + 1;
				unsigned int p2 = backRightGridElement * verticesPerGridElement + 2;
				unsigned int p3 = backRightGridElement * verticesPerGridElement + 5;

				Vector3 edge0 = meshData[p1].position - meshData[p0].position;
				edge0.normalize();
				Vector3 edge1 = meshData[p2].position - meshData[p0].position;
				edge1.normalize();
				normal += crossProduct(edge0, edge1);

				Vector3 edge2 = meshData[p2].position - meshData[p0].position;
				edge2.normalize();
				Vector3 edge3 = meshData[p3].position - meshData[p0].position;
				edge3.normalize();
				normal += crossProduct(edge2, edge3);
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

		void growGrass(const MeshData& groundData, unsigned int vertexIndex)
		{
			unique_ptr<Entity> grass(new Entity);

			float averageBladeHeight = 0.5f;
			unsigned int blades = 20;
			unsigned int vertexCount = blades * 3;
			unsigned int indexCount = blades * 6;

			shared_ptr<MeshBuffer> buffer = ModelFactory::getInstance()->createBuffer(vertexCount, indexCount);
			unique_ptr<Mesh> mesh(new Mesh(buffer));
			MeshData& grassData = mesh->getData(false, true);
			grassData.vertexCount = vertexCount;
			grassData.indexCount = indexCount;

			Vector3 center = (groundData[vertexIndex].position + groundData[vertexIndex + 1].position +
					groundData[vertexIndex + 2].position) / 3.0f;

			for (unsigned int blade = 0; blade < blades; blade++)
			{
				Vector3 grassPosition = center;
				for (unsigned int cornerIndex = vertexIndex; cornerIndex < vertexIndex + 3; cornerIndex++)
				{
					Vector3 toCorner = groundData[cornerIndex].position - grassPosition;
					grassPosition += toCorner * getRandomFloat(0.0f, 1.0f);
				}

				float saturation = getRandomFloat(0.25f, 0.75f);
				float height = getRandomFloat(0.5f, 1.5f) * averageBladeHeight;
				float angle = getRandomFloat(0.0f, 1.0f);

				ModelFactory::insertTriangleVertices(grassData.vertexData, vertexCount,
						grassPosition + Vector3(0.0f, height, 0.0f),
						Vector3(sin(angle) * height * 0.1f, -height, cos(angle) * height * 0.1f),
						Vector3(-sin(angle) * height * 0.1f, -height, -cos(angle) * height * 0.1f),
						Vector4(0.0f, saturation, 0.0f, 1.0f));

				ModelFactory::insertTriangleIndices(grassData.indexData, 0, 0);
				ModelFactory::insertTriangleIndices(grassData.indexData, indexCount, 0, true);
			}

			unique_ptr<Model> bounds = ModelFunctions::getCircleBoundsXZ(grassData.vertexData, grassData.vertexCount);

			mesh->releaseData();

			grass->addUniqueComponent(move(mesh));
			grass->addUniqueComponent(move(bounds));

			Simplicity::getScene()->addEntity(move(grass));
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
				vector<vector<float>>& heightMap, vector<vector<float>>& slopeMap, float heightFactor)
		{
			if (getRandomBool(0.8f))
			{
				heightMap[x][z] = heightFactor;// + slopeFactor;
			}
			else
			{
				float distance = floor(sqrt(pow(fabs((float) radius - x), 2) + pow(fabs((float) radius - z), 2)));
				heightMap[x][z] = profile[static_cast<unsigned int>(distance)];
			}

			float randomization = getRandomFloat(-0.1f, 0.1f);
			heightMap[x][z] += randomization;

			slopeMap[x][z] = heightMap[x][z] - heightFactor;
		}

		void smoothen(MeshData& meshData, unsigned int vertexIndex)
		{
			unsigned int gridElement = vertexIndex / 6;
			unsigned int edgeLength = static_cast<unsigned int>(sqrt(meshData.vertexCount / 6));

			unsigned int x = gridElement / edgeLength;
			unsigned int z = gridElement % edgeLength;

			if (vertexIndex % 2 == 0)
			{
				meshData[vertexIndex].normal = getSmoothNormal(meshData, x, z);
				meshData[vertexIndex + 1].normal = getSmoothNormal(meshData, x, z + 1);
				meshData[vertexIndex + 2].normal = getSmoothNormal(meshData, x + 1, z + 1);
			}
			else
			{
				meshData[vertexIndex].normal = getSmoothNormal(meshData, x, z);
				meshData[vertexIndex + 1].normal = getSmoothNormal(meshData, x + 1, z + 1);
				meshData[vertexIndex + 2].normal = getSmoothNormal(meshData, x + 1, z);
			}
		}
	}
}
