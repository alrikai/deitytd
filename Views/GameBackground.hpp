#ifndef TD_GAME_BACKGROUND_HPP__
#define TD_GAME_BACKGROUND_HPP__

#include <OGRE/Ogre.h>

#include <string>


//holds the data and functionality for drawing the background
class GameBackground
{
public:
    GameBackground(Ogre::SceneManager*);

    void draw_background(Ogre::Viewport*);

private:
    
    void make_background();

    const static std::string map_material;
    const static std::string minimal_obj_material;

	Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;

    Ogre::AxisAlignedBox inf_box;
    Ogre::Rectangle2D* background_rect;

    Ogre::Rectangle2D* map_rect;

    Ogre::SceneNode* map_node;
    Ogre::SceneNode* background_node;
};

#endif

