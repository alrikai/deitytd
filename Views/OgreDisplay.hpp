#ifndef TD_OGRE_DISPLAY_HPP__
#define TD_OGRE_DISPLAY_HPP__

#include <iostream>
#include <memory>
#include <chrono>
#include <string>

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include "GameBackground.hpp"
#include "ControllerUtil.hpp"
#include "Controller.hpp"

class OgreDisplay
{
public:
    OgreDisplay()
        : root (new Ogre::Root(plugins_cfg_filename)), cam_rotate(0.13f), cam_move(50.0f),
        background(nullptr) 
    {
        ogre_setup();
        setup_camera(); 

		//not really sure if these should get their own method, or be rolled into an existing one. So for now, just have them here...
		view_port = render_window->addViewport(camera);
        view_port->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        light = scene_mgmt->createLight("MainLight");
        light->setPosition(20.0f, 80.0f, 50.0f); 

        background.reset(new GameBackground(scene_mgmt));

        input_events = std::unique_ptr<ControllerUtil::ControllerBufferType>(new ControllerUtil::ControllerBufferType());
    }

	void start_display();
    
    /*
     *  note: need to figure out how to go about adding manual objects to the scene in a meaningful manner.
     *  we have the possibility of having this be quite difficult, as we also have to figure out what to do
     *  with the whole scene/scene manager. 
     *  IN SFML, we just had a double-buffer scheme, where we would draw whatever we wanted to the 2nd buffer
     *  then swap the buffers. Here, we have scene nodes and a tree-like hierarchy of scene nodes and their
     *  contents. 
     *  --> maybe we have to break the problem down a bit more; i.e. we could have a draw_tower method, which
     *  would generate the manual object and have all the tower manual objects attached to the same tower_root
     *  scene node
     */ 
    void draw_tower(const std::vector<std::vector<uint32_t>>& polygon_mesh, const std::vector<std::vector<float>>& polygon_points, 
                    std::string tower_material, std::string&& tower_name = "");

    void register_input_controller(Controller* controller);

    Ogre::Root* get_root() const
    { return root.get(); }
    Ogre::RenderWindow* get_render_window() const 
    { return render_window; }
	Ogre::SceneManager* get_scene_mgmt() const
    { return scene_mgmt; }

private:        
    bool ogre_setup();
    void setup_camera(); 

    void setup_background();
    void handle_user_input();

    const static std::string resource_cfg_filename; 
    const static std::string plugins_cfg_filename;

    std::unique_ptr<Ogre::Root> root;

    Ogre::RenderWindow* render_window;
	Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;

    //the coefficients for camera movement (there will be more as we add more functionality)
    const float cam_rotate;
    const float cam_move;

	Ogre::Camera* camera;
    Ogre::Viewport* view_port;
    Ogre::Light* light;

    std::unique_ptr<GameBackground> background;
    std::unique_ptr<ControllerUtil::ControllerBufferType> input_events;

};










/*
    Ogre::ManualObject* minimal_obj = scene_mgmt->createManualObject("minimal_square");
    minimal_obj->setDynamic(false);

    const float obj_size = 0.7;
    minimal_obj->begin(minimal_obj_material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        float cp = 10.0f * obj_size;
        float cm = -10.0f * obj_size;

        
        std::vector<std::tuple<float,float,float>> vertices;
        vertices.emplace_back(cm,cp,cm);
        vertices.emplace_back(cp,cp,cm);
        vertices.emplace_back(cp,cm,cm);
        vertices.emplace_back(cm,cm,cm);
        vertices.emplace_back(cm,cp,cp);
        vertices.emplace_back(cp,cp,cp);
        vertices.emplace_back(cp,cm,cp);
        vertices.emplace_back(cm,cm,cp);

        for (int i = 0; i < vertices.size(); ++i)
        {
            auto vertex = vertices.at(i);
            minimal_obj->position(std::get<0>(vertex), std::get<1>(vertex), std::get<2>(vertex));
            minimal_obj->colour(Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f));
        }
        
        // face behind / front
        minimal_obj->triangle(0,1,2);
        minimal_obj->triangle(2,3,0);
        minimal_obj->triangle(4,6,5);
        minimal_obj->triangle(6,4,7);

        // face top / down
        minimal_obj->triangle(0,4,5);
        minimal_obj->triangle(5,1,0);
        minimal_obj->triangle(2,6,7);
        minimal_obj->triangle(7,3,2);

        // face left / right
        minimal_obj->triangle(0,7,4);
        minimal_obj->triangle(7,0,3);
        minimal_obj->triangle(1,5,6);
        minimal_obj->triangle(6,2,1); 
        
    }
    minimal_obj->end();

    const std::string mesh_name {"MinimalMesh"};
    minimal_obj->convertToMesh(mesh_name);

    Ogre::Entity* minimal_entity = scene_mgmt->createEntity(mesh_name);
    auto child_node = root_node->createChildSceneNode();
    child_node->attachObject(minimal_entity);

*/
/*
void render_mesh(Ogre::SceneManager* scene_mgmt, std::vector<std::vector<uint32_t>>&& polygon_mesh)
{
    Ogre::ManualObject* minimal_obj = scene_mgmt->createManualObject("polygon_mesh");
}

*/



/*
template <typename PixelType>
void make_mesh(Ogre::ManualObject* obj, const std::string& mesh_name, std::vector<cv::Mat_<PixelType>>&& object_volume)
{
    const float obj_size = 0.7;
    obj->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        for (int depth_plane = 0; depth_plane < object_volume.size(); ++depth_plane)
        {
            auto volume_slice = object_volume.at(depth_plane);
            for (int row_idx = 0; row_idx < volume_slice.rows; ++row_idx)    
            {
                for (int col_idx = 0; col_idx < volume_slice.cols; ++col_idx)
                {
                    //add vertices that are within the object; in actuality, we just need the outer shell
                    if(volume_slice(row_idx,col_idx) > cv::Vec3b(0, 0, 0))
                    {
                        obj->position(col_idx, row_idx, depth_plane);
                        obj->colour(Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f));
                    }   
                }
            }
        }
*/

/*
        // face behind / front
        minimal_obj->triangle(0,1,2);
        minimal_obj->triangle(2,3,0);
        minimal_obj->triangle(4,6,5);
        minimal_obj->triangle(6,4,7);

        // face top / down
        minimal_obj->triangle(0,4,5);
        minimal_obj->triangle(5,1,0);
        minimal_obj->triangle(2,6,7);
        minimal_obj->triangle(7,3,2);

        // face left / right
        minimal_obj->triangle(0,7,4);>
        minimal_obj->triangle(7,0,3);
        minimal_obj->triangle(1,5,6);
        minimal_obj->triangle(6,2,1); 
*/
/*
    }
    obj->end();
    obj->convertToMesh(mesh_name);
}
*/
#endif
