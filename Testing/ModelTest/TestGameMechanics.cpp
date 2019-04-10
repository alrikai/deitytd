#include "gtest/gtest.h"

#include "Model/AttackLogic.hpp"
#include "Model/Monster.hpp"
#include "Model/TowerDefense.hpp"
#include "Model/Towers/Combinations/ModifierParser.hpp"
#include "util/TowerModifiers.hpp"
#include "util/Types.hpp"

#include "Common/shared/Player.hpp"
#include "Common/shared/common_information.hpp"

#include "Testing/fractal_util.hpp"

#include <chrono>
#include <thread>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace TestStubs {
// mock-up of the frontend
template <typename BackendType> struct FrontStub {
  using ModifierType = tower_properties;
  using TowerEventQueueType = typename UserTowerEvents::EventQueueType<
      UserTowerEvents::tower_event<BackendType>>::QType;
  using GameInformationType =
      GameInformation<CommonTowerInformation, TDPlayerInformation>;

  void draw_maptiles(const int width, const int height) {}

  void register_backend_eventqueue(ViewEvents *events) {}

  void register_tower_eventqueue(TowerEventQueueType *tevt_queue) {
    td_event_queue = tevt_queue;
  }

  void register_shared_info(std::shared_ptr<GameInformationType> s_info) {
    shared_gamestate_info = s_info;
  }

  std::shared_ptr<GameInformationType> shared_gamestate_info;
  TowerEventQueueType *td_event_queue;
};

template <typename TDType> void add_tower_displayinfo(TDType &td) {
  using PixelType = uint8_t;
  std::string mesh_filename{TDHelpers::get_basepath() +
                            "/data/meshfractal3d.vtk"};

  std::vector<std::vector<uint32_t>> polygon_mesh;
  std::vector<std::vector<float>> polygon_points;
  views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

  const std::string tower_material{"Examples/Chrome"};
  const std::string tower_name{"ViewTest"};
  td->add_tower(std::move(polygon_mesh), std::move(polygon_points),
                tower_material, tower_name);
}

// TODO: this should be part of the tower utils class -- this will load a yaml
// file, and generate a tower_property_modifier object from the stats in the yaml
// file.

} // namespace TestStubs

// breakpoint on failure:
// gdb --args ./bin/GameMechanicsTest --gtest_break_on_failure

namespace {
class DTDBackendTest : public testing::Test {
protected:
  using TDBackendType = TowerLogic;
  using TDType = TowerDefense<TestStubs::FrontStub, TDBackendType>;

  DTDBackendTest() : attack_id("T0_atk") {
    int test_seed = 42;
    td = std::make_shared<TDType>(test_seed);
    view = td->get_td_frontend();

    tower_xcoord = 0;
    tower_ycoord = 0;
    mob_xcoord = 1;
    mob_ycoord = 1;
    tid = 0;

    for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM;
         dmg_idx++) {
      if (dmg_idx == 0) {
        expected_base_props.modifier.damage_value[dmg_idx].low = 2;
        expected_base_props.modifier.damage_value[dmg_idx].high = 5;
      } else {
        expected_base_props.modifier.damage_value[dmg_idx].low = 0;
        expected_base_props.modifier.damage_value[dmg_idx].high = 0;
      }
    }
    expected_base_props.modifier.attack_speed_value = 1;
    expected_base_props.modifier.attack_range_value = 3;
    expected_base_props.modifier.crit_chance_value = 0;
    expected_base_props.modifier.crit_multiplier_value = 50;
  }

  virtual ~DTDBackendTest() {}

  void SetUp() override {
    td->init_game();
    TestStubs::add_tower_displayinfo(td);
    // td->start_game();
  }

  void TearDown() override {
    // td->stop_game();
  }

  std::shared_ptr<Monster> make_mob_from_cfg(const std::string &mob_cfg) {
    std::vector<std::shared_ptr<Monster>> mobs =
        parse_monster(mob_cfg, "TEST_", 1);
    auto mob = mobs[0];
    mob->set_position(Coordinate<float>(mob_xcoord, mob_ycoord));
    return mob;
  }

  std::shared_ptr<Tower> make_tower_from_cfg(const std::string &tower_cfg) {}

  TestStubs::FrontStub<TDBackendType> *view;
  std::shared_ptr<TDType> td;

  // common game mechanics state values
  float tower_xcoord;
  float tower_ycoord;
  float mob_xcoord;
  float mob_ycoord;
  uint32_t tid;
  tower_properties expected_base_props;
  ModifierMapper tower_modifier_loader;

  const std::string attack_id;
};

void assert_tower_properties_almost_equals(
    const tower_properties &attack_vals,
    const tower_properties &expected_vals) {
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM;
       dmg_idx++) {
    EXPECT_FLOAT_EQ(attack_vals.modifier.damage_value[dmg_idx].low,
                    expected_vals.modifier.damage_value[dmg_idx].low);
    EXPECT_FLOAT_EQ(attack_vals.modifier.damage_value[dmg_idx].high,
                    expected_vals.modifier.damage_value[dmg_idx].high);
  }
  EXPECT_FLOAT_EQ(attack_vals.modifier.attack_speed_value,
                  expected_vals.modifier.attack_speed_value);
  EXPECT_FLOAT_EQ(attack_vals.modifier.attack_range_value,
                  expected_vals.modifier.attack_range_value);
  EXPECT_FLOAT_EQ(attack_vals.modifier.crit_chance_value,
                  expected_vals.modifier.crit_chance_value);
  EXPECT_FLOAT_EQ(attack_vals.modifier.crit_multiplier_value,
                  expected_vals.modifier.crit_multiplier_value);
}

TEST_F(DTDBackendTest, BasicTower) {
  const int tier = 1;
  auto td_backend = td->get_td_backend();
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  auto basic_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  basic_tower->set_target(mob);
  const std::string basic_attack_id{"fundamental_atk"};
  auto basic_attack = basic_tower->generate_attack(basic_attack_id, 0);
  // this is the default (fundamental) tower stats, without any boosts
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);
}

TEST_F(DTDBackendTest, Basic_FlatDMG) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // TODO: load a config for the monster, create a mob from that
  // make the mob next
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);
  const std::string basic_attack_id{"flat_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  // this is the default (fundamental) tower stats, without any boosts
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_flatED.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);

  tower_properties expected_handprops = expected_base_props;
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM;
       dmg_idx++) {
    expected_handprops.modifier.damage_value[dmg_idx].low += 10;
    expected_handprops.modifier.damage_value[dmg_idx].high += 20;
  }
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  // attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 889);
}

TEST_F(DTDBackendTest, Basic_PctDMG) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"pct_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_pctED.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  // auto tdamage = tower->compute_attack_damage();

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM;
       dmg_idx++) {
    expected_handprops.modifier.enhanced_damage_value[dmg_idx] = 0.3;
  }
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 996);
}

TEST_F(DTDBackendTest, Basic_flat_crit_multiplier) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // TODO: load a config for the monster, create a mob from that
  // make the mob next
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);
  const std::string basic_attack_id{"flatcrit_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  // this is the default (fundamental) tower stats, without any boosts
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_critamt.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);
  test_tower->add_modifier(std::move(tmod));

  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  expected_handprops.modifier.crit_multiplier_value += 25;
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 996);
}

TEST_F(DTDBackendTest, Basic_critpct) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"critpct_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_critpct.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  // auto tdamage = tower->compute_attack_damage();

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  expected_handprops.modifier.crit_chance_value += 1.0;
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 754);
}

TEST_F(DTDBackendTest, Basic_range) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"range_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_range.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);
  test_tower->add_modifier(std::move(tmod));

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  expected_handprops.modifier.attack_range_value += 5;
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 998);
}

TEST_F(DTDBackendTest, Basic_spd) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"spd_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_spd.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  // auto tdamage = tower->compute_attack_damage();

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  expected_handprops.modifier.attack_speed_value += 0.15;

  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 996);
}

TEST_F(DTDBackendTest, Basic_type_FED) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"typeflatED_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_typeflatED.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  // auto tdamage = tower->compute_attack_damage();

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;

  auto water_index = static_cast<int>(Elements::WATER);
  expected_handprops.modifier.damage_value[water_index].low += 20;
  expected_handprops.modifier.damage_value[water_index].high += 30;
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 964);
}

TEST_F(DTDBackendTest, Basic_type_PCTED) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();

  // make the monster
  const std::string mob_fpath{TDHelpers::get_basepath() +
                              "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

  // make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord, tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

  // make the (unmodified) basic tower attack.
  // this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id{"typepctED_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath{TDHelpers::get_basepath() +
                                "/data/tests/towers/basic_typepctED.yaml"};
  tower_property_modifier tmod =
      tower_modifier_loader.parse_tower_config(tower_fpath);

  tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  // auto tdamage = tower->compute_attack_damage();

  test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;

  auto chaos_index = static_cast<int>(Elements::CHAOS);
  expected_handprops.modifier.enhanced_damage_value[chaos_index] += 0.5;
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist;
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  // check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 993);
}

#if 0
TEST_F (DTDBackendTest, Basic_) {
  // create the basic fundamnetal tower
  const int tier = 1;
  auto td_backend = td->get_td_backend();
  
	//make the monster
  const std::string mob_fpath { TDHelpers::get_basepath() + "/data/tests/monsters/basic_t0.yaml"};
  auto mob = make_mob_from_cfg(mob_fpath);

	//make the basic tower
  td_backend->make_tower(tid, tier, tower_xcoord,tower_ycoord);
  auto test_tower = td_backend->get_tower(tower_xcoord, tower_ycoord);
  test_tower->set_target(mob);

	//make the (unmodified) basic tower attack. 
  //this is the default (fundamental) tower stats, without any boosts
  const std::string basic_attack_id {"pct_atk"};
  auto basic_attack = test_tower->generate_attack(basic_attack_id, 0);
  auto basic_attack_vals = basic_attack->get_attack_attributes();
  assert_tower_properties_almost_equals(basic_attack_vals, expected_base_props);

  // load the tower modifiers, apply them to the base tower
  const std::string tower_fpath { TDHelpers::get_basepath() + "/data/tests/towers/basic_pctED.yaml"};
  tower_property_modifier tmod = tower_modifier_loader.parse_tower_config(tower_fpath);

	tower_properties expected_props = basic_attack_vals;
  expected_props.apply_property_modifier(tmod);

  test_tower->add_modifier(std::move(tmod));
  //auto tdamage = tower->compute_attack_damage();
  
	test_tower->set_target(mob);
  auto attack = test_tower->generate_attack(attack_id, 0);
  auto attack_vals = attack->get_attack_attributes();
  assert_tower_properties_almost_equals(attack_vals, expected_props);
  tower_properties expected_handprops = expected_base_props;
  for (size_t dmg_idx = 0; dmg_idx < tower_property_modifier::NUM_ELEM; dmg_idx++) {
    expected_handprops.damage[dmg_idx].low *= (1.0 + 0.3);
    expected_handprops.damage[dmg_idx].high *= (1.0 + 0.3);
  }
  assert_tower_properties_almost_equals(attack_vals, expected_handprops);

  attack->set_target(Coordinate<float>(mob_xcoord, mob_ycoord));

  std::list<std::weak_ptr<Monster>> moblist; 
  moblist.emplace_back(mob);
  compute_attackhit(moblist, std::move(attack));

  //check the mob state, make sure the correct damage was done
  MonsterStats mob_stats = mob->get_attributes();
  EXPECT_FLOAT_EQ(mob_stats.health, 94.2195969);
}/*
        basic_flat_pierce.yaml
        basic_flatED.yaml
        basic_pctED.yaml
        basic_range.yaml
        basic_spd.yaml
        basic_t0.yaml
        basic_typeflatED.yaml
        basic_typepctED.yaml
        compound_full.yaml
        flat_d.yaml
*/

#endif

/* TODO: Other tests to have:
 *
 * 1. the full set of attack types / attributes
 * 2. the full set of mob armor / effects
 * 3.
 */

} // namespace
