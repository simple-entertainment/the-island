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
#ifndef ROCKFACTORY_H_
#define ROCKFACTORY_H_

#include <memory>

#include <simplicity/API.h>

namespace theisland
{
	namespace RockFactory
	{
		SIMPLE_API void createRock(const simplicity::Vector3& position, std::shared_ptr<simplicity::MeshBuffer> buffer,
				float radius, unsigned int detail);
	}
}

#endif /* ROCKFACTORY_H_ */
