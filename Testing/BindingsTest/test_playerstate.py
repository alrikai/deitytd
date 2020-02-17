import pyDeityTD as deitytd

def test_create():
    player_info = deitytd.TDPlayerInformation(10, 20, 30)
    assert player_info.get_num_lives() == 10
    assert player_info.get_num_essence() == 20
    assert player_info.get_num_gold() == 30

def test_lives_update():
    player_info = deitytd.TDPlayerInformation(10, 20, 30)
    assert player_info.get_num_lives() == 10
    player_info.gain_life()
    assert player_info.get_num_lives() == 11
    player_info.lose_life()
    assert player_info.get_num_lives() == 10
    player_info.lose_life()
    assert player_info.get_num_lives() == 9
    [player_info.lose_life() for _ in range(9)]
    assert player_info.get_num_lives() == 0
    player_info.lose_life()
    assert player_info.get_num_lives() == 0

    assert player_info.get_num_essence() == 20
    assert player_info.get_num_gold() == 30


def test_gold_update():
    player_info = deitytd.TDPlayerInformation(10, 20, 30)
    assert player_info.get_num_gold() == 30
    player_info.update_gold(10)
    assert player_info.get_num_gold() == 40
    player_info.update_gold(-20)
    assert player_info.get_num_gold() == 20

def test_essence_update():
    player_info = deitytd.TDPlayerInformation(10, 20, 30)
    assert player_info.get_num_essence() == 20

    player_info.update_essence(-10)
    assert player_info.get_num_essence() == 10
    player_info.update_essence(20)
    assert player_info.get_num_essence() == 30
