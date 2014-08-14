#include "GameBackground.hpp"

const std::string GameBackground::map_material {"Examples/GrassFloor"};
const std::string GameBackground::skybox_material {"TD/StarSky"};     //{"Examples/CloudySky"};  //SpaceSkyBox"};
const std::string GameBackground::map_name {"GameBGMap"};

GameBackground::GameBackground(Ogre::SceneManager* scene, Ogre::Viewport* vport)
    : scene_mgmt(scene), root_node(scene_mgmt->getRootSceneNode()), view_port(vport)
{
    make_background();
}

//set up the background
void GameBackground::make_background()
{
    auto cam = view_port->getCamera();
    std::vector<Ogre::Vector2> map_extents {{0.1f, 0.1f}, {0.9f, 0.9f}};
    std::vector<Ogre::Vector3> world_extents (map_extents.size());

    Ogre::AxisAlignedBox ray_aab (Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE);
    for (size_t pt_idx = 0; pt_idx < map_extents.size(); ++pt_idx)
    {
        Ogre::Ray ray = cam->getCameraToViewportRay(map_extents.at(pt_idx)[0], map_extents.at(pt_idx)[1]);
        auto world_coord = ray.intersects(ray_aab);

        if(world_coord.first)
            std::cout << "@ " << pt_idx << " --> " << ray.getPoint(world_coord.second) << std::endl;
        else
            std::cout << "NOTE: DIDNT INTERSECT " << std::endl;
        world_extents[pt_idx] = ray.getPoint(world_coord.second);
    }

    float map_height = world_extents[1][1] - world_extents[0][1]; 
    float map_width = world_extents[1][0] - world_extents[0][0]; 

    std::cout << "map (@world coords) [" << map_height << ", " << map_width << "]" << std::endl;

    //the above doesn't really work too well, so just discard it and use the extents below...

    map_height = 150;
    map_width = 200;

    std::string map_planename = map_name+"_plane";
    Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0);
    Ogre::MeshManager::getSingleton().createPlane(map_planename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            plane, map_width, map_height);
    auto map_plane = scene_mgmt->createEntity(map_name, map_planename);

    map_plane->setMaterialName(map_material);
    map_plane->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
    map_node = scene_mgmt->getRootSceneNode()->createChildSceneNode(map_name);
    map_node->attachObject(map_plane);

    map_aab = map_plane->getBoundingBox();
}

void GameBackground::draw_background()
{
    //create the background 
    scene_mgmt->setSkyDome(true, skybox_material, 1.0f, 1.0f, 5000.0f);
    view_port->setSkiesEnabled(true);

    std::cout << "height: " << view_port->getActualHeight() << std::endl;
    std::cout << "width: " << view_port->getActualWidth() << std::endl;
}

//
//mock up the lines for the map grid. Assume uniform spacing
//
void GameBackground::draw_tiles(const int num_cols, const int num_rows)
{
    //NOTE: assume this has the world gamemap plane
    auto aab_size = map_aab.getMaximum() - map_aab.getMinimum();
    const float ytile_space = aab_size.y * (1.0f / num_rows);
    const float xtile_space = aab_size.x * (1.0f / num_cols);

    //need to have (#tiles + 1) #lines (for the borders)
    std::vector<Ogre::ManualObject*> map_grid_rowlines (num_rows + 1);
    std::vector<Ogre::ManualObject*> map_grid_collines (num_cols + 1);

    //TODO: we need to offset the lines to be drawn to the semi-transparent middle rectangle
    float row_offset = -map_aab.getHalfSize().y;
    float col_offset = -map_aab.getHalfSize().x; 
    float depth_offset = 1;
    //the current indices to draw the lines at. Need to update for each line
    float row_idx = row_offset;
    float col_idx = col_offset;
   
    const std::string mapgrid_material {"BaseWhiteNoLighting"};
    for (size_t row = 0; row < map_grid_rowlines.size(); ++row)
    {
        const std::string line_name = "mapline_row_" + std::to_string(row);
        map_grid_rowlines.at(row) = scene_mgmt->createManualObject(line_name);
        map_grid_rowlines.at(row)->begin(mapgrid_material, Ogre::RenderOperation::OT_LINE_LIST);
        {
            //make the start and end points for the row lines
            map_grid_rowlines.at(row)->position(col_idx, row_idx, depth_offset);
            map_grid_rowlines.at(row)->position(-1*col_idx, row_idx, depth_offset);
            row_idx += ytile_space;
        }
        map_grid_rowlines.at(row)->end();
        map_grid_rowlines.at(row)->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    }

    row_idx = row_offset;
    col_idx = col_offset;
    for (size_t col = 0; col < map_grid_collines.size(); ++col)
    {
        const std::string line_name = "mapline_col_" + std::to_string(col);
        map_grid_collines.at(col) = scene_mgmt->createManualObject(line_name);
        map_grid_collines.at(col)->begin(mapgrid_material, Ogre::RenderOperation::OT_LINE_LIST);
        {
            //make the start and end points for the column lines
            map_grid_collines.at(col)->position(col_idx, row_idx, depth_offset);
            map_grid_collines.at(col)->position(col_idx, -1*row_idx, depth_offset);
            col_idx += xtile_space;
        }
        map_grid_collines.at(col)->end();
        map_grid_collines.at(col)->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    }

    std::vector<Ogre::SceneNode*> row_line_nodes (num_rows+1);
    std::vector<Ogre::SceneNode*> col_line_nodes (num_cols+1);

    for (size_t row = 0; row < map_grid_rowlines.size(); ++row)
    {
        row_line_nodes.at(row) = map_node->createChildSceneNode();
        row_line_nodes.at(row)->attachObject(map_grid_rowlines.at(row));
    }

    for (size_t col = 0; col < map_grid_collines.size(); ++col)
    {
        col_line_nodes.at(col) = map_node->createChildSceneNode();
        col_line_nodes.at(col)->attachObject(map_grid_collines.at(col));
    }

    //end of the background setup


}




namespace view_detail
{
// Go through all sections & settings in the file
void load_resources(const std::string& resource_cfg_filename)
{
    Ogre::ConfigFile config;
    config.load(resource_cfg_filename);
    Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
    std::string secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}

//returns whether the click intersected the GameMap, and if so, what the distance was  
std::tuple<bool, float, Ogre::MovableObject*> check_point(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y)
{
    std::cout << "Viewport Actual Dim: [" << view_port->getActualLeft() << ", " << view_port->getActualTop() 
              << " -- " << view_port->getActualHeight() << ", " << view_port->getActualWidth() << "]" << std::endl;
    std::cout << "Viewport Relative  Dim: [" << view_port->getLeft() << ", " << view_port->getTop() 
              << " -- " << view_port->getHeight() << ", " << view_port->getWidth() << "]" << std::endl;
      
    auto cam = view_port->getCamera();
    Ogre::Ray ray = cam->getCameraToViewportRay(x,y);
    std::cout << "Ray [" << x << ", " << y << "] origin: " << ray.getOrigin() << " @direction " << ray.getDirection() << std::endl;
    std::cout << "Ray @ near plane: " << ray.getPoint(cam->getNearClipDistance()) << "Ray @ far plane: " << ray.getPoint(cam->getFarClipDistance()) << std::endl;
    std::cout << "Ray (non-normalized) [" << x * view_port->getActualWidth() << ", " << y * view_port->getActualHeight() << "]" << std::endl;

    std::tuple<bool, float, Ogre::MovableObject*> click_intersection {false, 0, nullptr};
    auto r_query = scene_mgmt->createRayQuery(ray);
    r_query->setSortByDistance(true);
    auto& q_hits = r_query->execute();
    if(!q_hits.empty())
    {
        for (auto q_it = q_hits.begin(); q_it != q_hits.end(); ++q_it)
        {
            if(q_it->movable)
            {
                Ogre::MovableObject* obj = q_it->movable;
                std::cout << "moveable -- " << obj->getName() << ": " << obj->getMovableType() << " bounding: " << obj->getWorldBoundingBox() << std::endl;
                if(obj->getName() == GameBackground::map_name)
                {
                    std::get<0>(click_intersection) = true;
                    std::get<1>(click_intersection) = q_it->distance;
                    std::get<2>(click_intersection) = obj;

                    std::cout << "Ray Length: " << q_it->distance << std::endl;
                    return click_intersection;
                }
            }
            else if(q_it->worldFragment)
                std::cout << "world fragment" << std::endl;
            else
                std::cout << "???" << std::endl;
        }
    }
    else
        std::cout << "[" << x << ", " << y << "] Did not intersect with anything" << std::endl;
 
    return click_intersection;
}

}
