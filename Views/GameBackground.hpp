#ifndef TD_GAME_BACKGROUND_HPP
#define TD_GAME_BACKGROUND_HPP

#include "FflameGenerator.hpp"

#include <OGRE/Ogre.h>
#include <opencv2/opencv.hpp>

#include <string>
#include <memory>

//holds the data and functionality for drawing the background
class GameBackground
{
    using texture_t = uint8_t;
    using data_t = double;
    using pixel_t = cv::Vec<data_t, 3>;

public:
    GameBackground(Ogre::SceneManager*, Ogre::Viewport*);
    ~GameBackground();

    void draw_background();
    void draw_tiles(const int num_cols, const int num_rows);

    Ogre::AxisAlignedBox get_map_aab() const
    { return map_aab; }

    const static std::string map_name;
private:
    void render_background_texture(std::unique_ptr<texture_t []> texture_buffer);
    void make_background();

    //maximum size of background frame queue buffer
    static constexpr uint8_t MAX_BGQUEUE = 50;
    //procedurally generates the background textures
    std::unique_ptr<fflame_generator<data_t, pixel_t>> bg_generator;
    std::unique_ptr<EventQueue<texture_t []>> bg_framequeue;

    const static std::string map_material;
    const static std::string skybox_material;

	Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;
    Ogre::Viewport* view_port;
    Ogre::SceneNode* map_node;

    //we want to have a dynamic texture for the background that we update as we compute new fflames
    int bg_width;
    int bg_height;
    Ogre::Rectangle2D* bg_rect;
    Ogre::TexturePtr bg_texture;
    Ogre::MaterialPtr bg_material; 
    Ogre::SceneNode* bg_node;
    Ogre::AxisAlignedBox infinite_bgaab;     
    Ogre::AxisAlignedBox map_aab;
};

namespace view_detail
{

void load_resources(const std::string& resource_cfg_filename);
//should refactor these -- have one just look for movables, have another that looks for the gamemap and the ray distance
std::tuple<bool, float, Ogre::MovableObject*> check_point(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y);
Ogre::MovableObject* user_select(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y);

}

#endif

