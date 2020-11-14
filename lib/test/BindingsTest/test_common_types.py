import pyDeityTD as deitytd

import pytest

def test_range_init():
    default_range = deitytd.range()
    assert default_range.low == 0
    assert default_range.high == 0

    custom_range = deitytd.range(-1, 10)
    assert custom_range.low == -1 
    assert custom_range.high == 10

    custom_range = deitytd.range(0.314, 159265)
    assert custom_range.low == pytest.approx(0.314)
    assert custom_range.high == 159265

    copy_range = deitytd.range(custom_range)
    assert copy_range.low == pytest.approx(0.314) 
    assert copy_range.high == 159265

def test_range_ops():
    range_lhs = deitytd.range(10, 100)
    range_rhs = deitytd.range(1, 10)

    new_range = range_lhs + range_rhs
    assert new_range.low == 11
    assert new_range.high == 110

    new_range = range_lhs * range_rhs
    assert new_range.low == 10
    assert new_range.high == 1000

    new_range = range_lhs + 15 
    assert new_range.low == 25
    assert new_range.high == 115

    new_range = 25 + range_rhs
    assert new_range.low == 26
    assert new_range.high == 35

    new_range = range_lhs * 15 
    assert new_range.low == 150 
    assert new_range.high == 1500

    new_range = 25 * range_rhs
    assert new_range.low == 25
    assert new_range.high == 250


def test_inplace_range_ops():
    range_lhs = deitytd.range(10, 100)
    range_rhs = deitytd.range(1, 10)

    test_range = deitytd.range(range_lhs)
    test_range += range_rhs
    assert test_range.low == 11
    assert test_range.high == 110
    
    test_range = deitytd.range(range_lhs)
    test_range *= range_rhs
    assert test_range.low == 10
    assert test_range.high == 1000
    
    test_range = deitytd.range(range_lhs)
    test_range += 15 
    assert test_range.low == 25
    assert test_range.high == 115
    
    test_range = deitytd.range(range_lhs)
    test_range *= 25 
    assert test_range.low == 250
    assert test_range.high == 2500


