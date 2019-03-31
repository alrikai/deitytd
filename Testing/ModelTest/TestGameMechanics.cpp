#include "gtest/gtest.h"

#include "util/TowerModifiers.hpp"
#include "Common/tower_combiner/ModifierParser.hpp"
#include "Model/TowerDefense.hpp"
#include "util/Types.hpp"
#include "Model/Monster.hpp"

#include "Common/shared/common_information.hpp"
#include "Common/shared/Player.hpp"

#include "Testing/fractal_util.hpp"

#include <thread>
#include <chrono>

#include <memory>
#include <vector>
#include <string>

namespace TestStubs {
//mock-up of the frontend
template <typename BackendType>
struct FrontStub
{
    using ModifierType = tower_properties;
    using TowerEventQueueType = typename UserTowerEvents::EventQueueType<UserTowerEvents::tower_event<BackendType>>::QType;
	using GameInformationType = GameInformation<CommonTowerInformation, TDPlayerInformation>;

    void draw_maptiles(const int width, const int height) {}

    void register_backend_eventqueue(ViewEvents* events) {}
    
    void register_tower_eventqueue(TowerEventQueueType* tevt_queue) {
        td_event_queue = tevt_queue;
    }

	void register_shared_info(std::shared_ptr<GameInformationType> s_info) {
		shared_gamestate_info = s_info;
    }

	std::shared_ptr<GameInformationType> shared_gamestate_info;
    TowerEventQueueType* td_event_queue;
};

template <typename TDType>
void add_tower_displayinfo(TDType& td)
{
	using PixelType = uint8_t;
	std::string mesh_filename { TDHelpers::get_basepath() + "/data/meshfractal3d.vtk"};

	std::vector<std::vector<uint32_t>> polygon_mesh;
	std::vector<std::vector<float>> polygon_points;
	views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

	const std::string tower_material {"Examples/Chrome"};
	const std::string tower_name {"ViewTest"};
	td->add_tower(std::move(polygon_mesh), std::move(polygon_points), tower_material, tower_name);
}

//TODO: this should be part of the tower utils class -- this will load a yaml file, and 
//generate a tower_property_modifier object from the stats in the yaml file.
  tower_property_modifier parse_tower(const std::string& tower_cfg) {
      tower_property_modifier tower_modifier;
	  YAML::Node cfg_root = YAML::LoadFile(tower_cfg);
	  if (cfg_root.IsNull()) {
		  std::ostringstream ostr;
		  ostr << "ERROR -- config yaml file " << tower_cfg << " not found";
		  throw std::runtime_error(ostr.str());
	  }

	  YAML::Node modifier_node = cfg_root["TowerAttributes"];
	  std::cout << modifier_node.size() << " #modifiers" << std::endl;
	  for (auto mod_it = modifier_node.begin(); mod_it != modifier_node.end();
		   mod_it++) {

		YAML::Node key = mod_it->first;
		assert(key.Type() == YAML::NodeType::Scalar);
		YAML::Node value = mod_it->second;
		assert(value.Type() == YAML::NodeType::Map);

		auto modifier_name = key.as<std::string>();
		const uint32_t ID = value["ID"].as<uint32_t>();
		auto mod_attributes = value["attributes"];

		// std::cout << modifier_name << " ID: " << ID << " -- attributes: " <<
		// mod_attributes << std::endl;
	    
		switch (ID) {
		case TowerModifiers::flat_damage::ID: {
		  TowerModifiers::flat_damage::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_damage tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::enhanced_damage::ID: {
          TowerModifiers::enhanced_damage::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::enhanced_damage tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::enhanced_speed::ID: {
          TowerModifiers::enhanced_speed::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::enhanced_speed tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::flat_range::ID: {
          TowerModifiers::flat_range::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_range tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::flat_crit_chance::ID: {
          TowerModifiers::flat_crit_chance::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_crit_chance tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::flat_crit_multiplier::ID: {
          TowerModifiers::flat_crit_multiplier::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_crit_multiplier tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::flat_type_damage::ID: {
		  TowerModifiers::flat_type_damage::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_type_damage tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::enhanced_type_damage::ID: {
          TowerModifiers::enhanced_type_damage::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::enhanced_type_damage tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		case TowerModifiers::flat_damage_onhit::ID: {
          TowerModifiers::flat_damage_onhit::parameter_cfg cfg;
		  parse_modifier_parameters(mod_attributes, cfg);
          TowerModifiers::flat_damage_onhit tmod (cfg);
		  tmod.aggregate_modifier(tower_modifier);
		  break;
		}
		default: {
		  std::cerr << "Invalid ID " << ID << std::endl;
		  break;
		}
		}
	  }
	  return tower_modifier;
  }
}

namespace {
class DTDBackendTest : public testing::Test {
protected:
  using TDBackendType = TowerLogic;
  using TDType = TowerDefense<TestStubs::FrontStub, TDBackendType>;

  DTDBackendTest() {
    view = new TestStubs::FrontStub<TDBackendType>();
	//view is now owned by the td object, so no need to delete
    td = std::make_shared<TDType> (view);
  }

  virtual ~DTDBackendTest() {}

  void SetUp() override { 
    td->init_game();
	TestStubs::add_tower_displayinfo(td);
    td->start_game();
  }

  void TearDown() override {
	td->stop_game();
  }
  TestStubs::FrontStub<TDBackendType>* view;
  std::shared_ptr<TDType> td;
};


//breakpoint on failure:
//gdb --args ./bin/GameMechanicsTest --gtest_break_on_failure	
TEST_F (DTDBackendTest, BasicAttack) {
  // create the basic fundamnetal tower
  const uint32_t tid = 0;
  const int tier = 1;

  float xcoord = 0;
  float ycoord = 0;

  auto td_backend = td->get_td_backend();
  
  // TODO: load a config for the monster, create a mob from that
  //make the mob next
  float mob_pos_x = 1.0;
  float mob_pos_y = 1.0;
  auto mob = std::make_shared<Monster>(CharacterModels::ModelIDs::ogre_S, "T0", mob_pos_x,mob_pos_y);

  td_backend->make_tower(tid, tier, xcoord,ycoord);
  auto basic_tower = td_backend->get_tower(xcoord, ycoord);
  basic_tower->set_target(mob);
  const std::string basic_attack_id {"basic_T0_atk"};
  auto basic_attack = basic_tower->generate_attack(basic_attack_id, 0);
  //this is the default (fundamental) tower stats, without any boosts
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM; dmg_idx++) {
	  if (dmg_idx == 0) {
		ASSERT_EQ(basic_attack_vals.damage[dmg_idx].low, 2);
		ASSERT_EQ(basic_attack_vals.damage[dmg_idx].high, 5);
	  } else {
		ASSERT_EQ(basic_attack_vals.damage[dmg_idx].low, 0);
		ASSERT_EQ(basic_attack_vals.damage[dmg_idx].high, 0);
	  }
  }
  ASSERT_EQ(basic_attack_vals.attack_speed, 1);
  ASSERT_EQ(basic_attack_vals.attack_range, 3);
  ASSERT_EQ(basic_attack_vals.crit_chance, 0);
  ASSERT_EQ(basic_attack_vals.crit_multiplier, 100);


/*
  // make a base tower
  using build_evt_t = UserTowerEvents::build_tower_event<TDBackendType>;
  auto build_td_evt = std::make_unique<build_evt_t>(tid, tier, ycoord,xcoord);
  view->td_event_queue->push(std::move(build_td_evt));
*/

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath { TDHelpers::get_basepath() + "/data/tests/towers/basic_t0.yaml"};
  tower_property_modifier tmod = TestStubs::parse_tower(tower_fpath);

  tower_properties tprop;
  tprop.apply_property_modifier(tmod);


  td_backend->modify_tower(tprop, xcoord,ycoord);
/*
  using modify_evt_t = typename UserTowerEvents::modify_tower_event<TestStubs::FrontStub<TDBackendType>::ModifierType, TDBackendType>;
  auto mod_td_evt = std::make_unique<modify_evt_t>(tprop, ycoord,xcoord);
  view->td_event_queue->push(std::move(mod_td_evt));
*/

  // have the tower target the mob
  
  Tower* test_tower = nullptr; 
  test_tower = td_backend->get_tower(xcoord, ycoord);
  /*
  while (!test_tower) {
	  td_backend->get_tower(xcoord, ycoord);
	  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	  std::cout << "Trying again...." << std::endl;
  }
  */
  test_tower->set_target(mob);

  const std::string attack_id {"T0_atk"};
  // TODO: apply an attack from the tower to the mob
  auto attack = test_tower->generate_attack(attack_id, 0);

  //TODO: assert on the attack values
  auto attack_vals = attack->get_attack_attributes();
  
 
  auto expected_props = tprop + basic_attack_vals;
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM; dmg_idx++) {
    ASSERT_EQ(attack_vals.damage[dmg_idx].low, expected_props.damage[dmg_idx].low);
    ASSERT_EQ(attack_vals.damage[dmg_idx].high, expected_props.damage[dmg_idx].high);
  }
  ASSERT_EQ(attack_vals.attack_speed, expected_props.attack_speed);
  ASSERT_EQ(attack_vals.attack_range, expected_props.attack_range);
  ASSERT_EQ(attack_vals.crit_chance, expected_props.crit_chance);
  ASSERT_EQ(attack_vals.crit_multiplier, expected_props.crit_multiplier);


  attack->set_target(Coordinate<float>(mob_pos_x, mob_pos_y));

  // TODO: see if we can just define the set of tower types, and the set of mob types,
  // and automatically do the testing for every tower vs every mob

}



}
