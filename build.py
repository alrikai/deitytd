from typing import Dict, Any

from setuptools import setup, Distribution, find_packages

#https://stackoverflow.com/questions/24071491/how-can-i-make-a-python-wheel-from-an-existing-native-library
class BinaryDistribution(Distribution):
    def has_ext_modules(*_):
        return False

def make_params() -> Dict[str, Any]:
    params = {
        "name": "deitytd",
        "version": '0.1.1',
        "author": "Alrik Firl",
        "author_email": "afirlortwo@gmail.com",
        "description": "The Best (?) TD",
        "packages": find_packages("deitytd"),
        "package_dir": {'':'deitytd'},
        "zip_safe": False,
        "include_package_data": True,
        "py_modules": ['deitytd'],
        "package_data": {
            'deitytd': ['deitytd/build/lib/_pyDTD.cpython-36m-x86_64-linux-gnu.so'],
        },
        "distclass": BinaryDistribution
    }
    return params

def build(setup_kwargs: Dict[str, Any]) -> None:
    params = make_params()
    setup_kwargs.update(params)
