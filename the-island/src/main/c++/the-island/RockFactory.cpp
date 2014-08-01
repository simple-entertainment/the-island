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
#include "RockFactory.h"

using namespace simplicity;
using namespace std;

namespace theisland
{
	namespace RockFactory
	{
		void createRock(const Vector3& position, shared_ptr<MeshBuffer> buffer, float radius, unsigned int detail)
		{
			unique_ptr<Mesh> mesh = ModelFactory::getInstance()->createSphereMesh(radius, detail, buffer,
					Vector4(0.6f, 0.6f, 0.6f, 1.0f), false);
			MeshData& meshData = mesh->getData(false);

			float variance[detail][detail];
			for (unsigned int latitude = 0; latitude < detail; latitude++)
			{
				for (unsigned int longitude = 0; longitude < detail; longitude++)
				{
					variance[latitude][longitude] = getRandomFloat(0.75f, 1.25f);
				}
			}

			for (unsigned int latitude = 0; latitude < detail; latitude++)
			{
				for (unsigned int longitude = 0; longitude < detail; longitude++)
				{
					unsigned int segmentIndex = (latitude * detail + longitude) * 4;

					meshData.vertexData[segmentIndex].position *= variance[latitude][longitude];
					meshData.vertexData[segmentIndex + 1].position *= variance[(latitude + 1) % detail][longitude];
					meshData.vertexData[segmentIndex + 2].position *= variance[(latitude + 1) % detail][(longitude + 1) % detail];
					meshData.vertexData[segmentIndex + 3].position *= variance[latitude][(longitude + 1) % detail];

					Vector3 edge0 = meshData.vertexData[segmentIndex + 1].position -
							meshData.vertexData[segmentIndex].position;
					Vector3 edge1 = meshData.vertexData[segmentIndex + 2].position -
							meshData.vertexData[segmentIndex].position;
					Vector3 normal = crossProduct(edge0, edge1);
					normal.normalize();

					meshData.vertexData[segmentIndex].normal = normal;
					meshData.vertexData[segmentIndex + 1].normal = normal;
					meshData.vertexData[segmentIndex + 2].normal = normal;
					meshData.vertexData[segmentIndex + 3].normal = normal;
				}
			}

			unique_ptr<Model> bounds = ModelFunctions::getCircleBoundsXZ(meshData.vertexData, meshData.vertexCount);

			mesh->releaseData();

			unique_ptr<Entity> rock(new Entity);
			setPosition(rock->getTransform(), position);
			rock->addUniqueComponent(move(mesh));
			rock->addUniqueComponent(move(bounds));
			Simplicity::getScene()->addEntity(move(rock));
		}
	}
}
