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
#ifndef TREEFACTORY_H_
#define TREEFACTORY_H_

#include <memory>

#include <simplicity/API.h>

namespace theisland
{
	namespace TreeFactory
	{
		std::unique_ptr<simplicity::Entity> createTree(float age);
	}
}

#endif /* TREEFACTORY_H_ */
