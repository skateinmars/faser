from setuptools import setup

setup(
    name='faser_manager',
    packages=['faser_manager'],
    include_package_data=True,
    install_requires=[
        'flask',
        'Flask-HTTPAuth',
        'pyserial',
    ],
)
