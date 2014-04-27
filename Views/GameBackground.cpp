#include "GameBackground.hpp"

const std::string GameBackground::map_material {"GameMap"};
const std::string GameBackground::minimal_obj_material{"BackgroundMap"};

GameBackground::GameBackground(Ogre::SceneManager* scene)
    : scene_mgmt(scene), root_node(scene_mgmt->getRootSceneNode())
{
    make_background();
}

//set up the background
void GameBackground::make_background()
{
    /*
     *  First part: render a (plain) 2D rectangle
     *              draw a loaded texture to the rectange
     *              draw a smaller, semi-transparent rectangle ontop of the background rectangle
     */
    background_rect = new Ogre::Rectangle2D(true);
    background_rect->setCorners(-1.0, 1.0, 1.0, -1.0);
    background_rect->setMaterial(minimal_obj_material);

    background_rect->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);
    
    inf_box.setInfinite();
    background_rect->setBoundingBox(inf_box);

    auto background_node = root_node->createChildSceneNode("Background");
    background_node->attachObject(background_rect);

    map_rect = new Ogre::Rectangle2D(true);
    map_rect->setCorners(-0.7, 0.7, 0.7, -0.7);
    map_rect->setMaterial(map_material);

    auto map_node = root_node->createChildSceneNode("GameMap");
    map_node->attachObject(map_rect);
}

void GameBackground::draw_background(Ogre::Viewport* view_port)
{
    std::cout << "height: " << view_port->getActualHeight() << std::endl;
    std::cout << "width: " << view_port->getActualWidth() << std::endl;

    //
    //mock up the lines for the map grid
    //

    const int map_height = 0.7*view_port->getActualHeight();
    const int map_width = 0.7*view_port->getActualWidth();

    const float tile_pxheight = 2.f;
    const float tile_pxwidth = 2.f;
    const int num_row_tiles = map_height / tile_pxheight;
    const int num_col_tiles = map_width / tile_pxwidth;

    //need to have (#tiles - 1) #lines
    std::vector<Ogre::ManualObject*> map_grid_rowlines (num_row_tiles - 1);
    std::vector<Ogre::ManualObject*> map_grid_collines (num_col_tiles - 1);


    //TODO: we need to offset the lines to be drawn to the semi-transparent middle rectangle
    float row_offset = -num_row_tiles/10;
    float col_offset = -num_col_tiles/10;
    float depth_offset = 1;
    //the current indices to draw the lines at. Need to update for each line
    float row_idx_start = row_offset;
    float col_idx_start = col_offset;
    float depth_idx_start = depth_offset;
    float row_idx_end = row_offset;
    float col_idx_end = col_offset;
    float depth_idx_end = depth_offset;    
    
    for (int row = 0; row < num_row_tiles-1; ++row)
    {
        const std::string line_name = "mapline_row_" + std::to_string(row);
        map_grid_rowlines.at(row) = scene_mgmt->createManualObject(line_name);
        map_grid_rowlines.at(row)->begin("Ogre/Compositor/GlassPass", Ogre::RenderOperation::OT_LINE_LIST);
        {
            //make the start and end points for the row lines
            col_idx_start += tile_pxwidth;
            map_grid_rowlines.at(row)->position(row_idx_start, col_idx_start, depth_idx_start);
            map_grid_rowlines.at(row)->position(-1*row_idx_end, col_idx_start, depth_idx_end);
        }
        map_grid_rowlines.at(row)->end();
    }

    row_idx_start = row_offset;
    col_idx_start = col_offset;
    depth_idx_start = depth_offset;
    row_idx_end = row_offset;
    col_idx_end = col_offset;
    depth_idx_start = depth_offset;    
    for (int col = 0; col < num_col_tiles-1; ++col)
    {
        const std::string line_name = "mapline_col_" + std::to_string(col);
        map_grid_collines.at(col) = scene_mgmt->createManualObject(line_name);
        map_grid_collines.at(col)->begin("Ogre/Compositor/GlassPass", Ogre::RenderOperation::OT_LINE_LIST);
        {
            //make the start and end points for the column lines
            row_idx_start += tile_pxheight;
            map_grid_collines.at(col)->position(row_idx_start, col_idx_start, depth_idx_start);
            map_grid_collines.at(col)->position(row_idx_start, -1*col_idx_end, depth_idx_end);
        }
        map_grid_collines.at(col)->end();
    }

    std::vector<Ogre::SceneNode*> row_line_nodes (num_row_tiles-1);
    std::vector<Ogre::SceneNode*> col_line_nodes (num_col_tiles-1);

    for (int row = 0; row < num_row_tiles-1; ++row)
    {
        row_line_nodes.at(row) = root_node->createChildSceneNode();
        row_line_nodes.at(row)->attachObject(map_grid_rowlines.at(row));
    }

    for (int col = 0; col < num_col_tiles-1; ++col)
    {
        col_line_nodes.at(col) = root_node->createChildSceneNode();
        col_line_nodes.at(col)->attachObject(map_grid_collines.at(col));
    }

    //end of the background setup


}
