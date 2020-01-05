from setuptools import setup, Distribution

#https://stackoverflow.com/questions/24071491/how-can-i-make-a-python-wheel-from-an-existing-native-library
class BinaryDistribution(Distribution):
    def has_ext_modules(*_):
        return True

kwargs = dict(
    name='pyDeityTD',
    version='0.0.1',
    author='Alrik Firl',
    author_email='afirlortwo@gmail.com',
    description='Best (?) TD',
    zip_safe=False,
    include_package_data=True,
    py_modules=['deitytd'],
    packages=['pyDeityTD'],
    package_data={
        'pyDeityTD': ['_pyDTD.cpython-36m-x86_64-linux-gnu.so'],
    },
    distclass=BinaryDistribution
)

setup(**kwargs)
