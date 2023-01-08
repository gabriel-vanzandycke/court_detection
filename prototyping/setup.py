from setuptools import setup, find_packages

setup(
    name='court_detection',
    author='Gabriel Van Zandycke',
    author_email="gabriel.vanzandycke@hotmail.com",
    url="https://github.com/gabriel-vanzandycke/hawkeye_assignment",
    licence="LGPL",
    python_requires='>=3.8',
    description="Assignment for Hawk-Eye interview",
    version='0.2',
    packages=find_packages(),
    install_requires=[
        "numpy",
        "calib3d",
        "jupyter",
        "scikit-network",
        "opencv-python",
        "opencv-contrib-python",
        "matplotlib>=3.6.0",
        "distinctipy"
    ],
)
