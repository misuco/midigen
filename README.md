
apt-get install libjsoncpp-dev

git clone git@github.com/misuco/midigen.git
cd midigen
git submodule init
git submodule update

cd ..
mkdir midigen-build
cd midigen-build
cmake ../midigen
make
