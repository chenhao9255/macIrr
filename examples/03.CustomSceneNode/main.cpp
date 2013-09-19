
#include <irrlicht.h>
#include "driverChoice.h"

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif



class CSampleSceneNode : public scene::ISceneNode
{

	/*
	First, we declare some member variables:
	The bounding box, 4 vertices, and the material of the tetraeder.
	*/
	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;


public:

	CSampleSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
		: scene::ISceneNode(parent, mgr, id)
	{
		Material.Wireframe = false;
		Material.Lighting = true;

		Vertices[0] = video::S3DVertex(0,0,10, 1,1,0,
				video::SColor(255,0,255,255), 0, 1);
		Vertices[1] = video::S3DVertex(10,0,-10, 1,0,0,
				video::SColor(255,255,0,255), 1, 1);
		Vertices[2] = video::S3DVertex(0,20,0, 0,1,1,
				video::SColor(255,255,255,0), 1, 0);
		Vertices[3] = video::S3DVertex(-10,0,-10, 0,0,1,
				video::SColor(255,0,255,0), 0, 0);

	/*
	The Irrlicht Engine needs to know the bounding box of a scene node.
	It will use it for automatic culling and other things. Hence, we
	need to create a bounding box from the 4 vertices we use.
	If you do not want the engine to use the box for automatic culling,
	and/or don't want to create the box, you could also call
	irr::scene::ISceneNode::setAutomaticCulling() with irr::scene::EAC_OFF.
	*/
		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);
	}

	/*
	Before it is drawn, the irr::scene::ISceneNode::OnRegisterSceneNode()
	method of every scene node in the scene is called by the scene manager.
	If the scene node wishes to draw itself, it may register itself in the
	scene manager to be drawn. This is necessary to tell the scene manager
	when it should call irr::scene::ISceneNode::render(). For
	example, normal scene nodes render their content one after another,
	while stencil buffer shadows would like to be drawn after all other
	scene nodes. And camera or light scene nodes need to be rendered before
	all other scene nodes (if at all). So here we simply register the
	scene node to render normally. If we would like to let it be rendered
	like cameras or light, we would have to call
	SceneManager->registerNodeForRendering(this, SNRT_LIGHT_AND_CAMERA);
	After this, we call the actual
	irr::scene::ISceneNode::OnRegisterSceneNode() method of the base class,
	which simply lets also all the child scene nodes of this node register
	themselves.
	*/
	virtual void OnRegisterSceneNode()
	{
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnRegisterSceneNode();
	}

	/*
	In the render() method most of the interesting stuff happens: The
	Scene node renders itself. We override this method and draw the
	tetraeder.
	*/
	virtual void render()
	{
		u16 indices[] = {	0,2,3, 2,1,3, 1,0,3, 2,0,1	};
		video::IVideoDriver* driver = SceneManager->getVideoDriver();

		driver->setMaterial(Material);
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		driver->drawVertexPrimitiveList(&Vertices[0], 4, &indices[0], 4, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
	}

	/*
	And finally we create three small additional methods.
	irr::scene::ISceneNode::getBoundingBox() returns the bounding box of
	this scene node, irr::scene::ISceneNode::getMaterialCount() returns the
	amount of materials in this scene node (our tetraeder only has one
	material), and i/Users/chenhao/Work/irrlicht/include/SExposedVideoData.hrr::scene::ISceneNode::getMaterial() returns the
	material at an index. Because we have only one material here, we can
	return the only one material, assuming that no one ever calls
	getMaterial() with an index greater than 0.
	*/
	virtual const core::aabbox3d<f32>& getBoundingBox() const
	{
		return Box;
	}

	virtual u32 getMaterialCount() const
	{
		return 1;
	}

	virtual video::SMaterial& getMaterial(u32 i)
	{
		return Material;
	}	
};

/*
That's it. The Scene node is done. Now we simply have to start
the engine, create the scene node and a camera, and look at the result.
*/
int main()
{
	// ask user for driver
	video::E_DRIVER_TYPE driverType=driverChoiceConsole();
	if (driverType==video::EDT_COUNT)
		return 1;

	// create device

	IrrlichtDevice *device = createDevice(driverType,
			core::dimension2d<u32>(640, 480), 16, false);
		
	if (device == 0)
		return 1; // could not create selected driver.

	// create engine and camera

	device->setWindowCaption(L"Custom Scene Node - Irrlicht Engine Demo");

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

  scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS();
	camera->setPosition(core::vector3df(-200,200,-200));
	/*
	Create our scene node. I don't check the result of calling new, as it
	should throw an exception rather than returning 0 on failure. Because
	the new node will create itself with a reference count of 1, and then
	will have another reference added by its parent scene node when it is
	added to the scene, I need to drop my reference to it. Best practice is
	to drop it only *after* I have finished using it, regardless of what
	the reference count of the object is after creation.
	*/
	CSampleSceneNode *myNode =
		new CSampleSceneNode(smgr->getRootSceneNode(), smgr, 666);
  myNode->setScale(core::vector3df(5,5,5));
  myNode->setPosition(core::vector3df(-70,130,45));

	/*
	To animate something in this boring scene consisting only of one
	tetraeder, and to show that you now can use your scene node like any
	other scene node in the engine, we add an animator to the scene node,
	which rotates the node a little bit.
	irr::scene::ISceneManager::createRotationAnimator() could return 0, so
	should be checked.
	*/
	scene::ISceneNodeAnimator* anim =
		smgr->createRotationAnimator(core::vector3df(0.8f, 0, 0.8f));

	if(anim)
	{
		myNode->addAnimator(anim);
		
		/*
		I'm done referring to anim, so must
		irr::IReferenceCounted::drop() this reference now because it
		was produced by a createFoo() function. As I shouldn't refer to
		it again, ensure that I can't by setting to 0.
		*/
		anim->drop();
		anim = 0;
	}
  
  
  driver->setFog(video::SColor(0,138,125,81), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, false);
  
  video::ITexture * myTexture = driver->getTexture("../../media/mypic.png");
  myNode->setMaterialTexture(0, myTexture);
  myNode->setMaterialType(video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA);
  myNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
  
	/*
	I'm done with my CSampleSceneNode object, and so must drop my reference.
	This won't delete the object, yet, because it is still attached to the
	scene graph, which prevents the deletion until the graph is deleted or the
	custom scene node is removed from it.
	*/
//	myNode->drop();
//	myNode = 0; // As I shouldn't refer to it again, ensure that I can't

  
  scene::ILightSceneNode * mylight = smgr->addLightSceneNode(myNode, core::vector3df(50,50,50),video::SColorf(0.5f, 1.0f, 0.5f,0.0f), 800.0f);
  scene::ISceneNodeAnimator * myAni = smgr->createFlyCircleAnimator(core::vector3df(30,50,0),190.0f, -0.003f);

  mylight->addAnimator(myAni);
  myAni->drop();
  
  
  scene::IAnimatedMesh * roomMesh = smgr->getMesh("../../media/room.3ds");
  scene::ISceneNode* room = 0;
	scene::ISceneNode* earth = 0;
  
  if (roomMesh) {
    smgr->getMeshManipulator()->makePlanarTextureMapping(roomMesh->getMesh(0), 0.003f);
    
    video::ITexture* normalMap = driver->getTexture("../../media/rockwall_height.bmp");
    
		if (normalMap)
			driver->makeNormalMapTexture(normalMap, 9.0f);
    
    scene::IMesh* tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(roomMesh->getMesh(0));
		room = smgr->addMeshSceneNode(tangentMesh);
    
    room->setMaterialTexture(0, driver->getTexture("../../media/rockwall.jpg"));
		room->setMaterialTexture(1, normalMap);
    
    room->getMaterial(0).SpecularColor.set(0,0,0,0);
		room->getMaterial(0).Shininess = 0.f;
    
		room->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		room->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
		// adjust height for parallax effect
		room->getMaterial(0).MaterialTypeParam = 1.f / 64.f;
    
		// drop mesh because we created it with a create.. call.
		tangentMesh->drop();
  }
  
  
	/*
	Now draw everything and finish.
	*/
	u32 frames=0;
	while(device->run())
	{
		driver->beginScene(true, true, video::SColor(0,100,100,100));

		smgr->drawAll();

		driver->endScene();
		if (++frames==100)
		{
			core::stringw str = L"Irrlicht Engine [";
			str += driver->getName();
			str += L"] FPS: ";
			str += (s32)driver->getFPS();

			device->setWindowCaption(str.c_str());
			frames=0;
		}
	}

	device->drop();
	
	return 0;
}

/*
That's it. Compile and play around with the program.
**/
