#ifndef TD_GAME_BACKGROUND_HPP
#define TD_GAME_BACKGROUND_HPP

#include <OGRE/Ogre.h>

#include <string>


//holds the data and functionality for drawing the background
class GameBackground
{
public:
    GameBackground(Ogre::SceneManager*, Ogre::Viewport*);
    
    void draw_background();
    void draw_tiles(const int num_cols, const int num_rows);

    const static std::string map_name;
private:
    void make_background();

    const static std::string map_material;
    const static std::string skybox_material;

	Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;
    Ogre::Viewport* view_port;
    Ogre::SceneNode* map_node;

    Ogre::AxisAlignedBox map_aab;
};

#endif

