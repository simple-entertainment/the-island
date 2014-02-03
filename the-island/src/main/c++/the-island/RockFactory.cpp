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
		unique_ptr<Entity> createRock(float radius)
		{
			unique_ptr<Mesh> mesh = ModelFactory::getInstance().createSphereMesh(radius, 10,
					Vector4(0.6f, 0.6f, 0.6f, 1.0f), false);

			float variance[10][10];
			for (unsigned int latitude = 0; latitude < 10; latitude++)
			{
				for (unsigned int longitude = 0; longitude < 10; longitude++)
				{
					variance[latitude][longitude] = getRandomFloat(0.75f, 1.25f);
				}
			}

			vector<Vertex>& vertices = mesh->getVertices();
			for (unsigned int latitude = 0; latitude < 10; latitude++)
			{
				for (unsigned int longitude = 0; longitude < 10; longitude++)
				{
					unsigned int segmentIndex = (latitude * 10 + longitude) * 4;

					vertices[segmentIndex].position *= variance[latitude][longitude];
					vertices[segmentIndex + 1].position *= variance[(latitude + 1) % 10][longitude];
					vertices[segmentIndex + 2].position *= variance[(latitude + 1) % 10][(longitude + 1) % 10];
					vertices[segmentIndex + 3].position *= variance[latitude][(longitude + 1) % 10];

					Vector3 edge0 = vertices[segmentIndex + 1].position - vertices[segmentIndex].position;
					Vector3 edge1 = vertices[segmentIndex + 2].position - vertices[segmentIndex].position;
					Vector3 normal = crossProduct(edge0, edge1);
					normal.normalize();

					vertices[segmentIndex].normal = normal;
					vertices[segmentIndex + 1].normal = normal;
					vertices[segmentIndex + 2].normal = normal;
					vertices[segmentIndex + 3].normal = normal;
				}
			}

			unique_ptr<Model> bounds = ModelFunctions::getCircleBoundsXZ(mesh->getVertices());

			unique_ptr<Entity> rock(new Entity);
			rock->addUniqueComponent(move(mesh));
			rock->addUniqueComponent(move(bounds));

			return move(rock);
		}
	}
}
