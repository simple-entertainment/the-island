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

#include "IslandFactory.h"

using namespace simplicity;
using namespace std;

namespace theisland
{
	namespace RockFactory
	{
		unique_ptr<Entity> createRock(float radius)
		{
			unique_ptr<Model> mesh = ModelFactory::getInstance().createSphereMesh(radius, 10,
					Vector4(0.6f, 0.6f, 0.6f, 1.0f));

			for (Vertex& vertex : dynamic_cast<Mesh*>(mesh.get())->getVertices())
			{
				vertex.position *= MathFunctions::getRandomFloat(0.75f, 1.25f);
			}

			// TODO Fix normals...

			unique_ptr<Entity> rock(new Entity);
			rock->addUniqueComponent(move(mesh));

			return move(rock);
		}
	}
}
