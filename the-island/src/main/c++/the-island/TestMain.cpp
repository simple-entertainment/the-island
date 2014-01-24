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
#include <fstream>
#include <memory>

#include <simplicity/freeglut/windowing/FreeGLUTEngine.h>
#include <simplicity/graph/SimpleTree.h>
#include <simplicity/math/MathConstants.h>
#include <simplicity/math/MathFunctions.h>
#include <simplicity/math/Vector.h>
#include <simplicity/opengl/model/OpenGLModelFactory.h>
#include <simplicity/opengl/rendering/OpenGLRenderingEngine.h>
#include <simplicity/opengl/rendering/OpenGLShader.h>
#include <simplicity/opengl/rendering/SimpleOpenGLRenderer.h>
#include <simplicity/opengl/scene/OpenGLCamera.h>
#include <simplicity/opengl/scene/OpenGLLight.h>
#include <simplicity/scene/FlyingCameraEngine.h>
#include <simplicity/Simplicity.h>

#include "IslandFactory.h"
#include "TreeFactory.h"

using namespace simplicity;
using namespace simplicity::freeglut;
using namespace simplicity::opengl;
using namespace std;
using namespace theisland;

int main(int argc, char** argv)
{
	// Windowing
	/////////////////////////
	unique_ptr<Engine> windowingEngine(new FreeGLUTEngine("The Island"));
	Simplicity::addEngine(move(windowingEngine));

	// World Representations
	/////////////////////////
	unique_ptr<Graph> world(new SimpleTree);
	Graph* rawWorld = world.get();
	Simplicity::addWorldRepresentation(move(world));

	// Models
	/////////////////////////
	unique_ptr<ModelFactory> modelFactory(new OpenGLModelFactory);
	ModelFactory::setInstance(move(modelFactory));

	// Rendering
	/////////////////////////
	unique_ptr<OpenGLRenderingEngine> renderingEngine(new OpenGLRenderingEngine);
	unique_ptr<Renderer> renderer(new SimpleOpenGLRenderer);

	// Shaders
	ifstream vertexShaderFile("src/main/glsl/my.vs");
	ifstream fragmentShaderFile("src/main/glsl/my.fs");
	unique_ptr<OpenGLVertexShader> vertexShader(new OpenGLVertexShader(vertexShaderFile));
	unique_ptr<OpenGLFragmentShader> fragmentShader(new OpenGLFragmentShader(fragmentShaderFile));
	vertexShaderFile.close();
	fragmentShaderFile.close();
	unique_ptr<Shader> shader(new OpenGLShader(move(vertexShader), move(fragmentShader)));
	renderer->setShader(move(shader));

	// Camera
	unique_ptr<Entity> cameraEntity(new Entity);
	MathFunctions::setTranslation(cameraEntity->getTransformation(), Vector3(0.0f, 20.0f, 120.0f));
	unique_ptr<Camera> camera(new OpenGLCamera);
	camera->setPerspective(60.0f, 4.0f / 3.0f);
	cameraEntity->addUniqueComponent(move(camera));

	// Flying Camera
	unique_ptr<FlyingCameraEngine> cameraEngine(new FlyingCameraEngine(*cameraEntity));
	Simplicity::addEngine(move(cameraEngine));

	// Light
	unique_ptr<Light> light(new OpenGLLight("theOnly"));
	light->setAmbientComponent(Vector4(0.7f, 0.7f, 0.7f, 1.0f));
	light->setDiffuseComponent(Vector4(0.7f, 0.7f, 0.7f, 1.0f));
	light->setSpecularComponent(Vector4(0.7f, 0.7f, 0.7f, 1.0f));
	light->setDirection(Vector3(0.0f, -1.0f, 0.0f));
	light->setStrength(32.0f);

	renderingEngine->addLight(move(light));
	renderingEngine->addRenderer(move(renderer));
	renderingEngine->setCamera(move(cameraEntity));
	renderingEngine->setClearingColour(Vector4(0.0f, 0.5f, 0.75f, 1.0f));
	renderingEngine->setGraph(rawWorld);
	Simplicity::addEngine(move(renderingEngine));

	// The Island!
	/////////////////////////
	unsigned int radius = 64;
	vector<float> profile;
	profile.reserve(radius * 2);
	float peakHeight = 32.0f;
	for (unsigned int index = 0; index < radius * 2; index++)
	{
		// Drop below sea level outside of the radius.
		if (index > radius)
		{
			profile.push_back((float) radius - index);
			continue;
		}

		// The cone.
		//profile.push_back(peakHeight - index * (peakHeight / radius));

		// The bagel.
		//profile.push_back(peakHeight * ((sinf(index / 5.5f) + 1.0f) / 4.0f));

		// Mountains and beaches.
		profile.push_back(peakHeight * (pow(radius - index, 3) / pow(radius, 3)));
	}
	unique_ptr<Entity> island = IslandFactory::createIsland(radius, profile);
	Simplicity::addEntity(move(island));

	/*unique_ptr<Entity> test(new Entity);
	unique_ptr<Model> testModel =
			ModelFactory::getInstance().createCylinderMesh(5.0f, 10.0f, 10, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	test->addUniqueComponent(move(testModel));*/
	unique_ptr<Entity> test = TreeFactory::createTree(100.0f);
	MathFunctions::setTranslation(test->getTransformation(), Vector3(0.0f, 100.0f, 80.0f));
	Simplicity::addEntity(move(test));

	// GO!
	/////////////////////////
	Simplicity::play();
}
